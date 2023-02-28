# Basics

### Initializing the Julia State

Before any Julia or jluna functionality can be accessed, we need to initialize the Julia state using [`jluna::initialize()`]. This sets up the Julia environment and initializes many global jluna variables.

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
```
[JULIA][LOG] initialization successful (1 thread(s)).
```

> **C++ Hint**: `using namespace jluna` makes it, such that, in the scope the statement was declared, we do not have to prefix jluna functions or objects with their namespace `jluna::`. This is why we can write `initialize()`, instead of `jluna::initialize()`.

Where `#include <jluna.hpp>` automatically includes all headers intended for end-users, including `julia.h`.

Note that the C-APIs `jl_atexit_hook` should never be called. At the end of runtime, jluna safely frees the Julia state automatically.

While a no-argument `initialize` will usually work, on some systems, this function may fail with a message. This is usually
caused by the systems directory structure and can be addressed using two of the four optional arguments of `initialize`. See
the section on [troubleshooting](troubleshooting.md) for more information.

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

This function is called **safe**_eval, because it forwards any exceptions, that may have occurred during parsing or execution of the string, to the C++ state as a `jluna::JuliaException`:

```cpp
Main.safe_eval("sqrt(-1)");
```
```test
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

Along with the exception, we also get a full Julia- and C++-side stacktrace.

> **C++ Hint**: In C++, an exception can be caught using a `try`-`catch` block. An error, on the other hand, cannot be caught. It will always terminate the application.

#### Executing a File

We can execute an entire Julia file using `safe_eval_file`. This function calls the Julia-side function `Main.include`, using the path we provided:

```cpp
Main.safe_eval_file("path/to/julia_file.jl");
```

#### Executing Multiple Lines of Code

If we want to execute more than just a single line of code, or, if we want any special characters to be escaped automatically, C++s **raw string literal** conveniently accomplishes both:

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
> **C++ Hint**: Any characters in the text in-between the `R"(` `)"` are automatically escaped (if necessary). Without `R"()"`, we would have to write the above as `"println(\"first line\");\nprintln(\"second line\")";`.


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

Where `Int64` is a type alias of `long int`. Most of the C++ primitive types have been aliased in jluna, such that their name corresponds to their Julia-side equivalent. `Float64` instead of `double`, `UInt32` instead of `unsigned int`, etc.

While the general case will be explained later, for now, be aware that **we have to manually specify the type** of the C++-side variable that captures the result, rather than using `auto`. In the example above, we used `Int64`.

> **C++ Hint**: `auto` is a keyword that asks the compiler to automatically deduce the type of variable at compile time.

If the result of a Julia-side expression is of a different type than the declared C++-side variable, jluna will try to convert the result, such that its type matches the declared C++-side type:

```cpp
UInt64 result = Main.safe_eval("return Char(14)");
std::cout << result << std::endl;
```
```
14
```

Here, a Julia-side `Base.Char` was implicitly converted to a C++-side `UInt64`.

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

// use my_modules safe_eval, instead of Mains
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

Instead, users are encouraged to execute any given piece of Julia code by using a **function**. We are able to access a function (once) by using `save_eval("return function_name")`, then use that function over and over:

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

Where we now used `auto` as `f`s type. We will learn why this is necessary in the [section on proxies](./proxies.md).

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

Note how we could not replace `Main.safe_eval("return [1, 2, 3]")` with `Main["[1, 2, 3]"]`. This is because the latter is invalid. **We can only access named Julia-side variables** using `operator[](std::string)`. There is no variable named `[1, 2, 3]` in Main scope, therefore `operator[]` would fail and raise an exception.

In summary:
+ `Module.safe_eval` executes a string. We can use `return` to return the value of that expression
+ `Module["name"]` accesses an already existing named variable, returning its value

### Accessing Julia Struct Fields

We learned how to get the value of a named variable in a module, by using `jluna::Module::operator[](std::string)`. This is convenient, however, modules are not the only jluna type that supports `operator[]`.

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

We can use `operator[]` to **access** Julia-side variables in a module or fields of a structtype. However, we can actually also **mutate** the variable itself:

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

We still have our `jl_instance` from the last section. We can access it using `Main::operator[]`:

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

