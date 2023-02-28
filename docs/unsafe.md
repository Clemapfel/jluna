## The `unsafe` Library

> **Waning**: Misuse of this part of jluna can lead to exception-less crashes, data corruption, and memory leaks. Only user who are confident at handling raw memory with no safety-nets are encouraged to employ these functions.

So far, a lot of jlunas internal workings were intentionally obfuscated to allow more novice users to use jluna in a way that is easy to understand and 100% safe. For some applications, however, the gloves need to come off. This section will detail how to surrender the most central of jlunas conceits: safety. In return, we get **optimal performance**, achieving overheads of 0 - 5% compared to the C-API.

Most of these functions are contained in the `jluna::unsafe` namespace, appropriately reminding users to be careful when using them.

### Unsafe Types

Without `jluna::Proxy`, we reference Julia-side values through raw C-pointers. While C-pointers do have a type, there is no guarantee that whatever the C-pointer is pointing to is actually of that type.

The types of `unsafe` Julia pointers are:

+ `unsafe::Value*`
    - pointer to any Julia value, but not necessarily a value of type `Any`
+ `unsafe::Function*`
    - pointer to a callable Julia object, not necessarily a `Base.Function`
+ `unsafe::Symbol*`
    - pointer to a `Base.Symbol`
+ `unsafe::Module*`
    - pointer to a `Base.Module`
+ `unsafe::Expression*`
    - pointer to a `Base.Expr`
+ `unsafe::Array*`
    - pointer to a `Base.Array` of arbitrary value type and dimensionality (rank)
+ `unsafe::DataType`
    - pointer to a `Base.Type`, potentially a `UnionAll`

Note that an `unsafe::Value*` could be pointing to a `Base.Symbol`, `Base.Module`, `Base.Array`, etc. These pointer types are not mutually exclusive, one section in memory can be safely handled through multiple of these pointers at the same time. For example, a `Base.Module` can be handled using `unsafe::Value*` and `unsafe::Module*`. A callable strucctype can also be handled using `unsafe::Function*`, etc.

> **Julia Hint**: A callable structtype is any structtype for whom the call operator is defined:
>```julia
># declare struct
>struct Callable end
>
># declare call operator
>(instance::Callable)(xs...) = # ...
>```

### Acquiring Raw Pointers

#### From Proxies

For any object inheriting from `jluna::Proxy`, we can access the raw `unsafe::Value*` to the memory it is managing using `operator unsafe::Value*()`

> **C++ Hint**: The `operator T()` of object `x` is invoked when performing a cast of the form `static_cast<T>(x)`.

```cpp
// generate unnamed proxy
auto proxy = Main.safe_eval("return 1234");

/// get pointer via static cast
unsafe::Value* raw = static_cast<unsafe::Value*>(proxy);

// cast pointer from unsafe::Value* to Int64* 
// then dereference it to access the raw Int64 memory
std::cout << *(reinterpret_cast<Int64*>(raw)) << std::endl;
```
```
1234
```

`jluna::Proxy` can only be cast to `unsafe::Value*` directly. If we want a different `unsafe` pointer type instead, we will need to `reinterpret_cast` the result of `Proxy::operator unsafe::Value*()` to that pointer type.

Furthermore, specialized proxies can be implicitly cast to their `unsafe` pointer-type equivalent, `jluna::Array` to `unsafe::Array*`, `jluna::Symbol` to `unsafe::Symbol*`, etc.

#### From Julia-side Values

Sometimes, we want a Julia-side pointer to a Julia-side object. This is useful when handling large arrays, or when trying to reference a Julia-side object that is not bound to any variable. Julia's standard library provides `pointer_from_objref`, however, this function is highly unreliable because it can only be used on mutable types.

Instead, jluna provides the C++ function `as_julia_pointer`, which returns a pointer to arbitrary Julia-side objects, even if they are immutable:

```cpp
// allocate Julia-side Int64
auto int64_value = Main.safe_eval("return Int64(1234)");

// get Julia-side Ptr to that Int64
auto* int64_ptr = as_julia_pointer(int64_value);

// assign Julia-side Ptr to variable in Main
Main.create_or_assign("ptr_to_immutable", int64_ptr);

// print type, convert to object
Main.safe_eval(R"(
    println(ptr_to_immutable)
    println(unsafe_pointer_to_objref(ptr_to_immutable))
)");
```
```
Ptr{Int64} @0x00007f57ebc2b520
1234
```

This function returns an object of type `Ptr{T}`, where `T` is the same type as the value used as its argument.

As stated, this only works for **Julia types** (including types forwarded using `jluna::Usertype`). If we want a pointer to a **non-Julia type** in Julia, we will have to use `jluna::box<void*>`, which returns a `Ptr{Cvoid}` to arbitrary memory:

```cpp
// create non-Julia, non-boxable object
auto non_julia_type = std::thread([](){});

// print C++-side pointer
std::cout << &non_julia_type << std::endl;

// box to Ptr{Cvoid}
auto* julia_side_ptr = box<void*>(&non_julia_type);

// print Julia-side pointer
Base["println"](julia_side_ptr);
```
```
0x7ffdf8ae1520
Ptr{Nothing} @0x00007ffdf8ae1520
```

Where `std::thread([](){})` returns an object that is neither boxable, nor usertype-boxable, yet its pointer value can still be moved to Julia. Note that de-referencing that pointer in Julia will lead to undefined behavior.

We will learn more about `box<T>` shortly.

### Calling Julia Functions

So far, we used proxies to call Julia-functions with Julia-values. In the `unsafe` library, we instead use `unsafe::call`, which takes a `unsafe::Function*` as its first argument and any number of `unsafe::Value*`, which will be used to invoke the given function.

This has a few downsides. Firstly, we need to acquire a pointer to any given function. We could do this through proxies, but the most performant way is through `unsafe::get_function`. This functions takes two argument: the module the function is located in, as well as the name of a function as a symbol:

```cpp
unsafe::Function* println = unsafe::get_function(Base, "println"_sym);
```

Where `_sym` is a string-literal operator that converts its argument to a Julia-side symbol.

We used jlunas pre-initialized `Base`, which is of type `jluna::Module`.  In reality, this call is implicitly executing:

```cpp
unsafe::Function* println = unsafe::get_function(
    static_cast<unsafe::Module*>(Base), // implicit cast
    "println"_sym
);
```

Where, as stated, most of jlunas proxies are able to be `static_cast` to their corresponding `unsafe` pointer type. `jluna::Module` to `unsafe::Module*`, in this case.

Despite these operators being [explicit](https://en.cppreference.com/w/cpp/language/explicit), we can use these classes directly with functions expecting their pointer type, such as most `unsafe` functions. This allows for some convenience (with no additional performance overhead).

Having acquired a pointer to `println`, we can call the function like so:

```cpp
auto* res = unsafe::call(println, (unsafe::Value*) jluna::Main);
```
```
Main
```

If an exception is raised during `unsafe::call`, the user will not be notified and the function will silently return a `nulltpr`, potentially causing a segfault when accessing the returned value.

jluna offers a middle ground between the `Proxy::operator()` and `unsafe::call`: `jluna::safe_call`. This function also returns an `unsafe::Value*`, however any exception that is raised during invocation of the function is forwarded to C++. Unless absolute peak performance is needed, it is generally recommended to use `jluna::safe_call` in place of `unsafe::call`, as both functions have the same signature.

### Executing Strings as Code

For an unsafe version of `Module::safe_eval`, the `unsafe` library provides the `_eval` string literal operator:

```cpp
"println(\"unsafely printed\")"_eval;
```
```
unsafely printed
```

Which will similarly forward its potential result as a return value. Unlike `safe_eval`, the return value is not a proxy but a `unsafe::Value*`, pointing to the result.

Just like with `unsafe::call`, no exception forwarding happens during `_eval` and it will return `nullptr` if parsing the string or its execution was unsuccessful.

As discussed before, executing code as strings is fairly slow and not recommended unless unavoidable.

### Boxing / Unboxing

So far, we have used the proxies `operator=` to transfer C++-side memory to Julia, while `Proxy::operator T()` moved Julia-side memory to C++. Internally, both of these functionalities are actually handled by two functions in `jluna::` global scope: `box` and `unbox`.

Recall that Julia-side memory and C++-side memory do not necessarily have the same memory layout.

Transforming C++-side memory such that it is compatible with the Julia-state is called **boxing**. `box` has the following signature:

```cpp
template<typename T>
unsafe::Value* box(T);
```

Where `T` is the type of the C++-side value that we want to box.

Transforming Julia-side memory such that it is compatible with the C++-state is called **unboxing**, which has the following signature:

```cpp
template<typename T>
T unbox(unsafe::Value*);
```

Where `T` is the type of the resulting C++-side value, after unboxing.

A simple example would be the following:

```cpp
char cpp_side = 120;
```

If we want to move this value to Julia, we use `box<char>`, which will return a pointer to the freshly allocated Julia memory of type `Base.Char`:

```cpp
// move to Julia, returns pointer
unsafe::Value* julia_side = box<char>(cpp_side);

// print Julia-side value using Julia-side println
Base["println"](julia_side);
```
```
x
```

Where `x` is the [120th ASCII](https://en.wikipedia.org/wiki/ASCII#Printable_characters) character.

To move the now Julia-side memory back C++-side, we use `unbox<char>`:

```cpp
// move to C++, returns value
auto back_cpp_side = unbox<char>(julia_side);

// print using C++-side std::cout
std::cout << (int) back_cpp_side << std::endl;
```
```
120
```

Where `auto` is deduced to `char`, as `unbox<T>` always returns a value of type `T`.

This way of explicitly moving values between states can be quite cumbersome syntactically, which is why `jluna::Proxy` does all of this implicitly. Unlike `jluna::Proxy`, manually calling box/unbox is the fastest **and safe** way to move values between states. Unlike the C-APIs version of `box` / `unbox`, in jluna, the value type of the underlying Julia / C++ value does not have to match. If they this is the case, an implicit conversion is performed:

```cpp
// declared 64bit UInt64
size_t value_64bit = 64;

// box as int8
auto* boxed_int8 = box<Int8>(value_64bit);
Base["println"](Base["typeof"](boxed_int8), " ", boxed_int8);

// unbox as complex
auto unboxed_c64 = unbox<std::complex<Float64>>(boxed_int8);
std::cout << unboxed_c64.real() << " + " << unboxed_c64.imag() << "i" << std::endl;
```
```text
Int8 64
64 + 0i
```

Where unbox implicitly converted the Julia-side value of type `Int8` to a complex.

Note that `box<T>` and `unbox<T>` should always be called with an explicit template argument. This is to make sure a user consciously chooses the type a value is boxed / unboxed to, eliminating the possibility of accidental implicit conversions reducing performance.

### Protecting Values from the Garbage Collector

The result of `box` is a `unsafe::Value*`, a raw C-pointer. The value this pointer points to **is not protected from the garbage collector** (GC). At any time after the resolution of `box`, the Julia GC may deallocate our value right from under our nose. This can't happen when using proxies - it is the entire point of them - but it can, when handling raw pointers.

Because of this, we have to micromanage each value depending on how it was allocated. As a general rule of thumb: any value that is not explicitly referenced by either a Julia-side named variable, or a Julia-side `Ref`, can be garbage collected at any point, usually segfaulting the entire application at random times. This includes the result of `box`, `jluna::safe_call`, `unsafe::call` and most `unsafe` functions returning pointers. Any pointer acquired by calling `static_cast<unsafe::Value*>` on a proxy is safe, as long as the proxy stays in memory.

To prevent accidental deallocation without using `jluna::Proxy`, the `unsafe` library provides `unsafe::gc_preserve`. This function takes a raw C-Pointer and protects its memory from the garbage collector. `gc_preserve` returns a `size_t`, which is called the **id** of a value:

```cpp
// move 1234 to Julia
unsafe::Value* value = box<Int64>(1234);

// protect from GC
size_t value_id = unsafe::gc_preserve(value);
```

Keeping track of this id is incredibly important. We need it to call `unsafe::gc_release`, which takes the id as an argument and frees the protected memory, allowing it to be garbage collected:

```cpp
unsafe::Value* value = box<Int64>(1234);
size_t value_id = unsafe::gc_preserve(value);

// value is safe from the GC here

unsafe::gc_release(value_id);

// value may be garbage collected here
```

If we loose track of `value_id`, or we forget to call `gc_release`, the value will never be deallocated, and a [memory leak](https://en.wikipedia.org/wiki/Memory_leak#:~:text=In%20computer%20science%2C%20a%20memory,accessed%20by%20the%20running%20code.) will occur.

#### Disabling the GC

Alternatively to using `gc_preserve`, we can also simply disable the GC globally for a certain section. The `unsafe` library provides `gc_disable`, `gc_enable` and `gc_is_enabled` for this. jluna also provides a convenient macro:

```cpp
gc_pause; 

// everything here is safe from the GC

gc_unpause;
```

Unlike `gc_disable` / `gc_enable`, `gc_pause` will remember the state of the GC when it was called and restore it during `gc_unpause`, regardless of whether the GC was active or inactive at the time of `gc_pause`.

### Accessing & Mutating a Variable

In lieu of `jluna::Proxy`, the best way to access or change a Julia-side variables value are:

+ `unsafe::get_value(Module* m, Symbol* name)`
    - get the value of variable `name` in module `m` as an unsafe pointer
+ `unsafe::set_value(Module* m, Symbol* name, Value* new_value)`
    - set the value of variable `name` in module `m` to `new_value`
+ `unsafe::get_field(Value* owner, Symbol* field)`
    - get value of field named `:field` of `owner`
+ `unsafe::set_field(Value* owner, Symbol* field, Value* new_value*)`
    - set field `:field` of owner `owner` to `new_value`

Where no exception forwarding is performed.

```cpp
// create Julia-side variable
Main.safe_eval("jl_char = Char(121)");

// access raw pointer to value using `get_value`
auto* jl_char_ptr = unsafe::get_value(Main, "jl_char"_sym);

// unbox and print
std::cout << unbox<char>(jl_char_ptr) << std::endl;
```
```
y
```

Here, we did not need to `gc_preserve`. We can be sure that `Char(121)` is protected, because we just created a named Julia-side variable, `jl_char`, pointing to it. If we were to reassign `jl_char`, `Char(121)` may be garbage collected at any point afterwards.

All of `unsafe::get_*` / `unsafe::set_*` functions will be vastly superior, in terms of performance, when compared to `Module::safe_eval`. They [are even faster](benchmarks.md#mutating-julia-side-variables--results) than `Module::assign`.

---

### `unsafe` Array Interface

One of the most important high-performance computing tasks is modifying large arrays. `box` usually invokes a copy, which is unacceptable in environments like this. The `unsafe` library provides a 0-overhead interface for creating and modifying arrays, internally operating on the raw Julia memory for us.

Note that we can simply access the data of a `jluna::Array` by `static_cast`ing it to `unsafe::Array*`:

```cpp
Array<Int64, 2> array = // ...
unsafe::Array* raw_data = static_cast<unsafe::Array*>(array);
```

This will cause no reallocation, it simply forwards the pointer to the Julia-side memory of the `Base.Array` which can then be operated on through the C-API or Julia. The array data is protected, as long as the array proxy stays in scope.

If we have a `unsafe::Array*` to a large, already Julia-side array, we can convert it to a `jluna::Array` using its specialized constructor:

```cpp
unsafe::Array* big_array = // ...
auto big_array_wrapper = Array<size_t, 2>(big_array);
```

This causes no reallocation, making all the convenient functions of `jluna::Array` available to us. Note that the user is responsible for assuring that the dimensionality and value type of the declared `jluna::Array` match that of the underlying `unsafe::Array*`. If this is not true, data corruption or potential `nullptr`-access crashes may occur. `jluna::Array` does take ownership of the array used for this constructor, therefore, it does not need to be manually protected.

Elements of an array are stored in column-first order. To access the actual memory of the elements (not the memory of the array itself), we can use the field `data` of `unsafe::Array*`, or the C-API function `jl_array_data`, both of which return a `void*`.

```cpp
// create Julia-side array
jluna::Array<Int64, 1> array = Main.safe_eval("return [987, 123, 21]");

// get raw data, cast to Int64*
Int64* array_data = reinterpret_cast<Int64*>(array.data());

// access like a C-Array
std::cout << array_data[1] << std::endl;
```
```
123
```

### Accessing an Array Element

The `unsafe` library provides `get_index(Array*, size_t...)` and `set_index(Array*, size_t...)`, two 0-overhead functions that manipulate array elements by index. Unlike with `jluna::Array`, no bounds-checking is performed and there is no mechanism in place to verify that potential return values are actually of the expected type.

Similarly to `jluna::Array`, a 1d array takes 1 index, a 2d array 2 indices, and a Nd array N indices. Any array of any rank can furthermore be linear-index using just one index, again accessing elements in column-major order, just like with `jluna::Array`.

### Allocating a New Array

`unsafe` supports arrays of any rank, however, arrays of rank 1 (vectors) and rank 2 (matrices) are far better optimized and should be preferred in performance-critical environments, if at all possible.

To allocate a new array, we use `unsafe::new_array`. This function takes `Rank + 1` arguments. The first argument is the arrays value type (as an `unsafe::Value*`), each subsequent argument is a `size_t` representing the size in that dimension. For example:

```cpp
// allocate a 10-element vector of strings
auto* vec_10 = unsafe::new_array(String_t, 10);

// allocate a 5x5 matrix of Int32s
auto* mat_5x5 = unsafe::new_array(Int32_t, 5, 5);

// allocated a 2x2x2x2 array of bools
auto* arr_2x2x2x2 = unsafe::new_array(Bool_t, 2, 2, 2, 2);
```

Where we used the jluna-provided global type proxies, which are implicitly `static_cast` to their corresponding `unsafe::Value*` pointer.

After allocation, all values of an array will be of the given type, initialized as `undef`.

#### Creating a Thin Wrapper Around Already Existing Data

In high-performance applications, we often do not have enough RAM to have two of the same array in memory at the same time. To address this, the `unsafe` library provides a function that creates a *thin wrapper* around already existing data. A thin wrapper is an array whose data does not belong to it. Its data pointer points to valid data, however, the data is not inlined or managed by the array. If the array goes out of scope, the data remains. If the data goes out of scope, the array will segfault on access. The user is responsible for preventing the latter.

Similarly to `new_array`, `new_array_from_data` takes Rank + 2 elements:

+ the value type (as an `unsafe::Value*`)
+ a pointer to already existing data (as a `void*`)
+ Rank-many indices, specifying the size of an array in each dimension

```cpp
// allocate C-array
const int c_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// create thin wrapper around it
auto* jl_array = unsafe::new_array_from_data(Int32_t, (void*) c_array, 10);

// wrapper can now be used Julia-side
auto* println = unsafe::get_function(Base, "println"_sym);
jluna::safe_call(println, jl_array);
```

Where we used a C-style cast to forward `c_array` as a `void*`.

It is important to realize that no bounds-checking or verification, that the data is actually formatted according to the arrays value type, is performed. If we have an array of `size_t`s, which are 64-bit, and we use its data to create an array of `uint8_t`s, the user will not be made aware of this, instead leading to silent data corruption:

```cpp
// allocate C-array of UInt64
const UInt64 c_array[3] = {1234, 101, 111};

// incorrectly declare value type as Int8
auto* jl_array = unsafe::new_array_from_data(Int8_t, (void*) c_array, 3);

// print
auto* println = unsafe::get_function(Base, "println"_sym);
jluna::safe_call(println, jl_array);
```
```
Int8[-46, 4, 0]
```
Here, `unsafe::get_function` assumed the underlying memory of `c_array` to be 3 `Int8`s in column-first order. Because they are not, the first 3 * 8 bytes of the first `UInt64` in `c_array` was interpreted as 3 `Int8`s, explaining the corrupted result.

### Resizing an Array

The `unsafe` library provides `resize_array`, which takes, as its arguments, the array and the new size (in each dimension), just like `new_array`.

For 1d and 2d arrays only, "slicing" (making an array smaller in one or more dimensions) is very fast. No allocation is performed when slicing, unlike with "growing" an array. Furthermore, resizing a 1d array into another 1d array is far more performant than resizing a 1d array into a 2d array or vice-versa. In the latter case, the equivalent of `Base.reshape` has to be called, potentially leading the entire array to be re-allocated during the memory shuffling.

Any newly added values are set to `undef`. Expanding a matrix in the x-direction will lead to new values being inserted into each column, potentially corrupting the order of elements.

#### Replacing an Arrays Data

To avoid copying, jluna provides `override_array`, which replaces an arrays data with that of another. No allocation is performed, and at no point will the amount of memory grow beyond the initial space of both arrays.
As no copy is made, after overriding array `B` with array `A`, the user is responsible for keeping array `A` in scope, lest `B` data be potentially deallocated as well. `override_array` can be conceptualized as transforming an array into a thin-wrapper of another array.

```cpp
// alloc two arrays of length 3 and 5
const Int64 array_l3[3] = {1234, 101, 111};
const Int64 array_l5[5] = {12, 112, 9, 0, 1};

auto* jl_array_l3 = unsafe::new_array_from_data(Int64_t, (void*) array_l3, 3);
auto* jl_array_l5 = unsafe::new_array_from_data(Int64_t, (void*) array_l5, 5);

// print length 3
auto* println = unsafe::get_function(Base, "println"_sym);
jluna::safe_call(println, box<std::string>("before: "), jl_array_l3);

// transform l3 into a thin wrapper of l5
unsafe::override_array(jl_array_l3, jl_array_l5);

// print again
jluna::safe_call(println, box<std::string>("after : "), jl_array_l3);
```
```
before: [1234, 101, 111]
after : [12, 112, 9, 0, 1]
```

Where we used `box<std::string>` to forward an inline string as a `unsafe::Value*`, as `jluna::safe_call` needs all its elements to already be Julia-side values.

Similarly, `swap_array_data` replaces array `B`s data with that of `A`, and array `A`s data with that of `B`. Like `override_array`, no copy is performed: the space needed in RAM for this operation will always be exactly the size of `A` + `B`. The user is responsible for both arrays' memory staying in scope until `swap_array_data` has returned.

### Shared Memory

In the section on proxies, we said that, to move a value from C++ to Julia (or vice-versa), we first need to **change its memory format** such that it is interpretable by the other language. This is not always true. For a very limited number of types, Julia and C++ **already have the exact same memory format**.

For these types only, `box` / `unboxing` is a 0-cost operation. No actual computation is performed, we simply forward the memory pointer to Julia. This is obviously desired in performance-critical applications.

An exhaustive list of all of these types is provided here:

```cpp
// C++ Type       // Julia Alias    // Julia Common Name
int8_t              Cchar           Int8
int16_t             Cshort          Int16
int32_t             Cint            Int32
int64_t             Clonglong       UInt64
uint8_t             Cuchar          UInt8
uint16_t            Cushort         UInt16
uint32_t            Cuint           UInt32
uint64_t            Culonglong      UInt64
float               Cfloat          Float32
double              Cdouble         Float64

size_t              Csize_t         UInt64
void                Cvoid           Nothing

const char*         Cstring         Ptr{UInt8}

T*                  n/a             Ptr{T}      //[1]
unsafe::Value*      n/a             Ptr{Any}
void*               Ptr{Cvoid}      Ptr{Nothing}
    
//[1] Where T is also a no-cost-(Un)Boxable except void
```


Most of these types have the distinction of having a Julia-side type named `C*` (where `*` is the C-side typename): `Csize_t`, `Cstring`, `Cvoid`, etc.

For these types, the `unsafe` library provides truly 0-cost `box` / `unbox` functions, called `unsafe::unsafe_box` / `unsafe::unsafe_unbox`. The "safe" `box` / `unbox` do some sanity checking, exception forwarding and implicit conversions, even for C-types. Their `unsafe` counterparts do none of these, achieving optimal performance.

```cpp
auto* int64_memory = "return 1234"_eval;

std::cout << unsafe::unsafe_unbox<Int64>(int64_memory) << std::endl;
// exactly as fast as:
std::cout << *(reinterpret_cast<Int64*>(int64_memory)) << std::endl;
```
```
1234
1234
```

However, unlike with `box` / `unbox`, the user is responsible for avoiding potential data corruption or GC-issues when using their `unsafe` versions.
