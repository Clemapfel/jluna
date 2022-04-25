# jluna: Manual & Tutorial

This manual will cover most of jlunas features, how to use them, and what to use them for. It is structured in a way that ramps up in complexity. Starting with how to accomplish basic things like calling functions and basic interaction between the C++ and Julia state, we will then be moving on to more specialized applications, like the array and module interface, how to deal with usertypes, finally ending with multi-threading and performance optimization.

It is recommended to read this manual in order, an interactive table of contents is provided below for easy access.

This manual assumes that all users are familiar with the basics of Julia. If this is not the case, it may be necessary to first work through the excellent [official manual](https://docs.julialang.org/en/v1/) to get an overview of Julia's syntax and structure.

jluna makes extensive use of newer C++20 features and high-level techniques such as [SFINAE](https://en.cppreference.com/w/cpp/language/sfinae) and template meta functions. This may provide a barrier of entry to users less familiar with the intricacies of the language. To address this, hints in the form of quotation blocks are provided where necessary:

> **Hint**: Quotation blocks like these are reserved for additional information or explanations of C++ and/or Julia features.

---

## Table of Contents

0. [Installation](#installation)<br>
1. [Initializing the Julia State](#initializing-the-julia-state)<br>
2. [Modifying the Julia State](#executing-julia-code)<br>
  2.1 [Executing a Single Line of Code](#executing-a-single-line-of-code)<br>
   2.2 [Executing a File](#executing-a-file)<br>
   2.3 [Executing Multiple Lines of Code](#executing-multiple-lines-of-code)<br>
   2.4 [Getting the Result of Julia Code](#getting-the-result-of-julia-code)<br>
   2.5 [Executing Code in Module Scope](#executing-julia-code-in-module-scope)<br>
   2.6 [Calling Julia Functions](#calling-julia-functions)<br>
   2.7 [Accessing Named Variables in Modules](#accessing-named-variables-in-a-module)<br>
   2.8 [Accessing Julia Struct Fields](#accessing-julia-struct-fields)<br>
   2.9 [Mutating Julia-side Variables](#mutating-julia-side-variables)<br>
   2.10 [Mutating Array Elements](#mutating-variables-using-operatorsize_t)<br>
3. [Proxies](#proxies)<br>
  3.1 [Memory Sharing](#shared--separate-memory)<br>
   3.2 [Constructing a Proxy](#constructing-a-proxy)<br>
   3.3 [Updating a Proxy](#updating-a-proxy)<br>
   3.4 [Boxing](#boxing)<br>
   3.5 [Unboxing](#unboxing)<br>
   3.6 [List of (Un)Boxables](#unboxable-types)<br>
   3.7 [Unnamed Proxies](#named--unnamed-proxies)<br>
   3.8 [Named Proxies](#named-proxy)<br>
   3.9 [Accessing a Proxies Name](#additional-member-functions)<br>
   3.10 [Fixing Detached Proxies](#detached-proxies)<br>
   3.11 [Named & Unnamed: Implications](#implications)<br>
   3.12 [Proxy Inheritance](#proxy-inheritance)<br>
4. [Arrays](#specialized-proxies-arrays)<br>
  4.1 [Accessing Array Indices](#accessing-array-indices)<br>
   4.2 [Linear Indexing](#linear-indexing)<br>
   4.3 [List Indexing](#list-indexing)<br>
   4.4 [0-base vs. 1-base](#0-base-vs-1-base)<br>
    4.5 [Iterating](#iterating)<br>
   4.6 [Array Size](#accessing-the-size-of-an-array)<br>
   4.7 [Vector](#jlunavector)<br>
   4.8 [Generator Expressions](#generator-expressions)<br>
5. [Symbols](#specialized-proxies-symbols)<br>
  5.1 [Symbol Hashing](#symbol-hashing)<br>
6. [Modules](#specialized-proxies-modules)<br>
  6.1 [Assign in Module](#assign-in-module)<br>
   6.2 [Creating a new Variable](#creating-a-new-variable)<br>
   6.3 [Import, Using, Include](#import-using-include)<br>
7. [Calling C++ Functions from Julia](#calling-c-functions-from-julia)<br>
  7.1 [Creating a Function Object](#creating-a-function-object)<br>
   7.2 [Allowed Signatures](#allowed-signatures)<br>
   7.3 [Taking Any Number of Arguments](#taking-any-number-of-arguments)<br>
   7.4 [Using Non-Julia Objects](#using-non-julia-objects)<br>
8. [Types](#specialized-proxies-types)<br>
  8.1 [Constructing a Type](#constructing-a-type)<br>
   8.2 [Base Types](#base-types)<br>
   8.3 [Type Order](#type-order)<br>
   8.4 [Type Info](#type-info)<br>
   8.4.1 [Parameters](#parameters)<br>
   8.4.2 [Fields](#fields)<br>
   8.5 [Type Classification](#type-classification)<br>
   8.6 [Unrolling Types](#unrolling-types)<br>
9. [Usertypes](#usertypes)
  9.1 [The Usertype Interface](#usertype-interface)<br>
  9.2 [Step 1: Making the Type Compliant](#step-1-making-the-type-compliant)<br>
   9.3 [Step 2: Enabling the Interface](#step-2-enabling-the-interface)<br>
   9.4 [Step 3: Adding Property Routines](#step-3-adding-property-routines)<br>
   9.5 [Step 4: Implementing the Type](#step-4-implementing-the-type)<br>
   9.6 [Step 5: Usage](#step-5-usage)<br>
   9.7 [Code Examble](./rgba_example.cpp)<br>
   9.8 [Additional Member Functions](#additional-member-functions)<br>
10. [Multi-Threading](#multi-threading)<br>
  10.1 [Initializing the Julia Threadpool](#initializing-the-julia-threadpool)<br>
    10.2 [Creating a Task](#creating-a-task)<br>
    10.3 [Running a Task](#running-a-task)<br>
    10.5 [Managing a Tasks Lifetime](#managing-a-tasks-lifetime)<br>
    10.6 [Accessing a Tasks State](#accessing-a-tasks-state)<br>
    10.7 [Accessing a Tasks Result](#accessing-a-tasks-result)<br>
    10.8 [Data Race Freedom](#data-race-freedom)<br>
    10.9 [Thread-Safety](#thread-safety)<br>
    10.9.1 [In Julia](#thread-safety-in-julia)<br>
    10.9.2 [In C++](#thread-safety-in-c)<br>
    10.10 [Closing Notes](#multi-threading-closing-notes)<br>
11. [The `unsafe` Library](#the-unsafe-library)<br>
  11.1 [Unsafe Types](#unsafe-types)<br>
  11.2 [Acquiring Raw Pointers](#acquiring-raw-pointers)<br>
  11.3 [Calling Julia Functions](#calling-julia-functions)<br>
  11.4 [Executing Strings as Code](#executing-strings-as-code)<br>
    11.5 [Boxing / Unboxing](#boxing--unboxing)<br>
    11.6 [Protecting Values from the Garbage Collector](#protecting-values-from-the-garbage-collector)<br>
    11.7 [Disabling the GC](#disabling-the-gc)<br>
    11.8 [Accessing & Mutating a Variable](#accessing--mutating-a-variable)<br>
    11.9 [Array Interface](#unsafe-array-interface)<br>
    11.9.1 [Accessing an Array Element](#accessing-an-array-element)<br>
    11.9.2 [Allocating a New Array](#allocating-a-new-array)<br>
    11.9.3 [Resizing an Array](#resizing-an-array)<br>
    11.9.4 [Replacing an Arrays Data](#replacing-an-arrays-data)<br>
    11.10 [Shared Memory](#shared-memory)<br>
12. [Performance Optimization](#performance-optimization)<br>
12.1 [Benchmarks: The Setup](#the-setup)<br>
12.2 [Benchmarks: Results](#results)<br>
12.2.1 TODO
   

---

### Installation

This manual does not go over how download and build jluna. Instead, a step-by-step tutorial on how to install and create a C++ application using jluna from scratch is available [here](./installation.md).

---

### Initializing the Julia State

Before any Julia or jluna functionality can be accessed, we need to initialize the Julia state using `jluna::initialize()`. This sets up the Julia environment and many global jluna variables. 

A basic `main.cpp` will have the following structure:

```cpp
// main.cpp
#include <jluna.hpp>

using namespace jluna;

int main()
{
    initialize();
    
    // our application here
    
    return 0;
}
```
> **C++ Hint**: `using namespace jluna` makes it, such that, in the scope the statement was declared, we do not have to prefix jluna functions or objects with their namespace `jluna::`. This is why we can write `initialize()`, instead of `jluna::initialize()`.

Where `#include <jluna.hpp>` automatically includes all headers intended for end-users.

Note that the C-APIs `jl_atexit_hook` should never be called. At the end of runtime, jluna safely closes the Julia state automatically.

---

### Executing Julia Code

#### Executing a Single Line of Code

Similarly to using the REPL, we can execute an arbitrary string as code using `safe_eval`:

```cpp
Main.safe_eval("println(\"hello jluna\")");
```
```
hello jluna
```

Where `Main` is a pre-defined jluna object representing the Julia-side `Main` module. 

> **Julia Hint**: `Main` is the default module that serves as the global scope of the Julia state. No module contains `Main`.

This function is called **safe**_eval, because it forwards any exceptions that may have occurred during parsing or execution of the string to the C++ state as a `jluna::JuliaException`:

```cpp
Main.safe_eval("sqrt(-1)");
```
```
terminate called after throwing an instance of 'jluna::JuliaException'
  what():  [JULIA][EXCEPTION] DomainError with -1.0:
sqrt will only return a complex result if called with a complex argument. Try sqrt(Complex(x)).
Stacktrace:
 [1] throw_complex_domainerror(f::Symbol, x::Float64)
   @ Base.Math ./math.jl:33
 [2] sqrt
   @ ./math.jl:567 [inlined]
 [3] sqrt(x::Int64)
   @ Base.Math ./math.jl:1221
 [4] top-level scope
   @ none:1
 [5] eval
   @ ./boot.jl:373 [inlined]
 [6] eval(m::Module, x::Expr)
   @ Base ./Base.jl:68
 [7] safe_call(::Function, ::Module, ::Vararg{Any})
   @ Main.jluna ./none:18

signal (6): Aborted
in expression starting at none:0
gsignal at /lib/x86_64-linux-gnu/libc.so.6 (unknown line)
abort at /lib/x86_64-linux-gnu/libc.so.6 (unknown line)
unknown function (ip: 0x7f39c3dbaa30)
unknown function (ip: 0x7f39c3dc65db)
_ZSt9terminatev at /lib/x86_64-linux-gnu/libstdc++.so.6 (unknown line)
__cxa_throw at /lib/x86_64-linux-gnu/libstdc++.so.6 (unknown line)
safe_call<_jl_module_t*, _jl_value_t*> at /home/clem/Workspace/jluna/.src/safe_utilities.inl:41
safe_eval at /home/clem/Workspace/jluna/.src/safe_utilties.cpp:103
main at /home/clem/Workspace/jluna/.benchmark/main.cpp:21
__libc_start_main at /lib/x86_64-linux-gnu/libc.so.6 (unknown line)
_start at /home/clem/Workspace/jluna/cmake-build-debug/jluna_benchmark (unknown line)
Allocations: 1612085 (Pool: 1611171; Big: 914); GC: 2

Process finished with exit code 134 (interrupted by signal 6: SIGABRT)
```

Along with the exception, we also get the full Julia- and C++-side stacktrace. 

> **C++ Hint**: In C++, an exception can be caught using a `try`-`catch` block. An error, on the other hand, cannot be caught, it will always terminate the application.

#### Executing a File

We can execute an entire Julia file using `safe_eval_file`. This function calls the Julia-side function `Main.include`, using the path we provided:

```cpp
Main.safe_eval_file("path/to/julia_file.jl");
```

#### Executing Multiple Lines of Code

If we want to execute more than just a single line, or, if we want any special characters to be escaped automatically, C++s **raw string literal** conveniently accomplishes both:

```cpp
Main.safe_eval_file(R"(
    println("first line");
    println("second line");
)");
```
```
first line
second line
```
> **C++ Hint**: Any characters in the text in-between the `R"(` `)"` are automatically escaped (if necessary). Without `R"()"`, we would have to write the above as `"println(\"first line\");\nprintln(\"second line\")";`


### Getting the Result of Julia Code

Often, we want to access whatever result a piece of code returns. To do this using jluna, we capture the C++-side return value of `safe_eval` (or `safe_eval_file`):

```cpp
Int64 result = Main.safe_eval("return 1234");
std::cout << result << std::endl;
```
```
1234
```
> **C++ Hint**: `std::cout << x << std::endl` is C++s version to Julia's `println(x)`.

Where `Int64` is a type alias of `long int`. Most of the C++ primitive types have been aliased in jluna, such that their name corresponds to the Julia-side name. `Float64` instead of `double`, `UInt32` instead of `unsigned int`, etc.

While the general case will be explained later, for now, be aware that **we have to manually specify the type** of the C++-side variable that captures the result, rather than using `auto`. 

> **C++ Hint**: `auto` is a keyword that asks the compiler to automatically deduce the type of a variable at compile time.

If the result of a Julia-side expression is of a different type than the declared C++-side variable, jluna will try to convert the result of the Julia-side expression, such that it matches the C++-side type:

```cpp
UInt64 result = Main.safe_eval("return Char(14)");
std::cout << result << std::endl;
```
```
14
```

In general, if the Julia-side value can be `Base.convert`ed to the declared C++-side type, jluna will do so implicitly. 

### Executing Julia Code in Module Scope

So far, we have used `Main.safe_eval`, which evaluates its argument in `Main` scope. Sometimes, this may not be desired. Consider the following example:

```cpp
// declare module
Main.safe_eval(R"(
    module MyModule
        var = 1234
    end
)");

// assign `var`
Main.safe_eval("var = 9999;");

// print `MyModule.var`s value
Main.safe_eval("println(MyModule.var)");
```
```
1234
```

What happened here? In the statement `var = 9999`, instead of assigning the variable `MyModule.var`, we actually created a new variable `Main.var` and assigned it, instead. This is because the line `var = 9999` was executed **in Main scope**, that is, the function `Main.eval` was used Julia-side to evaluate the expression. 

> **Julia Hint**: Every module has their own `eval` function, which evaluates an expression in that modules scope.

To avoid this, we can instead do:

```cpp
// access `MyModule` as jluna::Module
Module my_module = Main.safe_eval("return MyModule");

// use its safe_eval instead of Mains
my_module.safe_eval("var = 777");

// print `MyModule.var`s value
Main.safe_eval("println(MyModule.var)");
```
```
777
```

Where we have explicitly declared the C++-side variable `my_module` to be of type `jluna::Module`, because the member function `.safe_eval` is only available to variables of that type.

### Calling Julia Functions

While executing code using `safe_eval` is convenient, it has a significant performance impact. This is, because Julia needs to first `Meta.parse` the string we hand it, then evaluate the resulting expression.


> **Julia Hint**: `Meta.parse` takes a string and treats it as a line of code. It returns an object of type `Expr`, which can then be `eval`ed and compiled. To learn more about expressions, see the [official manual page on metaprogramming](https://docs.julialang.org/en/v1/manual/metaprogramming/).

Instead, users are encouraged to execute any given piece of Julia code by using a **function**. We are able to access a function (once) by using `save_eval("return function_name")`, then use that function over and over, at no performance cost compared to using only Julia:

```cpp
/// declare function
Main.safe_eval("f(x) = x^x");

// bind function to C++-side variable
auto f = Main.safe_eval("return f");

// call function
Int64 result = f(12);

// print result
std::cout << result << std::endl;
```
```
8916100448256
```

Where we now used `auto` as `f`s type. We will learn why this is necessary in the [section on proxies](#proxies).

Calling functions using `()` is `safe`, meaning any exception is forwarded to C++, just like with `safe_eval`:

```cpp
f(-2);
```
```
terminate called after throwing an instance of 'jluna::JuliaException'
  what():  [JULIA][EXCEPTION] DomainError with -2:
Cannot raise an integer x to a negative power -2.
Make x or -2 a float by adding a zero decimal (e.g., 2.0^-2 or 2^-2.0 instead of 2^-2), or write 1/x^2, float(x)^-2, x^float(-2) or (x//1)^-2
Stacktrace:
    (...)
```

In jluna, Julia-side functions can be used with both Julia-side and C++-side arguments. They can even be freely mixed:

```cpp
auto println = Main.safe_eval("return println");
println(
    "cpp side string",             // cpp side
    "\n",                          // cpp side
    Main.safe_eval("return Main"), // julia side
    "\n",                          // cpp side
    Main.safe_eval("return [1, 2, 3]")  // julia side
);
```
```
cpp side string
Main
[1, 2, 3]
```

Where `"cpp side string"` is a string on the C++-side, while `Main` and `[1, 2, 3]` are entirely allocated Julia-side. 

#### Accessing Named Variables in a Module

The above example illustrates how using `safe_eval("return x")` can be quite clumsy syntactically. To address this, jluna offers the much more elegant `operator[](std::string)`. 

> **C++ Hint**: `operator[](std::string)` is a function that is called by using `x["<string here>"]`, where `x` is of a type that supports the operator.

Rewriting the above, we get:

```cpp
auto println = Main["println"];
println(
    "cpp side string", 
    "\n", 
    Main["Main"], 
    "\n", 
    Main.safe_eval("return [1, 2, 3]")
);
```

Note how we could not replace `Main.safe_eval("return [1, 2, 3]")` with `Main["[1, 2, 3]"]`. This is because the latter is **invalid**. We can only access named Julia-side variables using `operator[](std::string)`. There is no variable named `[1, 2, 3]` in Main scope, therefore `operator[]` would fail and raise an exception.

In summary:
  + `Module.safe_eval` executes a string. We can use `return` to return the value of that expression
  + `Moulde["name"]` access an already existing, named variable, returning its value

### Accessing Julia Struct Fields

We learned how to access a named variable in a module by using `jluna::Module::operator[](std::string)`. This is convenient, however, modules are not the only jluna type that supports `operator[]`. 

Let's say we have the following structtype:

> **Julia Hint**: A structtype is any type declared using `struct` or `mutable struct`.

```cpp
// declare struct and instance
Main.safe_eval(R"(
    mutable struct MyStruct
       _field::Int64
    end
    
    jl_instance = MyStruct(9876);
)");

// access instance C++-side
auto cpp_instance = Main["jl_instance"];
```

Here, we defined a new structtype `MyStruct`, which has a single field `_field`. We then created a Julia-side variable, `jl_instance`, to which we bound a newly constructed `MyStruct`. 
To access the Julia-side `jl_instance`, we used `Main["jl_instance"]`. We are allowed to use `operator[]` because `jl_instance` is a proper named variable.

We can then access the Julia-side field of `cpp_instance` like so:

```cpp
// access field named `_field`
Int64 field_value = cpp_instance["_field"];

// print its value
std::cout << field_value << std::endl;
```
```
9876
```

Note, again, how we manually declared the type of the C++-side variable `field_value` to be `Int64`. We did not use `auto`.

### Mutating Julia-side Variables

We can use `operator[]` to access Julia-side values that are variables in a module or fields of a structtype. However, we can actually also **mutate** the variable itself:

> **Hint**: To mutate a variable means to change its value without changing its name.

```cpp
// declare struct and instance
Main.safe_eval(R"(
    mutable struct MyStruct
       _field::Int64
    end
    
    jl_instance = MyStruct(9876);
)");

// mutate field
Main["jl_instance"]["_field"] = 666;

// print value after mutation
Main.safe_eval("println(jl_instance._field)");
```
```
666
```

Let's go through this step-by-step.

We still have our `jl_instance` from the last section. We can access it using `operator[]`:

```cpp
auto instance = Main["jl_instance"];
```

Note the use of `auto`. We then access the instances field:

```cpp
auto instance_field = instance["_field"];
```

Because we used `auto`, `instance_field` does not refer to the value of `jl_instance._field` but to the *variable*. We can get the value of the variable by using `static_cast`:

```cpp
std::cout << static_cast<Int64>(instance_field) << std::endl;
```
```
666
```
> **C++ Hint**: C++ has multiple ways of changing a values type. `static_cast<T>` works by calling the conversion operator `operator T()` of a given type and returning its result. The following are exactly equivalent:
> ```cpp
> auto value = static_cast<Int64>(instance_field);
> ```
> ```cpp
> auto value = instance_field.operator Int64();
> ```
> ```cpp
> Int64 value = instance_field;
> ```
> Where the latter is preferred for stylistic reasons.
> 
> In the last example, `static_cast` (and thus the conversion operator) was implicitly called. This means we have already been using `static_cast` in the previous sections.
> 
> See the [C++ manual](https://en.cppreference.com/w/cpp/language/static_cast) for more information.

The C++-variable `instance_field` directly refers to the Julia-side variable `Main.jl_instance._field` (recall that `MyStruct` is mutable). Because of this, **if we mutate the C++-side variable, we will also mutate the corresponding Julia-side variable**:

```cpp
// access Main.instance
auto instance = Main["jl_instance"];

// access Main.instance._field
auto instance_field = instance["_field"];

// mutate Main.instance._field
instance_field = 1234;

// print value after mutation
Main.safe_eval("println(jl_instance._field)");
```
```
1234
```

Or, in just one line:

```cpp
Main["jl_instance"]["_field"] = 1234;
```

Which is highly convenient.

### Mutating Variables using `operator[](size_t)`

Accessing and assigning the result of `operator[]` works for variables in Modules, fields of structs and even elements of collections. Instead of `operator[](std::string)`, we will be using `operator[](size_t)` for this:

```cpp
// declare Julia-side vector
Main.safe_eval("vec_var = [1, 2, 3, 4]");

// mutate vector[1]
Main["vec_var"][0] = 9999;

// print value
Main.safe_eval("println(vec_var)");
```
```
[9999, 2, 3, 4]
```

Where the Julia-side `vec_var` is a `Base.Array`.

> **Hint**: In C++, indices are 0-based. In Julia, they are 1-based. This can be quite confusing, considering we are operating in both languages at the same time. For now, using `operator[]` on a C++-side object requires indices to be 0-based. 
>
> See the [section on arrays]() for more information.

---

## Proxies

So far, this manual has intentionally obfuscated what type `auto` resolves to in a statement like this:

```cpp
// declare variable
jluna::safe_eval("new_var = 1234");

// access variable (not variables value)
auto new_var = Main["new_var"];
```

Indeed, `new_var` is not an integer, it is deduced to be a `jluna::Proxy`. To fully understand how this class works, we first need to discuss how / if memory is shared between the C++ and Julia state.

### Shared & Separate Memory

By default, most memory allocated C++-side is incompatible with Julia. For example, a C++-side variable of type `char` is an 8-bit number, while a Julia-side `Char` has 32 bits. If we were to directly exchange memory between states, the value of the char would be corrupted. For more complex classes, for example comparing `std::set` to `Base.set`, the memory layout is even more different, making directly sharing memory impossible.

Because of this, it is best to assume that we have to **reformat** any C++-side memory before moving it Julia-side. 

In jluna, the **inverse is not true**. We do not have to reformat Julia-side memory to interact with it C++-side. This is the entire purpose of the library, it is made possible through `jluna::Proxy`.

`jluna::Proxy` is a proxy, or stand-in, of arbitrary Julia-side memory. Internally, it simply holds a pointer to whatever memory it is managing. Copying the proxy copies this pointer, not the Julia-side memory. Moving the proxy does not affect the Julia-side memory. If we want to actually change the Julia-side memory being managed, we use the proxies member functions.

### Constructing a Proxy

We rarely construct proxies from raw pointers (see the [section on `unsafe` usage](#the-unsafe-library)), instead, a proxy will be the return value of a member function of another proxy.

How can we make a proxy if we need a proxy to do so? We've done so already, jluna offers many pre-initialized proxies, including those for modules `Main`, `Base` and `Core`. We will limit ourself to `Main` for this section. We can use these proxies to generate new proxies for us.

To create a proxy from a Julia-side value, we use `Main.safe_eval("return x")`, where x is a value or a variable (though only the value of that variable will be returned):

```cpp
auto proxy = Main.safe_eval("return 1234");
```

To reiterate, the resulting proxy, `proxy`, **does not contain the Julia-side value `1234`**. It only keeps a pointer to wherever `1234` is located.

We can actually check the the pointer using `static_cast<unsafe::Value*>`, where `unsafe::Value*` is a raw pointer to arbitrary Julia-side memory:

```cpp
std::cout << static_cast<unsafe::Value*>(proxy) << std::endl;
```
```
0x7f3a9e5cd8d0
```

(this pointer will, of course, be different anytime `1234` is allocated)

As long as `proxy` stays in scope, `1234` cannot be deallocated by the garbage collector. This is, despite the fact that there is no Julia-side reference to it.

> **C++ Hint**: The term "going out of scope" or "staying in scope" is used to refer to whether a values' [destructor](https://en.cppreference.com/w/cpp/language/destructor) has yet been called.

> **Julia Hint**: In pure Julia, any value that is not the value of a named variable, inside a collection, or referenced by a `Base:Ref`, is free to be garbage collected at any point. `jluna::Proxy` prevents this

When using `proxy` with Julia-functions, it behaves exactly like whatever memory it is pointing to:
  
```cpp
// call `println(1234^2)` entirely Julia-side
Base["println"](Base["^"](proxy, 2));
```
```
1522756
```

Here, we're using the Julia-side function `Base.println` to print the result of `(^)(x, 2)` where `x` is the proxies values.

> **Julia Hint**: In Julia, an infix operator that has the symbol `x`, has the function definition 
> ```julia
> (x)(<arguments>) = <body>
> ```
> The braces signal to Julia that whatever symbol is in-between them, is an infix operator. Only certain characters can be used as infix operators, see [here](https://discourse.julialang.org/t/is-not-an-operator/20221).

No memory was moved during this call. We called a Julia-side function (`println` and `(^)`) with a Julia-side value `1234`. Other than having to access the proxies in the first place, the actual computation is exactly the same as if it was done in only Julia. This also means it is just as performant as pure Julia would be.


### Updating a Proxy

The most central feature of jluna is that of **updating a proxy**. If we assign a proxy a C++-side value, jluna will move this value Julia-side, then set the proxies pointer to that newly allocated memory:

```cpp
// create proxy to "string_value"
auto proxy = Main.safe_eval("return \"string_value\"");

// print value of proxy
Base["println"](proxy);

// print type of proxy
Base["println"](Base["typeof"](proxy));

// assign a proxy a C++ vector
proxy = std::vector<Int64>{4, 5, 6};

// print again after mutation
Base["println"](proxy);
Base["println"](Base["typeof"](proxy));
```
```
string_value
String
[4, 5, 6]
Vector{Int64}
```

We initialized a proxy, `proxy`, with the value of a Julia-side string `"string_value"`. Its type is, of course, `Base.String`. We then assigned `proxy` a C++ `std::vector`. This updated the proxies value, now pointing to `[4, 5, 6]`, which is of type `Vector{Int64}`.

Here, `proxy` is not pointing to the C++-side vector. jluna has implicitly converted the C++-side `std::vector<Int64>` to a Julia-side `Base.Vector{Int64}`, creating a deepcopy Julia-side (we will learn later on how to avoid this copying behavior, if desired). It has even accurately deduced the type of the resulting vector, based on the declared type and value-type of the C++-side vector.

> **Hint**: Let `std::vector<T> x`, then `x`s type is `std::vector<T>`, `x`s value-type is `T`

What about the other way around? Recall that `proxy` is now pointing to a Julia-side `Int64[4, 5, 6]`. We can actually do the following:

```cpp
// assign a C++ vector a Proxy
std::vector<UInt64> cpp_vector = proxy;

// print the C++-side elements
for (UInt64 i : cpp_vector)
    std::cout << i << " ";
```
```
4 5 6 
```
We have used the proxy as the right-hand side of an assignment. During execution, jluna has moved the Julia-side value `proxy` is pointing to (`Int64[4, 5, 6]`) to C++, potentially converting its memory layout such that it can be assigned to a now fully C++-side `std::vector<UInt64>`.

Note how we changed value-types. Julia-side, the value type was `Int64`, C++-side it is now `UInt64`. jluna has detected this discrepancy and, during assignment of the C++-side `std::vector`, implicitly converted all elements of the Julia-side `Array` from `Int64` to `UInt64`.

### Boxing & Unboxing

> **Hint**: An assignment is any line of code of the form `x = y`. `x` is called the "left-hand expression", `y` is called the "right-hand expression", owing to their respective positions relative to the `=`.

#### Boxing

The process of moving a C++-side value to Julia is called **boxing**. We **box** a value by assigning it to a proxy , that is:
+ the proxy is the left-hand expression of the assignment
+ the value to be boxed is the right-hand expression of the assignment 

```cpp
<Type Here> value = // ...
jluna::Proxy proxy = value;
```
#### Unboxing

The process of moving a Julia-side value to C++ is called **unboxing**. We **unbox** a proxy (and thus a Julia-side value) by assigning it to a non-proxy C++-side variable, that is:

+ the C++-side variable is the left-hand expression of the assignment
+ the proxy is the right-hand expression of the assignment

```cpp
jluna::Proxy proxy = // ...
<Type Here> value = proxy;
```

These concepts are important to understand, as they are central to how to move values between the Julia- and C++-state.

### (Un)Boxable Types

Note that not all types can be boxed and/or unboxed. A type that can be boxed is called a **Boxable**. A type that can be unboxed is called **Unboxable**. jluna offers two concepts `is_boxable` and `is_unboxable` that represent these properties.

> **C++ Hint**: A concept is a [new feature of C++20](https://en.cppreference.com/w/cpp/language/constraints). It is used like so:
> ```cpp
> template<//...
> concept is_boxable = //...
> 
> // jluna function:
> template<is_boxable T>
> void do_something(T value);
> ```
> Because we specified the template argument of `do_something` to be a `is_boxable`, at compile time, C++ will evaluate this condition for the template argument type. If `do_something` was called with a value of a type that is not boxable, a compiler error will be raised.

A type that is both boxable and unboxable is called **(Un)Boxable**. This is an important conceptualization, because:

+ we can only use **Boxables** as the **right-hand expression** of a proxy-assignment
+ we can only use **Unboxables** as the **left-hand expression** of an assignment.
  
Of course, an (Un)Boxables can be used on either side.

Out-of-the-box, the following types are (Un)Boxable:

```cpp
// cpp type (unboxed)    // Julia-side type (boxed)
bool                     <=> Bool
char                     <=> Char
int8_t                   <=> Int8
int16_t                  <=> Int16
int32_t                  <=> Int32
int64_t                  <=> Int64
uint8_t                  <=> UInt8
uint16_t                 <=> UInt16
uint32_t                 <=> UInt32
uint64_t                 <=> UInt64
float                    <=> Float32
double                   <=> Float64
        
nullptr_t                <=> Nothing
void*                    <=> Ptr{Cvoid}

std::string              <=> String
std::string              <=> Symbol
std::complex<T>          <=> Complex{T}    //[1]
std::vector<T>           <=> Vector{T}     //[1]
std::array<T, R>         <=> Vector{T}     //[1]
std::pair<T, U>          <=> Pair{T, U}    //[1]
std::tuple<Ts...>        <=> Tuple{Ts...}  //[1]
std::map<T, U>           <=> Dict{T, U}    //[1]
std::unordered_map<T, U> <=> Dict{T, U}    //[1]
std::set<T>              <=> Set{T, U}     //[1]

jluna::Proxy             <=> /* value-type deduced during runtime */
jluna::Symbol            <=> Symbol
jluna::Type              <=> Type
jluna::Array<T, R>       <=> Array{T, R}   //[1][2]
jluna::Vector<T>         <=> Vector{T}     //[1]
jluna::JuliaException    <=> Exception
jluna::Mutex             <=> Base.ReentrantLock

// [1] where T, U are also (Un)Boxable
// [2] where R is the rank of the array
        
std::function<TR()>           <=> jluna.UnnamedFunction{0} //[3]
std::function<TR(T1)>         <=> jluna.UnnamedFunction{1} //[3]
std::function<TR(T1, T2)>     <=> jluna.UnnamedFunction{2} //[3]
std::function<TR(T1, T2, T3)> <=> jluna.UnnamedFunction{3} //[3]
        
// [3] where TR, T1, T2, T3 are also (Un)Boxable     

Usertype<T>::original_type   <=> T //[4]
        
// [4] where T is an arbitrary C++ type

unsafe::Value*          <=> /* value-type deduced during runtime */
unsafe::Module*         <=> Module
unsafe::Function*       <=> /* value-type deduced during runtime */
unsafe::Symbol*         <=> Symbol
unsafe::Expression*     <=> Expr
unsafe::Array*          <=> Array<T, R> //[5][6]
unsafe::DataType*       <=> Type
        
// [5] where T is an arbitrary Julia type
// [6] where R is the rank of the array
```

There are a lot of things on this list that we have not discussed yet. It was considered important to have an exhaustive list at this point, as it will be valuable when referencing back to this section.

Most relevant `std::` types are supported out-of-the-box. All jluna types that reference Julia-side objects can be unboxed into their corresponding Julia-side values, just like any Julia-side value can be managed by their corresponding proxy.

If we are unsure of what a particular C++ type will be boxed to, we can use `to_julia_type<T>::value`. This template meta function has two points of interaction.

> **C++ Hint**: A template meta function is an advanced technique, where a structs template arguments, [partial specialization](https://en.cppreference.com/w/cpp/language/partial_specialization), [concepts](https://en.cppreference.com/w/cpp/language/constraints) and [SFINAE](https://en.cppreference.com/w/cpp/language/sfinae) are used to add functionality. For end-users, all that is needed is to know how to access any particular return value, as most template meta functions are not actual `std::function`s.

We can access the name of the Julia type, any particular type `T` will be boxed to, using `as_julia_function<T>::type_name`:

```cpp
auto name = as_julia_type<
    std::pair<
        std::complex<Float32>,
        std::string
    >
  >::type_name;

std::cout << name << std::endl;
```
```
Pair{Complex{Float32}, String}
```

Similarly, we can get the actual Julia-side Type object, `T` will be boxed to, using `as_julia_function<T>::type()`.


### Named & Unnamed Proxies

We've seen before that we can mutate Julia-side variable with proxies. To understand how this works, we need to be aware of the fact that there are two types of proxies in jluna: **named** and **unnamed**.

+ A **unnamed proxy** manages a Julia-side **value**
+ A **named proxy** manages a Julia-side **variable**

Here, "manage" means that, whatever the proxy points to, is safe from the garbage collector. Furthermore, when the proxy is mutated, the object being managed is mutated at the same time.

#### Unnamed Proxies

We create an unnamed proxy using `Module::safe_eval("return x")`:

```cpp
auto unnamed_proxy = Main.safe_eval("return [7, 7, 7]");
```

We've seen this type of proxy before, if we assign it a value:

```cpp
auto unnamed_proxy = std::vector<std::string>{"abc", "def"};
```

That value will be boxed and moved to Julia, after which the proxy will point to that value.

If we have a Julia-side variable `jl_var`:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct proxy
auto unnamed_proxy = Main.safe_eval("return jl_var");
```

Then, using `Main.safe_eval("return` will **only forward its value** to the unnamed proxy. If we modify the unnamed proxy again, the variable will not be modified:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// forward value to proxy
auto unnamed_proxy = Main.safe_eval("return jl_var");

// reassign proxy
unnamed_proxy = 1234;

// print proxy value
Base["println"]("cpp: ", unnamed_proxy);

// print Julia-side variable value
Main.safe_eval(R"(println("jl : ", jl_var))");
```
```
cpp: 1234
jl : [7, 7, 7]
```

The proxy has changed the value it is pointing to, the value of `jl_var` remains unchanged. All unnamed proxies exhibit this behavior.

### Named Proxy

We construct a named proxy using `operator[]`:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct proxy
auto named_proxy = Main["jl_var"];
```

This proxy behaves exactly the same as an unnamed proxy, except, **when the named proxy is mutated, it will mutate the corresponding Julia-side variable**:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct proxy
auto named_proxy = Main["jl_var"];

// reassign proxy
named_proxy = 1234;

// print proxy value
Base["println"]("cpp: ", named_proxy);

// print Julia-side variable value
Main.safe_eval(R"(println("jl : ", jl_var))");
```
```
cpp : 1234
jl  : 1234
```

The value the proxy is pointing to, **and** the value of the variable `jl_var` has changed. This is because a named proxy remembers the name of the variable it was constructed from. If we modify the proxy, it will also modify the variable. Only named proxies exhibit this behavior.

In summary: 

+ We construct an **unnamed proxy** using `Module.safe_eval("return <value>")`. Mutating an unnamed proxy will mutate its value, but it will **not** mutate any Julia-side variables.
<br><br>
+ We construct a **named proxy** using `Module["<variable name>"]`. Mutating a named proxy will mutate its value **and** mutate its corresponding Julia-side variable.

### Additional Member Functions

We can check which (if any) variable a proxy is managing using `get_name`:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct named & unnamed proxy
auto named_proxy = Main["jl_var"];
auto unnamed_proxy = Main.safe_eval("return jl_var");

// print name
std::cout << "named  : " << named_proxy.get_name() << std::endl;
std::cout << "unnamed: " << unnamed_proxy.get_name() << std::endl;
```
```
named  : jl_var
unnamed: <unnamed proxy #104>
```

We see that the named proxy indeed manages `Main.jl_var`. The unnamed proxy does not manage any variable, its name is an internal id.

If we just want to know whether a proxy is mutating, we ca use `is_mutating()`, which returns a `bool`:

```cpp
std::cout << "named  : " << named_proxy.is_mutating() << std::endl;
std::cout << "unnamed: " << unnamed_proxy.is_mutating() << std::endl;
```
```
named  : 1
unnamed: 0
```

### Detached Proxies

Consider the following:

```cpp
// create proxy to variable in main
Main.safe_eval("x = [1, 2, 3]");
auto x_proxy = Main["x"];

// print current value
Base["println"]("before: ", x_proxy);

// update x without the proxy
Main.safe_eval("x = -15");

// print again
Base["println"]("after : ", x_proxy);
```
```
before: [1, 2, 3]
after : [1, 2, 3]
```

Here, we created a vector with the value `[1, 2, 3]`, bound to `Main.x`. We created a proxy, `x_proxy`, managing that variable. When then updated the value of `Main.x` to `-15`, **without the proxy**. Printing the value of the proxy again, we see that it still points to `[1, 2, 3]`.

This proxy is now in a **detached state**. Despite being a named proxy, its value does not correspond to the value of its variable.

This is an artifact of how the proxy manages the memory it is pointing to. For performance reasons, the pointer only gets updated when mutation happens through the proxy. Because of this, if we modify the value of a variable a named proxy manages completely Julia-side, **we need to update that proxy** to tell it that the value changed. For this, jluna provides `Proxy::upate()`:

```cpp
Main.safe_eval("x = [1, 2, 3]");
auto x_proxy = Main["x"];

// print current value
Base["println"]("before: ", x_proxy);

// update x
Main.safe_eval("x = -15");

// update proxy
x_proxy.update();

// print again
Base["println"]("after : ", x_proxy);
```
```
before: [1, 2, 3]
after : -15
```

A simple `.update()` makes a named proxy query the current state of its variable, updating its value pointer and releasing whatever other value it managed before, such that it can now be collected by the garbage collector.

### Implications

With our newly acquired knowledge of named and unnamed proxies, we can investigate code from the previous sections more closely:

```cpp
// declare field an instance
Main.safe_eval(R"(
    mutable struct MyStruct
       _field::Int64
    end
    
    jl_instance = MyStruct(9876);
)");

// mutate jl_instance._field
Main["jl_instance"]["_field"] = 666;
```
This code snippet from [the section on mutating Julia-side values](#mutating-julia-side-values) uses a named proxy to update the field of `jl_instance`. 

Because we are using `operator[]`, `Main["jl_instance"]` returns a named proxy managing the Julia-side variable `Main.jl_instance`. Calling `["_field"]` on this result creates another named proxy, this time managing `Main.jl_instance._field`. We know that mutating a named proxy mutates its corresponding variable, thus, assigning the resulting proxy the C++-side value `666`, it is first boxed, then assigned to `Main.jl_instance._field`. 

In this example, the actual proxies were temporary - they only stayed in scope for the duration of one line. 

This syntax is highly convenient, we have used it frequently already:

```cpp
Base["println"]("hello julia");
```

In this expression, we create a named proxy to the Julia-side variable `Base.println`. We then use the proxies call operator ,`operator()`, to call the proxy as a function with the argument `"hello julia"`, which is a C++-side `std::string`. jluna implicitly boxes it to a Julia-side `String`, which is then used as the argument for calling the Julia-side `println`.

### Proxy Inheritance

Many of jlunas classes not named `jluna::Proxy` are actually still proxies, because they **inherit** from `jluna::Proxy`.

> **C++ Hint**: Inheritance is not the same as subtyping in Julia. If class `A` inherits from class `B`, then `A` has access to all non-private member and member functions of `B`. `A` can furthermore be `static_cast` to `B`, and any pointer to `A` can be directly treated as if it was a pointer to `B`.

Therefore, any of these classes provide the same functionalities of proxies, along with some additional ones. The next few sections will introduce `jluna::Proxies` "children", all of which have a more specialized purpose. It is important to keep in mind that both the concepts of named and unnamed proxies and their implied behavior also apply to all of these classes.

---

## Specialized Proxies: Arrays

One of the nicest features of Julia is its array interface. Therefore, it is only natural a Julia wrapper should extend all that functionality to a C++ canvas. While we learned before that we can use a `jluna::Proxy` like an array using `operator[](size_t)`:

```cpp
auto array_proxy = Main.safe_eval("return [1, 2, 3, 4]");

Int64 third = array_proxy[2];
std::cout << third << std::endl;
```
```
3
```

This is as far at it goes for array-related functionalities of `jluna::Proxy` itself. To access the full breadth of features, we need to use `jluna::Array`.

To transform a `jluna::Proxy` to a `jluna::Array`, we can use `.as`:

```cpp
// declare regular proxy
Proxy array_proxy = Main.safe_eval("return Int64[1, 2, 3, 4]");

// convert to array proxy
auto array = array_proxy.as<Array<Int64, 1>>();
```

or, more conveniently, we can use an implicit `static_cast`:

```cpp
Array<Int64, 1> array = Main.safe_eval("return Int64[1, 2, 3, 4]");
```

which implicitly calls `as<Array<Int64, 1>` for us.

We see that `jluna::Array<T, N>` has two template parameters:

+ `T` is the **value type** of the array
+ `N` is the **dimensionality** of the array, sometimes called the **rank**. A 1d array is a vector, a 2d array is a matrix, etc.

Both these template arguments directly correspond to the Julia-side parameters of `Base.Array{T, N}`.

Note that we have declared the Julia-side vector `[1, 2, 3, 4]` to be of value-type `Int64`. This is important, because we specified the C++-side array to also have that value-type. On construction, `jluna::Array` will check if the value types match (that is, whether for all elements `e_n` in the Julia-side array, it holds that `e_n <: T`). If they do not, an exception is thrown.

Because the match does not have to be exact (see the section on [type ordering](#type-order)), we can simply declare the value type to be `Any`. This lets us move any assortment of objects into the same array. jluna provides a typedef for this:

```cpp
using ArrayAny1d = // equivalent to Base.Array<Any, 1>
using ArrayAny2d = // equivalent to Base.Array<Any, 2>
using ArrayAny3d = // equivalent to Base.Array<Any, 3>
```

> **C++ Hint**: A `typedef` or "`using` declaration" is a way to declare a type alias. It is giving a type a new name, while keeping the old name valid. This is most commonly used to simplify syntax for the convenience of the programmer.

Which, while convenient, can introduce [type-instability](https://docs.julialang.org/en/v1/manual/performance-tips/#man-performance-value-type) to a piece of code may not require it.

## Accessing Array Indices

Before continuing, we need to set up our example arrays for this section:

```cpp
// create arrays
Array<Int64, 1> array_1d = jluna::safe_eval("return Int64[1, 2, 3, 4, 5]");
Array<Int64, 2> array_2d = jluna::safe_eval("return reshape(Int64[i for i in 1:9], 3, 3)");

// print
Base["println"]("array_1d: ", array_1d);
Base["println"]("array_2d: ", array_2d);
```
```
array_1d: [1, 2, 3, 4, 5]
array_2d: [1 4 7; 2 5 8; 3 6 9]
```

We created two arrays, a `Base.Array{Int64, 1}` bound to the C++-side array proxy `array_1d`, as well as a `Base.Array{Int64, 2}`, bound to `array_2d`.

To get a specific element of any array, we use `.at`:

```cpp
// access elements at index (0-based)
Int64 one_d_at_3 = array_1d.at(2);
Int64 two_d_at_2_2 = array_2d.at(1, 1);

// print
std::cout << one_d_at_3 << std::endl;
std::cout << one_d_at_2_2 << std::endl;
```
```
3
5
```
For a 1d array, `at` takes a single argument, its **linear index** (see below). For a 2d array, `at` takes two arguments, one for each dimension. This extends to any dimensionality, for a 5d array, we would call `at` with 5 integers.

For syntactic consistency, `jluna::Array` also offers `operator[]`, which is called in the exact same way as `at` would be. The difference is that, for `at` only, the indices are bounds-checked C++-side. This adheres to the convention set forth by `std::vector<T>::at` and `std::vector<T>::operator[]`.

### Linear Indexing

While n-dimensional indexing is only available for arrays of rank 2 or higher, linear indexing is available for all arrays. We can linear-index any array using `operator[](size_t)`

```cpp
Array<Int64, 3> array_3d = jluna::safe_eval("return reshape(Int64[i for i in 1:(3*3*3)], 3, 3, 3)");
std::cout << (Int64) array_3d[3] << std::endl;
```
```
4
```
Linear indexing accesses the n-th element in column-first order, just like it would in Julia (except that the index in C++ is 0-based).

### List Indexing

jluna also supports Julia-style list indexing for `operator[]`:

```cpp
Array<Int64, 1> vector = jluna::safe_eval("return [i for i in 1:10]");
auto sub_vector = vector.at({1, 5, 2, 7});

Base["println"](sub_vector);
```
```
[2, 6, 3, 8]
```
> **C++ Hint**: Here, the syntax used, `{1, 5, 2, 7}`, is called a "brace-enclosed initializer list", which is a form of [aggregate initialization](https://en.cppreference.com/w/cpp/language/aggregate_initialization) in C++. It can be best though of as a proto-vector, the compiler will infer the vectors type and construct it for us. In our case, because `Array::operator[]` expects a list of integer, it will interpret `{1, 5, 2, 7}` as the argument for an implicitly called constructor for that type, creating a `std::vector<size_t>`.<br>
> 
> see also: [list initialization](https://en.cppreference.com/w/cpp/language/list_initialization).

### 0-base vs. 1-base

This is about as good a place as any to talk about index bases. Consider the following:

```cpp
// create vector
Array<Int64, 1> array = jluna::safe_eval("return Int64[1, 2, 3, 4, 5, 6]");

// access element through C++ function
std::cout << "cpp: " << (Int64) array.at(3) << std::endl;

// access element through Julia function
std::cout << "jl : " << (Int64) Base["getindex"](array, 3) << std::endl;
```
```
cpp: 4
jl : 3
```

C++ indices are 0-based, this means `at(3)` will give use the `(3 - 0)`th element, which for our vector is `4`. In Julia, indices are 1-based, meaning `getindex(array, 3)` will gives us the `(3 - 1)`th element, which is `3`.

The following table illustrates how to translate C++-side indexing into Julia-side indexing:

| N | Julia | jluna |
|------|-------|--------------------------|
| 1    | `M[1]` | `M.at(0)` or `M[0]`|
| 2    | `M[1, 2]` | `M.at(0, 1)`|
| 3    | `M[1, 2, 3]` | `M.at(0, 1, 2)`|
| Any  | `M[ [1, 13, 7] ]` | `M[ {0, 12, 6} ]` |
| Any  | `M[i for i in 1:10]` | `M["i for i in 1:10"_gen]`
|      |        |     |
| *    | `M[:]` | not available |

Where `_gen` is a string-literal operator that  constructs a generator expression from the code it was called with. We will learn more about them shortly.

### Iterating

The main advantage `jluna::Array` has over the C-APIs `jl_array_t` is that it is **iterable**:

```cpp
Array<Int64, 1> array = jluna::safe_eval("return Int64[1, 2, 3, 4, 5, 6]");

for (Int64 i : array)
    std::cout << i << " ";

std::cout << std::endl;
```
```
1 2 3 4 5 6 
```
> **C++ Hint**: `std::endl` adds a `\n` to the stream, then flushes it.

Note how we manually declared the iterators type to be `Int64`.

If we declare the iterator type as `auto`, similar to how proxies work, each iterator is assignable:

```cpp
for (auto iterator : array)
    it = (Int64) it + 1;

Base["println"](array);
```
```
[2, 3, 4, 5, 6, 7]
```

This modifies the underlying Julia-array with minimal overhead.

If the array is also a named proxy, it will also modify that specific element of whatever variable the proxy is managing.

### Accessing the Size of an Array

To get the size of an array, we use `get_n_elements`:

```cpp
Array<size_t, 1> vec = jluna::safe_eval("return UInt64[i for i in 1:333]");
std::cout << vec.get_n_elements() << std::endl;
```
```
333
```

This returns the number of elements in the array, not the size along a specific dimension. If we want the latter, we instead use `Array::size`, which takes as its only argument the index of the dimension (0-based). The size of a 3d array `array_3d` along its second dimension would be accessible via `array_3d.size(1)`.

### jluna::Vector

For arrays of dimensionality 1, a special proxy called `jluna::Vector<T>` is provided. It directly inherits from `jluna::Array<T, 1>`, all of `Array`s functionalities are also available to `Vector`. 

In addition, the following member functions are only available for `jluna::Vector`:

+ `insert(size_t pos, T value)`
  - insert element `value` at position `pos` (0-based)
+ `erase(size_t pos)`
  - delete the element at position `pos` (0-based)
+ `push_front(T)`
  - push element to the front, such that it is now at position 0
+ `push_back(T)`
  - push element to the back of the vector
  
When boxing a `jluna::Vector<T>`, the resulting Julia-side value will be of type `Base.Vector{T}`. When boxing a `jluna::Array<T, 1>`, the result will be a value of type `Base.Array{T, 1}`.

## Generator Expressions

One of Julia's most convenient features are [**generator expressions**](https://docs.julialang.org/en/v1/manual/arrays/#man-comprehensions) (also called list- or array-comprehensions). These are is a special kind of syntax that creates an iterable, in-line, lazy-eval range. 

> **Hint**: A lazy-eval range is a collection that only allocates and/or computes its actual elements when that specific element is queried. After construction, no allocation is performed until requested by the user by indexing the range or iterating it.

For example:

```julia
# in Julia

# comprehension
out = [i*i for i in 1:10 if i % 2 == 0]

# mostly equivalent to
out = Vector()
f = i -> i*i
for i in 1:10
    if i % 2 == 0
        push!(out, f(i))
    end
end
```

In Julia, we use `[]` when we want the expression to be vectorized, and `()` when we want the expression to stay an object of type `Base.Generator`. The latter has the significant advantage that iterating and accessing its values is *lazy-eval*, meaning that we only perform computation when actually accessing the value. If we use `[]`, the generator is serialized on construction.

In jluna, we can create a generator expression using the postfix string-literal operator `_gen`:

> **C++ Hint**: A postfix string-literal operator has syntax `_x`, where x is an arbitrary name. We call it by appending it to the end of a C-string: `"string value here"_x`.
> 
> See [here](https://en.cppreference.com/w/cpp/language/user_literal) for more information.

```cpp
// in Julia:
(i for i in 1:10 if i % 2 == 0)

// in cpp:
"(i for i in 1:10 if i % 2 == 0)"_gen
```
Note that when using `_gen`, **only round brackets are allowed**. Every generator expression has to be in round brackets, they cannot be omitted or replaced with another form of brackets. Otherwise, an exception will be raised.

We can iterate through a generator expression like so:

```cpp
for (jluna::Proxy i : "(i for i in 1:10 if i % 2 == 0)"_gen)
    std::cout << static_cast<Int64>(i) << " ";
```
```
2 4 6 8 10
```

Where `i` was explicitly declared to be of type `jluna::Proxy`.

While this is convenient, we can actually use generator expressions as arguments for many member functions of arrays, just like in Julia:

```cpp
// initialize a vector from a generator expression
auto vec = Vector<size_t>("i*i for i in 1:99"_gen);

// use a generator expressions as a list index
vec["i for i in 1:99 if i < 50"_gen];
```

This imitates Julia syntax very closely, despite C++ being a language that does not have array comprehension. 


---

## Specialized Proxies: Symbols

Another specialized type of proxy is the **symbol proxy**. It manages a Julia-side `Base.Symbol`.

We can create a symbol proxy in the following ways:

```cpp
// new unnamed proxy
auto unnamed_symbol = Symbol("symbol_value");

// new named proxy
auto named_symbol = Main.new_symbol("symbol_var", "symbol_value");

// from already existing proxy
auto non_symbol_proxy_managing_symbol = Main.safe_eval("return :symbol_value");
auto unnamed_symbol = non_symbol_proxy_managing_symbol.as<Symbol>();
```

Where, unlike with `jluna::Proxy`, the value the proxy is pointing to is asserted to be of type `Base.Symbol`. 

### Symbol Hashing

The main additional functionality `jluna::Symbol` brings is that of **constant time hashing**.

A hash is essentially a `UInt64` we assign to things as a label. In Julia, hashes are unique and there a no hash collisions. This means if `A != B` then `hash(A) != hash(B)` and, furthermore, if `hash(A) == hash(B)` then `A === B`.

> **Julia Hint**: `(==)` checks if the value of two variables is the same. `(===)` checks whether both variables have the exact identical location in memory - if they are the same instance.

Unlike with other classes, `Base.Symbol` can be hashed in constant time. This is because the hash is precomputed at the time of construction. 

We can access the hash of a symbol proxy using `.hash()`. To get the symbol as a string, we use `static_cast`:

```cpp
// create symbol
auto symbol = Symbol("abc");

// print name and hash
std::cout << "name: " << static_cast<std::string>(symbol) << std::endl;
std::cout << "hash: " << symbol.hash() << std::endl;
```
```
name: abc
hash: 16076289990349425027
```

In most cases, it is impossible to predict which hash will be assigned to which symbol. This also means for Symbols `s1` and `s2` if `string(s1) < string(s2)` this does not mean `hash(s1) < hash(s2)`. This is very important to realize, **symbol comparison is not lexicographical comparison**.

Having taken note of this, `jluna::Symbol` provides the following comparison operators:

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

To further illustrate the way symbols are compared, consider the following example using `std::set`, which orders its elements according to their user-defined comparison operators:

```cpp
// create set
auto set = std::set<Symbol>();

// add newly constructed symbols to it
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

## Specialized Proxies: Modules

We have already used `jluna::Module` in limited ways before, it is now time to learn about all its features. `jluna::Module` (`Module`, henceforth) inherits publicly from `jluna::Proxy`, meaning it has all the same functions and functionalities we discussed previously, along with a few additional ones.

### Assign in Module

Let's say we have a module `M` in main scope that has a single variable `var`:

```cpp
// declare module
Main.safe_eval(R"(
    module M
        var = []
    end
)");

// access module as unnamed proxy
Module M = Main.safe_eval("return M");
```

We've already seen that we can modify this variable using `M.safe_eval`, however, this is fairly slow performance-wise. This is, because we force Julia to `Meta.parse`, then `eval` the `"return M"`.

`Module::assign` is much faster:

```cpp
// works but slow:
M.safe_eval("var = [777]");

// works and super fast:
M.assign("var", 777);
```

Where the first argument of assign is the variables name, the second argument is the new desired value.

If the variable we want to assign does not exist yet, we can performantly create it using `create_or_assign`:

```cpp
// create variable `new_var` and assign it 777
M.create_or_assign("new_var", 777);

// print value
Base["println"](M["new_var"]);
```
```
777
```

As the name suggest, if the variable does not exist, it is created. If the variable does exist, `create_or_assign` acts identically to `assign`. If we call `assign` for a variable that does not yet exist, an exception is raised.

### Creating a new Variable

A convenient and well-performing function is `Module::new_*`. `Module::new_undef("var_name")`, for example, creates a new variable named `var_name` in that module, then assigns it `undef`. The return value of this function is a named proxy pointing to that variable, making the following syntax possible:

```cpp
Main.new_undef("variable") = // any value here
```

The following `new_*` functions are available:

| jluna Name | C++ Argument Type(s) | Julia-side Type of Result |
|------------|-----------|---------------------------|
| `new_undef`  | `void`  | `UndefInitializer` |
| `new_bool`   | `bool`  |  `Bool`
| `new_uint8`   | `UInt8`  | `UInt8`  | 
| `new_uint16`   | `UInt16`  |  `UInt16`  |
| `new_uint32`   | `UInt32`  |  `UInt32`  |
| `new_uint64`   | `UInt64`  |  `UInt64`  | 
| `new_int8`   | `Int8`  |   `Int8`  |
| `new_int16`   | `Int16`  |  `Int16`  | 
| `new_int32`   | `Int32`  |  `Int32`  |
| `new_int64`   | `Int64`  | `Int64`  |
| `new_float32`   | `Float32`  | `Float32`  |
| `new_float64`   | `Float64`  | `Float64`  |
| `new_string` | `std::string` | `String` |
| `new_symbol` | `std::string` | `Symbol` |
| `new_complex` | `T`, `T` | `Complex{T}` |
| `new_vector` | `std::vector<T>` | `Vector{T}` |
| `new_dict` | `std::map<T, U>` | `Dict{T, U}` |
| `new_dict` | `std::unordered_map<T, U>` | `Dict{T, U}` |
| `new_set` | `std::set<T>` | `Set{T}` |
| `new_pair` | `T`, `T` | `Pair{T, T}` |
| `new_tuple` | `T1` , `T2`, `...`, `Tn` | `Tuple{T1, T2, ..., Tn}` |
| `new_array<T>` | `D1`, `D2`, `...`, `Dn` | `Array{T, n}` of size `D1 * D2 * ... * Dn` |

This is the most performant, safe way to create new variables in module scope. 

### Import, Using, Include

Additionally, `jluna::Module` provides the following two functions wrapping the `using` and `import` functions of modules:

+ `M.import("PackageName")`
  - equivalent to calling `import PackageName` inside `M`
+ `M.add_using("PackageName")`
  - equivalent to calling `using PackageName` inside `M`
+ `M.include("path/to/file.jl")`
  - equivalent to calling `include("path/to/file.jl")` inside `M`
  
Where `M` is a module.

---

## Calling C++ Functions from Julia

We've seen how to call Julia functions from C++. Despite being more of a Julia-wrapper for C++ than a C++-wrapper for Julia, in jluna, calling C++ functions from Julia is actually just as convenient and performant.

To call a C++ function, we need to assign a named or unnamed proxy a **lambda that wraps the C++ function**.

> **C++ Hint**: Lambdas are C++s anonymous function objects. Before continuing with this section, it is recommend to read up on the basics of lambdas [here](https://docs.microsoft.com/en-us/cpp/cpp/lambda-expressions-in-cpp). Users are expected to know about basic syntax, trailing return types and capture clauses from this point onward.

### Creating a Function Object

Let's say we have the following, simple lambda:

```cpp
auto add = [](Int64 a, Int64 b) -> Int64 
{
    return a + b;
};
```

This function has the signature `(Int64, Int64) -> Int64`. 

When interfacing with jluna, we should always manually specify the trailing return type of a lambda using `->`. We should never use `auto`, either for the lambdas return- or any of the argument-types.

To make this function available to Julia, we use `as_julia_function`. 
+ the argument of `as_julia_function` is a lambda or `std::function` object
+ the template argument of `as_julia_function` is the functions signature

Because `add` has the signature `(Int64, Int64) -> Int64`, we use `as_julia_function<Int64(Int64, Int64)>`. 

> **C++ Hint**: `std::function` and thus `as_julia_function` uses the C-style syntax for a functions signature. A function with return-type `R` and argument types `T1, T2, ..., Tn` has the signature `(T1, T2, ..., Tn) -> R`, or `R(T1, T2, ..., Tn)` in C-style.

We can then assign the result of `as_julia_function` to a Julia variable like so:

```cpp
// declare lambda
auto add = [](Int64 a, Int64 b) -> Int64 
{
    return a + b;
};

// bind to Julia-side variable
Main.create_or_assign("add", as_julia_function<Int64(Int64, Int64)>(add));
```

From this point onwards, we can simply call the C++-side `add` by using the newly created Julia-side variable `Main.add`:

```cpp
Main.safe_eval("println(add(1, 3))");
```
```
4
```

The return value of `as_julia_function` is a Julia-side object. This means, we can assign it to already existing proxies or otherwise handle it like any Julia-side value.

The Julia function can be called with Julia- or C++-side arguments, and its return value can be directly accessed from both Julia and C++.

### Allowed Signatures

Not all function signatures are supported for `as_julia_function`. Its argument (the C++ function) can only have one of the following signatures:

```cpp
() -> T_r
(T1) -> T_r
(T1, T2) -> T_r
(T1, T2, T3) -> T_r
```

Where 
+ `T_r` is `void` or an [(Un)Boxable]((Un)Boxable)
+ `T1`, `T2`, `T3` are an [(Un)Boxable]((Un)Boxable)

This may seem limiting at first, how could we execute arbitrary C++ code when we are only allowed to use functions with a maximum of three arguments using only (Un)Boxable types? 

### Taking Any Number of Arguments

Let's say we want to write a function that takes any number of `String`s and concatenates them. Obviously, just 3 arguments are not enough for this. Luckily, there is a workaround. Instead of using a n-argument function, we can use a 1-argument function where the argument is a n-element vector:

```cpp
// declare lambda, jluna::Array (aka. Base.Array) as argument
auto concat_all = [](jluna::Array<std::string, 1> arg) -> std::string
{
    // append through stringstream
    std::stringstream str;
    for (std::string s : arg)
        str << s;
    
    str << std::endl;
    
    // return string
    return str.str();
};
```
> **C++ Hint**: `std::stringstream` is a stream that we can write strings into using `operator<<`. We then flush it using `std::endl` and convert its contents to a single `std::string` using the member function `str`. More info about `std::stringstream` can be found [here](https://en.cppreference.com/w/cpp/io/basic_stringstream).

This lambda has the signature `(jluna::Array<std::string, 1>) -> std::string`. Because `jluna::Array<T, N>` boxes into `Base.Array{T, N}`, Julia-side, the resulting function will have the signature `(Base.Array{String, 1}) -> String`. We move it Julia-side using `as_julia_function`:

```cpp
// create new variable
Main.create_or_assign(
    "concat_all",           // variable name
    as_julia_function<      // as_julia_function call
        std::string(jluna::Array<std::string, 1>) // C++ signature
    >(concat_all)           // lambda
);
```

We can then call `concat_all` Julia-side, with any number of arguments, by wrapping the arguments in a Julia-side vector:

```cpp
Main.safe_eval(R"(
    println(concat_all(["GA", "TT", "AC", "A"]))
)");
```
```
GATTACA
```

If we want to truly call it with any number of arguments, not just a vector, we can simply do:

```cpp
auto concat_all = [](jluna::Array<std::string, 1> arg) -> std::string
{
    // ...
};

Main.create_or_assign(
    "concat_all_aux",   // now named concat_all_aux
    as_julia_function<std::string(jluna::Array<std::string, 1>)>(concat_all)
);

Main.safe_eval(R"(
    concat_all(xs::String...) = concat_all_aux(String[xs...])
    println(concat_all("now ", "callable ", "like ", "this"))
)");
```
```
now callable like this
```
Where we renamed the object holding the C++ lambda `concat_call_aux`, then called that object using a Julia-method with signature `(::String...) -> String`, which forwards its arbitrary number of arguments as a vector to `concat_call_aux`, thus achieving the desired syntax.

If we want our lambda to take any number of *differently-typed* arguments, we can either wrap them in a `jluna::ArrayAny1d` (which has the value type `Any` and thus can contain elements of any type), or we can use a `std::tuple`, both of which are (Un)Boxable. The latter should be preferred for performance reasons.

### Using Non-Julia Objects

We've learned how to work around the restriction on the number of arguments, but what about the types? Not all types are [(Un)Boxable](), yet we can still use arbitrary C++ types. How? By using **captures**.

Let's say we have the following C++ class:

```cpp
// declare non-julia class
struct NonJuliaObject
{
    // member
    Int64 _value;
    
    // ctor
    NonJuliaObject(Int64 in)
        : _value(in)
    {}
    
    // member function: doubles _value n-times
    void double_value(size_t n)
    {
        for (size_t i = 0; i < n; ++i)
            _value = 2 * _value;
    }
};

// instance the class C++-side
auto instance = NonJuliaObject(13);
```

This object is obviously not (Un)Boxable.

The naive approach to modifying `instance` would be with the following lambda:

```cpp
auto modify_instance = [](NonJuliaObject& instance, size_t n) -> void
{
    instance.double_value(n);
};
```

This lambda has the signature `(NonJuliaObject&, size_t) -> void`, which is a disallowed signature because `NonJuliaObject&` is not (Un)Boxable.

Instead of handing `instance` to the lambdas function body through an argument, we can instead **forward it through its capture**:

```cpp
auto modify_instance = [instance_ref = std::ref(instance)] (size_t n) -> void
{
    instance_ref.get().double_value(n);
    return;
};
```

> **C++ Hint**: `std::ref` is used to create a [reference wrapper](https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper) around any instanced object. It is very similar to Julia's `Base.Ref` in functionality. To "unwrap" it, we use `.get()` in C++, where we would use `[]` in Julia.
    
Lambda syntax can get quite complicated, so let's talk through this step-by-step. 

Firstly, this second lambda has the signature `(size_t) -> void`. Capture variables do not affect a lambdas signature.

Inside the capture `[]`, we have the expression `instance_ref = std::ref(instance)`. This expression creates a new variable, `instance_ref`, that will be available inside the lambdas body. We initialize `instance_ref` with `std::ref(instance)`, which creates a reference wrapper around our desired C++-side instance. A reference wrapper acts the same as a plain reference in terms of memory ownership, as long as the reference wrapper stays in scope, `instance` will too. 

Having captured `instance` through the reference wrapper, we can modify it inside our body by first unwrapping it using `.get()`, then applying whatever mutation we intend to. In our case, we are calling `double_value` with the `size_t` argument of the lambda.

After all this wrapping, we can simply:

```cpp
auto instance = NonJuliaObject(13);

auto modify_instance = // ...

// create julia-side variable
Main.create_or_assign(
    "modify_instance",  // variable name
    as_julia_function<void(size_t)>(modify_instance)  // lambda
)

// used function Julia-side
Main.safe_eval("modify_instance(3)");

// print value of C++-side instance
std::cout << instance._value << std::endl;
```
```
104
```

The now-Julia-function modified our C++-side instance, despite its type being uninterpretable to Julia.

By cleverly employing captures and collections / tuples, the restriction on what functions can be forwarded to Julia using `as_julia_function` are lifted. Any arbitrary C++ function (and thus any arbitrary C++ code) can now be executed Julia-side.

---

## Specialized Proxies: Types

We've seen specialized module-, symbol- and array-proxies. jluna has a fourth kind of proxy, `jluna::Type`, which wraps all of Julia's varied introspection functionalities in one class.

> **Hint**: [Introspection](https://en.wikipedia.org/wiki/Type_introspection) is the act of gaining information about the language itself, such as properties of types and functions. While possible, introspection in C++ can be quite cumbersome. Julia, on the other hand, was build from the ground up with it in mind.

While some overlap is present, `jluna::Type` is not a direct equivalent of `Base.Type{T}`. It is asserted to manage an object of type `T` such that `T isa Type`, though. It just provides more functions than are available using only Julia's `Base`.

### Constructing a Type

There are multiple ways to construct a type proxy:

```cpp
// get type of proxy
auto proxy = Main.safe_eval("return " + /* ... */);
auto type_proy = general_proxy.get_type();

// implicit cast
Type type_proxy = Main.safe_eval("return Base.Vector");

// result of `Base.typeof`
Type type = Base["typeof"](/* ... */);
```

Most often, we will want to know the type of a variable. For this we can either use `Base.typeof`, binding the result to an explicitly declared `jluna::Type`, or we can use`jluna::Proxy::get_type`.

### Base Types

For most types in `Base`, jluna offers a pre-defined type proxy in `jluna::` namespace, similar to the `Main` and `Base` module proxies.

The following types are available this way:

| jluna Constant Name | Julia-side Name|
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

Julia types can be ordered. To conceptualize this, the relation of types is best thought of as a directed graph. Each node of the graph is a type, each edge is directed, where, if the edge goes from type `A` to type `B`, then `B <: A`. That is, `B` is a subtype of `A`, or, equivalently, `A >: B`: `A` is a supertype of `B`.

This relational nature is heavily used in Julia's multiple dispatch and type inference, for now, it gives us a way to put types in relation to each other. `jluna::Type` offers multiple functions for this:

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

This ordering becomes relevant when talking about `TypeVar`s. 

> **Julia Hint**: `Base.TypeVar` is a class that represents a not-yet-defined type, such as a parameter for a struct. It has a lower bound `lb` and upper bound `ub`, where, for all types `t` represented by the `TypeVar`, it holds that `lb <: t <: ub`


`TypeVar`s can be thought of as a sub-graph of the type-graph. An unrestricted types upper bound is `Any`, while its lower bound is `Union{}`. A declaration like `T where {T <: Integer}` restricts the upper bound to `Integer`. Any type that is "lower" along the sub-graph originating at `Integer`, `Int64` for example, can still bind to `T`. This is useful to keep in mind.

### Type Info

When gathering information about types, it is important to understand the difference between a types **fields**, its **parameters**, its **properties** and its **methods**. Consider the following type declaration:

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
# in Julia
println(propertynames(MyType.body.body))
```
```
(:name, :super, :parameters, :types, :instance, :layout, :size, :hash, :flags)
```

Properties are often reserved for "hidden" members, such as those containing internal implementation details.

### Parameters

We can access the name and upper type bounds of the parameters of a type using `jluna::Type::get_parameters`:

```cpp
// declare type
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

// access type as Type proxy
Type my_type = Main["MyType"];

// get parameters
std::vector<std::pair<Symbol, Type>> parameters = my_type.get_parameters();

// print elements of vector with Julia-style pair syntax
for (auto& pair : parameters)
    std::cout << pair.first.operator std::string() << " => " << pair.second.operator std::string() << std::endl;
```
```
T => Integer
U => Any
```
`get_parameters` returns a vector of pairs where:
+ `.first` is a symbol that is the name of the corresponding parameter 
+ `.second` is the parameters upper type bound, as a `jluna::Type`
  
In case of `T`, the upper bound is `Base.Integer`, because we restricted it as such in the declaration. For `U`, there is no restriction, which is why its upper bound is the default: `Any`.

We can retrieve the number of parameters directly using `get_n_parameters()`. This saves allocating the vector of pairs.

### Fields

We can access the fields of a type in a similar way, using `jluna::Type::get_fields`:

```cpp
// declare type
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

// access type as Type proxy
Type my_type = Main["MyType"];

// get fields as vector
std::vector<std::pair<Symbol, Type>> fields = my_type.get_fields();

// print
for (auto& pair : fields)
    std::cout << pair.first.operator std::string() << " => " << pair.second.operator std::string() << std::endl;
```
```
_field1 => Any
_field2 => Integer
_field3 => Any
```
We, again, get a vector of pairs where `.first` is the name, `.second` is the upper type bound of the corresponding field.

If we actually want the value of a field, we need use `operator[]` on a `jluna::Proxy` that is an instance of that type, not the type itself.

While less useful, we can access a types methods using the Julia-side `Base.methods`. Similarly, to access a types properties, `Base.propertynames` and `Base.getproperty` can be used.


### Type Classification

To classify a type means to evaluate a condition based on a types attributes, in order to get information about how similar or different clusters of types are. jluna offers a number of classifications, some of which are available as part of the Julia standard library, some of which are not. This section will list all offered by jluna, along with their meaning:

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
+ `is_isbits`: is the type a [`isbits` type](https://docs.julialang.org/en/v1/base/base/#Base.isbits), meaning it is immutable and contains no references to other values that are not also isbits or primitives
    ```cpp
    Bool_t.is_declared_mutable()    // true
    Module_t.is_declared_mutable(); // false
    ```
+ `is_singleton`: a type `T` is a [singleton](https://docs.julialang.org/en/v1/base/base/#Base.issingletontype) iff:
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
+ `is_abstract_ref_type`: is the type a reference whose value type is an abstract type
    ```cpp
    Type(jluna::safe_eval("return Ref{AbstractFloat}")).is_abstract_ref_type(); //true
    Type(jluna::safe_eval("return Ref{AbstractFloat}(Float32(1))")).is_abstract_ref_type(); // false
    ```
+ `is_typename(T)`: is the `.name` property of the type equal to `Base.typename(T)`. Can be thought of as "is the type a T"
    ```cpp
    // is the type an Array:
    Type(State::safe_eval("return Array")).is_typename("Array"); // true
    Type(State::safe_eval("return Array{Integer, 3}")).is_typename("Array"); // also true
    ```
  
### "Unrolling" Types

There is a subtle difference between how jluna evaluates properties of a type and how pure Julia does. Consider the following:

```julia
# in julia
function is_array_type(type::Type) 
    return getproperty(type, :name) == Base.typename(Array)
end

println(is_array_type(Base.Array))
```

Some may expect this to print `true`, however, this is not the case. It actually prints `false`.

This is because `Base.Array` is a parametric type. `typeof(Array)` is actually `UnionAll`, which does not have a property `:name`:

```julia
# in Julia
println(propertynames(Array))
```
```
(:var, :body)
```
To access the property `:name`, we need to first **unroll** the type, meaning we need to specialize all its parameters. Once we do so, it seizes to be a `UnionAll`, gaining the properties expected of a `Base.Type`:

```julia
# in Julia

# access type
type = Array
    
# specialize fully
while (hasproperty(type, :body))
    type = type.body
end

# print type after unrolling
println(type)

# print propertynames of unrolled type
println(propertynames(type))
```
```
Array{T, N}
(:name, :super, :parameters, :types, :instance, :layout, :size, :hash, :flags)
```

Once fully unrolled, we have access to the properties necessary for introspection. jluna does this unrolling automatically for all types initialized by `jluna::initialize` (see the [previous sections list](#base-types)).

If desired, we can fully specialize a user-initialized type manually using the member function `.unroll()`. Without this, many of the introspection features will be unavailable.

---

## Usertypes

So far, we were only able to move (Un)Boxables to and from Julia. In some applications, this can be quite limiting. To address this, jluna provides a user-interface for making **any C++ type (Un)Boxable**. 

> **Hint**: A usertype is any type not defined by the standard library or jluna itself.

### Usertype Interface

Consider the following C++ class:

```cpp
// class representing color in the RGBA system
struct RGBA
{
    float _red;     // red component, in [0,1]
    float _green;   // green component, in [0, 1]
    float _blue;    // blue component, in [0, 1]
    float _alpha;   // transparency, in [0, 1]
    
    // construct
    RGBA(float r, float g, float b)
        : _red(r), _green(g), _blue(b), _alpha(1)
    {}
};
```

While it may be possible to manually translate this class into a Julia-side `NamedTuple`, this is rarely the best option. For more complex classes, this is often not possible at all. To make classes like this (Un)Boxable, we use `jluna::Usertype<T>`, the **usertype interface**.

### Step 1: Making the Type Compliant

For a type `T` to be manageable by `Usertype<T>`, it needs to be **default constructable**. `RGBA` currently has no default constructor, so we need to add it:

> **C++ Hint**: The default constructor of type T is `T()`. It can sometimes be declared as `T() = default`, see [here](https://en.cppreference.com/w/cpp/language/default_constructor).

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
    
    // added default ctor
    RGBA() 
        : _red(0), _green(0), _blue(0), _alpha(1)
    {}
};
```

If the type `T` is not default constructable, a static assertion is raised at compile time.

### Step 2: Enabling the Interface

To make jluna aware that we will be using the usertype interface for `RGBA`, we need to **enable it at compile time**. To do this, we use the `set_usertype_enabled` macro, executed in non-block scope.

> **C++ Hint**: Non-block scope (also called "global scope") is any scope that is not inside a namespace, function, struct, or block. As an example, `int main()` has to be declared in global scope.

```cpp
struct RGBA
{
    /* ... */
};

// enable usertype at compile time
set_usertype_enabled(RGBA); 
```

This sets up `Usertype<T>` for us. Among other things, it declares the Julia-side name of `RGBA`. This name is that same as the C++ side name, `"RGBA"` in our case.

### Step 3: Adding Property Routines

To add a property for `RGBA`s `_red`, we use the following function (at runtime):

> **Julia Hint**: In usage, both properties and fields have the exact same syntax. In C++, we would call these "members". For this section, all three terms will be used interchangeably.

```cpp
Usertype<RGBA>::add_property<Float32>(     // template argument
    "_red_jl",                             // field name
    [](RGBA& in) -> Float32 {              // boxing routine
        return in._red;
    },
    [](RGBA& out Float32 red_jl) -> void { // unboxing routine
        out._red = red_jl;
    }
);
```

This call has a lot going on so it's best to investigate it closely.

Firstly, we have the template argument, `Float32`. This decides the Julia-side type of the Julia-side instances field. 

The first argument is the Julia-side instances' fields name. Usually, we want this name to be the same as C++-side, `_red`, but to avoid confusion for this section only, the C++-side field is called `_red` while the corresponding Julia-side field is `_red_jl`.

The second argument of `add_property` is called the **boxing routine**. This function always has the signature `(T&) -> Property_t`, where `T` is the usertype-manage type (`RGBA` for us) and `Property_t` is the type of the field (`Float32`).
The boxing routine governs what value to assign the corresponding Julia-side field. In our case, it takes a C++-side instance of `RGBA`, accesses the value that instances `_red`, then assigns it to the Julia-side instances' `_red_jl`.

The third argument is optional, it is called the **unboxing routine**. It always has the signature `(T&, Property_t) -> void`. When a Julia-side instance of `RGBA` is moved back C++-side, the unboxing routine governs what value the now C++-sides `RGBA` fields `_red` will be assigned. If left unspecified, the value will be the value set by the default constructor. In our case, we assign `_red` the value of `_red_jl`, which is the second argument of the unboxing routine.

In summary:

+ the template argument governs the Julia-side type of the field
+ the first argument is the name of the Julia-side field
+ the boxing routine decides what value the Julia-side field will be assigned when moving the object C++ -> Julia
+ the unboxing routine decides what value the C++-side field will be assigned when moving the object Julia -> C++

Now that we know how to add fields, we can do so for `_green`, `_blue` and `_alpha`:

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
        : _red(0), _green(0), _blue(0), _alpha(1)
    {}
};
set_usertype_enabled(RGBA);

// ###

// in function scope, i.e. inside main
Usertype<RGBA>::add_property<float>(
    "_red", // now named `_red`, not `_red_jl`
    [](RGBA& in) -> float {return in._red;},
    [](RGBA& out, float in) -> void {out._red = in;}
);
Usertype<RGBA>::add_property<float>(
    "_green",
    [](RGBA& in) -> float {return in._green;},
    [](RGBA& out, float in) -> void {out._green = in;}
);
Usertype<RGBA>::add_property<float>(
    "_blue",
    [](RGBA& in) -> float {return in._blue;},
    [](RGBA& out, float in) -> void {out._blue = in;}
);
Usertype<RGBA>::add_property<float>(
    "_alpha",
    [](RGBA& in) -> float {return in._alpha;},
    [](RGBA& out, float in) -> void {out._alpha = in;}
);
```

Note that, now, the Julia-side field is actually called `_red`, which is better style than the `_red_jl` we used only for clarity.

To illustrate that properties do not have to directly correspond with members of the C++ class, we'll add another Julia-side-only field that represents the `value` component from the [HSV color system](https://en.wikipedia.org/wiki/HSL_and_HSV) (sometimes also called "lightness"). It is defined as the maximum of red, green and blue:

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

We leave the unboxing routine for `_value` unspecified, because the C++-side instance does not have any corresponding field to assign to.

### Step 4: Implementing the Type

Having added all properties to the usertype interface, we make the Julia state aware of the interface by calling:

```cpp
// in main
Usertype<RGBA>::implement();
```

This creates a new Julia-side type that has the architecture we just gave it. For end-users, this happens automatically. 

Internally, the following expression is assembled and evaluated:

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

We see that jluna assembled a mutable structtype, whose field names and types are as specified. Even the order in which we called `add_property` for specific names is preserved. This becomes important for the default constructor (a constructor that takes no arguments). The default values for each of the types' fields, are those of an unmodified, default-initialized instance of `T` (`RGBA()` in our case). This is why the type needs to be default constructable.

If we want the type to be implemented in a different module, we can specify this module (as a `jluna::Module`) as an argument to `Usertype<T>::implement`.

If we desire additional constructors, we can simply add them as external constructors in the same scope the usertype was `implement`ed in:

```cpp
Main.safe_eval(R"(
    function RGBA(r::Float32, g::Float32, b::Float32, a::Float32) ::RGBA
        out = RGBA()
        out._red = r
        out._green = g
        out._blue = b
        out._alpha = a
        out._value = max(r, g, b, a)

        return out
    end
)");
```

### Step 5: Usage

After `Usertype<RGBAB>::implement()`, we can use `RGBA` just like any other (Un)Boxable type:

```cpp
// create new variable and assign it a RGBA
Main.create_or_assign("jl_rgba", RGBA(1, 0, 1));

// print value of that variable
Main.safe_eval("println(jl_rgba);");

// print fieldnames of the Julia-side type
Main.safe_eval("println(fieldnames(RGBA))");
```
```
RGBA(1.0f0, 0.0f0, 1.0f0, 1.0f0)
(:_red, :_green, :_blue, :_alpha, :_value)
```
> **Julia Hint**: `Base.fieldnames` takes a type (not an instance of a type) and returns the symbols of a types fields, in order.

We see that now, `Main.RGBA` is a proper Julia type and `jl_rgba` got the correct values according to each fields boxing / unboxing routine.

The same applies when moving `Main.RGBA` from Julia to C++:

```cpp
// create a Julia-side RGBA and assign it to a C++-side RGBA
RGBA cpp_rgba = Main.safe_eval("return RGBA(0.5, 0.5, 0.3, 1.0)");

// print member values
std::cout << cpp_rgba._red << " ";
std::cout << cpp_rgba._green << " ";
std::cout << cpp_rgba._blue << " ";
```
```
0.5 0.5 0.3
```

This section was quite complicated, a fully working `main.cpp` replicating this `RGBA` example can be found [here](./rgba_example.cpp). Users are encouraged to play with it, to further their understanding of the usertype interface.

### Additional Member Functions

In addition to the steps outlined in this section, `Usertype<T>` offers the following additional members / member functions:

+ `Usertype<T>::original_type` 
  - typedef equal to `T`
+ `is_enabled()` 
  - was `set_usertype_enabled` called for `T`
+ `get_name()`
  - get the Julia side name of `T` after unboxing
+ `is_implemented()`
  - was implement called at least once for this `T`

Lastly, after `implement` was called, the  `as_julia_type<Usertype<T>>` template meta function will work, just like it would for other (Un)Boxables.

## Multi Threading

Given Julia's application in high-performance computing and its native multi-threading support, it is only natural a Julia wrapper should also allow for asynchronous execution in a similarly convenient and performant manner. 

In relation to specifically a C++ <-> Julia environment, we run into a problem, however:

```cpp
#include <jluna.hpp>
#include <thread>

int main()
{
    jluna::initialize();
    
    // create a lambda that does nothing
    auto noop = []() -> void {
        jl_eval_string("");
        return;
    };
    
    // execute it in a std::thread
    auto thread = std::thread(noop);
    
    // wait fo thread to finish
    thread.join();
    
    // exit main
    return 0;
}
```

Here, we're doing a very simple operation. We created a C++ lambda that *does nothing*. It executes no julia-side code by handing `jl_eval_string` (the C-APIs way to execute strings as code) an empty string, after which is simply returns. We then create a C++-side thread using `std::thread`. In C++, once a thread is created, it starts running immediately. We wait for its execution to finish using `.join()`.

Running the above code, the following happens:
```
signal (11): Segmentation fault
in expression starting at none:0
Allocations: 782030 (Pool: 781753; Big: 277); GC: 1

Process finished with exit code 139 (interrupted by signal 11: SIGSEGV)
```

It segfaults without an error. 

This is, because the Julia C-API is seemingly hardcoded to prevent any call of C-API functions from anywhere but master scope (the scope of the thread `main` is executed in). It is assumed that this is done so the garbage collector can safely keep track of allocations. 

For obvious reasons, this makes things quite difficult, because, **we cannot access the Julia state from within a C++-side thread**. This has nothing to do with jluna, it is how the C-API was designed. 

Given this, we can immediately throw out using any of the C++ `std::thread`-related multi-threading support, as well as libraries like [libuv](https://github.com/libuv/libuv). It is important to keep this in mind, if our application already uses these frameworks, we have to take care to **never execute code that interacts with Julia from within a thread**. This includes any of jlunas functionalities, as it is, of course, build entirely on the Julia C-API.

All is not lost, however: jluna offers its own multi-threading framework, allowing for parallel execution of truly arbitrary C++ code - even if that code interacts with the Julia state.

### Initializing the Julia Threadpool

In Julia, we have to decide the number of threads we want to use **before startup**. 
In the Julia REPL, we would use the `-threads` (or `-t`) argument. In jluna, we instead specify the desired number of threads as an argument to `jluna::initialize`:

```cpp
int main()
{
    jluna::initialize(8); 
    // equivalent to `julia -t 8`
    
    // application here
    
    return 0;
}
```
```
[JULIA][LOG] initialization successful (8 threads).
```

If left unspecified, jluna will initialize Julia with exactly 1 thread. We can set the number of threads to `auto` by supplying the following constant to `jluna::initialize`:

```cpp
using namespace jluna;

initialize(JULIA_NUM_THREADS_AUTO);
// equivalent to `julia -t auto`
```

This sets the number of threads to number of local CPUs, just like setting environment variable `JULIA_NUM_THREADS` to `auto` would do for pure Julia.

Note that any already existing `JULIA_NUM_THREAD` variable, in the environment the jluna executable is run in, is **ignored and overridden**. We can only specify the number of threads through `jluna::initialize`.

### Creating a Task

Owing to its status of being in-between two languages with differing vocabulary and design, jlunas thread pool architecture borrows from both C++ and Julia.

To execute a C++-side piece of code, we have to first wrap it into a C++ lambda, then wrap that lambda in a `jluna::Task`.

We cannot initialize a task directly, rather, we use `jluna::ThreadPool::create`:

```cpp
using namespace jluna;

// declare lambda that prints to console and returns its argument
auto forward_arg = [](size_t in) -> size_t {
    Base["println"]("lambda called with ", in);
    return in;
};

// create task
auto task = ThreadPool::create<size_t(size_t)>( // signature
    forward_arg,    // function
    size_t(1234)    // function arguments
);
```

Here, `ThreadPool::create` takes multiple arguments:

Its template argument is the signature of the lambda we want the task to execute. Just like with `as_julia_function`, it expects a C-style signature. `forward_arg` has the signature `(size_t) -> size_t`, making `size_t(size_t)` the appropriate template argument.

The first argument is the function object itself. This can be a lambda, like `forward_arg`, or a `std::function` object.

Any following arguments of `ThreadPool::create` will be used as the **arguments for the given function**. In our case, because we specified `size_t(1234)`, the thread pool will invoke our lambda `forward_arg` with that argument (and only that argument).

Note that `create` invokes the copy constructor on all its argument. If this behavior is not desired, we can wrap the argument in a `std::reference_wrapper` using `std::ref`, meaning only the reference itself will be copied, not the actual object.

### Running a Task

Unlike C++-threads (but much like Julia tasks), `jluna::Task` does not immediately start execution once it is constructed. We need to manually "start" is using `.schedule()`. 

We can wait for its execution to finish using `.join()`. This stalls the thread `.join()` was called from (usually the master thread in which `main` is executed) until the task completes:

```cpp
// declare lambda
std::function<size_t(size_t)> forward_arg = [](size_t in) -> size_t {
    Base["println"]("lambda called with ", in);
    return in;
};

// create task
auto task = ThreadPool::create(forward_arg, size_t(1234));

// start task
task.schedule();

// wait for task to finish
task.join();
```
```
lambda called with 1234
```

Note how, even though we called the Julia function `println`, the task **did not segfault**. Using jlunas thread pool is the only way to call C++-side functions that also access the Julia state concurrently.

### Managing a Tasks Lifetime

The result of `ThreadPool::create` is a `jluna::Task<T>`, where `T` is the return type of the C++ function used to `create` it, or `void`. **The user is responsible for keeping the task in memory**. If the variable the task is bound to goes out of scope, the task simply ends:

```cpp
/// in main.cpp
jluna::initialize(8);

// open block scope
{
    // declare function that counts to 9999
    std::function<void()> count_to_9999 = []() -> void
    {
        for (size_t i = 0; i < 9999; ++i)
            std::cout << i << std::endl;
    };

    // create task
    auto task = ThreadPool::create(count_to_9999);
    
    // start task
    task.schedule();

    // wait for 1 millisecond
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
}
// task is destructed here

std::this_thread::sleep_for(10ms); // wait for another 10ms
return 0; // exit main
```
```
(...)
2609
2609
2610
2611

Process finished with exit code 0
```
> **C++ Hint**: A *block* or *anonymous block scope* is created using `{` `}`. It acts similar to Julia's `begin` `end`, anything declared inside the block will go out of scope when the block ends. In C++, all variables declared inside the block are local to only that block.

> **C++ Hint**: `std::chrono` are C++s time-related functionalities. calling `std::this_thread::sleep_for(1ms)` will stall the master thread main is executed in for 1 millisecond.

Here, our task is supposed to count all the way up to 9999. Instead, the task went out of scope before it could finish, only being able to count to 2611 before it was terminated.

A way to solve this is to store the task in a collection that is itself in master scope:

```cpp
// task storage
std::vector<Task<void>> tasks;

{
    std::function<void()> print_numbers = []() -> void
    {
        for (size_t i = 0; i < 10000; ++i)
            std::cout << i << std::endl;
    };

    // add task to storage
    tasks.push_back(ThreadPool::create(print_numbers));
    
    // start just pushed task
    tasks.back().schedule();
    
    // wait for 1ms
    std::this_thread::sleep_for(1ms);
}

// wait for another 10ms
std::this_thread::sleep_for(10ms);
return 0;
```
```
(...)
9996
9997
9998
9999

Process finished with exit code 0
```

This time, because the tasks was not destructed prematurely, it had 10 milliseconds more time to finish. This happened to be enough for it to reach its intended count.

jluna has no equivalent to Julia's `Threads.@spawn`. This is, to force users to keep track of their tasks: to `.schedule` a task, they need to first cache it in a variable. This hopefully avoids situations where tasks end unexpectedly.

### Accessing a Tasks State

We can check the status of any task using the following member functions:

+ `is_done`: is the task finished or failed, c.f. `Base.istaskdone`
+ `is_failed`: has the task failed, c.f. `Base.istaskfailed`
+ `is_running`: has the task been scheduled but not yet finished, c.f. `Base.istaskstarted`

Furthermore, any Julia function that works with tasks can be called directly using the `jluna::Task` object as an argument.

### Accessing a Tasks Result

Returning to our example from before:

```cpp
std::function<size_t(size_t)> forward_arg = [](size_t in) -> size_t {
    
    // print
    Base["println"]("lambda called with ", in);
    
    // return argument
    return in;
};

auto task = ThreadPool::create(forward_arg, size_t(1234));
task.schedule();
task.join();
```

Here, `forward_arg` does more than just print to the command line, it also returns a value.

To access the return value of a task, we use the member function `.result()`. This function returns an object of type `jluna::Future`.

A future is a thread-safe container that may or may not (yet) contain a value. The future itself is available immediately when its corresponding task is created.
```cpp
auto task = ThreadPool::create(forward_arg, size_t(1234));
auto future = task.result();
```

Until the task has successfully completed, however, the future will not contain a value. Once the task is done, the return value will be copied into the future, after which we can access it. If we want to avoid the copying, we need to wrap the result in a `std::reference_wrapper`.

To get the potential value of a future, we use `.get()`, which returns a `std::optional<T>` where `T` is the return type of the C++ function used to `create` the task. Once completed, we can access the value of the optional using `std::optional::value()`. To check whether the value is already available, we can use `jluna::Future::is_available()`:

```cpp
// create task
auto task = ThreadPool::create(forward_arg, size_t(1234));

// get future of task (currently empty)
auto future = task.result();

std::cout << future.is_available() << std::endl; // false

// start task
task.schedule();

// wait for task to finish
task.join();

std::cout << future.is_available() << std::endl; // true

// print value of future
std::cout << future.get().value() << std::endl;
```
```
0
1
1234
```
> **C++ Hint**: Trying to access the value of a std::optional before it is available will raise an exception. See the [standard libraries documentation](https://en.cppreference.com/w/cpp/utility/optional) for more ways to interact with `std::optional<T>`.

We can wait for the value of a future to become available by calling `.wait()`. This will stall the thread `.wait()` is called from until the value becomes accessible, after which the function will return that value. This way, we don't necessarily need to keep track of the futures task, just having the future allows us to access the tasks result. We do still need to make sure the corresponding task stays in scope, however.

### Data Race Freedom

The user is responsible for any potential data races a `jluna::Task` may trigger. Useful C++-side tools for this application include the following (where their Julia-side functional equivalent is listed for reference):

| C++ | Julia | C++ Documentation  |
|------------|--------------|---------------------|
| `std::mutex` | `Base.ReentrantLock` |  [[here]](https://en.cppreference.com/w/cpp/thread/mutex)                  |
| `std::lock_guard`| `Threads.@lock` | [[here]](https://en.cppreference.com/w/cpp/thread/lock_guard)
| `std::condition_variable` | `Threads.Condition` | [[here]](https://en.cppreference.com/w/cpp/thread/condition_variable)
| `std::unique_lock` | `n/a` | [[here]](https://en.cppreference.com/w/cpp/thread/unique_lock)

Furthermore, jluna provides its own lock-like object `jluna::Mutex`, which is a simple wrapper around a Julia-side `Base.ReentrantLock`. It has the same usage and interface as `std::mutex`, except that it works when called both from C++ and Julia because it is (Un)Boxable.

### Thread-Safety

As a general rule, any particular part of jluna is thread-safe, as long as two threads are **not modifying the same object at the same time**.

For example, `jluna::safe_call` can be called from multiple threads, and `safe_call` itself will work. If both `safe_call` modify the same value, however, concurrency artifacts may occur.

Similarly, we can freely create unrelated proxies and modify them individually. If we create two proxies that reference the same Julia-side variable, mutating both proxies (if they are named) at the same time will possibly trigger an error or data corruption.

The user is required to ensure thread-safety in these conditions, just like they would have to in Julia. jluna has no hidden pitfalls or behind-the-scene machinery that multi-threading may throw a wrench into, all its internals (that are unaccessible to the user) are thread-safe as of version 0.9.0. 

Any user-created objects are outside of jlunas responsibility, however.

Calls to C++ lambdas forwarded to the Julia state using `as_julia_function` are thread-safe to call, though if they modify they same object or if the C++-function deadlocks C++-side, artifacts will be forwarded to the Julia state.

Notably, `Module::new_*`, `Module::create` and `Module::create_or_assign` are thread-safe. Each `jluna::Module` carries exactly one lock, which allows these calls to happen safely in a multi-threaded environment. All other functions of `jluna::Module` do not make use of this lock. Instead, users will be required to manually "lock" an object, and manage concurrent interaction with it, themselves.

### Thread-Safety in Julia

As an example for how to make modifying an object thread-safe, let's say we have the following variable in `Main`:

```julia
# in julia
to_be_modified = []
```

We want to thread-safely add elements to this vector. One way to do this would be with a Julia-side Lock:

```julia
# in julia
to_be_modified = []
to_be_modified_lock = Base.ReentrantLock()
```

Now, when a thread wants to modify `to_be_modified`, it first need to acquire this lock:

```cpp
// in cpp

// thread function
auto push_to_modify = [](size_t) -> void
{
    // access Base.lock
    static auto lock = Base["lock"];
    
    // access Base.unlock
    static auto unlock = Base["unlock"];
  
    // access Julia-side lock
    auto* mutex = Main["to_be_modified_lock"];
    
    // lock 
    lock(mutex);
  
    // modify vector here
  
    // unlock
    unlock(mutex);
    return;
}
```
> **C++ Hint**: Using `static` in block-scope declares (and potentially defines) a variable *exactly once*. Anytime the code is run through, afterwards, the line with `static` is skipped. For this example, by making `lock` and `unlock` static variables, the lookup performed by `Base::operator[]` is only done a single time across the entire runtime of the application, increasing performance.
> 
> See [here](https://en.cppreference.com/w/cpp/language/storage_duration) for more information on storage classifiers.

Since `push_to_modify` will be executed through `jluna::ThreadPool`, the Julia function `lock` will stall the thread, allowing for safe access.

### Thread-Safety in C++

A similar approach can be taken when trying to safely modify C++-side objects. Instead of a Julia-side lock, we use a C++-side `jluna::Mutex`:

```cpp
// C++-side variable we want to thread-safely access
jluna::Vector<size_t> to_be_modified;

// C++-side mutex
auto to_be_modified_lock = jluna::Mutex();

auto push_to_modify = [](size_t)
{
    to_be_modified_lock.lock();
  
    // modify here
  
    to_be_modified_lock.unlock();
    return;
}
```

Similarly, `std::mutex` or `std::unique_lock` can be used for the same purpose.

### Multi-Threading: Closing Notes

#### Interacting with `jluna::Task` from Julia

Internally, jluna makes accessing the Julia-state from a C++-sided, asynchronously executed function possible by wrapping it in a Julia-side `Task`. jluna can then use Julia's native threadpool, allowing for C-API functions to be safely executed. This has some side-effects, most of them useful.

For example, `yield`, called from C++ like so:

```cpp
static auto yield = Main.safe_eval("return Base.yield");
yield();
```

Will actually yield the thread this C++ code is executed in, letting another Julia thread take over. This applies to all Julia-side functions such as `fetch`, `bind`, etc. <br>Calling them from within a `jluna::Task` has exactly the same effect as calling them from within a `Base.Task`. 

We can access the Julia-side object `jluna::Task` is managing using `operator unsafe::Value*()`, which returns a raw C-pointer to the Julia-side taks. This allows us to create a `jluna::Proxy` of a `jluna::Task` like so:

```cpp
// declare lambda
auto lambda = [](){ //...
    
// create task
auto task = ThreadPool::create<void>(lambda);

// create proxy to task
auto task_proxy = Proxy(static_cast<unsafe::Value*>(task));

// all Julia-only attributes can now be accessed:
std::cout << (bool) task_proxy["sticky"] << std::endl;
```
```
true
```

> **Julia Hint**: `Threads.Tasks.sticky` is a property that governs wether a task can be executed concurrently. By default, `sticky` is set to `false`, making it "stick" to the main thread, instead of "detaching" and being run on its own.

We can then use this proxy as we would use a Julia-side variable, allowing for full freedom on how to manage and schedule tasks.

#### Do **NOT** use `@threadcall`

Lastly, a warning: Julia has a macro called `@threadcall`, which purports to simply execute a `ccall` in a new thread. Internally, it actually uses the libuv thread pool for this, not the native Julia thread pool. Because the C-API is seemingly hardcoded to segfault any attempt at accessing the Julia state through any non-master C-side thread, using `@threadcall` to call arbitrary code will also trigger this segfault. Because of this, it is not recommended to use `@threadcall` in any circumstances. Instead, we can `ccall` from within a proper `Base.Task`, or use jlunas thread pool to execute C-side code in the first place.

---

## The `unsafe` Library

> **Waning**: Misuse of this part of jluna can lead to exception-less crashes, data corruption, and memory leaks. Only user who are confident when  handling raw memory with no safety-nets are encouraged to employ these functions.

> **Tip**: Unlike with other headers, it may be instructive to [visit `unsafe_utilities.hpp`](../include/unsafe_utilities.hpp) periodically while reading this section, the inline documentation may clear up potential questions.

So far, a lot of jlunas internal workings were intentionally obfuscated to allow more novice users to use jluna in a way that is easy to understand and 100% safe. For some applications, however, the gloves need to come off. This section will detail how to surrender the most central of jlunas conceits: safety. In return, we get **optimal performance**, achieving overheads of near 0% compared to the C-API, with some jluna functions actually being faster than their C-API counterpart.

Most of these functions are contained in the `jluna::unsafe` namespace, appropriately reminding users to be careful when using them at all times.

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
>(instance::Callable)(xs...) = // ...
>```

### Acquiring Raw Pointers

For any object inheriting from `jluna::Proxy`, we can access the raw `unsafe::Value*` to the memory it is managing using `operator unsafe::Value*()`

> **C++ Hint**: The `operator T()` of object `x` is implicitly invoked when performing a cast of the form `static_cast<T>(x)`.
 
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

`jluna::Proxy` can only be cast to `unsafe::Value*`. If we want a different `unsafe` pointer type instead, we will need to `reinterpret_cast` the result of `Proxy::operator unsafe::Value*()` to that pointer type.
  
### Calling Julia Functions

So far, we used proxies to call Julia-functions with Julia-values. In the `unsafe` library, we instead use `unsafe::call`, which takes a `unsafe::Function*` as its first argument and any number of `unsafe::Value*` as subsequent arguments.

This has a few downsides. Firstly, we need to acquire a pointer to any given function. We could do this through proxies, but the most performant way is through `unsafe::get_function`. This functions takes two argument: the module the function is located in, as well as the name of a function as a symbol:

```cpp
unsafe::Function* println = unsafe::get_function(Base, "println"_sym);
```

Where `_sym` is a string-literal operator that converts its argument to a Julia-side symbol.  Using symbols instead of strings achieves a <60% performance gain, making this function much faster than the C-APIs `jl_get_function`.

We used jlunas pre-initialized `Base`, which is of type `jluna::Module`.  In reality, this call is implicitly executing:

```cpp
auto* println = unsafe::get_function(
    static_cast<unsafe::Module*>(Base), // implicit cast
    "println"_sym
);
```

Most of jlunas proxies are able to be `static_cast` to their corresponding `unsafe` pointer type. `jluna::Module` to `unsafe::Module*`, `jluna::Symbol` to  `unsafe::Symbol*`, `jluna::Type` to `unsafe::DataType*`, etc. Because these operators are [not explicit](https://en.cppreference.com/w/cpp/language/explicit), we can use objects of a type that supports directly with the `unsafe` functions, allowing for some convenience (with no additional performance overhead).

Having acquired a pointer to `println`, we can call the function like so:

```cpp
auto* res = unsafe::call(println, (unsafe::Value*) jluna::Main);
```
```
Main
```

If an exception is raised during `unsafe::call`, the user will not be notified and the function will silently return a `nulltpr`, potentially causing a segfault when accessing the returned value.

jluna offers a middle ground between the `Proxy::call` and `unsafe::call`: `jluna::safe_call`. This function also returns an `unsafe::Value*`, however any exception that is raised during invokation of the function is forwarded to C++. Unless absolute peak performance is needed, it is generally recommended to use `jluna::safe_call` in place of `unsafe::call`, as both functions have the same signature.

### Executing Strings as Code

For an unsafe version of `Module::safe_eval`, the `unsafe` library provides the `_eval` string literal operator:

```cpp
"println(\"unsafely printed\")"_eval;
```
```
unsafely printed
```

Which will similarly forwad its potential result as a return value. Unlike `safe_eval`, the return value is not a proxy but a `unsafe::Value*`, pointing to the resul.

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
auto back_cpp_side = unbox<char>(julia_side);
std::cout << (int) back_cpp_side << std::endl;
```
```
120
```

Where `auto` is deduced to `char`, as `unbox<T>` always returns a value of type `T`.

This way of explicitly moving values between states can be quite cumbersome syntactically, which is why `jluna::Proxy` does all of this implicitly. In return, it offers the fastest possible way to move values between states.

Note that `box<T>` and `unbox<T>` should always be called with an explicit template argument. This is to make sure a value is boxed / unboxed into what the user desired, eliminating the possibility of accidental implicit conversions reducing performance.

### Protecting Values from the Garbage Collector

The result of `box` is a `unsafe::Value*`, a raw C-pointer. The value this pointer points to **is not protected from the garbage collector** (GC). At any time after the resolution of `box`, the Julia GC may deallocate our value right under our nose. This can't happen when using proxies - it is the entire point of them - but it can, when handling raw pointers. 

Because of this, we have to micro-manage each value depending on how it was allocated. As a general rule of thumb: any value that is not explicitly referenced by either a Julia-side named variable, or a Julia-side `Ref`, can be garbage collected at any point, usually segfaulting the entire application at random times. This includes the result of `box`, `jluna::safe_call`, `unsafe::call` and most `unsafe` functions returning pointers in general. Any pointer acquired by calling `static_cast<unsafe::Value*>` on a proxy is safe, as long as the proxy stays in memory.

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

Where, for none of the functions, any exception forwarding is performed.

For example, we can employ them like so:

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

Here, we did not need to `gc_preserve`. We can be sure that `Char(121)` is protected, because we just created a named Julia-side variable, `jl_char`, pointing to it. If we were to reassign `jl_char`, `Char(121)` may be garabge collected at any point afterwards.

All of `unsafe::get_*` / `unsafe::set_*` functions will be vastly superior, in terms of performance, when compared to `Module::safe_eval`. They are even slightly faster than `Module::assign`.

---

### `unsafe` Array Interface

One of the most important high-performance tasks is modifying large arrays. `box` usually invokes a copy, which is unacceptable in environments like this. The `unsafe` library provides a 0-overhead interface for creating and modifying arrays, internally operating on the raw Julia memory for us.

Note that we can simply access the data of a `jluna::Array` by `static_cast`ing it to `unsafe::Value*`:

```cpp
Array<Int64, 2> array = // ...
unsafe::Array* raw_data = static_cast<unsafe::Array*>(array);
```

This will cause no reallocation, it simply forwards the pointer to the Julia-side memory of the `Base.Array`.

If we have a `unsafe::Array*` to a large, already Julia-side array, we can convert it to a `jluna::Array` using its specialized constructor:

```cpp
unsafe::Array* big_array = // ...
auto big_array_wrapper = Array<size_t, 2>(big_array);
```

This causes no reallocation, making all the convenient functions of `jluna::Array` available to us. Note that the user is responsible for assuring that the dimensionality and value type of the declared `jluna::Array` match that of the underlying `unsafe::Array*`. If this is not true, data corruption or potential `nullptr`-access crashes may occur.

Elements of an array are stored in column-first order. To access the actual memory of the elements (not the memory of the array itself), we can use the field `data` of `unsafe::Array*`, or the C-API function `jl_array_data`, both of which return a `void*`.

### Accessing an Array Element

The `unsafe` library provides `get_index(Array*, size_t...)` and `set_index(Array*, size_t...)`, two 0-overhead functions that manipulate array elements by index. Unlike with `jluna::Array`, no bounds-checking is performed and there is no mechanism in place to verify that potential return values are actually of the expected type.

Similarly to `jluna::Array`, a 1d array takes 1 index, a 2d array 2 indices, and a Nd array N indices. Any array of any rank can furthermore be linear-index using just one index, again accessing elements in column-major order, just like with `jluna::Array`.

#### Allocating a New Array

`unsafe` supports arrays of any rank, however arrays of rank 1 (vectors) and rank 2 (matrices) are far better optimized and should be preferred in performance-critical environments, if at all possible.

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

In high-performance applications, we often do not have enough RAM to have two of the same array in memory at the same time. To address this, the `unsafe` library provides a function that creates a *thin wrapper* around already existing data. A thin wrapper is an array who's data does not belong to it. We basically set the data-pointer of a newly created array to already existing memory, but not allocate any new memory. The user is responsible for keeping the original memory correctly formatted and in-scope.

Similarly to `new_array`, `new_array_from_data` takes Rank + 2 elements:

+ the value type (as an `unsafe::Value*`)
+ a pointer to already existing data (as a `void*`)
+ Rank-many indices, specifying the size of an array in each dimension

It is important to realize that no bounds-checking or verification, that the data is actually formatted according to the arrays value type, is performed. If we have an array of `size_t`s, which are 64-bit, and we use its data to create an array of `uint8_t`s, the user will not be made aware of this, instead leading to silent data corruption.

#### Resizing an Array

The `unsafe` library provides `resize_array`, which takes as its arguments the array, and new size in each dimension, similar to `new_array`.

For 1d and 2d arrays only, "slicing" (making an array smaller in one or more dimensions) is very fast. No allocation is performed when slicing, unlike "growing" an array. Furthermore, resizing a 1d array into another 1d array is far more performant than resizing a 1d array into a 2d array. In the latter case, the equivalent of `Base.reshape` has to be called, potentially leading the entire array to be re-allocated during the shuffling.

The newly added values are set to `undef`. Recall that Julia-arrays are stored column-first order. Expanding a matrix in the x-direction will lead to new values being inserted into each column, potentially corrupting the order of elements.

#### Replacing an Arrays Data

To avoid copying, jluna provides `override_array`, which replaces an arrays data with that of another. No allocation is performed, and at no point will the amount of memory grow beyond the initial space of both arrays. 
As no copy is made, after overriding array `B` with array `A`, the user is responsible for keeping array `A` in scope, lest `B` data be potentially deallocated as well.

Similarly, `swap_array_data` replaces array `B`s data with that of `A`, and arrays `A` data with that of `B`. Unlike `override_array`, jluna needs to create a temporary copy of one of the arrays, potentially increasing the amount of memory consumed by the size of one of the arrays.

---

### Shared Memory

In the section on proxies, we said that, to move a value from C++ to Julia or vice-versa, we first need to **change its memory format** such that it is interpretable by the other language. This is not always true. For a very limited number of types, Julia and C++ **already have the exact same memory format**. 

For these types only, `box` / `unboxing` is a 0-cost operation. No actual computation is performed. This is obviously desired in performance-critical applications.

An exhaustive list of these types is provided here:

```cpp
bool
int8_t
int16_t
int32_t
int64_t
uint8_t
uint16_t
uint32_t
uint64_t
float
double

const char*
std::string

nullptr_t
std::vector<T> [1]
T**            [1][2]


[1] Where T is also a no-cost-(Un)Boxable
[2] Where T** is a C-array
```
> **C++ Hint**: To learn more about C-array usage from within C++, consider reading [this manual page](https://en.cppreference.com/w/c/language/array).

All of these types have the distinction of having a `C*` type (where `*` is the C-side typename) Julia-side: `Csize_t`, `Cstring`, `Cvoid`, etc.

The `unsafe` library provides truly 0-cost `box` / `unbox` functions, called `unsafe::unsafe_box` / `unsafe::unsafe_unbox`. The "safe" `box` / `unbox` do some sanity checking and exception forwarding, even for C-types, making `unsafe_box`/ `unsafe_unbox` calls the performance-wise optimal way of moving memory between states:

```cpp
gc_pause;

auto* int64_memory = "return 1234"_eval;

std::cout << unsafe::unsafe_unbox<Int64>(int64_memory) << std::endl;
// exactly as fast as:
std::cout << *(reinterpret_cast<Int64*>(int64_memory)) << std::endl;

gc_unpause;
```
```
1234
1234
```

> **C++ Hint**: Here, `reinterpret_cast` is the C++ equivalent of a C-style cast, writing `*((Int64*) int64_memory)` would exactly equivalent.

---

## Performance Optimization

In this section, we will investigate jlunas performance, analyzing benchmark results and explaining why certain things are faster than others. Hopefully, this will educate users on how they can achieve the best performance using jluna.

In general, when optimizing a program using jluna, the following is appropriate:

+ speed, safety, convenience - you can only pick two

The first part of this manual dealt with functions that offer **safety and convenience**. Proxies and the more abstracted parts of jluna are easy to use, and forward any exception to the user, preventing any of the C-style errors like segfaults and data corruption from occurring. <br>
The `unsafe` library drops safety, leaving use with **speed and convenience**. We can be very fast, the `unsafe` functions are much more convenient than handling raw memory or dealing with C-API, but in return, we have no safety net. <br>
If we want **speed and safety**, we need to use the `unsafe` library or C-API and do all the exception catching and GC-safety ourselves, which drops convenience from the equation.

The obvious question is: how much slower is the "safe way"? This section will answer this question by giving an exact percentage of incurred overhead, compared to doing things the optimal way performance-wise.

### The Setup

> **Hint**: This section explains the benchmarks methodology for the sake of transparency and reproducibility. It can be safely skipped for users just interested in the results. 

All benchmarks were done on a machine with the following CPU (output of unix' `lshw`)

```
*-memory
    description: System memory
    physical id: 0
    size: 8064MiB

*-cpu
    product: Intel(R) Core(TM) i5-8250U CPU @ 1.60GHz
    vendor: Intel Corp.
    physical id: 1
    bus info: cpu@0
    size: 3040MHz
    capacity: 3400MHz
    width: 64 bits
    capabilities: fpu fpu_exception wp vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp x86-64 constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb invpcid_single pti ssbd ibrs ibpb stibp fsgsbase tsc_adjust sgx bmi1 avx2 smep bmi2 erms invpcid mpx rdseed adx smap clflushopt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp md_clear flush_l1d cpufreq
```

Benchmarks were limited to run only in a single thread, unless otherwise specified. The machine was kept clean of other processes during benchmarking. The benchmarks were run long enough (>30s per benchmark) for noise to not affect the results. Care was taken to never fill up the RAM, as to not run into swap-space related artifacts. Potential cache-effects were not accounted for.

A [custom benchmark library](../.benchmark/benchmark.hpp) was used, which timed each benchmark using a `std::chrono::steady_clock`. The full sourcecode of all benchmarks used for this section and general testing can be found [here](../.benchmark/main.cpp). The results of the benchmarks, as a `.csv` can be access [TODO](TODO).

Relative overhead was measured using the following function:

```cpp
using Duration_t = std::chrono::duration<double>;
double overhead(Duration_t a, Duration_t b) 
{
    return (a < b ? 1 : -1) * (abs(a - b) / a); 
};
```

This function was deemed to represent the intuitive notion of speedup and overhead. 

+ `A` is 10% faster than `B`, if `B`s run is 110% the runtime of `A`. 
  - `A` exhibits a -10% overhead compared to B
+ `C` is 25% slower than `D`, if `C`s run is 125% the runtime of `D`.
  - `C` exhibits a +25% overhead compared to `D`
 


The median of all results of a particular benchmark run was used for this comparison, somewhat mitigating potential spikes due to noise or one-time-only allocations, which a simple mean would have exhibited.

## Results

When measuring performance, absolute number are rarely very informative. We either need to normalize them relative to the machine it was run on, or, **compare two results**, both run during the same benchmarking session, on the same machine.

The latter approach was used for these results. We will start each benchmark with a "baseline", this is usually the fastest possible way to a certain task using only the Julia C-API. Then, jluna functionalities that accomplish the same task are run and compared against that baseline.

Each following section will deal with a certain task, showing how to accomplish this task in different ways. Then, we will commenting on which way was the fastest - and why.

### Accessing Julia-side Values

One of the most basic tasks in jluna is getting the value of something Julia-side. This can be accomplished in many different ways:

```cpp
// number of cycles
size_t n_reps = 1000000;

// allocate function object Julia-side
Main.safe_eval(R"(
function f()
    sum = 0;
    for i in 1:100
        sum += rand();
    end
end
)");

// C-API
Benchmark::run_as_base("C-API: get", n_reps, [](){
volatile auto* f = jl_get_function(jl_main_module, "f");
});

// unsafe::get_function
Benchmark::run("unsafe: get_function", n_reps, [](){
volatile auto* f = unsafe::get_function(jl_main_module, "f"_sym);
});

// unsafe::get_value
Benchmark::run("unsafe: get_value", n_reps, [](){
volatile auto* f = unsafe::get_value(jl_main_module, "f"_sym);
});

// named Proxy from unsafe
Benchmark::run("named proxy: from unsafe", n_reps, [](){
volatile auto* f = (unsafe::Function*) Proxy(unsafe::get_value(jl_main_module, "f"_sym), "f"_sym);
});

// Proxy.operator[](std::string)
Benchmark::run("named proxy: operator[]", n_reps, [](){
volatile auto* f = (unsafe::Function*) Main["f"];
});

// Main.get
Benchmark::run("module: get", n_reps, [](){
volatile auto* f = Main.get<unsafe::Function*>("f");
});

// C-API: eval
Benchmark::run("eval: get", n_reps / 4, [](){
volatile auto* f = (unsafe::Function*) jl_eval_string("return f");
});
```

Here, we created a Julia-side function `f`. We used various functions to access its value. The C-API only has a getter for functions specifically, which is why we choose the object, `get`, to be a function.

### Accessing Julia-side Values: Results

| name | median duration (ms) | overhead|
|------|----------------------|-------------|
| `jl_get_function` | `8.1e-05ms` | `0%` |
| `unsafe::get_function` | `8.4e-05ms` | `4%` |
| `unsafe::get_value` | `8.4e-05ms` | `4%` |
| `Module::get<T>` | `0.000282ms` | `248%` |
| `Proxy ctor` | `0.005378ms` | `6190%` |
| `Proxy::operator[](std::string)` | `0.005378ms` | `6540%` |
| `jl_eval_string("return x")` | `0.085889ms` | `105936%` |

We see vast runtime difference between the different ways. Firstly, `unsafe` falls into the designed `< 5%` overhead range, which is good to see. Returning values by evaluating a string is, as stated many times in this manual, prohibitively slow and should never be used unless unavoidable. 

`Module::get<T>` may not look like it would be much better than just using `Module::operator[]`, however, this is not the case. `Module::get<T>` is much faster because it **never has to construct a proxy**. Because we know the final result type, `T`, an `unsafe::Function*` in this benchmark, is not a proxy, `Module::get<T>` can simply unbox the value directly, as if done through the C-API.<br>

We see how much constructing that proxy matters, if we really just want the value (or the pointer to a function, in this case), going through proxies just to discard them immediately introduces a 61x to 65x performance increase, making this option unacceptable in performance critical environments. This doesn't mean we should never use the proxy, however. If we only construct a proxy once and then use it over and over, the 60x increase may pay off. We will investigate if this is indeed the case in a coming section.

In summary, `unsafe` and the C-API are unmatched in performance, however, in applications where safety is preferred, `Module::get<T>` is the best way to access the **value of a variable** (not the variable itself).

### Mutating Julia-side Variables

We benchmarked **accessing** variables, now, we'll turn our attention to **mutating** them. Recall in the manual section on proxies, only named proxies mutate values. So far, we have seen that handling proxies tends to introduce a significant amount of overhead, we will see if this is still the case:

```cpp
// number of cycles
size_t n_reps = 10000000;

// initialize variable to be modified
Main.safe_eval("x = 1234");

// C-API
Benchmark::run_as_base("C-API: set", n_reps, [](){

    Int64 to_box = generate_number<Int64>();
    jl_set_global(jl_main_module, "x"_sym, jl_box_int64(to_box));
});

// unsafe
Benchmark::run("unsafe: set", n_reps, [](){

    Int64 to_box = generate_number<Int64>();
    unsafe::set_value(jl_main_module, "x"_sym, unsafe::unsafe_box<Int64>(to_box));
});

// Module::assign
Benchmark::run("Module::assign", n_reps / 10, [](){

    Int64 to_box = generate_number<Int64>();
    Main.assign("x", to_box);
});

// Proxy::operator=
auto x_proxy = Main["x"];
Benchmark::run("Proxy::operator=", n_reps / 10, [&](){

    Int64 to_box = generate_number<Int64>();
    x_proxy = to_box;
});
```

Here, we see 4 different methods to accomplish this task. The C-APIs `jl_set_globa` and the `unsafe` libraries `set_value` will most likely be the most performant options, much mor safe and convenient, however, are the familiar `Proxy::operator=` and `Module::assign`.

During each cycle of each benchmark, we generate a random `Int64` using `generate_number`. This function introduces the same amount of overhead anytime it is called, meaning it will introduce the exact same amount of runtime to each of the benchmarks, making comparison between them still valid.

### Mutating Julia-side Values: Results

| name | median duration (ms) | overhead|
|------|----------------------|-------------|
| `base` | `0.000137ms`           | `0%`  |
| `unsafe::set_value` | `0.000141ms` | `3%` |
| `Module::assign` | `0.000379ms` | `176%` |
| `named proxy` | `0.062295ms` | `45370.8%` | 

This time, `unsafe` is almost identical in runtime compared to the C-API. The next best performain option is `Module::assign`, introducing a similar amount of overhead as `Module::get` in the ast section. Lastly we have named proxies. The overhead here is so massive due to multiple reasons. Internally, the named proxy has to:
+ update its pointer
+ add the new value to the GC safeguard
+ evaluate its name
+ update the variable of that name

Only the latter of which has to be performed by any of the other functions. This explains the large amount of overhead, making named proxies a bad choice when trying to mutate variables in a performant manner. Once again, the specialized `jluna::Module` function gives the best compromise between performance and safety.


### Calling Julia-side Functions

It is important that executing Julia-side code through functions have as little overhead as possible. This section will investigate if this is the case.

```cpp
size_t n_reps = 10000000;

// function to test
Main.safe_eval(R"(
    function f()
        sum = 0;
        for i in 1:100
            sum += rand();
        end
    end
)");

auto* f_ptr = unsafe::get_function(jl_main_module, "f"_sym);

// C-API
Benchmark::run_as_base("C-API: jl_call", n_reps, [&](){

    jl_call0(f_ptr);
});

// unsafe
Benchmark::run("unsafe: call", n_reps, [&](){

    unsafe::call(f_ptr);
});

// jluna::safe_call
Benchmark::run("jluna::safe_call", n_reps, [&](){

    jluna::safe_call(f_ptr);
});

// proxy::safe_call<T>
auto f_proxy = Proxy(f_ptr);
Benchmark::run("Proxy::safe_call<T>", n_reps, [&](){

    f_proxy.safe_call<void>();
});

// proxy::operator()
Benchmark::run("Proxy::operator()", n_reps, [&](){

    f_proxy();
});
``` 

Here we access a pointer to our simple Julia-side function, outside of the benchmarks. We call that function with no arguments inside each benchmark. For the benchmarks using proxies, we construct that proxy outside of the measured runtime, to avoid proxy construction polluting the results.

### Calling Julia-side Functions: Results

| name | median duration (ms) | overhead|
|------|----------------------|-------------|
| `jl_call` | `0.000243ms` | `0%` |
| `unsafe::call` | `0.000256ms` | `5.34%` |
| `jluna::safe_call` | `0.000513ms` | `111%` | 
| `Proxy::safe_call<T>` | `0.000689ms` | `184%` |
| `Proxy::operator()` | `0.006068ms` | `2397.12%` |

`unsafe` barely hits misses its target of < 5% overhead. Next we have `jluna::safe_call`. This functions does full exception forwarding in the fastest way possible, so the `111%` overhead is basically the cost of safety. Compared to this, `Proxy::safe_call<T>` is relatively close. Unlike `Proxy::operator()`, `Proxy::safe_call<T>` does not construct a new proxy for the result value, it directly unboxes the result. This also explains the huge jump in overhead, just like with `Module::get<T>` vs `Module::operator[]`, constructing the proxy is what caused the overhead to become unacceptable.

`jluna::safe_call` offers the best compromise between safety and speed, it still provides exception forwarding without constructing a proxy or having to deal with proxy-related overheads.

### Calling C++ Functions from Julia

We've seen how fast we can call a Julia-side function from C++. In jluna, we can of course do it the other way around, which is hopefully just as performant.

```cpp
size_t n_reps = 1000000;

// actual function
static auto task_f = [](){
    for (volatile size_t i = 0; i < 10000; i = i+1);
};

// base
Benchmark::run_as_base("Call C++-Function in C++", n_reps, [](){
    task_f();
});

// move to julia, then call
Main.create_or_assign("task_f", as_julia_function<void()>(task_f));
auto* jl_task_f = unsafe::get_function(jl_main_module, "task_f"_sym);

Benchmark::run_as_base("Call C++-Function in Julia", n_reps, [&](){
    jl_call0(jl_task_f);
});
```

Here, we are measuring the time it takes for `task_f` to finish when called from either language. `task_f` itself just counts to 10000, where `volatile` prevents the loop from being optimized away by the compiler. After having simply run this function in C++, we move it to Julia outside of the benchmarks measured time, then measure how much overhead is present if calling the same function from Julia.

### Calling C++ Functions from Julia: Results

| name | median duration (ms) | overhead|
|------|----------------------|-------------|
| `called C++-side` | `0.015963ms` | `0%` |
| `called Julia-side` | `0.016145ms` | `1.14%` |

Results suggest that there is very little overhead calling the C++ function, we are very comfortable below the 5% threshold, making calling C++ functions from Julia pretty much exactly as fast as calling them from C++.

### Using jluna::Array

`jluna::Array` was mentioned as having very little overhead, making it much more performant than proxies for modifying Julia-side arrays. This section will see if this is true. The following code segment may be somewhat hard to follow for people unfamiliar with the C-API, however, be assured that each `run_as_base` function aims to implement whatever the corresponding `jluna::Array` function does as fast as possible using only the C-API




### Constructing `jluna::Task`

With all the wrapping going on to make `jluna::Task` be able to execute C-API functions, you may expect there to be a significant overhead to creating a task. Let's find out if this is indeed true, we've already seen that calling the C++ function itself introduces a ~2% overhead, any additional overhead would be due to task construction (including its futures) itself.

```cpp
// actual function
static auto task_f = [](){
    for (volatile size_t i = 0; i < 10000; i = i+1);
};

// base comparison: just call function
Benchmark::run_as_base("threading: no task", n_reps, [](){
    task_f();
});

// run task using std::thread
Benchmark::run("threading: std::thread", n_reps, [](){
    auto t = std::thread(task_f);
    t.join();
});

// run task using jluna::Task
Benchmark::run("threading: jluna::Task", n_reps, [](){
    auto t = ThreadPool::create<void()>(task_f);
    t.schedule();
    t.join();
});
```

Here, we are executing a function that simply iterates to 10000, where `volatile` assures the loop cannot be optimized away by the compiler. As a base comparison, we simply execute this function. Next we created a `std::thread`, which is then immediately scheduled and executes the function. Lastly, we do the same using `jluna::Task`, which, internally, using a Julia `Base.Task` to do the same.

### Constructing `jluna::Task`: Results

(number of cycles: `2000000`)

| name | median duration (ms) | overhead|
|------|----------------------|-------------|
| `base` | `0.01491ms`           | `0%`  |
| `std::thread` | `0.03202ms`   | `114.746%`  |
| `jluna::Task` | `0.02702ms`   | `81.1842%`  |

`jluna::Task` turned out to have about 30% less overhead on thread construction than `std::thread`, this is despite having to go through all the wrapping. Now, this doesn't mean the jluna threadpool is a better option than `std::thread` for C-side only computations, all it means is that users do not have to worry about potential overhead when using `jluna::ThreadPool`.




