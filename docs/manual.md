# jluna: Manual & Tutorial

This manual will cover most of jlunas features, how to use them and what to use them for. It is structured in a way that ramps up in complexity, starting with how to accomplish basic things like calling functions, basic interaction between the C++ and Julia state, more specialized applications like the array and module interface, how to deal with usertypes, then ending with multi-threaded applications and performance optimization.

It is recommend that you read this manual in order, an interactive table of contents is provided for easy access.

This manual assumes that all users are familiar with the basics of Julia. If this is not the case, it may be necessary to first work through the excellent [official manual](https://docs.julialang.org/en/v1/) to at least get an overview of Julias syntax and structure.

jluna makes extensive use of newer C++20 features and high-level techniques such as [SFINAE](https://en.cppreference.com/w/cpp/language/sfinae) and template meta functions. This may provide a barrier of entry to users less familiar with the intricacies of the language. To address this, hints in the form of quotation blocks are provided where necessary:

> **Hint**: Quotation blocks like these are reserved for additional information or explanations of C++ and/or Julia features. Users who consider themself very familiar with both, can safely skip them. 

---

## Table of Contents

---

### Installation

This manual does not go over how download and build jluna. Instead, a step-by-step tutorial on how to install and create a C++ application using jluna from scratch is available [here](./installation.md).

---

### Initializating the Julia State

Before any Julia or jluna functionality can be accessed, we need to initialize the Julia state. This sets up the Julia environment and loads most of jlunas functionalities. 

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
> **C++ Hint**: `using namespace jluna` makes it, such that, in the scope the statement was declared, we do not have to prefix jluna functions/object with their namespace `jluna::`. This is why we can write `initialize()`, instead of `jluna::initialize()`.

Where `#include <jluna.hpp>` makes all of jlunas functionality available to the user. 

Note that you should never call the C-APIs `jl_atexit_hook`. At the end of runtime, jluna safely closes the Julia state automatically.

---

### Executing Julia Code

#### Executing a single Line of Code

Similarly to using the REPL, we can execute an arbitrary string as code using `safe_eval`:

```cpp
Main.safe_eval("println(\"hello jluna\")");
```
```
hello jluna
```

Where `Main` is a pre-defined jluna object representing the `Main` module. 

The function is called **safe**_eval, because it forwards any exceptions that may have occurred during execution of the string as a `jluna::JuliaException`:

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

#### Executing a File

We can execute an entire file using `safe_eval_file`. This function calls the Julia-side function `Main.include`, using the path we provided:

```cpp
Main.safe_eval_file("path/to/julia_file.jl");
```

#### Executing Multiple Lines of Code

If we want to execute more than just a single line, or, if we want any special characters to be escaped automatically, C++s **raw string literal** conveniently accomplished this:

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

Often, we want to access whatever result a piece of code returns. To do this in jluna, we capture the C++-side return value of `safe_eval` using a C++-side variable:

```cpp
Int64 result = Main.safe_eval("return 1234");
std::cout << result << std::endl;
```
```
1234
```
> **C++ Hint**: `std::cout << x << std::endl` is C++s version to Julias `println(x)`

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

So far, we have used `Main.safe_eval`, which evaluates its argument in `Main` scope, similarly to how any code entered into the REPL would be evaluated. Sometimes, this may not be desired. Consider the following example:

```cpp
Main.safe_eval(R"(
    module MyModule
        var = 1234
    end
)");
Main.safe_eval("var = 9999;");
Main.safe_eval("println(MyModule.var)");
```
```
1234
```

What happened here? Instead of assigning the variable `MyModule.var`, we actually created a new variable `Main.var` and assigned it instead. This is because the line `var = 9999` was executed **in Main scope**, that is, the function `Main.eval` was used Julia-side to evaluated the expression. 

To avoid this, we can instead do:

```cpp
// access MyModule as jluna::Module
Module my_module = Main.safe_eval("return MyModule");

// use its safe_eval instead of Mains
my_module.safe_eval("var = 777");

Main.safe_eval("println(MyModule.var)");
```
```
777
```

Where we have explicitly declared the C++-side variable `my_module` to be of type `jluna::Module`.

### Calling Julia Functions

While executing code using `safe_eval` is convenient, it has a significant performance impact. This is, because Julia needs to first `Meta.parse` the string we hand it, then evaluate the resulting expression.


> **Julia Hint**: `Meta.parse` takes a string and treats it as a line of code. It returns an object of type `Expr`, which can then be `eval`ed and compiled. To learn more about expressions, see the [official manual page on metaprogramming](https://docs.julialang.org/en/v1/manual/metaprogramming/)

Instead, users are encouraged to execute any given piece of Julia code by using a **function**. We are able to access a function (once) by using `save_eval("return function_name")`, then use that function over and over, at no performance cost compared to using only Julia:

```cpp
Main.safe_eval("f(x) = x^x");
auto f = Main.safe_eval("return f");
Int64 result = f(12);
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

Where `"cpp side string"` is a string on the C++-side, while `Main` and `[1, 2, 3]` are memory allocated only Julia-side. 

#### Accessing Named Variables in a Module

The above example illustrates how using `safe_eval("return x")` can be quite clumsy syntactically. To address this, jluna offers the much more elegant `operator[](std::string)`. 

> **C++ Hint**: `operator[](std::string)` is a function that is called by using `x["<string here>"]`, where x is a value that supports the operator.

Rewriting the above using it, we get:

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

Note how we could not replace `Main.safe_eval("return [1, 2, 3]")` with `Main["[1, 2, 3]"]`. This is because the latter is **invalid**. We can only access named, Julia-side variables using `operator[](std::string)`. There is no variable named `[1, 2, 3]` in Main scope, therefore `operator[]` would fail.

In summary:
  + `Module.safe_eval` executes a string. Use `return` to return the value of that expression
  + `Moulde["name"]` access an already existing, named variable, returning its value

### Accessing Julia Struct Fields

We learned how to access a named variable in a module by using `jluna::Module::operator[](std::string)`. This is convenient, however modules are not the only jluna type that supports `operator[]`. 

Let's say we have the following structtype:

> **Julia Hint**: A structtype is any type declared using `struct` or `mutable struct`

```cpp
Main.safe_eval(R"(
    mutable struct MyStruct
       _field::Int64
    end
    
    jl_instance = MyStruct(9876);
)");
auto cpp_instance = Main["jl_instance"];
```

Here, we defined a new structtype `MyStruct`, which has a single field `_field`. We then created a Julia-side variable, `jl_instance`, to which we bound a newly constructed `MyStruct`. 
To access the Julia-side `jl_instance`, we used `Main["jl_instance"]`, which we were allowed to use, since `jl_instance` is a proper named variable.

We can access the field of `cpp_instance` like so:

```cpp
Int64 field_value = cpp_instance["_field"];
std::cout << field_value << std::endl;
```
```
9876
```

Note again, how we manually declared the type of the C++-side variable `field_value` to be `Int64`. We did not use `auto`.

### Mutating Julia-side Values

We can use `operator[]` to access Julia-side values, be it variables in a module or fields of a structtype. However, we can actually also **mutate** the variable itself:

> **Hint**: To mutate a variable means to change its value without changing its name

```cpp
Main.safe_eval(R"(
    mutable struct MyStruct
       _field::Int64
    end
    
    jl_instance = MyStruct(9876);
)");

Main["jl_instance"]["_field"] = 666;

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
> Int64 value = instance_field;
> ```
> In the latter example, `static_cast` (and thus the conversion operator) was implicitely called. This means we have already been using `static_cast` in the previous sections, just implicitly.
> 
> See the [C++ manual](https://en.cppreference.com/w/cpp/language/static_cast) for more information.

The C++-variable `instance_field` directly refers to the Julia-side variable `Main.jl_instance._field` (recall that `MyStruct` is mutable). Because of this, **if we mutate the C++-side variable, we will also mutate the corresponding Julia-side variable**:

```cpp
auto instance = Main["jl_instance"];
auto instance_field = instance["_field"];
instance_field = 1234;

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

#### Mutating Variables using `operator[](size_t)`

Accessing and assigning the result of `operator[]` like this works for variables in Modules, fields of structs and even elements of collections:

```cpp
Main.safe_eval("vec_var = [1, 2, 3, 4]");
Main["vec_var"][0] = 9999;
Main.safe_eval("println(vec_var)");
```
```
[9999, 2, 3, 4]
```

Where the Julia-side `vec_var` is a `Base.Array`.

> **Hint**: In C++, indices are 0-based. In Julia, they are 1-based. This can be quite confusing considering we are operating in both languages at the same time. For now, using `operator[]` on a C++-side object requires indices to be 0-based. 
>
> See the [section on arrays]() for more information

---

## Proxies

So far, this manual has intentionally obfuscated what type `auto` resolves to in a statement like this:

```cpp
jluna::safe_eval("new_var = 1234");
auto new_var = Main["new_var"];
```

Indeed, `new_var` is not an integer, it is deduced to be a `jluna::Proxy`. To fully understand how this class works, we first need to discuss how / if memory is shared between the C++ and Julia state.

### Shared & Separate Memory

By default, most memory allocated C++-side is incompatible with Julia. For example, a C++-side variable of type `char` is an 8-bit number, while a Julia-side `Char` has 32 bits. If we were to directly exchange memory between states, the value of the char would be corrupted. For more complex classes, for example comparing `std::set` to `Base.set`, the memory layout is even more different and directly sharing memoy is impossible.

Because of this, it is best to assume that we have to first **reformat** any C++-side memory before moving it Julia-side. 

In jluna, the **inverse is not true**. We do not have to reformat Julia-side memory to interact with it C++-side, which is the entire purpose of the library. This is made possible through `jluna::Proxy`.

`jluna::Proxy` is a proxy, or stand-in, of arbitrary Julia-side memory. Internally, it simply holds a pointer to whatever memory it is managing. Copying the proxy copies this pointer, not the Julia-side memory. Moving the proxy does not affect the Julia-side memory. If we want to actually change the Julia-side memory being managed, we use the proxies member functions.

### Constructing a Proxy

We rarely construct proxies from raw pointers (see the [section on `unsafe` usage](#unsafe)), instead, a proxy will be the return value of a member function of another proxy.

How can we make a proxy if we need a proxy to do so? As we've seen before, jluna offers pre-initialized proxies for `Main`, `Base` and `Core`. We will limit ourself to `Main` for this section. We can use these proxies to generate new proxies for us.

To create a proxy from a value, we use `Main.safe_eval("return x")`, where x is a value or a variable (though only the value of that variable will be returned):

```cpp
auto proxy = Main.safe_eval("return 1234");
```

To reiterate, the resulting proxy, `proxy`, **does not contain the Julia-side value `1234`**. It only keeps a pointer to wherever `1234` is allocated.

We can actually check the the pointer using `static_cast<unsafe::Value*>`, where `unsafe::Value*` is a raw pointer to arbitrary Julia-side memory:

```cpp
std::cout << static_cast<unsafe::Value*>(proxy) << std::endl;
```
```
0x7f3a9e5cd8d0
```

(this pointer will, of course, be different anytime `1234` is allocated)

As long as `proxy` stays in scope, `1234` cannot be deallocated by the garbage collector. This is, despite the fact that there is no Julia-side reference to it.

> **Julia Hint**: In pure Julia, any value that is not the value of a named variable, inside a collection or reference by a `Base:Ref` is free to be garabge collected at any point. `jluna::Proxy` pevents this

Not only does `proxy` point to Julia-side memory, it behaves exactly like it when used with Julia-side functions:

```cpp
Base["println"](Base["^"](proxy, 2));
```
```
1522756
```

Here we're using the Julia-side function `Base.println` to print the result of the entirely Julia-side computed `(^)`. 

> **Julia Hint**: In Julia, an infix operator that has the symbol `x`, has the function definition `(x) = # ...`. The braces signal to Julia that whatever symbol is in-between them is an infix operator. Only certain characters can be used as infix operators, see [here](https://discourse.julialang.org/t/is-not-an-operator/20221/2)

No memory was moved during this call. We called a Julia-function (`println` and `(^)`) with a Julia-side value `1234`. Other than having to access the proxies in the first place, the actual computation is exactly the same as if it was done in only Julia.


### Updating a Proxy

The most central feature of jluna is that of **updating** a proxy. If we assign a proxy a C++-side value, it will move this value Julia-side, then set its pointer to that newly allocated memory:

```cpp
auto proxy = Main.safe_eval("return [1, 2, 3]");
proxy = std::vector<Int64>{4, 5, 6};

Base["println"](proxy);
Base["println"](Base["typeof"](proxy));
```
```
[4, 5, 6]
Base["println"](Base["typeof"](proxy));
```

Our proxy, `proxy`, here, is not pointing to the C++-side vector. jluna has implicitly converted the C++-side `std::vector<Int64>` to a Julia-side `Base.Vector{Int64}`. It has even accurately deduced the type of the resulting vector, based on the type and value-type of the C++-side vector.

> **Hint**: Let `std::vector<T> x`, then `x`s type is `std::vector<T>`, `x`s value-type is `T`

What about the other way around? Recall that `proxy` is now pointing to a Julia-side `Int64[4, 5, 6]`. We can actually do the following:

```cpp
std::vector<UInt64> cpp_vector = proxy;
for (UInt64 i : cpp_vector)
    std::cout << i << " ";
```
```
4 5 6 
```
We have used the proxy as the right-hand side of an assignment. During execution, jluna has moved the Julia-side value `proxy` is pointing to (`Int64[4, 5, 6]`) to C++, potentially converting its memory layout. 

Note how we changed value-types, Julia-side the value type was `Int64`, C++-side it is `UInt64`. jluna has detected this and implicitly converted all elements of the Julia-side `Array` during assignment of the C++-side `std::vector`

### Boxing & Unboxing

> **C++ Hint**: An assignment is any line of code of the form `x = y`. `x` is called the "left-hand expression", `y` is called the right-hand expression, owning to their respective position relative to the `=`

#### Boxing

The process of moving a C++-side value to Julia is called **boxing**. We **box** a value by assigning it to a proxy , that is:
+ the proxy is left-hand expression of the assignment
+ the value to be boxed is the right-hand epression of the assignment 

```cpp
<Type Here> value = // ...
jluna::Proxy proxy = value;
```
#### Unboxing

The process of moving a Julia-side value to C++ is called **unboxing**. We **unbox** a proxy (and thus a Julia-side value) by assigning it to a non-proxy, C++-side variable, that is:

+ the C++-side variable is the left-hand expression of the assignment
+ the proxy is the right-hand expression of the assignment

```cpp
jluna::Proxy proxy = // ...
<Type Here> value = proxy;
```

These concepts are important to understand, as they are central to how to move values between the Julia- and C++-state.

### (Un)Boxable Types

Note all types can be boxed and/or unboxed. A type that can be boxed is called a **Boxable**. A type that can be unboxed is called **Unboxable**. jluna offers two concepts `is_boxable` and `is_unboxable` that represent these properties.

> **C++ Hint**: A concept is a new feature of C++20. It is used like so:
> ```cpp
> template<//...
> concept is_boxable = //...
> 
> // jluna function:
> template<is_boxable T>
> void do_something(T value);
> ```
> Because we specified the template argument of `do_something` to be a `is_boxable`, at compile time, C++ will evaluate this condition for the template argument type. If `do_something` was called with a value of a type that is not boxable, a compiler error will be raised

A type that is both boxable and unboxable is called **(Un)Boxable**. This is an important conceptualization, because:

+ we can only use **Boxables** as the **right-hand expression** of a proxy-assignment
+ we can only use **Unboxables** as the **left-hand expression** of an assignment.
  
Of course, an (Un)Boable can be used on either side.

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

Most relevant `std::` types are supported out-of-the-box. All jluna types that reference Julia-side objects can be unboxed into their corresponding Julia-side value.

### Named & Unnamed Proxies

We've seen before that we can mutate Julia-side variable with proxies. To understand how this works, we need to be aware of the fact that there are two types of proxies in jluna: **named** and **unnamed**.

+ A **unnamed proxy** manages a Julia-side **value**
+ A **named proxy** manages a Julia-side **variable**

Here, "manage" means that, whatever the proxy points to is safe from the garabge collector, and, when the proxy is mutated, the thing being managed is mutated at the same time.

#### Unnamed Proxies

We create an unnamed proxy using `Module::safe_eval("return x")`:

```cpp
auto unnamed_proxy = Main.safe_eval("return [7, 7, 7]");
```

We've seen this type of proxy before, if we assign it a value:

```cpp
auto unnamed_proxy = std::vector<std::string>{"abc", "def"};
```

That value will be boxed and move to Julia, after which the proxy will point to that value.

If we have a Julia-side variable `jl_var`:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct proxy
auto unnamed_proxy = Main.safe_eval("return jl_var");
```

Then using `Main.safe_eval("return` will **only forward its value** to the unnamed proxy. If we modify the unnamed proxy again, the variable will not be modified:

```cpp
// delcare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// forward value to proxy
auto unnamed_proxy = Main.safe_eval("return jl_var");

// reassign proxy
unnamed_proxy = 1234;

// print
Base["println"]("cpp: ", unnamed_proxy);
Main.safe_eval(R"(println("jl : ", jl_var))");
```
```
cpp: 1234
jl : [7, 7, 7]
```

The proxy has changed the value it is pointing to, the value of `jl_var` remained unchanged.

### Named Proxy

We construct a named proxy using `operator[]`:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct proxy
auto named_proxy = Main["jl_var"];
```

This proxy behaves exactly the same as an unnamed proxy, except **when the named proxy is mutated, it will mutate the corresponding Julia-side variable**:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct proxy
auto named_proxy = Main["jl_var"];

// reassign proxy
named_proxy = 1234;

// print
Base["println"]("cpp: ", unnamed_proxy);
Main.safe_eval(R"(println("jl : ", jl_var))");
```
```
cpp : 1234
jl  : 1234
```

The value the proxy is pointing to, **and** the value of the variable `jl_var` has changed. This is because a named proxy remembers the name of the variable it was constructed from. If we modify the proxy, it will also modify the variable. 

In summary: 

+ We construct an **unnamed proxy** using `Module.safe_eval("return <value>")`. Mutating an unnamed proxy will mutate its value, but it will not mutate any Julia-side variables.
<br><br>
+ We construct a **named proxy** using `Module["<variable name>"]`. Mutating a named proxy will mutate its value **and** mutate its corresponding Julia-side variable.

### Additional Member Functions

We can check which (if any) variable a proxy is managing using `get_name`:

```cpp
Main.safe_eval("jl_var = [7, 7, 7]");
auto named_proxy = Main["jl_var"];
auto unnamed_proxy = Main.safe_eval("return jl_var");

std::cout << "named  : " << named_proxy.get_name() << std::endl;
std::cout << "unnamed: " << unnamed_proxy.get_name() << std::endl;
```
```
named  : jl_var
unnamed: <unnamed function proxy #104>
```

We see that the named proxy indeed manages `Main.jl_var`. The unnamed proxy does not manage any variable, its name is an internal id.

If we just want to know whether a proxy is mutating, we ca use `is_mutating()` which returns a `bool`:

```cpp
std::cout << "named  : " << named_proxy.is_mutating() << std::endl;
std::cout << "unnamed: " << unnamed_proxy.is_mutating() << std::endl;
```
```
named  : 1
unnamed: 0
```

### Implications

With our newly acquired knowledge about named and unnamed proxies, we can investigate code from the previous sections more closely:

```cpp
Main.safe_eval(R"(
    mutable struct MyStruct
       _field::Int64
    end
    
    jl_instance = MyStruct(9876);
)");

Main["jl_instance"]["_field"] = 666;
```
This code snippet from [the section on mutating Julia-side values](#mutating-julia-side-values) uses a named proxy to update the field of `jl_instance`. 

Because we are using `operator[]`, `Main["jl_instance"]` returns a named proxy managing the Julia-side variable `Main.jl_instance`. Calling `["_field"]` on this result creates another named proxy, this time managing `Main.jl_instance._field`. We know that mutating a named proxy mutates its corresponding variable, thus, assigning the resulting proxy the C++-side value `666`, boxes that value and assigns it to `Main.jl_instance._field`.

In this example, the actual proxies were temporary - they only stayed in scope for the duration of one line. 

This syntax is highly convenient and we have used it frequently already:

```cpp
Base["println"]("hello julia");
```

In this expression, we create a named proxy to the Julia-side variable `Base.println`. When then use the proxies call operator `operator()` to call the proxy as a function, using the argument `"hello julia"`. This argument is a C++-side string, so jluna implicitly boxes it to a Julia-side string, then uses it as the argument for the variable the named proxy is managing: `Base.println`.

### Proxy Inheritance

Many of jlunas classes are actually proxies, because they **inherit** from `jluna::Proxy`.

> **C++ Hint**: Inheritance is not the same as subtyping in Julia. If class A inherits from class B, then A has access to all non-private member and member functions of B. A can furthermore be `static_cast` to B, and any pointer to A can be directly treated as if it was a pointer to B.

Therefore, any of these classes provide all of the same functionalities of proxies, along with a some additional ones. The next few sections will introduce `jluna::Proxies` "children", which all have a more specialized purpose. It is important to keep in mind that both the concepts of named and unnamed proxies, as well as the implied behavior also applies to any of these classes.

---

## Specialized Proxies: Arrays

One of the nicest features of Julia is its array interface. Therefor, it is only natural a Julia wrapper should extend all that functionality to a C++ canvas. While we learned before that we can use a `jluna::Proxy` like an array using `operator[](size_t)`:

```cpp
auto array_proxy = Main.safe_eval("return [1, 2, 3, 4]");

Int64 third = array_proxy[2];
std::cout << third << std::endl;
```
```
3
```

This is as far at it goes for array-related functionalities.

To tranform a `jluna::Proxy` to a `jluna::Array`, we use `.as`:

```cpp
Proxy array_proxy = Main.safe_eval("return Int64[1, 2, 3, 4]");
auto array = array_proxy.as<Array<Int64, 1>>();
```

or, more conveniently, we use an implicit static cast:

```cpp
Array<Int64, 1> array = Main.safe_eval("return Int64[1, 2, 3, 4]");
```

We see that `jluna::Array<T, N>` has two template parameters:

+ `T` is the **value type** of the array, all elements are assumed to be of this type
+ `N` is the **dimensionality** of the array, sometimes called the **rank**. A 1d array is a vector, a 2d array is a matrix, etc.

Both these template arguments directly correspond to the Julia-side parameters of `Base.Array`.

Note that we have declared the Julia-side vector `[1, 2, 3, 4]` to be of value-type `Int64`. This is important, because we specified the C++-side array to also have that value-type. On construction, `jluna::Array` will check if the value types match (that is, wether for all elements `e_n` in the Julia-side array, it holds that `e_n <: T`), if they do not, an exception is thrown.

Because the match does not have to be exact (see the section on [type ordering](#type-order)), we can simply declare the value type to be `Any`, which then lets us move any assortment of objects into the same array. jluna actually provides a typedef for this:

> **C++ Hint**: A `typedef` or "`using` declaration" is a way to declare a type alias. It is giving a type a new name while, keeping the old name valid. This is most commonly used to simplify syntax for the convenience of the programmer

```cpp
using Array1d = // equivalent to Base.Array<Any, 1>
using Array2d = // equivalent to Base.Array<Any, 1>
using Array3d = // equivalent to Base.Array<Any, 1>
```

## Accessing Array Indices

Firstly, we need to setup our two arrays that we will use for this section:

```cpp
Array<Int64, 1> array_1d = jluna::safe_eval("return Int64[1, 2, 3, 4, 5]");
Array<Int64, 2> array_2d = jluna::safe_eval("return reshape(Int64[i for i in 1:9], 3, 3)");

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
Int64 one_d_at_3 = array_1d.at(2);
Int64 two_d_at_2_2 = array_2d.at(1, 1);

std::cout << one_d_at_3 << std::endl;
std::cout << one_d_at_2_2 << std::endl;
```
```
3
5
```
For a 1d array, `at` takes a single argument, its **linear index** (see below). For a 2d array, `at` takes two arguments, one for each dimension. This extends to any dimensionality, for a 5d array, we would call `at` with 5 integers.

For syntactic consistency, `jluna::Array` also offers `operator[]`, which is called in the exact same way as `at` would be. The difference is that for `at` only, the indices are bounds-checked. This adheres to the convention set forth by `std::vector<T>::at` and `std::vector<T>::operator[]`.

### Linear Indexing

While n-dimensional indexing is only available for array of rank 2 or higher, linear indexing is available for all arrays. We can linear-index any array using `operator[](size_t)`

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

Base["printlnt"](sub_vector);
```
```
[2, 6, 3, 8]
```
> **C++ Hint**: Here, the sytnax used, `{1, 5, 2, 7}`, is called a "brace-enclosed initializer list", which is a form of [aggregate initialization](https://en.cppreference.com/w/cpp/language/aggregate_initialization) in C++. It can be best though of as a proto-vector, the compiler will infer the vectors type and construct it for us. In our case, because `Array::operator[]` expects a list of integer, it will interpret `{1, 5, 2, 7}` as the argument for an implicitly called constructor, creating a `std::vector<size_t>`.<br>
> 
> see also: [list initialization](https://en.cppreference.com/w/cpp/language/list_initialization)

### 0-base vs. 1-base

This is about as good a place as any to talk about index bases. Consider the following:

```cpp
Array<Int64, 1> array = jluna::safe_eval("return Int64[1, 2, 3, 4, 5, 6]");

std::cout << "cpp: " << (Int64) array.at(3) << std::endl;
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

Where `_gen` is a string-literal operator that  constructs a generator expression from the code it was called with. 

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
> **C++ Hint**: `std::endl` adds a `\n` to the stream, then flushes it

If we declare the iterator type as `auto`, each iterator is assignable:

```cpp
for (auto iterator : array)
    it = (Int64) it + 1;

Base["println"](array);
```
```
[2, 3, 4, 5, 6, 7]
```

This modifies the underlying Julia-array with minimal overhead.

If the array is also a named proxy, it will also modify that element of whatever variable the proxy is managing.

### Accessing the Size of an Array

To get the size of an array, we use `get_n_elements`:

```cpp
Array<size_t, 1> vec = jluna::safe_eval("return UInt64[i for i in 1:333]");
std::cout << vec.get_n_elements() << std::endl;
```
```
333
```

### jluna::Vector

For arrays of dimensionality 1, a special proxy called `jluna::Vector<T>` is provided. It directly inherits from `jluna::Array<T, 1>` so all of `Array`s functionalities are also available to `Vector`. In addition, the following member functions are only available for `jluna::Vector`:

+ `insert(size_t pos, T value)`
  - insert element `value` at position `pos` (0-based)
+ `erase(size_t pos)`
  - delete the element at position `pos` (0-based)
+ `push_front(T)`
  - push element to the front, such that it is now at position 0
+ `push_back(T)`
  - push element to the back of the vector
  
When boxing a `jluna::Vector<T>`, the resulting Julia-side value will be of type `Base.Vector{T}`. When boxing a `jluna::Array<T, 1>`, the result will be a value of type `Base.Array{T, 1}`.

---

## Specialized Proxies: Symbols

Another specialized type of proxy is the **symbol proxy**. It manages a Julia-side `Base.Symbol`.

We can create a symbol proxy in the following ways:

```cpp
// new unnamed proxy
auto unnamed_symbol = Symbol("symbol_value");

// new named proxy
auto named_symbol = Main.new_symbol("symbol_var", "symbol_value")

// from already existing proxy
auto non_symbol_proxy = Main.safe_eval("return :symbol_value");
auto symbol_proxy = non_symbol_proxy.as<Symbol>();
```

Where, unlike with `jluna::Proxy`, the value the proxy is pointing to is asserted to be of type `Base.Symbol`. 

### Symbol Hashing

The main additional functionality `jluna::Symbol` brings is that of *constant time hashing*.

A hash is essentially a `UInt64` we assign to things as a label. In Julia, hashes are unique and there a no hash collisions. This means if `A != B` then `hash(A) != hash(B)` and, furthermore, if `hash(A) == hash(B)` then `A === B`.

Unlike with other classes, `Base.Symbol` can be hashed in constant time. This is because the hash is precomputed at the time of construction. 

We can access the hash of a symbol proxy using `.hash()`

```cpp
auto symbol = Symbol("abc");
std::cout << "name: " << symbol.operator std::string() << std::endl;
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

## Specialized Proxies: Modules

We have already used `jluna::Module` before, now it is time to learn about all of its additional features. `jluna::Module` (`Module`, henceforth) inherits publicly from `jluna::Proxy`, meaning it has all the same functions and functionalities we discussed previously, along with a few additional ones.

### Assign in Module

Let's say we have a module `M` in main scope that has a single variable `var`:

```cpp
Main.safe_eval(R"(
    module M
        var = []
    end
)");

Module M = Main.safe_eval("return M");
```

We've already seen that we can modify this variable using `M.safe_eval`, however, this is a fairly badly performing option, because we force Julia to `Meta.parse`, then `eval`.

Much faster is `Module::assign`:

```cpp
// works but slow:
M.safe_eval("var = [777]");

// works and super fast:
M.assign("var", 777);
```

Where the first argument of assign is the variables name, the second argument is the new desired value.

If the variable we want to assign does not exist yet, we can performantly create it using `create_or_assign`:

```cpp
M.create_or_assign("new_var", 777);
Base["println"](M["new_var"]);
```
```
777
```

As the name suggest, if the variable does not exist, it is created. If the variable does exist, `create_or_assign` acts identical to `assign`. If we call `assign` for a variable that does not yet exist, an exception is raised.

### Creating a new Variable

A convenient and well-performing function is `Module::new_*`. `Module::new_undef("var_name")` creates a new variable named `var_name` in the module and assigns it `undef`. The return value of this function is a named proxy pointing to that variable, making the following syntax possible:

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

This is the most performant way to create variables new variables module scope. 

### Importing & Using

Additionally, `jluna::Module` provides the following two functions wrapping the `using` and `import` functions of modules:

+ `M.import("PackageName")`
  - equivalent to calling `import PackageName` inside `M`
+ `M.add_using("PackageName")`
  - equivalent to calling `using PackageName` inside `M`

---

## Calling C++ Functions from Julia

We've seen how to call Julia functions from C++, but in jluna, the other way around works just as well.

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

Always manually specify the trailing return type of a lambda using `->`, never use `auto` for either the lambdas return- or any of the argument-types when interfacing with jluna.

To make this function available to Julia, we use `as_julia_function`. 
+ the argument of `as_julia_function` is a lambda or `std::function` object
+ the template argument of `as_julia_function` is the functions signature

Because `add` has the signature `(Int64, Int64) -> Int64`, we use `as_julia_function<Int64(Int64, Int64)>`. 

> **C++ Hint**: `std::function` and thus `as_julia_function` uses the C-style syntax for a functions signature. A function with return-type `R` and argument types `T1, T2, ..., Tn` has the signature `(T1, T2, ..., Tn) -> R`, or `R(T1, T2, ..., Tn)` in C-style.

We can then assign the result of `as_julia_function` to a Julia variable like so:

```cpp
Main.create_or_assign("add", as_julia_function<Int64(Int64, Int64)>(add));
```

From this point onwards, we can simply call the C++-side `add` by using the newly created Julia-side variable `Main.add`:

```cpp
Main.safe_eval("println(add(1, 3))");
```
```
4
```

The return value of `as_julia_function` is a Julia-side object. This means we can assign it to already existing proxies, or use it as an argument for a Julia-side function.

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

This may seem limiting at first, how could we execute arbitrary C++ code when we are only allowed to use functions with this signature , and only (Un)Boxable types? 

### Taking Any Number of Arguments

Let's say we want to write a function that takes any number of `String`s and concatenates them. Obviously, just 3 arguments are not enough for this. Luckily, there is a workaround. Instead of using a n-argument function, we can use a 1-argument function where the argument is a n-element vector:

```cpp
auto concat_all = [](jluna::Array<std::string, 1> arg) -> std::string
{
    std::stringstream str;
    for (std::string : arg)
        str << arg;
    
    str << std::endl;
    return str.str();
};
```
> **C++ Hint**: `std::stringstream` is a stream that we can write strings into, using `operator<<`. We then flush it using `std::endl` and convert it's contents to a single `std::string` using the member function `str`. More info can be found [here](https://en.cppreference.com/w/cpp/io/basic_stringstream)

This lambda has the signatue `(jluna::Array<std::string, 1>) -> std::string`. We move it Julia-side using `as_julia_function`:

```cpp
Main.create_or_assign(
    "concat_all",
    as_julia_function<
        std::string(jluna::Array<std::string, 1>) // signature
    >(concat_all)
);
```

We can then call `concat_all` with any number of arguments by wrapping them in a Julia-side vector:

```cpp
Main.safe_eval(R"(
    println(concat_all(["GA", "TT", "AC", "A"]))
)");
```
```
GATTACA
```

If we want our lambda to take any number of *differently-typed* arguments, we can either wrap them in a `jluna::Array1d` (which has the value type `Any` and thus can contain elements of any type), or we can use a `std::tuple`, both of which are (Un)Boxable.

### Using Non-C++ Objects

We've learned how to workaround the restriction on the number of arguments, but what about the types? Not all types are [(Un)Boxable](), however we can still use arbitrary C++ types. How? By using **captures**:

Let's say we have the following C++ class:

```cpp
struct NonJuliaObject
{
    Int64 _value;
    
    NonJuliaObject(Int64 in)
        : _value(in)
    {}
    
    void double_value(size_t n)
    {
        for (size_t i = 0; i < n; ++i)
            _value = 2 * _value;
    }
};

auto instance = NonJuliaObject(13);
```

This object is obviously not (Un)Boxable.

The naive approach to modifying `instance` from Julia would be the following lambda:

```cpp
auto modify_instance = [](NonJuliaObject& instance, size_t n) -> void
{
    instance.double_value(n);
};
```

This lambda has the signature `(NonJuliaObject&, size_t) -> void`, which is a dissallowed signature because `NonJuliaObject&` is not (Un)Boxable.

Instead of handing `instance` to the lambdas function body through an argument, we can instead forward it through its capture:

```cpp
auto modify_instance = [instance_ref = std::ref(instance)] (size_t n) -> void
{
    instance_ref.get().double_value(n);
    return;
};
```

> **C++ Hint**: `std::ref` is used to create a [reference wrapper](https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper) around any instanced object. It is very similar to Julias `Base.Ref` in functionality. To "unwrap" it, we use `.get()` in C++, where we would use `[]` in Julia
    
Lambda syntax can get quite complicated, so let's talk through this step-by-step. 

Firstly, this second lambda with signature `(size_t) -> void`. Capture variables do not affects a lambdas signature.

Inside the capture `[]`, we have the expression `instance_ref = std::ref(instance)`. This expression creates a new variable `instance_ref` that will be available inside the lambdas body. We initialize `instance_ref` with `std::ref(instance)`, which creates a reference wrapper around our desired instance. A reference wrapper acts the same as a plain reference in terms of memory ownership, as long as the reference wrapper stays in scope, `instance` will too. 

Having capture `instance` through the reference wrapper, we can modify it inside our body by first unwrapping it using `.get()`, then applying whatever mutation we intend to, in our case we are calling `double_value` with the `size_t` argument of the lambda.

After all this wrapping, we can simply:

```cpp
Main.create_or_assign(
    "modify_instance", 
    as_julia_function<void(size_t)>(modify_instance)
)

Main.safe_eval("modify_instance(3)");
std::cout << instance._value << std::endl;
```
```
104
```

The now-Julia-function modified our C++-side instance, despite its type being uninterpretable to Julia.

By cleverly employing captures and collections / tuples, the restriction on the functions used for `as_julia_function` are lifted, any arbitrary C++ function (and thus any arbitrary C++ code) can now be executed Julia-side.

---

## Specialized Proxies: Types

We've seen specialized module-, symbol- and array-proxies. jluna has a fourth kind of proxy, `jluna::Type`, which wraps all of Julias very powerful introspection functionalities in one class.

While some overlap is present, `jluna::Type` is not a direct equivalent of `Base.Type{T}`. It is asserted to manage an object of type `T` such that `T isa Type`, though. It just provides more functions than are a available using only Julias `Base`.

### Constructing a Type

There are multiple ways to construct a type proxy:

```cpp
// get type of proxy
auto proxy = Main.safe_eval("return " + /* ... */);
auto type_proy = general_proxy.get_type();

// implicit cast
Type type_proxy = Main.safe_eval("return Base.Vector");

// using as<T>
auto type_proxy = Main["Base.Vector"].as<Type>();
```

Most often, we will want to know the type of a variable, for which we can either use `Base.typeof`, or `jluna::Proxy::get_type`.

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

This relational nature is heavily used in Julias multiple dispatch and type inference, for now, however, it gives us a way to put types in relation to each other. `jluna::Type` offers multiple functions for this:

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


TypeVars can be thought of as a sub-graph of the type-graph. An unrestricted types upper bound is `Any`, while its lower bound is `Union{}`. A declaration like `T where {T <: Integer}` restricts the upper bound to `Integer`, though any type that is "lower" along the sub-graph originating at `Integer` can still bind to `T`. This is useful to keep in mind.

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
+ `.second` is the parameters upper type bound
  
In case of `T`, the upper bound is `Base.Integer`, because we restricted it as such in the declaration. For `U`, there is no restriction, which is why its upper bound is the default: `Any`.

We can retrieve the number of parameters directly using `get_n_parameters()`. This saves allocating the vector of pairs.

### Fields

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

If we actually want the value of a field, we need use `operator[]` on a `jluna::Proxy` that is an instance of that type, not the type itself.

### Methods

(this feature is not yet implemented, until then, use `Base.methods(::Type)`)

### Properties

(this feature is not yet implemented, until then, use `Base.getproperty(::Type, ::Symbol)`)

### Type Classification

To classify a type means to evaluate a condition based on a types attributes, in order to get information about how similiar or different clusters of types are. jluna offers a number of convenient classifications, some of which are available as part of the Julia standard library, some of which are not. This section will list all, along with their meaning:

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

There is a subtle difference between how jluna evaluates properties and how pure Julia does. Consider the following:

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
To access the property `:name`, we need to first **unroll** the type, meaning we need to specialize all parameters of the type. Once we do so, it seizes to be a `UnionAll`:

```julia
# in Julia
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

Once fully unrolled, we have access to the properties necessary for introspection. jluna does this unrolling automatically for all types initialized by `jluna::initialize` (see the [previous sections list](#base-types)).

If desired, we can fully specialize a user-intialized type manually, using the member function `.unroll()`. Without this, many of the introspection features will be unavaiable.

---

## Usertypes

So far, we were only able to move (Un)Boxables to and from Julia. In some applications, this can be quite limiting. To address this, jluna provides a user-interface for making **any C++ type (Un)Boxable**. 

> **Hint**: A usertype is any type not defined by the standard library or jluna itself

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

While it may be possible to manually translate this class into a Julia-side `NamedTuple`, this is rarely the best option. For more complex classes, this is often not possible at all. To make classes like this (Un)Boxable, we use `jluna::Usertype<T>`, the **usertype interface**.

### Step 1: Making the Type compliant

For a type `T` to be manageable by `Usertype<T>`, it needs to be *default constructable*. `RGBA` currently has no default constructor, so we need to add it:

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
    
    RGBA() 
        : _red(0), _green(0), _blue(0), _alpha(1
    {}
};
```

If the type `T` is not default constructable, a static assertion is raised at compile time.

### Step 2: Enabling the Interface

To make jluna aware that we will be using the usertype interface for `RGBA`, we need to **enable it at compile time**. To do this, we use the `set_usertype_enabled` macro, executed in non-block scope.

> **C++ Hint**: Non-block scope (sometimes called global scope) is any scope that is not inside a namespace, function, struct, or block. As an example, `int main()` has to be declared in global scope

```cpp
struct RGBA
{
    /* ... */
};

set_usertype_enabled(RGBA); 
```

This sets up `Usertype<T>` for us. Among other things, it declares the Julia-side name of `RGBA`. This name is that same as the C++ side name, `"RGBA"` in our case.

### Step 3: Adding Property Routines

To add a property for `RGBA`s `_red`, we use the following function (at runtime):

> **C++ Hint**: A property of a structtype is what would be called a "field" in Julia or a "member" in C++. It is any named variable of any type (including functions) that is namespaced within a struct or class. All three terms will be used interchangeably here

```cpp
Usertype<RGBA>::add_property<Float32>(
    "_red_jl",
    [](RGBA& in) -> Float32 {
        return in._red;
    },
    [](RGBA& out Float32 red_jl) -> void {
        out._red = red_jl;
    }
);
```

This call has a lot going on so it's best to investigate it closely.

Firstly, we have the template argument, `Float32` here. This decides the Julia-side type of the Julia-side instances field. 

The first argument is the Julia-side instances' fields name. Usually we want this name to be the same as C++-side, `_red`, but to avoid confusion for this section only, the C++-side field is called `_red` while the corresponding Julia-side field is `_red_jl`.

The second argument of `add_property` is called the **boxing routine**. This function always has the signature `(T&) -> Property_t`, where `T` is the usertype-manage type (`RGBA` for us) and `Property_t` is the type of the field (`Float32`).
The boxing routine governs what value to assign the Julia-side field. It takes the C++-side instance, accesses the value of `_red`, then returns that value which will then be assigned to the Julia-side instances `_red_jl`.

The third argument is optional, it is called the **unboxing routine**. It always has the signature `(T&, Property_t) -> void`. When a Julia-side instance of `RGBA` is moved back C++-side, the unboxing routine governs, what value the now C++-sides `RGBA` fields `_red` will be assigned. If left unspecified, the value will be the default value. In our case, we assign `_red` the value of `_red_jl`, which is the second argument of the unboxing routine.

In summary:

+ the template argument governs the Julia-side type of the field
+ the first argument is the name of the Julia-side field
+ the boxing routine decides what value the Julia-side field will be assigned when moving the object C++ -> Julia
+ the unboxing routine decides what value the C++-side field will be assigned when moving the object Julia -> C++

Now that we know how to add fields, we can do so for the `_green`, `_blue` and `_alpha`:

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
    "_red",
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

Note that now, the Julia-side field is actually called `_red`, which is better style than the `_red_jl` we used only for clarity.

To illustrate that properties do not have to directly correspond with members of the C++ class, we'll add another Julia-side-only field that represents the `value` component from the HSV color system (sometimes also called "lightness"). It is defined as the maximum of red, green and blue:

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

We see that jluna assembled a mutable struct type, whose field names and types are as specified. Even the order in which we called `add_field` for specific names is preserved. This becomes important for the default constructor (a constructor that takes no arguments). The default values for each of the types fields are those of an unmodified, default-initialized instance of `T` (`RGBA()` in our case). This is why the type needs to be default constructable.

If we want the type to be implemented in a different module, we can give that module as a `jluna::Module` as an argument to `Usertype<T>::implement`.

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

Main.safe_eval("println(jl_rgba);");
Main.safe_eval("println(fieldnames(RGBA))");
```
```
RGBA(1.0f0, 0.0f0, 1.0f0, 1.0f0)
(:_red, :_green, :_blue, :_alpha, :_value)
```
> **Julia Hint**: `Base.fieldnames` takes a type (not an instance of a type) and returns the symbols of a types fields, in order.

We see that now, `Main.RGBA` is a proper Julia type and `jl_rgba` got the correct values according to each fields boxing / unboxing routine.

The same applies when moving `Main.RGBA` to C++:

```cpp
RGBA cpp_rgba = Main.safe_eval("return RGBA(0.5, 0.5, 0.3, 1.0)");
std::cout << cpp_rgba._red << " ";
std::cout << cpp_rgba._green << " ";
std::cout << cpp_rgba._blue << " ";
```
```
0.5 0.5 0.3
```


## Multi Threading

Given Julias application in high-performance computing, and its native multi-threading support, it is only natural a Julia wrapper should also allow for asynchronous execution in a similarly convenient and performant manner. 

In relation to specifically a C++ <-> Julia environment, we run into a problem, however. Consider the following:

```cpp
#include <jluna.hpp>
#include <thread>

int main()
{
    jluna::initialize();
    
    auto noop =[]() -> void {
        jl_eval_string("");
        return;
    };
    
    auto thread = std::thread(noop);
    thread.join();
    return 0;
}
```

Here, we're doing a very simple operation. We created a C++ lambda that *does nothing*. It executes no julia-side code by handing `jl_eval_string` (the C-APIs way to execute strings as code) an empty string, after which is simply returns. We then create a C++-side thread using `std::thread`. In C++, once a thread is created it starts running immediately. We wait for it's executing to finish using `.join()`.

Running the above code, the following happens:
```
signal (11): Segmentation fault
in expression starting at none:0
Allocations: 782030 (Pool: 781753; Big: 277); GC: 1

Process finished with exit code 139 (interrupted by signal 11: SIGSEGV)
```

It segfaults without an error. 

This is, because the Julia C-API is seemingly hardcoded to prevent any call of C-API functions from anywhere but master scope (the scope of the thread, `main` is executed in). For obvious reasons, this makes things quite difficult, because, **we cannot access the Julia state from within a C++-side thread**. This has nothing to do with jluna, it is how the C-API was designed. 

Given this, we can immediately throw out using any of the C++ `std::thread`-related multi-threading support, as well as libraries like libuv. It is important to keep this in mind, if our application already uses these frameworks, we have to take care to never execute code that interacts with Julia from within a thread. This includes any of jlunas functionalities, as it is, of course, build entirely on the Julia C-API.

All is not lost, however: jluna offers its own multi-threading framework, allowing for parallel execution of truly arbitrary C++ code - even if that code interacts with the Julia state.

### Initializing the Julia Threadpool

In Julia, we have to decide the number of threads we want to use *before startup*. 
In the Julia REPL, we would use the `-threads` (or `-t`) argument. In jluna, we instead give the desired threads as an argument to `jluna::initialize`:

```cpp
int main()
{
    jluna::initialize(8); 
    // equivalent to `julia -t 8`
    (...)
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

This sets the number of threads to number of local CPU threads, just like setting environment variable `JULIA_NUM_THREADS` to `auto` would do.

Note that any already existing `JULIA_NUM_THREAD` variable in the environment the jluna executable is run in, is ignored and overridden. We can only specify the number of threads through `jluna::initialize`.

### Spawning a Task

Owning to its status of being in-between two languages with differying vocabulary and design decisions, jlunas thread pool architecture borrows a bit from both C++ and Julia.

To execute a C++-side piece of code, we have to first wrap it into a C++ lambda, then wrap that lambda in a `jluna::Task`. We cannot initialize a task directly, rather, we use `jluna::ThreadPool::create`:

```cpp
using namespace jluna;

std::function<size_t(size_t)> forward_arg = [](size_t in) -> size_t {
    Base["println"]("lambda called with ", in);
    return in;
};

auto& task = ThreadPool::create(forward_arg, size_t(1234));
```
> **C++ Hint**: `std::function` describes C++ function objects with a fully specified signature (i.e. not template functions or pure lambdas (of type `auto`)). If our function has a the signature `(T1, T2, T3, ...) -> TR`, it can be only be bound to `std::function<TR(T1, T2, T3, ...)>`

> **C++ Hint**: Always specify the trailing return type of any C++ lambda using `->`. In this example, we manually specified `forward_arg` to return a value of type `size_t` by writing `[](size_t in) -> size_t { //...`.

Here, `ThreadPool::create` takes multiple arguments:

The first argument is a `std::function` object, which, in our case, was initialized by assigning a C++ lambda with the signature `(size_t) -> size_t` to the variable `forward_arg`. 

Any following objects given to `ThreadPool::create` will be used as the **arguments for the given function**. In our case, because we specified `size_t(1234)`, the thread pool will invoke our lambda `forward_arg` with that argument and only that argument.

Unlike C++-threads (but much like Julia tasks), `jluna::Task` does not immediately start execution once it is constructed. We need to manually "start" is using `.schedule()`. 

We can wait for its execution to finish using `.join()`. This  stalls the thread `.join()` was called from until the task completes:

```cpp
std::function<size_t(size_t)> forward_arg = [](size_t in) -> size_t {
    Base["println"]("lambda called with ", in);
    return in;
};

auto& task = ThreadPool::create(forward_arg, size_t(1234));
task.schedule();
task.join();
```
```
lambda called with 1234
```

Note how, even though we called the Julia function `println`, the task **did not segfault**. Using jlunas thread pool is the only way to call C++-side functions that access the Julia state concurrently.

We declared `task` to be a reference:

```cpp
// correct:
auto& task = ThreadPool::create(//...
        
// wrong:
auto task = ThreadPool::create(//...
```
This is because any created task is owned and managed only by the thread pool. It cannot be moved or copied. All we are allowed to do, is keep a reference to it, and interact with it. If a task goes out of scope before 

We can check the status of any task using the following member functions:

+ `is_done`: is the task finished or failed, c.f. `Base.istaskdone`
+ `is_failed`: has the task failed, c.f. `Base.istaskfailed`
+ `is_running`: has the task been scheduled but not yet finished, c.f. `Base.istaskstarted`

### Accessing a Tasks Result

Of course, `forward_arg` does more than just print to the command line, it also returns a value. To access the return value of a task, we use `.result()`, which returns an object of type `jluna::Future`.

In C++, a future is a thread-safe container that may or may not (yet) contain a value. The future is available immediately on its corresponding task being created, however, until the task has succesfully completed, it will not contain a value. Once the task is done, the return value will be moved into the future, after which we can access it.

We can get a tasks future using `.result()`:

```cpp
auto& task = ThreadPool::create(forward_arg, size_t(1234));
auto future = task.result();
```

To get the potential value of a future, we use `.get()` which returns a `std::optional`. If the futures task completed, we can access the value of the optional using `std::optional::value()`. We can check if the value is already available using `jluna::Future::is_available()`

```cpp
auto& task = ThreadPool::create(forward_arg, size_t(1234));
auto future = task.result();

future.is_available(); // false since task has not start

task.schedule();
task.join();

future.is_available(); // true
std::cout << future.get().value() << std::endl;
```
```
1234
```
> **C++ Hint**: See the [standard libraries documentation](https://en.cppreference.com/w/cpp/utility/optional) on more ways to interact with an `std::optional<T>`

We can wait for the value of a future to become available by calling `.wait()`. This will stall the thread `.wait()` is called from until the we can access the value. This way we don't necessarily need to keep track of the futures task, just having the future allows us to access to the tasks result.

### Data Race Freedom

The user is responsible for any potential data races a `jluna::Task` may trigger. Potentially useful C++-side tools to assure include the following (where their Julia-side functional equivalent is listed for reference):

| C++ | Julia | C++ Documentation  |
|------------|--------------|---------------------|
| `std::mutex` | `Base.ReentrantLock` |  [[here]](https://en.cppreference.com/w/cpp/thread/mutex)                  |
| `std::lock_guard`| `Threads.@lock` | [[here]](https://en.cppreference.com/w/cpp/thread/lock_guard)
| `std::condition_variable` | `Threads.Condition` | [[here]](https://en.cppreference.com/w/cpp/thread/condition_variable)
| `std::unique_lock` | `n/a` | [[here]](https://en.cppreference.com/w/cpp/thread/unique_lock)

### Thread Safety

As a general rule, any of jlunas functionality is thread-safe, as long as two threads are not modifying the same object at the same time.

For example, `jluna::safe_call` can be called from multiple threads, as the code executed within that `safe_call` is unrelated. Similarly, we can freely create unrelated proxies and modify them individually, but if we create two proxies that reference the same julia-variable and mutate both of them at the same time, we run into the data-race problems.

The user is required to ensure thread-safety in these conditions, just like the would in Julia. jluna has no hidden pitfalls or behind-the-scene machinery that multi-threading may throw a wrench into, however any user-created object is outside of jlunas responsibility.

A function-by-function run-down of thread-safety would be too expansive, so this is a list that aims to be a rule-of-thumb:

+ all functions in `include/safe_utilities.hpp` are thread-safe:
  - `safe_call`
  - `safe_eval`
  - `initialize`
  - `println`
+ creating a proxy through any method is thread-safe
  - modifying a proxy is not
+ allocating a `jluna::Array` is thread-safe
  - modifying the same array is not
+ `Module::new_*`, `Module::create`, `Module::create_or_assign` are thread-safe
  - all other interaction with modules is not, including `Module::safe_eval`
+ modifying a `Usertype` and implementing it is thread-safe
+ creating a generator expression is thread-safe
  - modifying the same generator expression is not
+ `unsafe::gc_preserve` and `unsafe::gc_release` are thread-safe
  
Any other functionality should be assumed to **not** be thread-safe. Instead, users will be required to manually "lock" and object and manage concurrent interaction. The following sections will give an example on how to achieve this using `jluna::Mutex` or `Base.ReentrantLock`.

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

Now, when a thread wants to modify `to_be_modified`, they first need to acquire this lock:

```cpp
// in cpp
auto push_to_modify = [](size_t)
{
  static auto* lock = unsafe::get_function(jl_base_module, "lock"_sym);
  static auto* unlock = unsafe::get_function(jl_base_module, "unlock"_sym);
  
  auto* lock = Main["to_be_modified_lock"]; // safe
  jluna::safe_call(lock);
  
  // modify here
  
  jluna::safe_call(unlock);
  return;
}
```

Since `push_to_modify` will be executed through `jluna::ThreadPool`, the Julia function `lock` will stall the thread, allowing for safe access.

### Thread-Safety in C++

A similar approach can be taken when trying to safely modify  C++-side objects. Instead of a Julia-side lock, we use a C++-side `jluna::Mutex`, which is a simple wrapper around a Julia-side `ReentrantLock`:

```cpp
jluna::Vector<size_t> to_be_modified;
auto to_be_modified_lock = jluna::Mutex();

auto push_to_modify = [](size_t)
{
    to_be_modified_lock.lock();
  
    // modify here
  
    to_be_modified_lock.unlock();
    return;
}
```

### Closing Notes

#### Interacting with `jluna::Task` from Julia

Internally, jluna makes accessing the Julia-state from a C++-sided, asynchronously executed function possible, by wrapping it in an actual Julia-side `Task`, then using Julias native thread pool to execute it. This has some beneficial side-effects.

`yield`, called from C++ like so:

```cpp
static auto* yield = unsafe::get_function(jl_base_module, "yield"_sym);
jluna::safe_call(yield);
```
Will actually yield the thread this C++ code is executed in, letting another Julia thread take over. This applies to all Julia-side functions. Calling them from within a `jluna::Task` has exactly the same effect as calling them from within a `Base.Task`. 

We can access the `Julia-side` object of a `jluna::Task` using `.operator unsafe::Value*()`. Because of this, we can manage a `jluna::Task` through an unnamed `jluna::Proxy` like so:

```cpp
auto lambda = [](){ //...
auto task = ThreadPool::create<void>(lambda);
auto task_proxy = Proxy(static_cast<unsafe::Value*>(task));

// all Julia-only attributes can now be accessed:
std::cout << (bool) task_proxy["stick"] << std::endl;
```
```
true
```

This allows advanced users to have more flexibility on how to handle a large set of tasks. Anything that woudl work in Julia also works on `jluna::Task`.

#### Using a lambda not bound to an std::function object

So far, we have created a `jluna::Task` using the following pattern:

```cpp
std::function<void()> lambda = []() -> void {
  return;
};

auto& task = ThreadPool::create(lambda);
```

This will of course work, however if we do not manually bind the lambda to an object of type `std::function<void()>`:

```cpp
auto lambda = []() -> void {    // auto this time
  return;
};

auto& task = ThreadPool::create(lambda);
```
```
/home/clem/Workspace/jluna/.test/main.cpp: In function ‘int main()’:
/home/clem/Workspace/jluna/.test/main.cpp:41:32: error: no matching function for call to ‘jluna::ThreadPool::create(main()::<lambda()>&)’
   41 | auto& task = ThreadPool::create(lambda);
      |              ~~~~~~~~~~~~~~~~~~^~~~~~~~
```
We get a compiler error. This is, because in C++, non-specialized lambdas do not yet have a specific return-type. This allows for the users to specify `auto` for both the return and argument types of a lambda, however for jluna, things need to have a clear type in order to know how to allocate both the task and its futures. To still allow any lambda to be used for `ThreadPool::create`, jluna offers a templated version of `create` that takes a single, user-specified template argument: the return type of the lamdba:

```cpp
auto lambda = []() -> void {    // still auto
  return;
};

auto& task = ThreadPool::create<void>(lambda); // <void> specified manually
```

jluna now knows to implicitely bind the lambda to a `std::function<void()>` and the above code will compile and work just as expected.

#### Do **NOT** use `@threadcall`

Lastly, a warning: Julia has a macro called `@threadcall`, which purports to simply execute a `ccall` in a new thread. Internally, it actually uses the libuv thread pool for this, not the native Julia thread pool. Because the C-API is seemingly hardcoded to segfault any attempt at accessing the Julia state through any non-master C-side thread, using `@threadcall` to call arbitrary code also segfaults. Because of this, it is not recommended to use `@threadcall` in any circumstances. Instead, call `ccall` from within a proper `Base.Task`, or use jlunas thread pool to execute C-side code.

---

## The `unsafe` Library

> Waning: Misuse of this part of jluna can lead to exception-less crashes, data corruption and memory leaks. Only user who are confident and familiar with C-style programs with no safety-nets are encouraged to employ these functions

So far, a lot of jlunas functionality was intentionally obfuscated to allow more novice users to use jluna in a way that is easy to understand and 100% safe. For some applications, however, the gloves need to come off. This section will detail how to surrender the most central of jlunas conceits: safety. In return, we get *optimal performance*, achieving overheads of 0 - 5% compared to the C-API, while some functions are actually faster than that native C-API functions.

In line with the way this topic was pushed to the back of this manual, all of the not-safe functionality in jluna is inside the `jluna::unsafe` nested namespace.

### Unsafe Types

Without `jluna::Proxy`, we reference Julia-side values through raw C-pointers. A C-pointer has a type, however there is no guarantee, only the assumption that whatever it is pointing to is actually of that type.

The most used Julia pointer types are:

+ `unsafe::Value*`
  - a pointer to any Julia value, but not necessarily a value of type `Any`
+ `unsafe::Function*`
  - a pointer to a callable Julia object, not necessarily a `Base.Function`
+ `unsafe::Symbol*`
  - pointer to a `Base.Symbol`
+ `unsafe::Module*`
  - pointer to a `Base.Module`
+ `unsafe::Expression*`
  - pointer to a `Base.Expr`
+ `unsafe::Array*`
  - pointer to a `Base.Array` of arbitrary value type and dimensionality (rank)
+ `unsafe::DataType`
  - pointer to a `Base.Type`, including `UnionAll` and not-yet-unrolled types
  
Note that an `unsafe::Value*` could be pointing to a `Base.Symbol`, `Base.Module`, `Base.Array`, etc.

For any object inheriting from `jluna::Proxy`, we can access the raw `unsafe::Value*` to the memory it is managing by using `operator unsafe::Value*()`:

> **C++ Hint**: `operator T()` of object `x` is implicitly invoked when either calling `static_cast<T>(x)` or performing a C-style cast `(T) x`
 
```cpp
auto proxy = Main.safe_eval("return 1234");
unsafe::Value* raw = static_cast<unsafe::Value*>(proxy);

std::cout << *(reinterpret_cast<Int64*>(raw)) << std::endl;
```
```
1234
```
  
### Calling Julia Functions

So far, we used proxies to call Julia-functions with Julia-values. In the `unsafe` library, we do so using `unsafe::call`. 

First, we need to get a pointer to any given function. This is done via `unsafe::get_function`, which takes the module the function is in, as well as the function name as a symbol:

```cpp
auto* println = unsafe::get_function(jl_base_module, "println"_sym);
```

Where `_sym` is a string-literal operator that converts its argument to a Julia-side symbol using the C-APIs `jl_symbol` function. `jl_base_module` is an `unsafe::Module*`, pointing to `Base`.

We can then call this function using `unsafe::call`:

```cpp
auto* res = unsafe::call(println, (unsafe::Value*) jl_main_module);
```
```
Main
```

If an exception is thrown during the `unsafe::call`, the user will not be notified and the function will return a `nulltpr`, potentially causing an exception-less segfault when accessing the returned value, expecting it to be valid.

jluna actually offers a middle ground between the `Proxy::call` and `unsafe::call`: `jluna::safe_call`. This function also returns an `unsafe::Value*`, however any exception that is raised during invokation of the function is forwarded to C++. Unless absolute peak performance is needed, it is generally recommended to use `jluna::safe_call` in place of `unsafe::call`, as both functions have the same signature.

For an unsafe version of `Module::safe_eval`, the unsafe library provides the `_eval` literal operator:

```cpp
"println(\"unsafely printed\")"_eval;
```
```
unsafely printed
```

Though, as discussed before, executing code as strings is fairly slow and not recommended unless unavoidable.

Just like with `unsafe::call`, no exception forwarding happens during `_eval` and it will return `nullptr` if parsing the string or its execution was unsuccesfull.
  
### Boxing / Unboxing

So far, we have used the proxies `operator=` to transfer C++-side memory to Julia, while `Proxy::operator T()` transformed Julia-side memory to C++. Internally, both of these functionalities are actually handled by two functions `box` and `unbox`. 

Recall that Julia-side memory and C++-side memory do not necessarily have the same memory layout. 

Transforming C++-side memory such that it is compatible with the Julia-state is called **boxing**. `box` has the following signature:

```cpp
template<typename T>
unsafe::Value* box(T);
```

Transforming Julia-side memory such that it is compatible with the C++-state is called **unboxing**, which has the following signature:

```cpp
template<typename T>
T unbox(unsafe::Value*);
```

As an example, consider the following C++-side value:

```cpp
char cpp_side = 120;
```

If we want to move this value to Julia, we use `box<char>`:

```cpp
unsafe::Value* julia_side = box<char>(cpp_side);
Base["println"](julia_side);
```
```
x
```

Where `x` is the 120th ASCII character.

The result `julia_side` is a pointer to the raw memory, containing the newly allocated char.

To then move the now Julia-side memory back C++-side, we use `unbox<char>`:

```cpp
char back_cpp_side = unbox<char>(julia_side);
std::cout << (int) back_cpp_side << std::endl;
```
```
120
```

This way of explicitly moving values between states can be quite cumbersome syntactically, which is why `jluna::Proxy` does all of this implicitly.

Note that `box<T>` and `unbox<T>` should always be called with an explicit template argument. This is actually required for `unbox`, but technically optional for `box`. To make sure a value is boxed into what the user desired, it should always be called with an explicit template argument.

### Protecting Values from the Garbage Collector

The result of `box` is a `unsafe::Value*`, a raw C-pointer. The value this pointer points to **is not protected from the garbage collector**. At any time after the resolution of `box`, the julia GC may deallocate our value right under our nose. This can't happen when using proxies - it is the entire point of them - but it can when handling raw pointers. 

Because of this, usage of the pure C-API can be very annoying, as we have to micro-manage each value depending on how it was allocated. As a general rule of thumb: Any value that is not explicitely referenced by either a Julia-side named variable or a Julia-side `Ref`, can be garbage collected at any point, usually segfaulting the entire application. This includes the result of `box`, `jluna::safe_call`, `unsafe::call` and most `unsafe` functions returning pointers in general.

To prevent this, the `unsafe` library provides `unsafe::gc_preserve`. This function does not invalidate whatever pointer it is given and protects the object pointed to from the GC. `gc_preserve` returns a `size_t` which is called the **id** of a value:

```cpp
unsafe::Value* value = box<Int64>(1234);
size_t value_id = unsafe::gc_preserve(value);
```

Keeping track of this id is incredibly important, because, to free the value such that it can be garbage collected when we want it to, we need to call `unsafe::gc_release`, which takes the id as an argument:

```cpp
unsafe::Value* value = box<Int64>(1234);
size_t value_id = unsafe::gc_preserve(value);

// value is safe from the GC here

unsafe::gc_release(value_id);

// value can be garbage collected here
```

If we loose track of `value_id` or we forget to call `gc_release`, the value will never be deallocated and a memory leak will occurr.

#### Disabling the GC

Alternatively to using `gc_preserve`, we can also simply disable the GC globally. The `unsafe` library provides `gc_disable`, `gc_enable` and `gc_is_enabled` for this. jluna provides a convenient macro for this:

```cpp
gc_pause; 

// everything here is safe from the GC

gc_unpause;
```

Unlike `gc_disable` / `gc_enable`, `gc_pause` will remember the state of the GC when it was called and restore it with `gc_unpause`, depending on whether it was enabled or disabled when `gc_pause` was first called.

### Accessing & Mutating a Variable

The `unsafe` library provides the following functions to access and mutate named Julia variables:

+ `get_value(Module* m, Symbol* name)`: get the value of variable `name` in module `m` as an unsafe pointer
+ `set_value(Module* m, Symbol* name, Value* new_value)`: set the value of variable `name` in module `m` to `new_value`. No exception forwarding occurres
+ `get_field(Value* owner, Symbol* field)`: get value of field named `field` of `owner`
+ `set_field(Value* owner, Symbol* field, Value* new_value*)`: set field `field` of owner `owner` to `new_value`

All of these functions will be vastly superior in terms of performance to assembling an expression or using a proxy to mutate a value.

---

### `unsafe` Array Interface

One of the most important high-performance tasks is modifying large arrays. `box` usually invokes a copy which is unacceptable in these circumstances. The unsafe library provides a 0-overhead interface to creating and modifying arrays, by internally operating on the raw Julia memory.

Note that we can simply access the data of a `jluna::Array` by `static_cast`ing it to `unsafe::Value*`:

```cpp
Array<Int64, 2> array = //...
unsafe::Array* raw_data = static_cast<unsafe::Array*>(array);
```

This will cause no reallocation, it simply forwards the pointer to the Julia-side data.

Elements of an array are stored in column-first order in Julia. To access the actual memory of these elements, we can use the field `data` of `unsafe::Array*`, or the C-API function `jl_array_data`.

### Accessing an Array Element

The `unsafe` library provides `get_index(Array*, size_t...)` and `set_index(Array*, size_t...=`, two 0-overhead functions that manipulate array indices. No bounds-checking is performed and there is no mechanism in place to verify that potential value types match. 


Similarly to `jluna::Array`, a 1d array takes 1 index, a 2d array 2 indices and a Nd array, N indices. Any array of any rank can furthermore be accessed with the linear index version which only takes a single index as an argument.

#### Allocate a New Array

`unsafe` supports arrays of any rank, however arrays of rank 1 (vectors) and rank 2 (matrices) are far better optimizied and should be preferred in performance-critical environemnts, if at all possible.

To allocate a new array, we use `unsafe::new_array`. This function takes `Rank + 1` arguments. The first argument is the arrays value type (as an `unsafe::Value*`), each subsequent argument is a `size_t` representing the size in that dimension. For example:

```cpp
// allocate a 10-element vector of strings
auto* vec_10 = unsafe::new_array(String_t, 10);

// allocate a 5x5 matrix of Int32s
auto* mat_5x5 = unsafe::new_array(Int32_t, 5, 5);

// allocated a 2x2x2x2 array of bools
auto* arr_2x2x2x2 = unsafe::new_array(Bool_t, 2, 2, 2, 2);
```

Where we used the jluna-provided global type proxies, which are implicitly `static_cast` to their pointer.

After allocation, all values of an array will be of the given type, initialized as `undef`. 

#### Creating a Thin Wrapper around already existing Data

In high-performance applications, we often do not have enough RAM to have two of the same data in memory at the same time. To address this, the `unsafe` library provides a function that creates a *thin wrapper* around already existing data. A thin wrapper is an array whos data does not belong to it. We basically set the data-pointer of a newly array to already existing memory, the user is responsible for keeping that memory correctly formatted and in-scope.

Simliar to `new_array`, `new_array_from_data` takes Rank + 2 elements:

+ value type (as an `unsafe::Value*`)
+ pointer to already existing data (as a `void*`)
+ Rank-many indices, specifying the size of an array

It is important to realize that no bounds-checking or verification that the data is actually formatted according to the arrays value type is performed If we have an array of `size_t`s, which are 64-bit long and we use its data to create an array of `uint8_t`s, the `size_t`s memory will simply be interpreted as if it were consecutively stored `uint8_t`s, leading to data corruption.

#### Resizing an Array

The `unsafe` library provides `resize_array`, which takes as its arguments the array, along with the new dimensions, similar to `new_array`. 

For 1D and 2D arrays only, "slicing", making an array smaller in one or more dimensions, is very fast. No allocation is performed when slicing, unlike when growing an array.

The values of the newly added data after expanding an array are set to `undef`. Recall that Julia-arrays are stored column-first order, so expanding a matrix in the x-direction will lead to new values being inserted into each column, potentially corrupting the order of elements.

#### Replacing an Arrays Data

To avoid copying, jluna provides `override_array`, which replaces an arrays data with that of another. Not allocation is performed and at no point will the amount of memory grow beyond the initial space of both arrays. 
As no copying is invoke, if we overrode array B with array A, the user is responsible for keeping array A in scope, otherwise array Bs data will also be deallocated.

Similarly, `swap_array_data` replaces arrays B data with that of A, and arrays A data with that of B. Unlike `override_array`, jluna needs to create a temporary copy of one of the arrays, potentially increasing the amount of memory consumed by the size of one of the arrays.

---

### Shared Memory

In the section on proxies, we said that, to move a value from C++ to Julia or vice-versa, we first need to change its memory format such that it is interpretable by the other language. This is not technically true, as for a very limited number of types, Julia and C++ have the exact same memory format. For these types, `box` / `unboxing` is a 0-cost operation, no actual computation is performed which is obviously desired in performance-critical applications.

For the following types only, `box` / `unboxing` has no cost

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
std::vector<T> °

° Where T is also a no-cost-(Un)Boxable
```

All of these types have the destinction of having a `C*` type Julia-side: `Csize_t`, `Cstring`, `Cvoid`, etc.. For these types and only these types, Julia and C++ truly share memory.

For these types, the `unsafe` library provides 0-cost `box` / `unbox` functions, called `unsafe::unsafe_box` / `unsafe::unsafe_unbox`. The "safe" `box` / `unbox` do some sanity checking and exception forwarding, even for C-types, making `unsafe_box`/ `unsafe_unbox` calls the performance-wise optimal way of moving memory between states.

---

## Performance Optimization

In this section, we will investigate jlunas performance, analyzing benchmark results and explaining why certain things are faster than others. Hopefully this will educate users on how they can achieve the best performance themslef.

(this section is not yet complete.)