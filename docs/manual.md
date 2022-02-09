# jluna: An Introduction

This page will give an overview of most of `jluna`s relevant features and syntax.

### Table Of Contents

Please navigate to the appropriate section by clicking the links below:

1. [Initialization & Shutdown](#initialization)<br>
2. [Executing Code](#executing-code)<br>
3. [Controlling the Garbage Collector](#garbage-collector-gc)<br>
  3.1 [Enabling/Disabling GC](#enablingdisabling-gc)<br>
  3.2 [Manual Collection](#manually-triggering-gc)<br>
4. [Boxing / Unboxing](#boxing--unboxing)<br>
  4.1 [Manual](#manual-unboxing)<br>
  4.2 [(Un)Boxable as Concepts](#concepts)<br>
  4.3 [List of (Un)Boxables](#list-of-unboxables)<br>
5. [Accessing Variables through Proxies](#accessing-variables)<br>
  5.1 [Mutating Variables](#mutating-variables)<br>
  5.2 [Accessing Fields](#accessing-fields)<br>
  5.3 [Proxies](#proxies)<br>
  5.4 [Named Proxies](#named-proxies)<br>
  5.5 [Unnamed Proxies](#unnamed-proxy)<br>
  5.6 [Detached Proxies](#detached-proxies)<br>
  5.7 [Making a Named Proxy Unnamed](#making-a-named-proxy-unnamed)<br>
6. [Specialized Proxies: Modules]()<br>
   6.1 [Eval]()<br>
   6.2 [Bindings & Usings]()<br>
   6.3 [Properties]()<br>
7. [Specialized Proxies: Symbols]()<br>
   7.1 [CTORs]()<br>
   7.3 [Hashing & Comparisons]()<br>
7. [Functions](#functions)<br>
   7.1 [Accessing julia Functions from C++](#functions)<br>
   7.2 [Calling julia Functions from C++](#functions)<br>
   7.3 [Accessing C++ Functions from julia](#registering-functions)<br>
   7.4 [Calling C++ Functions from julia](#calling-c-functions-from-julia)<br>
   7.5 [Allowed Function Names](#allowed-function-names)<br>
   7.6 [Allowed Function Signatures](#possible-signatures)<br>
   7.7 [Using arbitrary Objects in julia Functions](#using-non-julia-objects-in-functions)<br>
8. [Arrays](#arrays)<br>
  8.1 [Constructing Arrays](#ctors)<br>
  8.2 [Indexing](#indexing)<br>
  8.3 [Iterating](#iterating)<br>
  8.4 [Vectors](#vectors)<br>
  8.5 [Matrices](#matrices)<br>
9. [Introspection]()<br>
  9.1 [Core Types]()<br>
  9.2 [Fields]()<br>
  9.3 [Parameters]()<br>
  9.4 [Type Comparisons]()<br>
  9.5 [Type Classification]()<br>
  9.6 [Type Properties]()<br>
X. [~~Expressions~~](#expressions)<br>
X. [~~Usertypes~~](#usertypes)<br>

## Initialization

Before any interaction with `jluna` or julia can take place, the julia state needs to be initialized:

```cpp
#include <jluna.hpp>

using namespace jluna;
int main() 
{
    State::initialize();
    
    // all your application here
    
    return 0;
}
```

When a program exits regularly, all julia-side values allocated through `jluna` or the C-API are safely deallocated automatically. Do not call `jl_atexit_hook()` at any point, as this may unintentionally invalidate C++-side memory and cause undefined behavior.

## Executing Code

`jluna` has two ways of executing code (represented C++ side as string): 
+ *with* exception forwarding and
+ *without* exception forwarding. 
  
Code called without exception forwarding will not only not report any errors, but simply appear to "do nothing". If a fatal error occurs, the entire application wil crash without warning.<br>

Because of this, it is highly recommended to always air on the side of safety by using the `safe_` overloads whenever possible:

```cpp
// without exception forwarding
State::eval("your unsafe inline code");
State::eval(R"(
    your
    multi-line
    code
)");
jl_eval_string("your unsafe code");

// with exception forwarding
State::safe_eval("your safe inline code");
State::safe_eval(R"(
    your
    safe
    multi-line
    code
)");
```

`safe_` overloads have marginal overhead from try-catching execution and sanity checking inputs. If you want maximum performance and the correspondingly awful debug experience: `State::eval` is identical to just calling the pure C-API `jl_eval_string`.

## Garbage Collector (GC)

The julia-side garbage collector operates completely independently, just like it would in a pure julia program. However, sometimes it is necessary to disable or control its behavior manually. To do this, `jluna::State` offers the following functions:

#### Enabling/Disabling GC
```cpp
State::set_garbage_collector_enabled(bool);
```

#### Manually Triggering GC
```cpp
State::collect_garbage();
// always works, if currently disabled, enables, then collects, then disables again
```
#### Checking if the GC is Enabled
```cpp
bool State::is_garbage_collector_enabled();
```

When using `jluna` and allocating memory specifically through it, objects are safe from being garbage collected. It is therefore almost never necessary to manually disable the GC. See the section on [proxies](#accessing-variables) for more information.

## Boxing / Unboxing

Julia and C++ do not share any memory. Objects that have the same conceptual type can have very different memory layouts. For example, `Char` in julia is a 32-bit value, while it is 8-bits in C++. Comparing `std::set` to `Base.set` will of course be even more of a difference on a bit-level.<br>
Because of this, when transferring memory from one languages state to the others, we're not only moving memory but converting it by reformating its layout. 

**Boxing** is the process of taking C++-side memory, converting it and then allocating the now julia-compatible memory julia-side. Conversely, **unboxing** is the process of taking julia-side memory, converting it, then allocating the now C-compatible memory it C++-side. Boxing/Unboxing of any type are handled by an overload of the following functions:

```cpp
template<typename T>
Any* box(T);

template<typename T>
T unbox(Any*);
```
where `Any*` is an address of julia-side memory of arbitrary type (but not necessarily of type `Base.Any`).

All box/unbox functions have exactly this signature, ambiguity is resolved via [C++ concepts](https://en.cppreference.com/w/cpp/language/constraints), [SFINAE](https://en.cppreference.com/w/cpp/types/enable_if) and general template magic. Feel free to check the [source code](../.src/unbox.inl) to get a feel for how it is done behind the scenes.

### Concepts

The property of being (un)boxable is represented in C++ as two concepts:

```cpp
// an "unboxable" is any T for whom unbox<T>(Any*) -> T is defined
template<typename T>
concept Unboxable = requires(T t, Any* v)
{
    {unbox<T>(v)};
};

/// a "boxable" is any T for whom box(T) -> Any* is defined
template<typename T>
concept Boxable = requires(T t)
{
    {box(t)};
};
```
Given this, we can box/unbox any object that fulfills the above requirements like so:

### Manual (Un)Boxing
```cpp
size_t cpp_side = 1001;

// C++ -> julia: boxing
Any* jl_side = box(cpp_side);

// julia -> C++: unboxing
size_t back_cpp_side = unbox<size_t>(jl_side);

// verify nothing was lost in the conversion
std::cout << back_cpp_side << std::endl;
```
```
1001
```

Any type fulfilling the above requirements is accepted by most `jluna` functions. Usually, these functions will implicitly (un)box their arguments and return-types. This means, most of the time, we don't have to worry about manually calling `box`/`unbox<T>`. 

This also means that for a 3rd party class `MyClass` to be compatible with `jluna`, one only needs to define:

```cpp
template<Is<MyClass> T> 
Any* box(T value);

template<Is<MyClass> T>
T unbox(Any* value);
```

Where `Is<T, U>` is a concept that resolves to true if `U` and `T` are the same type. We will learn more about making usertypes that are compatible with jluna in the [~~section on usertypes~~](#usertypes).

### List of (Un)Boxables

The following types are both boxable and unboxable out-of-the-box. 

```cpp
// cpp type              // julia-side type after boxing

jl_value_t*              -> Any
jl_module_t*             -> Module
jl_function_t*           -> Function
jl_sym_t*                -> Symbol

Any*                     -> Any
jl_sym_t*                  -> Symbol

bool                     -> Bool
char                     -> Char
int8_t                   -> Int8
int16_t                  -> Int16
int32_t                  -> Int32
int64_t                  -> Int64
uint8_t                  -> UInt8
uint16_t                 -> UInt16
uint32_t                 -> UInt32
uint64_t                 -> UInt64
float                    -> Float32
double                   -> Float64

jluna::Any               -> Any
jluna::Proxy             -> /* value-type deduced during runtime */
jluna::Symbol            -> Symbol
jluna::Type              -> Type
jluna::Array<T, R>       -> Array{T, R}     * °
jluna::Vector<T>         -> Vector{T}       *
jluna::JuliaException    -> Exception

std::string              -> String
std::complex<T>          -> Complex{T}      *
std::vector<T>           -> Vector{T}       *
std::array<T, R>         -> Vector{T}       *
std::pair<T, U>          -> Pair{T, U}      *
std::tuple<Ts...>        -> Tuple{Ts...}    *
std::map<T, U>           -> IdDict{T, U}    *
std::unordered_map<T, U> -> Dict{T, U}      *
std::set<T>              -> Set{T, U}       *

* where T, U are also (Un)Boxables
° where R is the rank of the array

std::function<void()>                       -> function ()      ::Nothing        
std::function<void(Any*)>                   -> function (::Any) ::Nothing  
std::function<void(Any*, Any*)>             -> function (xs...) ::Nothing  *
std::function<void(Any*, Any*, Any*)>       -> function (xs...) ::Nothing  *
std::function<void(Any*, Any*, Any*, Any*)> -> function (xs...) ::Nothing  *
std::function<void(std::vector<Any*>)>      -> function (::Vector{Any}) ::Nothing

std::function<Any*()>                       -> function ()      ::Any        
std::function<Any*(Any*)>                   -> function (::Any) ::Any
std::function<Any*(Any*, Any*)>             -> function (xs...) ::Any      *
std::function<Any*(Any*, Any*, Any*)>       -> function (xs...) ::Any      *
std::function<Any*(Any*, Any*, Any*, Any*)> -> function (xs...) ::Any      *
std::function<Any*(std::vector<Any*>)>      -> function (::Vector{Any}) ::Any

* where xs... is enforced to be of approriate size at runtime
```
We will learn more on how to box/unbox functions in the [section on calling C++ functions from julia](#functions).

The template meta function `to_julia_type` is provided to convert a C++ type into a julia-type:

```
std::cout << to_julia_type<Array<size_t, 4>> std::endl;
``` 
```
Array{UInt64, 4}
```

This can help remind user what type gets converted and can be used when generating julia code as it can be called statically.

---

## Accessing Variables

Now that we know we can (un)box values to move them from C++ to julia, we'll actually use them.

Let's say we have a variable `var` julia-side:

```cpp
State::eval("var = 1234")
```

To access the value of this variable, we can use the C-API. We receive a pointer, to the memory `var` holds, using `jl_eval_string`. We then `unbox` that pointer:

```cpp
Any* var_ptr = jl_eval_string("return var");
auto as_int = unbox<int>(var_ptr);

std::cout << as_int << std::endl;
```
```
1234
```

`State::safe_return<T>` essentially does the same except it will forward any exception thrown during `return var` (such as `UndefVarError`):

```cpp
auto as_int = State::safe_return<int>("var");
std::cout << as_int << std::endl;
```
```
1234
```

While both ways get us the desired value, neither is a good way to actually manage the variable itself. How do we reassign it? Can we dereference the c-pointer? Who has ownership of the memory? Is it safe from the garbage collector? <br>All these questions are hard to manage using the C-API, however `jluna` offers a one-stop-shop solution: `jluna::Proxy`.

### Proxies

A proxy holds two things: the **memory address of its value** and a **symbol**. We'll get to the symbol later, for now, let's focus on the memory:<br>
Memory held by a proxy is safe from the julia garbage collector (GC) and assured to be valid. This means we don't have to worry about keeping a reference, or pausing the GC when modifying the variable. Any memory, be it temporary or something explicitly referenced by a julia-side variable or `Base.Ref`, is guaranteed to be safe to access as long as a C++-side proxy points to it

We rarely create a proxy ourself, most of the time it will be generated for us by `State::(safe_)eval` or similar functions:

```cpp
State::eval("var = 1234")
auto proxy = State::eval("return var")
```
Use of `auto` simplifies the declaration and is encouraged whenever possible.<br>

Now that we have the proxy, we need to convert it to a value. Unlike with the C-APIs `jl_value_t*` (aka. `Any*`) we do not need to call `box`/`unbox<T>`:

```cpp
// all following statements are exactly equivalent:

int as_int = proxy;   // recommended

auto as_int = proxy.operator int();

auto as_int = static_cast<int>(proxy);

auto as_int = (int) proxy;

auto as_int = unbox<int>(proxy) // discouraged
```

Where the first version is encouraged for style reasons. `jluna` handles implicit conversion behind the scenes. This makes it, so we don't have to worry what the actual type of the julia-value is. `jluna` will try it's hardest to make our declaration work:

```cpp
State::eval("var = 1234")
auto proxy = State::eval("return var")

// all of the following work by triggering julia-side conversion:
size_t as_size_t = proxy;
std::string as_string = proxy;
std::complex<double> as_complex = proxy;

std::cout << "size_t : " << as_size_t << std::endl;
std::cout << "string : " << as_string << std::endl;
std::cout << "complex: " << as_complex.real() << " | " << as_complex.imag() << std::endl;
```
```
1234
"1234"
1234 | 0
```

Of course, if the type of the julia variable cannot be converted to the target type, an exception is thrown:

```cpp
std::vector<double> as_vec = proxy;
```
```
terminate called after throwing an instance of 'jluna::JuliaException'
  what():  MethodError: Cannot `convert` an object of type Int64 to an object of type Vector
  (...)
```

This is already much more convenient than manually unboxing c-pointers, however the true usefulness of proxies lies in their ability to not only access but *mutate* julia-side values.

## Mutating Variables

To mutate a variable means to change its value, its type or both.<br>

As stated before, a proxy holds exactly one pointer to julia-side memory and exactly one symbol. There are two types of symbols it can hold:

+ a symbol starting with the character `#` is called an **internal id**
+ any other symbol is called a **name**

The behavior of proxies changes, depending on wether their symbol is a name or not. A proxies whos symbol is a name is called a **named proxy**, a proxy whos symbol is an internal id is called an **unnamed proxy**. 

To generate an unnamed proxy, we use `State::(safe_)eval`, `State::safe_return` or `State::new_unnamed_<T>`.<br> To generate a named proxy we use `Proxy::operator[]` or `State::new_named_<T>`:

```cpp
State::safe_eval("var = 1234");

auto unnamed_proxy = State::safe_eval("return var");
auto named_proxy = Main["var"];

// or, in one line:
auto named_proxy = State::new_named_int64() = 1234;
```

where `Main`, `Base`, `Core` are global, pre-initialized proxies holding the corresponding julia-side modules singletons.

We can check a proxies name using `.get_name()`:

```cpp
std::cout << unnamed_proxy.get_name() << std::endl;
std::cout << named_proxy.get_name() << std::endl;
```
```
<unnamed proxy #9>
Main.var
```
We see that `unnamed_proxy`s symbol is `#9`, while `named_proxy`s symbol is `Main.var`, the same name as the julia-side variable `var` that we used to create it.

### Named Proxies

If and only if a proxy is named, assigning it will change the corresponding variable julia-side:

```cpp
State::safe_eval("var = 1234")
auto named_proxy = Main["var"];

std::cout << "// before:" << std::endl;
std::cout << "cpp   : " << named_proxy.operator int() << std::endl;
State::safe_eval("println(\"julia : \", Main.var)");

named_proxy = 5678; // assign

std::cout << "// after:" << std::endl;
std::cout << "cpp   : " << named_proxy.operator int() << std::endl;
State::safe_eval("println(\"julia : \", Main.var)");
```
```
// before:
cpp   : 1234
julia : 1234
// after:
cpp   : 5678
julia : 5678
```

We see that after assignment, both the value `named_proxy` is pointing to, and the variable of the same name "`Main.var`" were affected by the assignment. This is somewhat atypical for julia but familiar to C++-users. A named proxy acts like a reference to the julia-side variable. <br><br> While somewhat unusual, because of this behavior we are able to do things like:

```cpp
State::safe_eval("vector_var = [1, 2, 3, 4]")
Main["vector_var"][0] = 999;   // indices are 0-based in C++
Base["println"]("julia prints: ", Main["vector_var"]);
```
```
julia prints: [999, 2, 3, 4]
```

Which is highly convenient. 

### Unnamed Proxy

Mutating an unnamed proxy will only mutate its value, **not** the value of any julia-side variable:
```cpp
State::safe_eval("var = 1234")
auto unnamed_proxy = State::safe_eval("return var");

std::cout << "// before:" << std::endl;
std::cout << "cpp   : " << named_proxy.operator int() << std::endl;
State::safe_eval("println(\"julia : \", Main.var)");

unnamed_proxy = 5678; // assign

std::cout << "// after:" << std::endl;
std::cout << "cpp   : " << named_proxy.operator int() << std::endl;
State::safe_eval("println(\"julia : \", Main.var)");
```
```
// before:
cpp   : 1234
julia : 1234
// after:
cpp   : 5678
julia : 1234
```

We see that `unnamed_proxy` was assigned a new memory address pointing to the newly allocated julia-side value `5678`. Meanwhile, `Main.var` is completely unaffected by this change. This makes sense, if we check `unnamed_proxy`s symbol again:

```cpp
std::cout << unnamed_proxy.get_name() << std::endl;
```
```
<unnamed proxy #9>
```

We see that it does not have a name, just an internal id. Therefore, it has no way to know where its value came from and thus has no way to mutate anything but the C++ variable.

When we called `return var`, julia did not return the variable itself but the value of said variable. An unnamed proxy thus behaves like a deepcopy of the value, **not** like a reference.

#### In Summary

+ We create a **named proxy**
  - using `State::new_named_<T>`, `Proxy::operator[]`
  - Assigning a named proxy mutates its value and mutates the corresponding julia-side variable of the same name
+ We create an **unnamed proxy**
  - using `State::new_unnamed_<T>`, `State::(safe_)eval`, `State::safe_return`
  - Assigning an unnamed proxy mutates its value but does not mutate any julia-side variable

This is important to realize and is the basis of much of `jluna`s syntax and functionality.

### Detached Proxies

Consider the following code:

```cpp
auto named_proxy = State::new_named_int("var", 1234);

State::safe_eval(R"(var = ["abc", "def"])");

std::cout << named_proxy.operator std::string() << std::endl;
```

What will this print? We know `named_proxy` is a named proxy so it should correspond to the variable `var` which we declared as having the value `1234`, however we reassigned `var` to a completely different value and type using only julia. `named_proxy` was never made aware of this, so it still currently points to its old value:

```
std::cout << named_proxy.operator std::string() << std::endl;
```
```
1234
```

While still retaining its name:

```
std::cout << named_proxy.get_name() << std::endl;
```
```
Main.var
```

This proxy is in what's called a *detached state*. Even though it is a named proxy, its current value does not correspond to the value of it's julia-side variable. This may have unforeseen consequences:

```cpp
auto named_proxy = State::new_int64("var", 1234);
State::safe_eval(R"(var = ["abc", "def"])");

std::cout << named_proxy[1].operator std::string() << std::endl;
```
```
terminate called after throwing an instance of 'jluna::JuliaException'
  what():  BoundsError
Stacktrace:
 [1] getindex(x::Int64, i::UInt64)
   @ Base ./number.jl:98
 [2] safe_call(::Function, ::Int64, ::UInt64)
   @ Main.jluna.exception_handler ~/Workspace/jluna/.src/julia/exception_handler.jl:80

signal (6): Aborted
```

Even though `var` is a vector julia-side, accessing the second index of `named_proxy` throws a BoundsError because `name_proxy`s value is still `Int64(1234)`, which does not have a second index.

Assigning a detached proxy will still mutate the corresponding variable:

```cpp
auto named_proxy = State::new_int64("var", 1234);
State::safe_eval(R"(var = ["abc", "def"])");

State::safe_eval("println(\"before:\", var)");

named_proxy = 5678; // assign cpp-side

State::safe_eval("println(\"after :\"var)");
```
```
before: ["abc", "def"]
after : 5678
```

Here, because `named_proxy` still has its name `Main.var` it reassign the variable to `5678` which was specificed through C++. 

While this behavior is internally consistent and does not lead to undefined behavior (a proxy will mutate its variable, regardless of the proxies value or the current value of the variable), it may cause unintentional bugs when reassigning the same variable frequently in both C++ and julia. To alleviate this, `jluna` offers a member function `Proxy::update()`, which evaluates the proxies name and replaces its value with value of the correspondingly named variable:

```cpp
auto named_proxy = State::new_int64("var", 1234);
State::safe_eval(R"(var = ["abc", "def"])");

named_proxy.update();

std::cout << named_proxy.operator std::string() << std::endl;
```
```
["abc", "def"]
```
While not necessary to do everytime an assignment happens, it is a convenient way to fix a detached proxy.

### Making a Named Proxy Unnamed

Sometimes it is desirable to stop a proxy from mutating the corresponding variable, even though it is a named proxy. While we cannot change a proxies name, we can generate a new unnamed proxy using the member function `Proxy::value()`. This functions returns an unnamed proxy pointing to a newly allocated deepcopy of the value of the original proxy. 

```cpp
State::eval("var = 1234");
auto named_proxy = Main["var"];

auto value = named_proxy.value();   // create nameless deepcopy

// assigning either does not affect the other
value = 9999;
named_proxy = 4567;

std::cout << "named: " << named_proxy.operator int() << std::end,
std::cout << "value: " << named_proxy.operator int() << std::end,
```
```
named: 4567
value: 9999
```

Therefore, to make a named proxy unnamed, we can simply assign its `value` to itself:

```cpp
auto proxy = /*...*/
proxy = proxy.value()
```

This way the actual julia-side memory stays safe, but the proxy can now be mutated without interferring with any variable of the former name.

Calling `.value()` on a proxy that is already unnamed simply creates another unnamed deepcopy with a new internal id.

### Accessing Fields

For a proxy whos value is a `structtype` or `<: Module`, we can access any field using `operator[]`:

```cpp
State::safe_eval(R"(
    mutable struct StructType
        _field
    end
    
    instance = StructType(1234)
)");

auto instance = Main["instance"];

std::cout << instance["_field"].operator int() << std::endl;
```
```
1234
```

As before, `operator[]` returns a named proxy. Assigning a named proxy also assigns the corresponding variable. This means we can assign fields just like we assigned variables in module-scope before:

```cpp
State::safe_eval(R"(
    mutable struct StructType
        _field
    end
    
    instance = StructType(1234)
)");

auto instance = Main["instance"];
auto instance_field = instance["_field"];

instance_field = 5678;

State::eval("println(instance)");
```
```
StructType(5678)
```

Of course, we could also do the above inline:

```cpp
Main["instance"]["_field"] = 9999;
State::eval("println(instance)");
```
```
StructType(9999)
```

Which is, again, highly convenient.

For proxies who are indexable (that is `getindex(::T, ::Integer)` is defined), `operator[](size_t)` is also provided, however we will learn more about Arrays and Vectors in [their own section](#arrays).

## Module Proxies

We've seen how proxies handler their value. While the general-purpose `jluna::Proxy` handles primitives and structtypes well, some julia classes provide additional functionality beyond what we've used so far. One of these more specialized proxy is `jluna::Module`. For the sake of brevity, henceforth, `jluna::Proxy` will be referred to as "general proxy" while `jluna::Module` will be called "module proxy".  

A general proxy can hold any value, including that of a module, however, `jluna::Module`, the module proxy, can only hold a julia-side module. Furthermore, `Module` inherits from `Proxy` and thus retains all public member functions. Being more specialized just means that we have additional member functions to work with.

### Constructing Module Proxies

We can create a module proxy like so:
    
```cpp
auto as_general = State::safe_eval("return Base");

// implicit cast
Module as_module = as_general;

// explicit cast
auto as_module = as_general.operator Module();

// using .as<T>
auto as_module = as_general.as<Module>();
```

If the original proxy `as_general` was unnamed then the resulting module proxy will also be unnamed, if the original was named, the newly generated module proxy will be named. 

`jluna` offers 3 already initialized module proxies that we've seen used in a previous section:

```cpp
// in module.hpp
Module Main;
Module Core;
Module Base;
```

These proxies are named proxies holding the singleton instance of the correspondingly named module. They are initialized by `State::initialize` and are thus globally available. 

Of course, just as before, we can access any named variable in a module by using `operator[]`:

```cpp
Module our_core = Main["Base"]["Core"];
std::cout << (our_core == jluna::Core) << std::endl;
```
```
1
```

### Creating or Assigning Variables in a Module

For demonstration purposes, let's first create our own module julia-side and then create a module proxy of it:

```cpp
State::safe_eval(R"(
    module OurModule
        variable = 123
    end
)");

Module our_module = Main["OurModule"];
```

Obviously `OurModule.variable` is not const, so it is mutable variable. If we try to assign it. however:

```cpp
our_module["variable"] = 456;
```
```
terminate called after throwing an instance of 'jluna::JuliaException'
  what():  [JULIA][EXCEPTION] cannot assign variables in other modules
Stacktrace:
 [1] setproperty!(x::Module, f::Symbol, v::Int32)
   @ Base ./Base.jl:36
 [2] top-level scope
   @ none:1
 [3] eval
   @ ./boot.jl:373 [inlined]
 [4] eval
   @ ./client.jl:453 [inlined]
 [5] assign(::Int32, ::Symbol, ::Vararg{Symbol})
   @ Main.jluna.memory_handler ~/Workspace/jluna/include/jluna.jl:586
 [6] safe_call(::Function, ::Int32, ::Symbol, ::Vararg{Symbol})
   @ Main.jluna.exception_handler ~/Workspace/jluna/include/jluna.jl:425

signal (6): Aborted
```

We get an exception. This is expected as the same expression `OurModule.variable = 456` in the julia REPL would throw the same exception. The reason for this is that both `State::eval` and `Proxy::operator[]` evaluate the expression containing assignment in `Main` scope. Thus, any variable that is not in `Main` cannot be assigned. `jluna::Module`, then, allows for exactly this through two familiar member functions `eval` and `safe_eval`:

```cpp
State::safe_eval(R"(
    module OurModule
        variable = 123
    end
)");

Module our_module = Main["OurModule"];
our_module.eval("variable = 456");
Base["println"](our_module["variable"]);
```
```
456
```

Where `eval` does not forward exceptions while `safe_eval` does, just like using the global `State::(safe_)eval`.<br>
Equivalently, module proxies offer two functions `assign` and `create_or_assign` that assign a variable of the given name the given value. If the variable does not exist, `assign` will throw an `UndefVarError`, while `create_or_assign` will create a new variable of that name and value in module-scope:

```cpp
our_module.create_or_assign("new_variable", std::vector<size_t>{1, 2, 3, 4});
Base["println"](our_module["new_variable"]);
```
```
[1, 2, 3, 4]
```

### Module Properties

A property, in julia, can be best though of as a "hidden" field. Some are accesible through `getproperty` in julia, however, some are only accesible through the C-API. `jluna` offers convenient member functions that directly access the following properties and return a C++ friendly type:

```cpp
// property :name
jl_sym_t* get_symbol() const;

// property :super
Module get_parent_module() const;

// hidden C-property: uuid
std::pair<size_t, size_t> get_uuid() const;

// hidden C-property: istopmod
bool is_top_module() const;

// hidden C-property: optlevel
int8_t get_optimization_level() const;

// hidden C-property: compile
int8_t get_compile_status() const;

// hidden C-property: infer
int8_t get_type_inference_status() const;

// hidden C-property: bindings
[[nodiscard]] std::map<Symbol, Any*> get_bindings() const;

// hidden C-property: usings
[[nodiscard]] std::vector<Module> get_usings() const;
```

While some of these are more useful than other, we will for now focus on the latter two: `usings` and `bindings`.

#### Usings

Usings list all modules included with the `using` keyword:

```cpp
State::eval(R"(
module OurModule
    using Base
    using Core
    using Core #sic
end
)");

Module our_module = Main["OurModule"];
for (auto& module : our_module.get_usings())
    std::cout << module.operator std::string() << std::endl;
```
```
Core
Base
```

#### Bindings

Bindings are a little more relevant. `Module::get_bindings` returns a map (or `IdDict` in julia parlance). For each pair in the map, `.first` is the *name* (of type `jluna::Symbol`) of a variable, `.second` is the *value* of the variable.

We will learn more about `Symbol` and `Type` in the [section on introspection](#introspection). For now, simply think of them as their julia-side equivalent `Base.Symbol` and `Base.Type{T}`:

```cpp
State::eval(R"(
module OurModule
    
    var1 = 0
    var2 = Int64(0)
    
    f(xs..) = println(xs)
end
)");

Module our_module = Main["OurModule"];
for (auto& pair : our_module.get_bindings())
    std::cout << pair.first.operator std::string() << " => " << jl_to_string(pair.second) << std::endl;
```
```
```

Where `jl_to_string` calls `Base.string` on an `Any*` and returns the resulting string.





## Functions

Functions in julia are movable, reassignable objects just like any other type, however in C++ they are usually handled separately. `jluna` aims to bridge this gap through its function interface.

### Calling julia Functions from C++

Proxies can hold any julia-side value. This includes functions:

```cpp
State::safe_eval("f(x) = sqrt(x^x^x)");

// with operator[]
auto named_f = Main["f"];

// or via return
auto unnamed_f = State::safe_eval("return f");
```

If the proxy is a function, it will be callable via `operator()`, `.call` and `.safe_call`, where `operator()` simply forwards the arguments to `.safe_call`:

```cpp
auto println = Main["println"];

// with exception forwarding
println(/* ... */);
println.safe_call(/* ... */);

// without exception forwarding
println.call(/* ... */);

// inline:
Base["println"](Base["typeof"](std::set<std::map<size_t, std::pair<size_t, std::vector<int>>>>({{{1, {1, {1, 1, 1}}}}})));
```
```
Set{IdDict{UInt64, Pair{UInt64, Vector{Int32}}}}
```

As mentioned before, any boxable type including proxies themself can be used as arguments directly, without manually having to call `box` or `unbox<T>`.

### Calling C++ Functions from julia
The previous section dealt with calling julia-side functions from C++. Now we will learn how to call C++-side functions from julia. To do this without using the C-API (which can call C-only functions directly but has no mechanism of calling C++-only functions), we need to limit our understanding of "functions" to actually mean C++-lambdas.

It is vital that you are familiar with lambdas, their syntax such as trailing return types, what capturing means and how templated lambdas work in C++20. If this is not the case, please consult the [official documentation](https://en.cppreference.com/w/cpp/language/lambda) before continuing.

To call a specific C++ lambda from julia, we first need to *register* it.

#### Registering Functions
```cpp
// always specify trailing return type manually
register_function("add_2_to_vector", [](Any* in) -> Any* {

    // convert in to jluna::Vector
    Vector<size_t> vec = in;

    // add 2 to each element
    for (auto e : vec)
        e = e.operator size_t() + 2;

    // return vector to julia
    return vec.operator Any*(); // box or cast to Any*
});

result = cppcall(:add_2_to_vector, [1, 2, 3, 4])
println("julia prints: ", result)
```

Note the explicit trailing return type `-> Any*`. It is recommended to always specify it when using lambdas in `jluna` (and, to some, in C++ in general). Specifying `-> void` will make the function return `nothing` to julia.

#### Calling Functions

After having registered a function, we can now call it (from within julia) using `cppcall`, which is provided by the julia-side `jluna` module (and exported into global scope). 

It has the following signature:

```julia
cppcall(::Symbol, xs...) -> Any
```

Unlike julias `ccall`, we do not supply return or argument types, all of these are automatically deduced by `jluna`. We are furthmernore not limited to C-friendly types:

+ any `Boxable` can be used as return type
+ any `Unboxable` can be used as argument type

This means for the C++-code to be able to use your julia-only typed arguments, you will have to first unbox them.

Let's call our above example which takes an arbitrary vector of integers and adds 2 to each element, then returns it:

```cpp
State::eval(R"(
  result = cppcall(:add_2_to_vector, [1, 2, 3, 4])
  println("julia prints: ", result)
)");
```
```
julia prints: [3, 4, 5, 6]
```

Through this method, we can call a very specific lambda only after registering it. This may seem restrictive at first, but we will soon see how through clever programming we can actually call truly arbitrary C++ code like this.

#### Allowed Function Names

We are, however, restricted to only certain function names. While arbitrary julia-valid characters (except `.` and `#`) in the functions name are allowed, it is recommended to stick to C++ style convention when naming functions. Do feel free to specifically use postfix `!` for mutating functions, as it is customary in julia. 

```julia
# good:
"print_vector", "add", "(=>)", "modify!"

# discouraged but allowed:
"h!e!l!p!", "get_∅", "écoltäpfel", "(☻)"

# illegal:
"anything.withadot", "#123", "0012"
```

 See the [julia manual entry on variable names](https://docs.julialang.org/en/v1/manual/variables/#man-allowed-variable-names) for more information about strictly illegal names. Any name disallowed there is also illegal in `jluna`. Additionally, `jluna` disallows names starting with `#` as they are reserved for internal IDs.

#### Possible Signatures

Only the following signatures for lambdas are allowed (this is enforced at compile time):

```cpp
() -> void
(Any*) -> void
(std::vector<Any*>) -> void
(Any*, Any*) -> void
(Any*, Any*, Any*) -> void
(Any*, Any*, Any*, Any*) -> void

() -> Any*
(Any*) -> Any*
(std::vector<Any*>) -> Any*
(Any*, Any*) -> Any*
(Any*, Any*, Any*) -> Any*
(Any*, Any*, Any*, Any*) -> Any*
```

Templated lambdas will be supported in a future version but are currently disallowed as of `jluna v0.5`

#### Using Non-julia Objects in Functions

While this may seem limiting at first, as state, it is not. While we are restricted to `Any*` for arguments, we can instead access arbitrary C++ objects by reference or by value through **captures**: 

```cpp
// a C++-object, uncompatible with julia
struct IncompatibleObject
{
    void operator()(size_t x) // not const
    {
        _field = x;
        std::cout << "object called " << _field << std::endl;
    }

    size_t _field = 123;
};

IncompatibleObject instance;

// wrap instance in mutable std::ref and hand it to lambda via capture
register_function("call_object", [instance_ref = std::ref(instance)](Any* in) -> void 
{
    instance_ref.operator()(unbox<size_t>(in));
});

State::safe_eval("cppcall(:call_object, 456)");
```
```
object called 456
```

This makes it possible to call C++-only static and non-static member function, and modify C++-side objects through their member functions. We may not be able to directly set a field, but we can simply write a setter, wrap the setter in a lambda, then call the lambda - along while capture the instance - from julia.

#### Calling Templated Lambda Functions

(this feature is not yet implemented but is planned for v0.6) 

Until then, we can simply register multiple lambdas, one for each template argument variation like so:

```cpp
// our templated function
template<typename T>
T template_function(T in)
{
    return do_somesthing<T>(in);
}

// overload for T = Int32
register_function("template_function_int32" [](Any* in) -> Any* {
    
    auto unbox = unbox<Int32>(in);
    auto res = template_function<Int32>(unbox);
    return box<Int32>(res);
});

// overload for T = float
register_function("template_function_float" [](Any* in) -> Any* {
    
    auto unbox = unbox<float>(in);
    auto res = template_function<float>(unbox);
    return box<float>(res);
})

// overloaf for T = std::vector<float>
register_function("template_function_vector_float_" [](Any* in) -> Any* {
    
    auto unbox = unbox<std::vector<float>>(in);
    auto res = template_function<std::vector<float>>(unbox);
    return box<std::vector<float>>(res);
})

// etc.
```

### Boxing Lambdas

While not mentioned for clarity until now, lambdas with the allowed signature are actually `Boxable`:

```cpp
// create a new empty variable named "lambda" julia-side
auto lambda_proxy = State::new_named_undef("lambda");

// assign the proxy a lambda
lambda_proxy = []() -> void {
    std::cout << "cpp called" << std::endl;
};
```

Here, we first create an uninitialized variable named `Main.lambda` julia-side, being returned a named proxy which manages this variable. <br>
This proxy is then assigned a lambda of signature `() -> void`, which an allowed signature. Because the proxy is named and because the lambda is boxable, this operation is valid and also assigns `Main.lambda`. `Main.lambda` now has the following value:

```cpp
State::eval("println(Main.lambda)");
```
```
Main.jluna._cppcall.UnnamedFunctionProxy(
  Symbol("#1"), 
  (...)
)
```

We see that it is an unnamed function proxy with internal id `#1`. This type of object is callable, exactly like the lambda is C++ side, so we can use it just like any other function:

```cpp
State::eval("Main.lambda()");
```
```
cpp called
```

For lambdas with different signature, they would of course expect 1, 2, etc. many arguments. The correct number of arguments is enforced at runtime.

## Arrays

julia-side objects of type `T <: AbstractArray` that are rectangular (or box or n-dimensional orthotopic) are represented C++-side by their own proxy type: `jluna::Array<T, R>`. Just like in julia, `T` is the value_type and `R` the rank (or dimensionality) of the array. <br>

julia array types and `jluna` array types correspond to each other like so:

| julia          | jluna v0.5    | jluna v0.6+    |
|----------------|---------------|---------------|
| `Vector{T}`    | `Vector<T>`   | `Vector<T>`   |
| `Matrix{T}`    | `Array<T, 2>` |`Matrix<T>` |
| `Array{T, R}`  | `Array<T, R>` |`Array<T, R>`  

Where `jluna::Vector` and `jluna::Matrix` inherit from `jluna::Array` and thus provides all of the same functionality while offering some exclusive methods in addition.

### CTORs

We can create an array proxy like so:

```cpp
// make 3d array with values 1:27
State::safe_eval("array = Array{Int64, 3}(reshape(collect(1:(3*3*3)), 3, 3, 3))");

// unnamed array proxy
jluna::Array<Int64, 3> unnamed = State::eval("return array");

// named array proxy
jluna::Array<Int64, 3> named = Main["array"];
```
Note that instead of `auto`, we declared `unnamed` and `named` to be explicitly of type `jluna::Array<Int64, 3>`. This declaration induces a conversion from `jluna::Proxy` (the super class) to `jluna::Array` (the child class). A functonally equivalent way to do this would be:

```cpp
auto array = State::eval("return array").as<Array<Int64, 3>();
```

However, the latter is discouraged for style reasons.


We can use the generic value type `Any` to make it possible for the array proxy to attach any julia-side array, regardless of value type. `jluna` provides 3 convenient typedefs for this:

```cpp
using Array1d = Array<Any*, 1>;
using Array2d = Array<Any*, 2>;
using Array3d = Array<Any*, 3>;
```

This is useful when the value type of the array is not know at the point of proxy declaration or if the actual value type of each element is non-homogenous, as this is a feature of julias array but not possible using `std::vector` or similar classes. 

```cpp
State::eval("hetero_array = [Int64(1), Float32(2), Char(3)]")
Array<UInt64, 1> as_uint64 = Main["hetero_array"]; // triggers cast to UInt64 (aka. size_t)
Array1d as_any = Main["hetero_array"]; // triggers no cast
```


### Indexing

There are two ways to index a multidimensional array jluna:

+ **linear** indexing treats the array as 1-dimensional and returns the n-th value in column-major order
+ **multi-dimensional** indexing requires one index per dimension and returns the array as if the index was a spacial coordinate

To keep with C-convention, indices in `jluna` are 0-based instead of 1-based.

```cpp
jluna::Array<Int64, 3> array = Main["array"];
Main["println"]("before ", array);

// 0-based linear indexing
array[12] = 9999;

// 0-based multi-dimensional indexing
array.at(0, 1, 2) = 9999;

Main["println"]("after ", array);
```
```
before [1 4 7; 2 5 8; 3 6 9;;; 10 13 16; 11 14 17; 12 15 18;;; 19 22 25; 20 23 26; 21 24 27]
after [1 4 7; 2 5 8; 3 6 9;;; 10 9999 16; 11 14 17; 12 15 18;;; 19 9999 25; 20 23 26; 21 24 27]
```

While it may be easy to remember to use 0 for `jluna` objects, make sure to be aware of when you're calling C++-functions and when you are calling julia-functions:

```cpp
State::eval("array = collect(1:9)");
Array<size_t, 1> cpp_array = Main["array"];

size_t cpp_at_3 = cpp_array.at(3);                  // c++: 0-based
size_t jl_at_3 = Base["getindex"](cpp_array, 3);    // julia: 1-based

std::cout << "cpp  : " << cpp_at_3 << std::endl;
std::cout << "julia: " << jl_at_3 << std::endl;
```
```
cpp  : 4
julia: 3
```

#### Julia-Style List Indexing

While `jluna` doesn't yet offer list-comprehension as nice as julias, `jluna::Array` does allow for julia-style indexing using a collection:

```cpp
auto sub_array = array.at({2, 13, 1}); // any iterable collection can be used
Base["println"](sub_array)
```
```
[3, 14, 2]
```

#### 0-based vs 1-based

To closer illustrate the relationship between indexing in `jluna` and indexing in julia, consider the following table (where `M` is a N-dimensional array)

| N | julia |jluna |
|------|-------|--------------------------|
| 1    | `M[1]`| `M.at(0)` or `M[0]`|
| 2    | `M[1, 2]`  | `M.at(0, 1)`|
| 3    | `M[1, 2, 3]`  | `M.at(0, 1)`|
| Any  | `M[ [1, 13, 7] ]`| `M[ {0, 12, 6} ]` |
|      |        |     |
| *    | `M[:]`  | not available | 

### Iterating

In `jluna`, arrays of any dimensionality are iterable in column-major order (just as in julia):

```cpp
Array<size_t, 2> array = State::eval("return [1:2 3:4 5:6]");
Base["println"](array);

// iterate using value type
for (size_t i : array)
  std::cout << i << std::endl;
```
```
[1 3 5; 2 4 6]
1
2
3
4
5
6
```

We can create an assignable iterator by doing the following (note the use of `auto` instead of `auto&`)

```cpp
Array<size_t, 2> array = State::eval("return [1:2 3:4 5:6]");
Base["println"]("before: ", array);

for (auto it : array)
    it = static_cast<size_t>(it) + 1;

Base["println"]("after : ", array);
```
```
before: [1 3 5; 2 4 6]
after : [2 4 6; 3 5 7]
```

Here, `auto` is deduced to a special iterator type that basically acts like a regular `jluna::Proxy` (for example, we need to manually cast it to `size_t` in the above example) but with faster, no-overhead read/write-access to the array data than an actual proxies accessed via `Proxy::operator[]`.

### Vectors

Just like in julia, all vectors are array, however their 1-dimensionality gives them access to additional functions:

```cpp
State::safe_eval("vector = collect(1:10)");
jluna::Vector<size_t> = Main["vector"];

vector.push_front(9999);
vector.push_back(9999);
vector.insert(12, 0);
vector.erase(12);

for (auto e : vector)
    e = static_cast<size_t>(e) + 1;

Base["println"](vector);
```
```
[10000, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 10000]
```

Note that `Array<T, R>::operator[](Range_t&&)` (linear indexing with a range) always returns a vector of the corresponding value type, regardless of the original arrays dimensionality.

## Matrices
(this feature is not yet implemented, simply use `Array<T, 2>` until then)

## Expressions

(this feature is not yet implemented)

## Usertypes

(this feature is not yet implemented)

## C-API

(this section is not fully formatted yet. Code examples are given, however)

```cpp
#include <julia.h>
#include <include/julia_extension.hpp>
```

### Meaning of C-Types

+ `jl_value_t*`: address julia-side value of arbitrary type
+ `jl_function_t*`: address of julia-side function
+ `jl_sym_t*`: address of julia-side symbol
+ `jl_module_t*`: address of julia-side singleton module
    - `jl_main_module`: Main
    - `jl_base_module`: Base
    - `jl_core_module`: Core
+ `jl_datatype_t*`: address of julia-side singleton type
    - `jl_any_type`: Any
    - `jl_type_type`: Type
    - `jl_module_type`: Module
    - `jl_integer_type`: Integer
    - etc.
    
### Executing C Code

```cpp
jl_eval_string("your code goes here")
jl_eval_string(R"(
    your 
    multi-line
    code
    goes
    here
)");
```

### Forwarding Exceptions in C
```cpp
jl_eval_string("sqrt(-1)"); 
// nothing happens

jluna::forward_last_exception();
// exception thrown
```
```
terminate called after throwing an instance of 'jluna::juliaException'
  what():  DomainError(-1.0, "sqrt will only return a complex result if called with a complex argument. Try sqrt(Complex(x)).")
Stacktrace: <no stacktrace available>
```

### Accessing Values in C

```cpp
// by address:
Any* address = jl_eval_string("return value_name");

// by value
T value = unbox<T>(jl_eval_string("return value_name"));

// there is no memory-safe, GC-safe way to modify variables from within C only
```
### Functions in C

#### Accessing Functions in C

```cpp
jl_function_t* println = jl_get_function((jl_module_t*) jl_eval_string("return Main.Base")), "println");
```

#### Calling Functions in C

```cpp
std::vector<T> cpp_arguments = /* ... */ // T is any boxable type

auto* function = jl_get_function(jl_base_module, "println");
std::vector<Any*> args;

for (auto a : cpp_arguments)
    args.push_back(box(a));
    
Any* res = jl_call(function, args, args.size());
```

### Arrays in C
#### Accessing & Indexing Arrays in C

```cpp
jl_array_t* array = (jl_array_t*) jl_eval_string("return [[1, 2, 3]; [2, 3, 4]; [3, 4, 5]]");

for (size_t i = 0; i < jl_array_len(array); ++i)    // 0-based
    std::cout << unbox<jluna::Int64>(jl_arrayref(array, i));    

// only linear indexing is available through the C-API
```
```
1 2 3 2 3 4 3 4 5
```

#### Mutating Arrays in C
```cpp
jl_array_t* array = (jl_array_t*) jl_eval_string("return [1, 2, 3, 4]");

jl_arrayset(array, box<Int64>(999), 0); 
// box<T> with exact equivalent of T julia
// jluna does fuzzy conversion behind the scenes, C doesn't

jl_function_t* println = jl_get_function(jl_base_module, "println");
jl_call1(println, (Any*) array);
```

### Strings in C
#### Accessing Strings in C
```cpp
Any* jl_string = jl_eval_string("return \"abcdef\"");
std::string std_string = std::string(jl_string_data(jl_string));
std::cout << std_string << std::endl;
```
```
abcdef
```
To modify strings, we need to cast them `jl_array_t` and modify them like arrays

### Initialization & Shutdown in C

```cpp
#include <julia.h>

int main() 
{
    jl_init();
    // or
    jl_init_with_image("path/to/your/image", NULL);
    
    // all your application
    
    jl_atexit_hook(0);
}
```






