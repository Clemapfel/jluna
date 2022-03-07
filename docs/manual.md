# jluna: An Introduction

This page will give an overview of most of `jluna`s relevant features and syntax.

### Table Of Contents

Please navigate to the appropriate section by clicking the links below:

1. [**Initialization & Shutdown**](#initialization)<br>
2. [**Executing Code**](#executing-code)<br>
3. [**Controlling the Garbage Collector**](#garbage-collector-gc)<br>
  3.1 [Enabling/Disabling GC](#enablingdisabling-gc)<br>
  3.2 [Manual Collection](#manually-triggering-gc)<br>
4. [**Boxing / Unboxing**](#boxing--unboxing)<br>
  4.1 [Manual](#manual-unboxing)<br>
  4.2 [(Un)Boxable as Concepts](#concepts)<br>
  4.3 [List of (Un)Boxables](#list-of-unboxables)<br>
5. [**Accessing Variables through Proxies**](#accessing-variables)<br>
  5.1 [Mutating Variables](#mutating-variables)<br>
  5.2 [Accessing Fields](#accessing-fields)<br>
  5.3 [Proxies](#proxies)<br>
  5.4 [Named Proxies](#named-proxies)<br>
  5.5 [Unnamed Proxies](#unnamed-proxy)<br>
  5.6 [Detached Proxies](#detached-proxies)<br>
  5.7 [Making a Named Proxy Unnamed](#making-a-named-proxy-unnamed)<br>
6. [**Specialized Proxies: Modules**](#module-proxies)<br>
   6.1 [Constructing Module Proxies](#constructing-module-proxies)<br>
   6.2 [Creating/Assigning new Variables](#creating-or-assigning-variables-in-a-module)<br>
   6.3 [Properties](#module-properties)<br>
   6.4 [Usings](#usings)<br>
   6.5 [Bindings & Savestates](#bindings)<br>
7. [**Specialized Proxies: Symbols**](#symbol-proxies)<br>
   7.1 [CTORs](#symbol-proxies)<br>
   7.3 [Hashing & Comparisons](#symbol-hashing)<br>
8. [**Functions**](#functions)<br>
   8.1 [Accessing Julia Functions from C++](#functions)<br>
   8.2 [Calling Julia Functions from C++](#functions)<br>
   8.3 [Accessing C++ Functions from Julia](#registering-functions)<br>
   8.4 [Calling C++ Functions from Julia](#calling-c-functions-from-julia)<br>
   8.5 [Allowed Function Names](#allowed-function-names)<br>
   8.6 [Allowed Function Signatures](#possible-signatures)<br>
   8.7 [Using arbitrary Objects in Julia Functions](#using-non-julia-objects-in-functions)<br>
9. [**Specialized Proxies: Arrays**](#arrays)<br>
  9.1 [Constructing Arrays](#ctors)<br>
  9.2 [Indexing](#indexing)<br>
  9.3 [Iterating](#iterating)<br>
  9.4 [Vectors](#vectors)<br>
  9.5 [Matrices](#matrices)<br>
  9.6 [Generator Expressions](#generator-expressions)
10. [**Introspection**](#introspection)<br>
  10.1 [Type Proxies](#type-proxies)<br>
  10.2 [Core Type Constants](#core-types)<br>
  10.3 [Type Order](#type-order)<br>
  10.4 [Type Info: Parameters](#type-info)<br>
  10.5 [Type Info: Fields](#fields)<br>
  10.6 [~~Type Info: Methods~~](#methods)<br>
  10.7 [~~Type Info: Properties~~](#properties)<br>
  10.8 [Type Classification](#type-classification)<br>
11. [~~**Expressions**~~](#expressions)<br>
12. [**Usertypes**](#usertypes)<br>
    12.1 [Usertype Interface](#usertype-interface)<br>
    12.2 [Making the Type Compliant](#step-1-making-the-type-compliant)<br>
    12.3 [Enabling the Interface](#step-2-enabling-the-interface)<br>
    12.4 [Adding Property Routines](#step-3-adding-property-routines)<br>
    12.5 [Implementing the Type](#step-4-implementing-the-type)<br>
13. [~~**Paralell Execution**~~](#performance)<br>
14. [**Performance**](#performance)<br>
    14.1 [Cheat Sheet](#cheat-sheet)<br>
    14.2 [Avoiding String Parsing](#avoid-string-parsing)<br>
    14.3 [Staying Julia-Side](#stay-julia-side)<br>
    14.4 [(Un)Boxing](#minimize-unboxing)<br>
    14.5 [Proxy Construction](#minimize-proxy-construction)<br>
    14.6 [Using the C-Library](#use-the-c-library)<br>

## Initialization

Before any interaction with `jluna` or Julia can take place, the Julia state needs to be initialized:

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

When a program exits regularly, all Julia-side values allocated through `jluna` or the C-API are safely deallocated automatically. Do not call `jl_atexit_hook()` at any point, as this may unintentionally invalidate C++-side memory and cause undefined behavior.

## Executing Code

`jluna` has two ways of executing code (represented C++-side as a string): 
+ *with* exception forwarding and
+ *without* exception forwarding. 
  
If code called without exception forwarding fails, it will not only not report any exceptions but simply seem to do nothing. If a fatal error occurs, the entire application wil crash without warning.<br>

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

`safe_` overloads have marginal overhead from try-catching and sanity checking inputs. If you optimal performance (and the corresponding debug experience) is needed,`State::eval` provides a 0-overhead version instead. 

To run an entire Julia file, we can either trigger a `Base.include` call Julia-side, or we can call `State::(safe_)eval_file`. This function simply reads the file as a string and uses it as the argument for `State::(safe_)eval`. 

## Garbage Collector (GC)

The Julia-side garbage collector operates completely independently, just like it would in a pure Julia application. However, sometimes it is necessary to disable or control its behavior manually. To do this, `jluna::State` offers the following functions:

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

When using `jluna` and allocating memory specifically through it, objects are safe from being garbage collected. It is therefore rarely necessary to manually disable the GC. See the section on [proxies](#accessing-variables) for more information.

## Boxing / Unboxing

Julia and C++ do not share any memory. Objects that have the same conceptual type can have very different memory layouts. For example, `Char` in Julia is a 32-bit value, while it is 8-bits in C++. Comparing `std::set` to `Base.set` will of course be even more of a difference on a bit-level.<br>
Because of this, when transferring memory from one languages state to the others, we're not only moving memory but *converting it* by reformating its layout. 

**Boxing** is the process of taking C++-side memory, converting it, then allocating the now Julia-compatible memory Julia-side. Conversely, **unboxing** is the process of taking Julia-side memory, converting it, then allocating the now C-compatible memory C++-side. Boxing/Unboxing of any type is handled by an overload of the following functions:

```cpp
template<typename T>
Any* box(T);

template<typename T>
T unbox(Any*);
```
where `Any*` is an address of Julia-side memory of arbitrary type (but not necessarily of type `Base.Any`).

All box/unbox functions have exactly this signature, ambiguity is resolved via [C++ concepts](https://en.cppreference.com/w/cpp/language/constraints), [SFINAE](https://en.cppreference.com/w/cpp/types/enable_if) and general template magic. Feel free to check the [source code](../.src/unbox.inl) to get a feel for how it is done behind the scenes, for a more user-friendly overview, see the [section on usertypes](#usertypes).

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

// C++ -> Julia: boxing
Any* jl_side = box(cpp_side);

// Julia -> C++: unboxing
size_t back_cpp_side = unbox<size_t>(jl_side);

// verify nothing was lost in the conversion
std::cout << back_cpp_side << std::endl;
```
```
1001
```

Any type fulfilling the above requirements is accepted by most `jluna` functions. Usually, these functions will implicitly (un)box their arguments and return-types. which means, most of the time, we don't have to worry about manually calling `box`/`unbox<T>`. 

This also means that for a 3rd party class `MyClass` to be compatible with `jluna`, one only needs to define:

```cpp
template<Is<MyClass> T> 
Any* box(T value);

template<Is<MyClass> T>
T unbox(Any* value);
```

Where `Is<T, U>` is a concept that resolves to true if `U` and `T` are the same type.

### List of (Un)Boxables

The following types are both boxable and unboxable:

```cpp
// cpp type              // Julia-side type after boxing

jl_value_t*              => Any
jl_module_t*             => Module
jl_function_t*           => Function
jl_sym_t*                => Symbol

Any*                     => Any

bool                     => Bool
char                     => Char
int8_t                   => Int8
int16_t                  => Int16
int32_t                  => Int32
int64_t                  => Int64
uint8_t                  => UInt8
uint16_t                 => UInt16
uint32_t                 => UInt32
uint64_t                 => UInt64
float                    => Float32
double                   => Float64

jluna::Proxy             => /* value-type deduced during runtime */
jluna::Symbol            => Symbol
jluna::Type              => Type
jluna::Array<T, R>       => Array{T, R}     * °
jluna::Vector<T>         => Vector{T}       *
jluna::JuliaException    => Exception

std::string              => String
std::string              => Symbol
std::complex<T>          => Complex{T}      *
std::vector<T>           => Vector{T}       *
std::array<T, R>         => Vector{T}       *
std::pair<T, U>          => Pair{T, U}      *
std::tuple<Ts...>        => Tuple{Ts...}    *
std::map<T, U>           => Dict{T, U}      *
std::unordered_map<T, U> => Dict{T, U}      *
std::set<T>              => Set{T, U}       *

* where T, U are also (Un)Boxables
° where R is the rank of the array

std::function<void()>                       => function () -> Nothing     
std::function<void(Any*)>                   => function (::Any) -> Nothing
std::function<void(Any*, Any*)>             => function (::Any, ::Any) -> Nothing
std::function<void(Any*, Any*, Any*)>       => function (::Any, ::Any, ::Any) -> Nothing
std::function<void(Any*, Any*, Any*, Any*)> => function (::Any, ::Any, ::Any, ::Any) -> Nothing
std::function<void(std::vector<Any*>)>      => function (::Vector{Any}) -> Nothing

std::function<Any*()>                       => function () -> Any  
std::function<Any*(Any*)>                   => function (::Any) -> Any
std::function<Any*(Any*, Any*)>             => function (::Any, ::Any) -> Any
std::function<Any*(Any*, Any*, Any*)>       => function (::Any, ::Any, ::Any) -> Any
std::function<Any*(Any*, Any*, Any*, Any*)> => function (::Any, ::Any, ::Any, ::Any) -> Any
std::function<Any*(std::vector<Any*>)>      => function (::Vector{Any}) ::Any
        
Usertype<T>::original_type                  => T °
        
° where T is an arbitrary C++ type
```

We will learn more on how to box/unbox functions, specifically, in the [section on calling C++ functions from Julia](#functions). We can (un)box truly arbitrary C++ types through the [usertype interface](#usertypes), which we will explore later on, aswell.


For any of the above, the template meta function `to_julia_type` is provided to convert a C++ type into a Julia-type, where `to_julia_type<T>::type_name` is the name of the type as a string:

```cpp
std::cout << to_julia_type<Array<size_t, 4>>::type_name << std::endl;
``` 
```julia
Array{UInt64, 4}
```

---

## Accessing Variables

Now that we know that we can move values between C++ and Julia, we will learn how to actually do this.

Let's say we have a variable `var` Julia-side:

```cpp
State::eval("var = 1234")
```

To access the value of this variable, we could use the C-API. By calling `jl_eval_string`, we return a pointer to the memory `var` holds. We then `unbox` that pointer:

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

While both ways get us the desired value, neither is a good way to actually manage the variable itself. How do we reassign it? Can we dereference the C-pointer? Who has ownership of the memory? Is it safe from the garbage collector? <br>All these questions are hard to manage using the C-API, however `jluna` offers a simultaneous: `jluna::Proxy`.

### Proxies

A proxy holds two things: the **memory address of its value** and a **symbol**. We'll get to the symbol later, for now, let's focus on the memory:<br>
Memory held by a proxy is safe from the Julia garbage collector (GC) and assured to be valid. This means we don't have to worry about keeping a reference or pausing the GC when modifying the variable. Any memory, be it temporary or something explicitly referenced by a Julia-side variable or `Base.Ref`, is guaranteed to be safe to access as long as a C++-side proxy points to it.

We rarely create a proxy ourself, most of the time it will be generated for us by `State::(safe_)eval` or similar functions:

```cpp
State::eval("var = 1234")
auto proxy = State::eval("return var")
```
Use of `auto` simplifies the declaration and is encouraged whenever possible.<br>

Now that we have the proxy, we need to convert it to a value. Unlike with the C-APIs `jl_value_t*` (aka. `Any*`) we *do not* need to call `box`/`unbox<T>`:

```cpp
// all following statements are exactly equivalent:

int as_int = proxy;   // recommended

auto as_int = proxy.operator int();

auto as_int = static_cast<int>(proxy);

auto as_int = (int) proxy;

auto as_int = unbox<int>(proxy) // discouraged
```

Where the first version is encouraged for style reasons. `jluna` handles implicit conversion behind the scenes. This makes it, so we don't have to worry what the actual type of the Julia-value is. `jluna` will try to make our declaration work:

```cpp
State::eval("var = 1234")
auto proxy = State::eval("return var")

// all of the following works by triggering Julia-side conversion implicitly:
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

Of course, if the type of the Julia variable cannot be converted to the target type, an exception is thrown:

```cpp
std::vector<double> as_vec = proxy;
```
```
terminate called after throwing an instance of 'jluna::JuliaException'
  what():  MethodError: Cannot `convert` an object of type Int64 to an object of type Vector
  (...)
```

This is already much more convenient than manually unboxing c-pointers, however the true usefulness of proxies lies in their ability to not only access but *mutate* Julia-side values.

## Mutating Variables

To mutate a variable means to change its value, its type, or both.<br>

As stated before, a proxy holds exactly one pointer to Julia-side memory and exactly one symbol. There are two types of symbols a proxy can hold:

+ a symbol starting with the character `#` is called an **internal id**
+ any other symbol is called a **name**

The behavior of proxies changes, depending on whether their symbol is a name, or not. A proxies whose symbol is a name, is called a **named proxy**, a proxy whose symbol is an internal id, is called an **unnamed proxy**. 

To generate an unnamed proxy, we use `State::(safe_)eval`, `State::safe_return` or `State::new_unnamed_<T>`.<br> To generate a named proxy we use `Proxy::operator[]` or `State::new_named_<T>`:

```cpp
State::safe_eval("var = 1234");

auto unnamed_proxy = State::safe_eval("return var");
auto named_proxy = Main["var"];

// or, in one line:
auto named_proxy = State::new_named_int64("var", 1234);
```

where `Main`, `Base`, `Core` are global, pre-initialized proxies holding the corresponding Julia-side modules.

We can check a proxies name using `.get_name()`:

```cpp
std::cout << unnamed_proxy.get_name() << std::endl;
std::cout << named_proxy.get_name() << std::endl;
```
```
<unnamed proxy #9>
var
```
We see that `unnamed_proxy`s symbol is `#9`, while `named_proxy`s symbol is the same name as the Julia-side variable `var` that we used to create it.

It may seem hard to remember whether a function returns a named or unnamed proxy, however there is a simple logic behind it. In Julia:

```julia
var = 1234
new_variable = begin return var end
``` 
The statement `return var` returns **by value**, `new_variable` has no way to know where that value came from, thus mutating `new_variable` leaves `var` unaffected. It's the same with proxies, calling `State::eval("return var")` works exactly like in Julia, the function returns **by value**. `Proxy::operator[]` on the other hand *does* have access to the exact name, we explicitly used it to call the operator, hence why the proxy can store it for later use.

### Named Proxies

If and only if a proxy is named, assigning it will change the corresponding variable Julia-side:

```cpp
State::safe_eval("var = 1234")
auto named_proxy = Main["var"];

std::cout << "// before:" << std::endl;
std::cout << "cpp   : " << named_proxy.operator int() << std::endl;
State::safe_eval("println(\"Julia : \", Main.var)");

named_proxy = 5678; // assign

std::cout << "// after:" << std::endl;
std::cout << "cpp   : " << named_proxy.operator int() << std::endl;
State::safe_eval("println(\"Julia : \", Main.var)");
```
```
// before:
cpp   : 1234
Julia : 1234
// after:
cpp   : 5678
Julia : 5678
```

After assignment, both the value `named_proxy` is pointing to, and the variable of the same name `Main.var` were affected by the assignment. This is somewhat atypical for Julia but familiar to C++-users. A named proxy acts like a reference to the Julia-side variable. <br><br> While initially somewhat hard to wread ones head around, because of this behavior, we are able to do things like:

```cpp
State::safe_eval("vector_var = [1, 2, 3, 4]")
Main["vector_var"][0] = 999;   // indices are 0-based in C++
Base["println"]("Julia prints: ", Main["vector_var"]);
```
```
Julia prints: [999, 2, 3, 4]
```

Which is highly convenient. 

### Unnamed Proxy

Mutating an unnamed proxy will only mutate its value, **not** the value of any Julia-side variable:
```cpp
State::safe_eval("var = 1234")
auto unnamed_proxy = State::safe_eval("return var");

std::cout << "// before:" << std::endl;
std::cout << "cpp   : " << named_proxy.operator int() << std::endl;
State::safe_eval("println(\"Julia : \", Main.var)");

unnamed_proxy = 5678; // assign

std::cout << "// after:" << std::endl;
std::cout << "cpp   : " << named_proxy.operator int() << std::endl;
State::safe_eval("println(\"Julia : \", Main.var)");
```
```
// before:
cpp   : 1234
Julia : 1234
// after:
cpp   : 5678
Julia : 1234
```

Here, `unnamed_proxy` was assigned a new memory address pointing to the newly allocated Julia-side value `5678`. Meanwhile, `Main.var` is completely unaffected by this change. This makes sense, if we check `unnamed_proxy`s symbol again:

```cpp
std::cout << unnamed_proxy.get_name() << std::endl;
```
```
<unnamed proxy #9>
```

We see that it does not have a name, just an internal id. Therefore, it has no way to know where its value came from and thus has no way to mutate anything but its C++-side memory address. An unnamed proxy thus behaves like a deepcopy of the value, **not** like a reference.

#### In Summary

+ We create a **named proxy**
  - using `State::new_named_<T>`, `Proxy::operator[]`
  - Assigning a named proxy mutates its value and mutates the corresponding Julia-side variable of the same name
+ We create an **unnamed proxy**
  - using `State::new_unnamed_<T>`, `State::(safe_)eval`
  - Assigning an unnamed proxy mutates its value but does not mutate any Julia-side variable

This is important to realize and is the basis of much of `jluna`s syntax and functionality.

### Detached Proxies

Consider the following code:

```cpp
auto named_proxy = State::new_named_int("var", 1234);

State::safe_eval(R"(var = ["abc", "def"])");

std::cout << named_proxy.operator std::string() << std::endl;
```

What will this print? We know `named_proxy` is a named proxy, so it should correspond to the variable `var`, which we declared as having the value `1234`. However, we reassigned `var` to a completely different value and type using only Julia. `named_proxy` was never made aware of this, so it still currently points to its old value:

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
var
```

This proxy is in what's called a *detached state*. Even though it is a named proxy, its current value does not correspond to the value of its Julia-side variable. This may have unforeseen consequences:

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

Even though `var` is a vector Julia-side, accessing the second index of `named_proxy` throws a BoundsError because `name_proxy`s value is still `Int64(1234)`, which does not support `getindex`.

Assigning a detached proxy will still mutate the corresponding variable, however:

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

Because `named_proxy` still has its name `Main.var`, it reassigns the variable to `5678` which was specified through C++. 

While this is internally consistent and does not lead to undefined behavior (a proxy will mutate its variable, regardless of the proxies value or the current value of the variable), it may cause unintentional bugs when reassigning the same variable frequently in both C++ and Julia. To alleviate this, `jluna` offers a member function, `Proxy::update()`, which evaluates the proxies name and replaces its value with the value of the correspondingly named variable:

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

Sometimes it is desirable to stop a proxy from mutating the corresponding variable, even though it is a named proxy. While we cannot change a proxies name, we can generate a new unnamed proxy pointing to the same value using the member function `Proxy::as_unnamed()`. This functions returns an unnamed proxy pointing to a newly allocated deepcopy of the value of the original proxy. 

```cpp
State::eval("var = 1234");
auto named_proxy = Main["var"];

auto value = named_proxy.as_unnamed();   // create nameless deepcopy

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

Therefore, to make a named proxy unnamed, we can simply assign the result of `as_unnamed` to itself:

```cpp
auto proxy = /*...*/
proxy = proxy.value()
```

This way, the actual Julia-side memory is unaffected, but the proxy can now be mutated without interfering with any variable of the former name.

Calling `.as_unnamed()` on a proxy that is already unnamed simply creates another unnamed deepcopy with a new internal id.

### Accessing Fields

For a proxy whos value is a `structtype` (or `Module`), we can access any field using `operator[]`:

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

For proxies who are indexable (that is `getindex(::T, ::Integer)` is defined), `operator[](size_t)` is also provided. We will learn more about Arrays and Vectors in [their own section](#arrays).

---

## Module Proxies

We've seen how proxies handle their value. While the general-purpose `jluna::Proxy` deals with primitives and structtypes well, some Julia classes provide additional functionality beyond what we've seen so far. One of these more specialized proxies is `jluna::Module`. For the sake of brevity, henceforth, `jluna::Proxy` will be referred to as "general proxy" while `jluna::Module` will be called "module proxy", `jluna::Symbol` as "symbol proxy", `jluna::Type` as "type proxy", etc.. 

A general proxy can hold any value, including that of a module, however, `jluna::Module`, the specialized module proxy, can only hold a Julia-side value of type `Base.Module`. Furthermore, `Module` inherits from `Proxy` and thus retains all public member functions. Being more specialized just means that we have additional member functions to work with on top of what was already available.

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

If the original proxy `as_general` was unnamed, then the resulting module proxy will also be unnamed. If the original was named, the newly generated module proxy will be named. 

`jluna` offers 3 already initialized module proxies, we've seen them used in the previous section:

```cpp
// in module.hpp
Module Main;
Module Core;
Module Base;
```

These proxies are named proxies, pointing to the instance of the correspondingly named module. They are initialized by `State::initialize` and are thus globally available from that point onwards.

We can access any named variable in a module by using `operator[]`, similar to accessing fields of structtypes

```cpp
Module our_core = Main["Base"]["Core"];
std::cout << (our_core == jluna::Core) << std::endl;
```
```
1
```

### Creating or Assigning Variables in a Module

For demonstration purposes, let's first create our own module Julia-side and then create a module proxy of it:

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

We get an exception. This is expected, as the same expression `OurModule.variable = 456` in the Julia REPL would throw the same exception. The reason for this is that both `State::eval` and `Proxy::operator[]` evaluate the expression containing assignment in `Main` scope. Thus, any variable that is not in `Main` cannot be assigned. `jluna::Module`, however, does allow for this by providing the functions `eval` and `safe_eval`, both of which evaluate in its own scope:

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

Where `eval` does not forward exceptions, while `safe_eval` does, just like using the global `State::(safe_)eval`.<br>
Equivalently, module proxies offer two functions, `assign` and `create_or_assign`, that assign a variable of the given name a  given value. If the variable does not exist, `assign` will throw an `UndefVarError`, while `create_or_assign` will create a new variable of that name in module-scope, then assign it the value:

```cpp
our_module.create_or_assign("new_variable", std::vector<size_t>{1, 2, 3, 4});
Base["println"](our_module["new_variable"]);
```
```
[1, 2, 3, 4]
```

### Module Properties

A property, in Julia, can be best though of as a "hidden" field. Some of these are accessible through `getproperty` in Julia, however, some are only accessible through the C-API. `jluna` offers convenient member functions that directly access the following properties and return a C++ friendly type:

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

While some of these are more useful than other, we will now focus on the latter two: `usings` and `bindings`.

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

`Module::get_bindings` returns a map (or `IdDict` in Julia parlance). For each pair in the map, `.first` is the *name* (of type `jluna::Symbol`) of a variable, `.second` is the *value* of the variable, as an unnamed general proxy.
We will learn more about `jluna::Symbol` soon. For now, simply think of them as their Julia-side equivalent `Base.Symbol`.

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
    std::cout << pair.first.operator std::string() << " => " << jl_to_string(pair.second).operator std::string() << std::endl;
```
```
include => include
f => f
OurModule => Main.OurModule
eval => eval
var1 => 0
var2 => 0
```

where `jl_to_string` is a C-function that takes an `Any*` and calls `Base.string`, returning the resulting string.

Because the proxies hold ownership of the bound values and are unnamed, the result of `get_bindings` is a stable snapshot of a module. Even if the module continues to be modified, the map returned by `get_bindings` stays the same. This means `get_bindings` gives us a way to save the current state of the module.

---

## Symbol Proxies

Another specialized type of proxy is the symbol proxy. It holds a Julia-side `Base.Symbol`.<br>
We can create a symbol proxy in the following ways:

```cpp
Main.eval("symbol_var = Symbol("abc"));
auto general_proxy = Main["symbol_var"];

// implicit cast
Symbol symbol_proxy = general_proxy;

// explicit cast using .as<T>
auto symbol_proxy = general_proxy.as<Symbol>();

// newly allocate from string
auto unnamed = Symbol("abc");

// newly allocated using box
auto unnamed = box<Symbol>("abc");
```

Where, unlike with general proxy, the value the proxy is pointing to is asserted to be of type `Base.Symbol`. 

### Symbol Hashing

The main additional functionality `jluna::Symbol` brings, is that of *constant time hashing*.

A hash is essentially a UInt64 we assign to things as a label. In Julia, hashes are unique and there a no hash collisions. This means if `A != B` then `hash(A) != hash(B)` and, furthermore, if `hash(A) == hash(B)` then `A == B`. Unlike with many other classes, `Base.Symbol` can be hashed in constant time. This is because the hash is precomputed at the time of construction. We access the hash of a symbol proxy using `.hash()`

```
auto symbol = Main["symbol_var"];
std::cout << "name: " << symbol.operator std::string() << std::endl;
std::cout << "hash: " << symbol.hash() << std::endl;
```
```
name: abc
hash: 16076289990349425027
```

In most cases, it is impossible to predict which number will be assigned to which symbol. This also means for Symbols `s1` and `s2` if `string(s1) < string(s2)` this **does not** mean `hash(s1) < hash(s2)`. This is very important to realize because `jluna::Symbol`s other main functionality is being constant-time *comparable*. This comparison is not lexicographical, rather, comparing two symbols means to compare their corresponding hashes.

Symbol proxies provide the following comparison operators:

```cpp
/// (*this).hash() == other.hash()
bool operator==(const Symbol& other) const;

/// (*this).hash() != other.hash()
bool operator!=(const Symbol& other) const;

/// (*this).hash() < other.hash()
bool operator<(const Symbol& other) const;

/// (*this).hash() <= other.hash()
bool operator<=(const Symbol& other) const;

/// (*this).hash() >= other.hash()
bool operator>=(const Symbol& other) const;

/// (*this).hash() > other.hash()
bool operator>(const Symbol& other) const;
```

To further illustrate the way symbols are compared, consider the following example using `std::set`, which orders its elements according to their comparison operators:

```cpp
auto set = std::set<Symbol>();
for (auto str : {"abc", "bcd", "cde", "def"})
    set.insert(Symbol(str));

// print in order
for (auto symbol : set)
    std::cout << symbol.operator std::string() << " (" << symbol.hash() << ")" << std::endl;
```
```
cde (10387276483961993059)
bcd (11695727471843261121)
def (14299692412389864439)
abc (16076289990349425027)
```

We see that, lexicographically, the symbols are out of order. They are, however, ordered properly according to their hashes.

---

## Functions

Functions in Julia are movable, reassignable objects just like any other type. In C++, raw functions are usually handled separately, and while object wrappers exist, they are very different from Julia functions. `jluna` aims to bridge this gap through its own function interface.

### Calling Julia Functions from C++

Proxies can hold any Julia-side value. This includes functions:

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
Set{Dict{UInt64, Pair{UInt64, Vector{Int32}}}}<T>
```

As mentioned before, any boxable type, including proxies themself, can be used as arguments directly without manually having to call `box` or `unbox`. If an argument is not (un)boxable, a compile-time error is thrown. This makes `jluna`s way of calling functions much, much safer than the C-APIs `jl_call`.

### Calling C++ Functions from Julia

The previous section dealt with calling Julia-side functions from C++. We will now learn how to call C++-side functions from Julia. To do this without using the C-API (which can call C-only functions directly but has no mechanism for calling C++-only functions), we need shift our vocabulary: From now on with "Functions", we mean "Lambdas".

> It is vital that you are familiar with lambdas, their syntax (such as trailing return types), what capturing means and how templated lambdas work in C++20. If this is not the case, please consult the [official documentation](https://en.cppreference.com/w/cpp/language/lambda) before continuing.

To call a specific C++ lambda from Julia, we first need to *register* it.

#### Registering Functions
```cpp
// always specify trailing return type manually
register_function("add_2_to_vector", [](Any* in) -> Any* {

    // convert in to jluna::Vector
    Vector<size_t> vec = in;

    // add 2 to each element
    for (auto e : vec)
        e = e.operator size_t() + 2;

    // return vector to Julia
    return vec.operator Any*(); // box or cast to Any*
});
```

Note the explicit trailing return type `-> Any*`. It is recommended to always specify it when using lambdas in `jluna` (and, for style reasons only, in C++ in general). Specifying `-> void` will make the function return `nothing` to Julia, specifying `-> Any*` will make the return value available to Julia, regardless of its type.

#### Calling Functions

After having registered a function, we are now able call it (from within Julia) using `cppcall`, which is provided by the Julia-side `jluna` module (and exported into global scope during `State::initialize`). 

It has the following signature:

```julia
cppcall(::Symbol, xs...) -> Any
```

Unlike Julias `ccall`, we do not supply return or argument types, all of these are automatically deduced by `jluna`. We are furthmernore not limited to C-friendly types:

+ any `Boxable` can be used as return type
+ any `Unboxable` can be used as argument type

We will learn more about how to make our own custom types compatible with functions in the [~~section on usertypes~~](#usertypes).

Let's call our above example function. It takes an arbitrary vector of integers and adds 2 to each element, then returns that vector:

```cpp
register_function("add_2_to_vector", [](Any* in) -> Any* {

    Vector<size_t> vec = in;

    for (auto e : vec)
        e = e.operator size_t() + 2;

    return box(vec);
});

// call from Julia
State::eval(R"(
  result = cppcall(:add_2_to_vector, [1, 2, 3, 4])
  println("Julia prints: ", result)
)");
```
```
Julia prints: [3, 4, 5, 6]
```

It may seem very limiting to only be able to call two kinds of function signatures and having to register them first everytime, however, through clever design, this system actually allows us to call any C++ function - without exception.

#### Allowed Function Names

We are, however, restricted to only certain function *names*. While arbitrary Julia-valid characters (except `.` and `#`) in the functions name are allowed, it is recommended to stick to C++ style convention when naming functions. Do feel free to specifically use postfix `!` for mutating functions, as it is customary in Julia. 

```julia
# good:
"print_vector", "add", "(=>)", "modify!"

# discouraged but allowed:
"h!e!l!p!", "get_∅", "écoltäpfel", "(☻)"

# illegal:
"anything.withadot", "#123", "0012"
```

 See the [Julia manual entry on variable names](https://docs.julialang.org/en/v1/manual/variables/#man-allowed-variable-names) for more information about strictly illegal names. Any name disallowed there is also illegal in `jluna`. Additionally, `jluna` disallows names starting with `#`, as they are reserved for internal IDs, and names containing `.`, as they can confuse expression parsing.

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

Templated lambdas will be supported in a future version but are currently disallowed as of `jluna v0.7`. The correct signature is enforced at compile time.

#### Using Non-Julia Objects in Functions

While this may seem limiting at first, as stated, it is not. We are restricted to `Any*` for **arguments**, however we can instead access arbitrary C++ objects by reference or by value through **captures**, which are unrestricted: 

```cpp
// a C++-object, incompatible with Julia
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

This makes it possible to call C++-only, static and non-static member function and modify C++-side objects using them. We could capture more objects, other functions and other lambdas or even proxies, the possibilities are endless, hence why this system is able to call arbitrary C++ code.

#### Calling Templated Lambda Functions

(this feature is not yet implemented but is planned for v0.6) 

> **Tip**: Until then, we can simply register multiple lambdas, one for each template argument variation like so:

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

While not mentioned for clarity until now, lambdas with the allowed signature are actually `Boxable`. This means we can assign lambdas to proxies:

```cpp
// create a new empty variable named "lambda" Julia-side
auto lambda_proxy = State::new_named_undef("lambda");

// assign the proxy a lambda
lambda_proxy = []() -> void {
    std::cout << "cpp called" << std::endl;
};
```

Here, we first create an uninitialized variable named `Main.lambda` Julia-side. `new_named_undef` returned a named proxy which manages this variable. <br>
This proxy is then assigned a lambda of signature `() -> void`, which is a valid, legal signature. Because the proxy is named and because the lambda is boxable, this operation also assigns `Main.lambda`. `Main.lambda` now has the following value:

```cpp
State::eval("println(Main.lambda)");
```
```
Main.jluna._cppcall.UnnamedFunctionProxy(
  Symbol("#1"), 
  (...)
)
```

We see that it is an unnamed function proxy with internal id `#1`. This type of object is callable, exactly like the lambda is C++-side, so we can use it just like any other function:

```cpp
State::eval("Main.lambda()");
```
```
cpp called
```

For lambdas with different signature, they would of course expect 1, 2, etc. many arguments. 

This exact same process of assigning lambdas to Julia-side variables also works in module scope using `jluna::Module.assign` which makes it possible to construct entire modules consisting of only c++-side functionality.

## Arrays

Julia-side objects of type `T <: AbstractArray` that are rectangular (or, more generally, n-dimensional orthotopic) are represented C++-side by their own proxy type: `jluna::Array<T, R>`. Just like in Julia, `T` is the value-type and `R` the rank (or dimensionality) of the array. <br>

Julia array types and `jluna` array types correspond to each other like so:

| Julia          | jluna v0.7    | jluna v0.8+    |
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
Note that instead of `auto`, we declared `unnamed` and `named` to be explicitly of type `jluna::Array<Int64, 3>`. This declaration triggers a conversion from `jluna::Proxy` to `jluna::Array`. Just like with `Module` and `Symbol`, we can also explicitely cast a proxy to an array:

```cpp
auto array = State::eval("return array").as<Array<Int64, 3>();
```

We can use the generic value type `Any*` to make it possible for the array proxy to attach any Julia-side array, regardless of value type. `jluna` provides 3 convenient typedefs for this:

```cpp
using Array1d = Array<Any*, 1>;
using Array2d = Array<Any*, 2>;
using Array3d = Array<Any*, 3>;
```

This is useful when the value type of the array is not know at the point of proxy declaration or if the actual value type of each element is non-homogenous, as this is a feature of Julias array but not possible using `std::vector` or similar classes. 

```cpp
State::eval("heterogenous_array = [Int64(1), Float32(2), Char(3)]")
Array<UInt64, 1> as_uint64 = Main["heterogenous_array"]; // triggers cast to UInt64 (aka. size_t)
Array1d as_any = Main["heterogenous_array"]; // triggers no cast
```

### Indexing

There are two ways to index a multidimensional array jluna:

+ **linear** indexing treats the array as 1-dimensional and returns the n-th value in column-major order
+ **multi-dimensional** indexing requires one index per dimension and returns the array as if the index was a spacial coordinate

To keep with C-convention, indices in `jluna` are 0-based (rather than 1-based, like in Julia).

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

While it may be easy to remember to use 0 for `jluna` objects, make sure to be aware of whether you are calling a C++-side function, or a julia-side function:

```cpp
State::eval("array = collect(1:9)");
Array<size_t, 1> cpp_array = Main["array"];

size_t cpp_at_3 = cpp_array.at(3);                  // C++: 0-based
size_t jl_at_3 = Base["getindex"](cpp_array, 3);    // Julia: 1-based

std::cout << "cpp  : " << cpp_at_3 << std::endl;
std::cout << "Julia: " << jl_at_3 << std::endl;
```
```
cpp  : 4
Julia: 3
```

Function proxies that call julia-side functions still expect 1-based indices.

#### Julia-Style List Indexing

While `jluna` doesn't yet offer a list-comprehension engine as nice as that of Julia, `jluna::Array` does allow for Julia-style indexing using a collection:

```cpp
auto sub_array = array.at({2, 13, 1}); // any iterable collection can be used
Base["println"](sub_array)
```
```
[3, 14, 2]
```

#### 0-based vs 1-based

To further illustrate the relationship between indexing in `jluna` and indexing in Julia, consider the following table (where `M` is a N-dimensional array)

| N | Julia | jluna |
|------|-------|--------------------------|
| 1    | `M[1]` | `M.at(0)` or `M[0]`|
| 2    | `M[1, 2]` | `M.at(0, 1)`|
| 3    | `M[1, 2, 3]` | `M.at(0, 1, 2)`|
| Any  | `M[ [1, 13, 7] ]` | `M[ {0, 12, 6} ]` |
| Any  | `M[i for i in 1:10]` | `M["i for i in 1:10"_gen]`
|      |        |     |
| *    | `M[:]` | not available | 

We will learn more C++-side generator expressions in the [section on vector](#vectors).

### Iterating

In `jluna`, arrays of any dimensionality are iterable in column-major order (just as in Julia):

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

Iterators are assignable, doing the following (note the use of `auto` instead of `auto&`)

```cpp
Array<size_t, 2> array = State::eval("return [1:2 3:4 5:6]");
Base["println"]("before: ", array);
 
for (auto it : array) // auto, not auto&
    it = static_cast<size_t>(it) + 1;

Base["println"]("after : ", array);
```
```
before: [1 3 5; 2 4 6]
after : [2 4 6; 3 5 7]
```

Here, `auto` is deduced to a special iterator type that basically acts like a regular `jluna::Proxy` (for example, we need to manually unbox it to `size_t`), but with faster, no-overhead read/write-access to the array data. This enhanced speed is the main difference between using `Array<T, R>::operator[]` and `Proxy::operator[]`.

### Vectors

Just like in Julia, `jluna` vectors 1-dimensional arrays that offer a few functions in addition to what N-dimensional arrays have available:

```cpp
// construct from std::vector
auto vector = Vector<Int64>(std::vector<Int64>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

// append to front
vector.push_front(9999)

// append to back
vector.push_back(9999);

// insert at index (0-based)
vector.insert(12, 0);

// erase at index (0-base)
vector.erase(12);

// raise all values by 1
for (auto e : vector)
    e = static_cast<size_t>(e) + 1;

Base["println"](vector);
```
```
[10000, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 10000]
```

## Matrices

(this feature is not yet implemented, simply use `Array<T, 2>` until then)

## Generator Expressions

One of Julias most convenient features are [**generator expressions**](https://docs.julialang.org/en/v1/manual/arrays/#man-comprehensions) (also called list- or array-comprehensions). These are is a special kind of syntax that creates an iterable, in-line, lazy-eval range. For example:

```julia
# comprehension
[i*i for i in 1:10 if i % 2 == 0]

# mostly equivalent to
out = Vector()
f = i -> i*i
for i in 1:10
    if i % 2 == 0
        push!(out, f(i))
    end
end
```

In Julia, we use `[]` when we want the expression to be vectorized, and `()` when we want the expression to stay an object of type `Base.Generator`. The latter has the significant advantage that iterating and accessing its values is *lazy-eval*, meaning that we only do the any computation only when actually accessing the value. 

In `jluna`, we can create a generator expression using the postfix literal operator `_gen` on a C-string:

```cpp
// in Julia:
(i for i in 1:10 if i % 2 == 0)

// in cpp:
"(i for i in 1:10 if i % 2 == 0)"_gen
```
Note that when using `_gen`, **only round brackets are allowed**. Every generator expression has to be in round brackets, they cannot be ommitted or a syntax error will be triggered.

We can iterate through generator expression like so:

```cpp
for (auto i : "(i for i in 1:10 if i % 2 == 0)"_gen)
    std::cout << unbox<size_t>(i) << std::endl;
```
```
2
4
6
8
10
```

Where `i` needs to be unboxed manually and behaves exactly like an `Any*`. While this is convenient, the true power of generator expressions lies in its interfacing with `jluna::Vector`. We can:

```cpp
// ...initialize a vector from a generator expression
auto vec = Vector<size_t>("i*i for i in 1:99"_gen);

// ...use a generator expressions just like a list index
vec["i for i in 1:99 if i < 50"_gen];
```

This imitates Julia syntax very closely, despite being in a language that does not have list comprehension. 

---

## Introspection

The main advantage Julia has over C++ are its introspection and meta-level features. 
While introspection is technically possible in C++, it can be quite cumbersome and complicated. In Julia, things like deducing the signature of a function or classifying type, is fairly straight-forward and accessible, even to novice programmers. `jluna` aims to take advantage of this by giving a direct interface to that part of Julias toolset: `jluna::Type`.

### Type Proxies

We've seen specialized module-, symbol- and array-proxies. `jluna` currently has a fourth kind of proxy, `jluna::Type`, which is valuable in introspection. While overlap is present, `jluna::Type` is not a direct equivalent of `Base.Type{T}`. This section will introduce its many functionalities and how to best use them.

There are multiple ways to construct a type proxy:

```cpp
// get type of other proxy
auto general_proxy = State::eval("return " + /* ... */);
auto type = general_proxy.get_type();

// implicit cast
auto type_valued_proxy = State::eval("return Base.Vector");
Type type = type_valued_proxy;

// explicit cast with .as<T>
auto type = type_valued_proxy.as<Type>();

// deduce Julia-side type from (Un)boxable C++ Type
auto type = Type::construct_from<std::vector<int>>();
    // is now Base.Vector<Int32> Julia-side
```

Where the latter uses `to_julia_type<T>` to deduce which Julia-side type to construct the type proxy from.

### Core Types 

For convenience, `jluna` offers most of the types in `Core` and `Base` as pre-initialized global constants, similar to how the modules `Main`, `Base`, and `Core` are available after initialization.<br><br>
The following types can be accessed this way:

| `jluna` constant name | Julia-side name|
|-------------------|---------|
| `AbstractArray_t` | `AbstractArray{T, N}` |
| `AbstractChar_t` | `AbstractChar` |
| `AbstractFloat_t`| `AbstractFloat` |
| `AbstractString_t`| `AbstractString` |
| `Any_t`| `Any` |
| `Array_t`| `Array{T, N}` |
| `Bool_t`| `Bool` |
| `Char_t`| `Char` |
| `DataType_t`| `DataType` |
| `DenseArray_t`| `DenseArray{T, N}` |
| `Exception_t`| `Exception` |
| `Expr_t`| `Expr` |
| `Float16_t`| `Float16` |
| `Float32_t`| `Float32` |
| `Float64_t`| `Float64` |
| `Function_t`| `Function` |
| `GlobalRef_t`| `GlobalRef` |
| `IO_t`| `IO` |
| `Int8_t`| `Int8` |
| `Int16_t`| `Int16` |
| `Int32_t`| `Int32` |
| `Int64_t`| `Int64` |
| `Int128_t`| `Int128` |
| `Integer_t`| `Integer` |
| `UInt8_t`| `UInt8` |
| `UInt16_t`| `UInt16` |
| `UInt32_t`| `UInt32` |
| `UInt64_t`| `UInt64` |
| `UInt128_t`| `UInt128` |
| `Unsigned_t`| `Unsigned` |
| `Signed_t`| `Signed` |
| `LineNumberNode_t`| `LineNumberNode` |
| `Method_t`| `Method` |
| `Module_t`| `Module` |
| `NTuple_t`| `NTuple{T, N}` |
| `NamedTuple_t`| `NamedTuple` |
| `Nothing_t`| `Nothing` |
| `Number_t`| `Number` |
| `Pair_t`| `Pair{T, U}` |
| `Ptr_t`| `Ptr{T}` |
| `QuoteNode_t`| `QuoteNode` |
| `Real_t`| `Real` |
| `Ref_t`| `Ref{T}` |
| `String_t`| `String` |
| `Symbol_t`| `Symbol` |
| `Task_t`| `Task` |
| `Tuple_t`| `Tuple{T...}` |
| `Type_t`| `Type{T}` |
| `TypeVar_t`| `TypeVar` |
| `UndefInitializer_t`| `UndefInitializer` |
| `Union_t`| `Union{T...}` |
| `UnionAll_t`| `UnionAlll` |
| `VecElement_t`| `VecElement{T}` |
| `WeakRef_t`| `WeakRef` |

Where `T`, `U` are arbitrary types, `N` is an Integer

### Type Order

Julia types can be ordered. To conceptualize this, the relation of types is best thought of as a directed graph. Each node of the graph is a type, each edge is directed, where, if the edge goes from type A to type B, then `B <: A`. That is, B is a subtype of A, or equivalently `A >: B`, A is a supertype of B.

This relational nature is heavily used in multiple dispatch and type inference, for now, however, it gives us a way to put types in relation to each other. `jluna::Type` offers multiple functions for this:

```cpp
// (*this) <: other
bool is_subtype_of(const Type& other) const;

// (*this) >: other
bool is_supertype_of(const Type& other) const;

// (*this) <: other && (*this) >: other
bool is_same_as(const Type&) const;
```

Because we can assign an order to types in this way, `jluna::Type` also provides proper boolean comparison operators:

```cpp
// (*this) <: other
bool operator<(const Type& other) const;

// other >: (*this)
bool operator>(const Type& other) const;

// (*this) === other
bool operator==(const Type& other) const;

// !((*this) === other)
bool operator!=(const Type& other) const;
```

This ordering becomes relevant when talking about `TypeVar`s. TypeVars can be thought of as a sub-graph of the type-graph that has an upper and lower bound. An unrestricted types upper bound is `Any`, while its lower bound is `Union{}`. A declaration like `T where {T <: Integer}` restricts the upper bound to `Integer`, though any type that is "lower" along the sub-graph originating at `Integer` can still bind to `T`. This is important to keep in mind.

### Type Info

When gathering information about types, it is important to understand the difference between a types fields, its parameters, its properties and its methods. Consider the following type declaration:

```julia
# in Julia
mutable struct MyType{T <: Integer, U}
    _field1
    _field2::T
    _field3::U
    
    function MyType(a::T, b::U) where {T, U}
        return new{T, U}(undef, a, b)
    end
end
```

This type is a parametric type, it has two **parameters** called `T` and `U`. `T`s upper bound is `Integer` while `U` is unrestricted, its upper bound is `Any`.

`MyType` has 3 **fields**:
+ `_field1` which is unrestricted
+ `_field2` which is declared as type `T`, thus it's upper bound is also `Integer`
+ `_field3` which is declared as type `U`, however because `U` is unrestricted, `_field3` is too

The type has 1 **method**, `MyType(::T, ::U)`, which is its constructor.

This type furthermore has the following properties:

```julia
println(propertynames(MyType.body.body))
```
```
(:name, :super, :parameters, :types, :instance, :layout, :size, :hash, :flags)
```
These behave like fields but are not part of the declaration. We will learn more about what exactly each property means, and how to access them later on.

### Parameters

We can access the name and types of the parameters of a type using `jluna::Type::get_parameters`:

```cpp
State::safe_eval(R"(
    mutable struct MyType{T <: Integer, U}
        _field1
        _field2::T
        _field3::U

        function MyType(a::T, b::U) where {T, U}
            return new{T, U}(undef, a, b)
        end
    end
)");

Type my_type = Main["MyType"];
std::vector<std::pair<Symbol, Type>> parameters = my_type.get_parameters();

for (auto& pair : parameters)
    std::cout << pair.first.operator std::string() << " => " << pair.second.operator std::string() << std::endl;
```
```
T => Integer
U => Any
```
`get_parameters` returns a vector of pairs where:
+ `.first` is a symbol that is the name of the corresponding parameter 
+ `.second` is the parameter upper type bound. 
  
In case of `T`, the upper bound is `Base.Integer`, because we restricted it as such in the declaration. For `U`, there is no restriction, which is why its upper bound is the default: `Any`.

We can retrieve the number of parameters directly using `get_n_parameters()`. This saves allocating the vector of pairs.

#### Fields

We can access the fields of a type in a similar way, using `jluna::Type::get_fields`:

```cpp
State::safe_eval(R"(
    mutable struct MyType{T <: Integer, U}
        _field1
        _field2::T
        _field3::U

        function MyType(a::T, b::U) where {T, U}
            return new{T, U}(undef, a, b)
        end
    end
)");

Type my_type = Main["MyType"];
std::vector<std::pair<Symbol, Type>> fields = my_type.get_fields();

for (auto& pair : fields)
    std::cout << pair.first.operator std::string() << " => " << pair.second.operator std::string() << std::endl;
```
```
_field1 => Any
_field2 => Integer
_field3 => Any
```
We, again, get a vector of pairs where `.first` is the name, `.second` is the upper type bound of the corresponding field.

#### Methods

(this feature is not yet implemented, until then, use `Base.methods(::Type)`)

#### Properties

(this feature is not yet implemented, until then, use `Base.getproperty(::Type, ::Symbol)`)

### Type Classification

To classify a type means to evaluate a condition based on a types attributes, in order to get information about how similiar or different clusters of types are. `jluna` offers a number of convenient classifications, some of which are available as part of the Julia core library, some of which are not. This section will list all, along with their meaning:

+ `is_primitive`: was the type declared using the keyword `primitive`
    ```cpp
    Bool_t.is_primitive()    // true
    Module_t.is_primitive(); // false
    ```
+ `is_struct_type`: was the type declared using the keyword `struct`
    ```cpp
    Bool_t.is_struct_type()    // false
    Module_t.is_struct_type(); // true
    ```
+ `is_declared_mutable`: was the type declared using `mutable`
    ```cpp
    Bool_t.is_declared_mutable()    // false
    Module_t.is_declared_mutable(); // true
    ```
+ `is_isbits`: is the type a `isbits` type, meaning it is immutable and contains no references to other values that rae not also isbits or primitives
    ```cpp
    Bool_t.is_declared_mutable()    // true
    Module_t.is_declared_mutable(); // false
    ```
+ `is_singleton`: a type `T` is a singleton iff:
    - `T` is immutable and a structtype
    - for types `A`, `B` if `A <: B` and `B <: A` then `A === B`
    ```cpp
    State.eval("struct Singleton end");
    Type singleton_type = Main["Singleton"];
    singleton_type.is_singleton(); // true
    ```
+ `is_abstract_type`: was the type declared using the `abstract` keyword
    ```cpp
    Float32_t.is_abstract_type()          // false
    AbstractFloat_t.is_abstract_type();   // true
    ```
+ `is_abstract_ref_type`: is the type a reference whose value type is an abstract type.
    ```cpp
    Type(State::eval("return Ref{AbstractFloat}")).is_abstract_ref_type(); //true
    Type(State::eval("return Ref{AbstractFloat}(Float32(1))")).is_abstract_ref_type(); // false
    ```
+ `is_typename(T)`: is the `.name` property of the type equal to `Base.typename(T)`. Can be thought of as "Is the type a T"
    ```cpp
    // is the type an Array:
    Type(State::safe_eval("return Array")).is_typename("Array"); // true
    Type(State::safe_eval("return Array{Integer, 3}")).is_typename("Array"); // also true
    ```
There is a subtle difference between how `jluna` evaluates properties and how pure Julia does. Consider the following:

```julia
# in julia
function is_array_type(type::Type) 
    return getproperty(type, :name) == Base.typename(Array)
end

println(is_array_type(Base.Array))
```

Some may expect this to print `true`, however this is not the case. `Base.Array` is a parametric type, because of that, `typeof(Array)` is actually a `UnionAll` which does not have a property `:name`:

```julia
println(propertynames(Array))
```
```
(:var, :body)
```
To access the property `:name` we need to first *unroll* the type. This means we need to specialize the parameters using `TypeVars` until the type seizes to be a `UnionAll`:

```julia
type = Array
while (hasproperty(type, :body))
    type = type.body
end

println(type)
println(propertynames(type))
```
```
Array{T, N}
(:name, :super, :parameters, :types, :instance, :layout, :size, :hash, :flags)
```

Once fully unrolled, we have access to the properties necessary for introspect. `jluna` does this unrolling automatically, meaning all parametric types held by a `jluna::Type` proxy are fully specialized. We can fully specialize a type manually using the member function `.unroll()`.

---

## Expressions

(this feature is not yet implemented)

---

## Usertypes

So far, we were only able to box and unbox types that were already supported by `jluna`. While this list of types is broad, in more specialized applications it is sometimes necessary to define our own boxing/unboxing routines. Luckily, `jluna` offers an easy-to-use and safe interface for transferring arbitrary C++ types between states. This section will give guidance on how to use it.

### Usertype Interface

Consider the following C++ class:

```cpp
struct RGBA
{
    float _red;
    float _green;
    float _blue;
    float _alpha;
    
    RGBA(float r, float g, float b)
        : _red(r), _green(g), _blue(b), _alpha(1)
    {}
};
```

While it may be possible to manually translate this class into a Julia-side `NamedTuple`, this is rarely the best option. For more complex classes, this is often not possible at all. To make classes like this (un)boxable, we use `jluna::Usertype<T>`, the usertype interface.

#### Step 1: Making the Type compliant

For a type `T` to be manageable by `Usertype<T>`, it needs to be *default constructable*. `RGBA` currently has no default constructor, so we need to add it:

```cpp
struct RGBA
{
    float _red;
    float _green;
    float _blue;
    float _alpha;
    
    RGBA(float r, float g, float b)
        : _red(r), _green(g), _blue(b), _alpha(1)
    {}
    
    RGBA() 
        : _red(0), _green(0), _blue(0), _alpha(1
    {}
};
```

If the type `T` is not default constructable, a static assertion is triggered at compile time.

#### Step 2: Enabling the Interface

To make `jluna` aware that we want to use the usertype interface for `RGBA`, we need to **enable it** at compile time. To do this, we use the macro `set_usertype_enabled(T)` (executed in non-block scope).

```cpp
struct RGBA
{
    /* ... */
};

set_usertype_enabled(RGBA); // best called right after the declaration
```

This generates a call to a template meta function that sets up `Usertype<T>` for us. Among other things, it sets the Julia-side name of the type we will unbox `RGBA` into, to the same name as that of the C++-side type. Meaning, C++s `RGBA` will be boxed into a new Julia-side type called `Main.RGBA`. We can change whether we want it to actually be in `Main` scope later on.

#### Step 3: Adding Property Routines

A *property* of a struct type is what would be called a "field" in Julia or a "member" in C++. It is any named variable of any type (including functions) that is namespaced within a struct. We'll use all three terms, "field", "member", and "property" interchangeably here. 

To add a property for `RGBA`s `_red`, we use the following function at runtime:

```cpp
Usertype<RGBA>::add_property<float>(
    "_red",
    [](RGBA& in) -> float {
        return in._red;
    },
    [](RGBA& out float in) -> void {
        out._red;
    }
);
```

Let's talk through this call one-by-one. 

First, we have the **template parameter**, `float` in this case. This is the type of the field. We use C++ types here, however. after `RGBA` is boxed to Julia, C++s `float` becomes Julias `Float32`. We can check which C++ types gets converted to which Julia type using `to_julia_type<T>::type_name`.

This first argument is the **name of the field**. It is best practice, to have this be the same name as the field in C++, `_red` in this case, however, there is no mechanism to enforce this.

The second argument is the **boxing routine**. This function is called during `box<RGBA>(instance)`. It has the signature `(T&) -> Property_t`, which for `RGBA` and this specific field becomes `(RGBA&) -> float`. The argument of the boxing routine is the instance of the type that is about to be boxed:

```cpp
auto instance = RGBA();
Any* boxed = box<RGBA>(instance);

// calls [](RGBA& in) -> float {return in._red;} with in = instance
// then assigns boxed._red with result
```

When `box` is called, the result of the boxing routine lambda is assigned to the field of the specified name.

The third argument is the **unboxing routine**. This lambda has the signature `(T&, Property_t) -> void`, which in this case becomes `(RGBA&, float) -> void`. The unboxing routine is called during unboxing, its first argument is the cpp-side instance that is the result of the `unbox` call, the second argument is the value of the field of the corresponding name of the Julia-side instance.

```cpp
auto instance = RGBA();
Any* boxed = box<RGBA>(instance);
RGBA unboxed = unbox<RGBA>(boxed);

// calls [](RGBA& out, float in) with out = unboxed, in = unboxed._red
```

Specifying the unboxing routine is optional. If left unspecified, no operation is performed during unboxing. The boxing routine is not optional, if we want it to always return a constant value, we can simple create a lambda for this like so:

```cpp
Usertype<T>::add_field<float>("_constant_field", [](T& _) -> float {return 1;});
```

Now that we know how to add fields, let's fully implement the usertype interface for `RGBA` (where previous code is reprinted here for clarity):

```cpp
// in namespace scope
struct RGBA
{
    float _red;
    float _green;
    float _blue;
    float _alpha;
    
    RGBA(float r, float g, float b)
        : _red(r), _green(g), _blue(b), _alpha(1)
    {}
    
    RGBA() 
        : _red(0), _green(0), _blue(0), _alpha(1
    {}
};
set_usertype_enabled(RGBA);

// ###

// in function scope, i.e. inside main
Usertype<RGBA>::add_property<float>(
    "_red",
    [](RGBA& in) -> float {return in._red;},
    [](RGBA& out, float in) -> void {out._red;}
);
Usertype<RGBA>::add_property<float>(
    "_green",
    [](RGBA& in) -> float {return in._green;},
    [](RGBA& out, float in) -> void {out._green;}
);
Usertype<RGBA>::add_property<float>(
    "_blue",
    [](RGBA& in) -> float {return in._blue;},
    [](RGBA& out, float in) -> void {out._blue;}
);
Usertype<RGBA>::add_property<float>(
    "_alpha",
    [](RGBA& in) -> float {return in._alpha;},
    [](RGBA& out, float in) -> void {out._alpha;}
);
```
To illustrate that properties do not have to directly correspond with members of the C++ class, we'll add another Julia-side field that represents the `value` component from the HSV color system (sometimes also called "lightness"). It is defined as the maximum of red, green and blue, given a color in RGBA:

```cpp
Usertype<RGBA>::add_property<float>(
    "_value",
    [](RGBA& in) -> float {
        float max = 0;
        for (auto v : {in._red, in._green, in._blue})
            max = std::max(v, max);
        return max;
    }
);
```

We leave the unboxing routine for `_value` unspecified, because there is no field called `_value` to assign to for C++-side instances.

#### Step 4: Implementing the Type

Having added all properties to the usertype interface, we make the Julia state aware of the interface by calling:

```cpp
// in main
Usertype<RGBA>::implement();
```

This creates a new type Julia-side type that has the architecture we just gave it. For end-users, this happens behind the scene, however, internally, the following expression is assembled and evaluated:

```julia
mutable struct RGBA
    _red::Float32
    _green::Float32
    _blue::Float32
    _alpha::Float32
    _value::Float32
    RGBA() = new(0.0f0, 0.0f0, 0.0f0, 1.0f0, 0.0f0)
end
```

We see that `jluna` assembled a mutable struct type, whose field names and types are as specified. Even the order in which we called `add_field` for specific names is preserved. This becomes important for the default constructor (a constructor that takes no arguments). The default values for each of the types fields are those of an unmodified, default initialized instance of `T` (`RGBA()` in our case). This is why the type needs to be default constructable.

Having evaluated the above expression, we have fully implemented the usertype interface for `RGBA`. From this point onwards, the following works:

```cpp
auto instance = RGBA();
instance._red = 1;
instance._blue = 1;

// now boxable
State::new_named_undef("julia_side_instance") = box<RGBA>(instance);   
jluna::safe_eval(R"(
    println(julia_side_instance)
    julia_side_instance._blue = 0.5;
)");

// and unboxable
auto cpp_side_instance = unbox<RGBA>(jluna::safe_eval("return julia_side_instance"));
std::cout << cpp_side_instance._blue << std::endl;
```
```
RGBA(1.0f0, 0.0f0, 1.0f0, 1.0f0, 1.0f0)
0.5
```

Because `box` and `unbox` are now defined and there is a corresponding Julia-side type for `RGBA`, all of `jluna`s other functionality is now also works with `RGBA`. This includes assigning proxies, casting proxies to `RGBA`, calling Julia-side functions with C++-side arguments, even using `RGBA` as value types for a `jluna` arrays, etc.

### Manual Implementation

`jluna::Usertype<T>` works great if we already have a C++-side type that we want to move to Julia. What, however, if we already have a Julia-side type and we want to map a the C++-side type onto it? In cases like this, we will have to resort to manually implementing `box`/`unbox` without the help of `Usertype<T>`. 

In this section, we'll work through a somewhat complex example, which provides a template for users to apply to their own specific classes. 

#### Example: Tadpoles & Frogs

Let's first introduce our problem: Consider the following classes:

```cpp
class Frog
{
    public:
        // tadpole, a baby frog
        struct Tadpole
        {
            // name, can be changed
            std::string _name
            
            // ctor, tadpoles are born without a name
            Tadpole() 
                : _name("")
            {}
            
            // if a tadpole has a name, it can evolve into a frog
            Frog evolve() const
            {
                assert(_name != "");
                return Frog(_name);
            }
        };
        
    public:
        // a frog can spawn a number of tadpoles
        std::vector<Frog::Tadpole> spawn(size_t n) const
        {
            std::vector<Frog::Tadpole> out;
            for (size_t i = 0; i < n; ++i)
                out.push_back(Frog::Tadpole());

            return out;
        }

        // get name
        std::string get_name()
        {
            return _name;
        }
    
    private:
        // private ctor, can only be called by Frog::Tadpole::evolve
        Frog(std::string name)
            : _name(name)
        {}
        
        // name, cannot be changed because there is no setter
        std::string _name;
};
```

These classes are deceptively simple, however they provide some unique challenges. We cannot instance a `Frog` (henceforth "frog"), the only way to construct a frog is by first creating a `Frog::Tadpole` (henceforth "tadpole"), naming it, then calling `evolve`. Because tadpole is an internal class of `Frog`, it has access to the private constructor. Furthermore, while we can *access* a frogs name, we cannot change it. The name of a tadpole can be both access and changed. 

Let's say we want to translate this functionality to Julia without using `Usertype<T>`. The Julia-versions of the above could be:

```julia
mutable struct Tadpole  # mutable because _name needs to be changable
    
    # name, can be changed
    _name::String
    
    # "member" function: (::Tadole) -> Frog
    evolve::Function
    
    # ctor without a name
    Tadpole() = new(
        "",
        (this::Tadpole) -> Frog(this._name)
    )
end

struct Frog # immutable because _name only has getter

    # name, cannot be changed
    _name::String
    
    # member function: (::Integer) -> Vector{Tadpole}
    spawn::Function
    
    # ctor from the tadpole
    Frog(as_tadpole::Tadpole) = new(
        as_tadpole._name, 
        (n::Integer) -> [Tadpole() for _ in 1:n]
    )
end
```

Where we defined the frogs ctor as only being constructable from a tadpole. This emulates the behavior the classes exhibit C++ side. Julia usually discourages member functions in the C++ sense, but for the sake of this example, we defined them using the following equivalence:

```julia
# in cpp:
auto instance = InstanceType();
instance.member_function(arguments);

# in Julia
instance = InstanceType()
instance.member_function(instance, arguments)
```

By making the instance an argument, the Julia function emulate the C++ behavior of being able to access all members of the struct they are part of.

### Function Definition

We now want to implement boxing and unboxing for both frogs and tadpoles. When defining these functions for a custom type, they need to adhere to the following signatures exactly:

```cpp
template<Is<U> T>
Any* box(T);

template<Is<U> T>
T unbox(Any*);
```

Where `Is<U>` is a concept that enforces `T` to be equal to `U`. We need to use concepts and template functions here, so the syntax `box<U>(/*...*/)` and `unbox<U>(/*...*/)` are valid, which is required by `jluna`s functions. The `box` and `unbox` signatures for `Frog` and `Frog::Tadpole` then look like this:

```cpp
template<Is<Frog::Tadpole> T>
Any* box(T in)
{}

template<Is<Frog> T>
Any* box(T in)
{}

template<Is<Frog::Tadpole> T>
T unbox(Any* in)
{}

template<Is<Frog> T>
T unbox(Any* in)
{}
```

Because we are handling raw C-pointers and not proxies, we need to manually protect ourself from the garbage collector (GC). This is done best, by using `jluna::GCSentinel`, which is a class that disables the GC while it is in scope:

```cpp
template<Is<Frog::Tadpole> T>
Any* box(T in)
{
    auto sentinel = GCSentinel();
}

template<Is<Frog> T>
Any* box(T in)
{
    auto sentinel = GCSentinel();
}
```

To instance the Julia-side versions of the classes, we need access to their Julia-side constructors. We can do so using `jl_find_function`, using the result like a function to create instances:

```cpp
template<Is<Frog::Tadpole> T>
Any* box(T in)
{
    auto sentinel = GCSentinel();
    static auto* tadpole_ctor = jl_find_function("Main", "Tadpole");
    
    auto* out = jluna::safe_call(tadpole_ctor);
}

template<Is<Frog> T>
Any* box(T in)
{
    auto sentinel = GCSentinel();
    static auto* frog_ctor = jl_find_function("Main", "Frog");
    
    auto* out = jluna::safe_call(frog_tor); // WILL FAIL
}
```

We run into a problem. `Frog` does not have a no-argument constructor. The only way to construct a frog, is from a tadpole. We cannot ask for a tadpole as an argument to the box function, though, because we have to strictly adhere to the `(T) -> Any*` signature. The only way to solve this conundrum, is to create a new Julia-side function that constructs a frog for us, in a round-about way:

```julia 
function generate_frog(name::String) ::Frog
    tadpole = Tadpole()
    tadpole._name = name
    return Frog(tadpole)
end
```
The example in this section was specifically setup this way, to illustrate how, when the Julia-type is pre-defined and cannot be extended, we will often do extra work to make C++ and Julia play nice together. We cannot extend the Julia-side `Frog` class, which is why we need to create this wrapper function to construct a frog instead. 

Accessing and calling the function inside `box<Is<Frog>>`:

```cpp
template<Is<Frog::Tadpole> T>
Any* box(T in)
{
    auto sentinel = GCSentinel();
    static auto* tadpole_ctor = jl_find_function("Main", "Tadpole");
    
    auto* out = jluna::safe_call(tadpole_ctor);
}

template<Is<Frog> T>
Any* box(T in)
{
    auto sentinel = GCSentinel();
    static auto* frog_ctor = jl_find_function("Main", "generate_frog");
    
    auto* out = jluna::safe_call(frog_tor, box<std::string>(in._name));
}
```

Where we also had to manually box the string, because we are not dealing with proxies, which do this automatically, anymore. 

Lastly, we need to update the newly created Julia-side instance with the actual values of the C++-side instance. To do this, we use `Base.setfield!`. Note, however, that the Julia-side `Frog` is immutable. This is why we created the generator function, the only way to make Julia-side frogs have the same name as C++-side frogs, is to assign their name to a tadpole, then evolving it. Because of this, the frog instanced in `box<Frog>` does not need to be modified further:

```cpp
template<Is<Frog::Tadpole> T>
Any* box(T in)
{
    auto sentinel = GCSentinel();
    static auto* tadpole_ctor = jl_find_function("Main", "Tadpole");
    
    auto* out = jluna::safe_call(tadpole_ctor);
    
    static auto* setfield = jl_find_function("Base", "setfield!");
    static auto field_symbol = Symbol("_name");
    jluna::safe_call(setfield, out, (Any*) field_symbol, box<std::string>(in._name));
    return out;
}

template<Is<Frog> T>
Any* box(T in)
{
    auto sentinel = GCSentinel();
    static auto* frog_ctor = jl_find_function("Main", "generate_frog");
    
    auto* out = jluna::safe_call(frog_tor, box<std::string>(in._name));
    return out;
}
```

Where we used `jluna::Symbol` create a symbol `:_name`, Julia-side. We used the `static` keyword, because the value of this symbol will never change, thus only initializing it once saves runtime on further calls to `box`.

Now that we have box fully written, we can turn our attention to unbox. Unboxing is much simpler, all we need to do, is to access the properties of the Julia-side instances using `Base.getfield`. Because we are now back C++-side, we use `Tadpole::evolve` to create a frog:

```cpp
template<Is<Frog::Tadpole> T>
T unbox(Any* in)
{
    auto sentinel = GCSentinel();
    static auto* getfield = jl_find_function("Base", "getfield");
    static auto field_symbol = Symbol("_name");
    
    Any* julia_side_name = jluna::safe_call(getfield, in, (Any*) field_symbol);
    
    auto out = Tadpole();
    out._name = unbox<std::string>(julia_side_name);
    return out;
}

template<Is<Frog::Tadpole> T>
T unbox(Any* in)
{
    auto sentinel = GCSentinel();
    static auto* getfield = jl_find_function("Base", "getfield");
    static auto field_symbol = Symbol("_name");
    
    Any* julia_side_name = jluna::safe_call(getfield, in, (Any*) field_symbol);
    
    auto tadpole = Tadpole();
    tadpole._name = unbox<std::string>(julia_side_name);
    
    return tadpole.evolve();
}
```

Where we allocate the symbol `:_name`, as before.

Because both box and unbox are now implemented for `Frog` and `Frog::Tadpole`, both are now compliant, and can be freely transferred between states to be used by all of `jluna`s functions:

```cpp
auto cpp_tadpole = Frog::Tadpole();
cpp_tadpole._name = "Ted";

State::new_named_undef("jl_tadpole") = box<Frog::Tadpole>(tadpole);
State::safe_eval("jl_frog = Tadpole.evolve(jl_tadpole)");

auto cpp_frog = Main["jl_frog"];
std::cout << cpp_frog.get_name();
```
```
Tadpole("Ted", var"#1#2"())
Frog("Ted", var"#3#5"())
Ted
```

The complete code for this example is available [here](./frog_tadpole_example.cpp).

---

## Performance

This section will give some tips on how to achieve the best performance using `jluna`. As of release 0.7, `jluna` went through extensive optimization, minimizing overhead as much as possible. Still, when compared to pure Julia, `jluna` will always be slower. Comparing `jluna` to the C-library, though, is a much fairer comparison and in that aspect, it can do quite well. It is still important to be aware of how to avoid overhead, and when it may be worth it to resort to only using the C-library.

### Cheat Sheet

Provided here is a grading of most of `jluna`s features in terms of runtime performance, where `A+` means there is literally 0-overhead compared to operating purely in Julia, `F` means "do not use this in performance critical code":

```cpp
// function or feature              // grade

// ### executing Julia code ###
jluna::call                         A+
jluna::safe_call                    A
Proxy::call                         A+
Proxy::safe_call                    A
Proxy::operator()                   A
jluna::eval                         A-
jluna::safe_eval                    A-
State::eval                         B
State::safe_eval                    B
Module::eval                        B
Module::safe_eval                   B
State::eval_file                    C+
State::safe_eval_file               C
GeneratorExpression                 F

// ### accessing Julia-side values ###
Array<T, N>::operator[](size_t)     A+
Proxy::operator Any*()              A+
State::safe_return                  A
Proxy::operator[](size_t)           A
Proxy::operator[](std::string)      A-
State::new_named_*                  A
jl_get_function                     A+
jl_find_function                    F

// ### box<T> / unbox<T> for T = ###
primitives (int, float, etc.)       A+
const char*                         A+
Proxy                               A
Array                               A
std::vector                         A
std::string                         A
Pair                                B
Tuple                               B
Set                                 B-
map / unordered map                 C
lambdas                             F   // but calling is A
Usertype<T>                         ?*
        
* Usertype<T> boxing performance is entirely dependend on user-supplied getter/setter
        
// ### other ###
State::initialize                   F
GCSentinel                          A+
State::set_gc_enabled               A+
State::collect_garbage              A+
jluna::Type Introspection           A
jluna::register_function            F   // but calling is A-
Usertype<T>::add_field              D
Usertype<T>::enable                 F
Usertype<T>::implement              F
```
---

### Tips

The following suggestions are good to keep in mind when writing performance-critical code:

### Avoid String Parsing

It's easy to fall into the pattern of treating `jluna` like the REPL: a way to control Julia through the use of commands *supplied as strings*. This is rarely the most optimal way to trigger Julia-side actions, however. Notably, when calling Julia functions, often, the following pattern of calling functions directly through C achieves much better performance:

```cpp
// 1) exactly as fast as pure julia
static jl_function_t* call_function = jl_find_function("Main.MyModule", "call_function"); 
jluna::call(call_function);

// 2) slightly slower
auto call_function = Main["MyModule"]["call_function"];
call_function();

// 3) slowest
jluna::eval("Main.MyModule.call_function()");
```

Where `static` was used, so the `call_function` pointer is only assigned exactly once over the course of runtime.

### Stay Julia-Side

All performance critical code should be inside a Julia file. All C++ should do, is to manage data and dispatch the Julia-side functions. For example, let's say we are working on very big matrices. Any matrix operation should happen inside a julia function, the actual data of the matrix should stay Julia-side as much as possible. 
 Relating to this, it is important to realize, when a C++ object does not actually manage any data C++-side. For example, `jluna::Proxy` is just a stand-in for Julia-side object. The C++-part of it basically only holds a pointer to the data. Thus, using Julia functions on `jluna::Proxy`s incurs very little overhead, compared to doing both only in Julia, because all we are doing is using Julia-side functions on Julia-side objects.

### Minimize (Un)Boxing

By far the easiest way to completely tank a programs' performance is to unnecessarily box/unbox values constantly. If our program requires operation `A`, `B`, `C` Julia-side and `X`, `Y`, `Z` C++-side, it's very important to execute everything in a way that minimizes the number of box/unbox calls. So:
`ABC <unbox> XYZ <box>` rather than `A <unbox> X <box> B <unbox> Y <box>`, etc.. This may seem obvious when abstracted like this, but in a complex application, it's easy to forget. Knowing what calls cause box/unboxing is essential to avoiding pitfalls, because of this it sometimes encouraged to not shy away from handling pure `Any*`, just make sure to also manually control the garbage collector.

Lastly, though it is cumbersome, manually implementing box/unbox calls instead of going through `Usertype<T>` often gives more freedom to optimize. Advanced users are therefore encouraged to consider this method more frequently, at the cost of convenience.

### Minimize Proxy Construction

The main overhead incurred by `jluna` is that of safety. Anytime a proxy is constructed, its value and name have to be added to an internal state that protects them from the garbage collector. This takes some amount of time; internally, the value is wrapped and a reference to it and its name is stored in a dictionary. Neither of these is that expensive, but when a function uses hundreds of proxies over its runtime, this can add up quickly. Consider the following:

```cpp
auto a = Main["Module1"]["Module2"]["vector_var"][1]["field"];
``` 

This statement constructs 5 proxies, it is exactly equivalent to:

```cpp
Proxy a;
{
    auto _0 = Main;
    auto _1 = _0["Module1"];
    auto _2 = _1["Module2"];
    auto _3 = _2["vector_var"];
    auto _4 = _3[1];
    a = _4["field"]
}
```

Where each declaration triggers a proxy to be created. At the end of the block, no proxies are deallocated, because the value of `a` depends on its host, `vector_var`, which needs to stay in scope for `a` to be able to be dereferenced.

If we instead access the value like so:

```cpp
size_t a = State::safe_return<size_t>("Main.Module1.Module2.vector_var[1].field")
```

We do not create any proxy at all, increasing performance by up to 6 times. Of course, doing this, we loose the convenience of being assignable, castable, and all other functionalities a named `jluna::Proxy` offers. Still, in performance-critical code, unnamed proxies are almost always faster than named proxies and should be preferred. A good middle-ground is the following style:

```cpp
auto a_proxy = jluna::Proxy(jluna::safe_eval("Main.Module1.Module2.vector_var[1].field"));
size_t a_value = a_proxy;
```
This calls the proxy constructor using a pure `Any*`, returned through `safe_eval`, thereby reducing the number of proxies constructed from 6 named to only 1 unnamed.

### Use the C-Library

When performance needs to be optimal, not "good" or "excellent, but mathematically optimal, it is sometimes necessary to resort to the C-library. Values are hard to manage, the syntax is very clunky and the garbage collector will probably steal many of our values from under our nose and segfault the program, but, that is the trade-off of performance vs. convenience. <br>
Luckily the C-library and `jluna` are freely mixable, though we may need to `.update` any proxies whos Julia-side value was modified outside `jluna`.

### In Summary

`jluna`s safety features incur an unavoidable overhead. Great care has been taken to minimize this overhead as much as possible, but it is still non-zero in many cases. Knowing this, `jluna` is still perfectly fine for most applications. If a part of a library is truly performance critical, however, it may be necessary to avoid using `jluna` as much as possible in order for Julia to do, what it's best at: being very fast. When mixing C++ and Julia, however, `jluna` does equally well at bridging that gap, and in many ways it does so better than the C-API.

---
