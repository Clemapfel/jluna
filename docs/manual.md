# jluna: Manual & Tutorial

This manual will go over all of jlunas features and how to use them. It is structured in a way that ramps up in complexity, starting with basic things like calling functions and basic interaction between the C++ and Julia state and ending with multi-threading and performance optimization.

It is recommend that you read this manual in order, an interactive table of contents is provided for easy access.

This manual assumes that all users are familiar with the basics of Julia. If this is not the case, it may be necessary to first work through the excellent [official manual](https://docs.julialang.org/en/v1/) to, at least, get an overview of Julias syntax and structure.

jluna makes extensive use of newer C++20 features and high-level techniques such as [SFINAE](https://en.cppreference.com/w/cpp/language/sfinae) and template meta functions. This may provide a barrier of entry to users less familiar with the intricacies of the language. To address this, hints in the form of qutation blocks are provided where necessary:

> **Hint**: Quotation blocks like these are reserved for additional information or explanations of C++ and/or Julia features only. Users who consider themself very familiar with both, can safely skip them. 

---

## Table of Contents

---

### Installation

This manual does not go over how download and build jluna. Instead, a step-by-step tutorial on how to install and create our own application using jluna, is available [here](./installation.md).

---

### Initializating the Julia State

Before any Julia or jluna functionality can be accessed, we need to *initialize the Julia state*. This sets up the Julia environment and loads most of jlunas functionalities. 

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
> **C++ Hint**: `using namespace jluna` makes it, such that in the scope the statement was declared, we do not have to prefix all jluna functions/object with their namespace `jluna::`. This is why we can write `initialize()`, instead of `jluna::initialize()`.

Where `#include <jluna.hpp>` makes all of jlunas functionality available to the user, and `jluna::initialize` should be executed during the programs runtime as early as possible.

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

The function is called *safe*_eval, because it forwards any exceptions that may have occurred during execution of the string, throwing a `jluna::JuliaException`:

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

Along with the exception, we also get the full stacktrace and line of code where the exception occurred. 

#### Executing a File

We can execute an entire file with `safe_eval_file`. This function calls the Julia-side function `Main.include`, using the path we provided:

```cpp
Main.safe_eval_file("path/to/julia_file.jl");
```

#### Executing Multiple Lines of Code

If we want to execute more than just a single line, or if we want any special characters to be escaped automatically: *C++s raw string literal* is easy to remember and convenient:

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
> **C++ Hint**: Any characters in the text in-between the `R"(` `)"` are automatically escaped (if necessary). Without `R"()"`, we would have to write the above as `println(\"first line\");\nprintln(\"second line\");`


### Getting the Result of Julia Code

Often, we want to access whatever result a piece of code returns. To do this, we simply capture the C++-side return value of `safe_eval` using a C++-side variable:

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

If the result of a Julia-side expression is of a different type than the declared C++-side variable, jluna will try to convert the result such that both types match:

```
UInt64 result = Main.safe_eval("return Char(14)");
std::cout << result << std::endl;
```
```
14
```

In general, if the Julia-side value can be `Base.convert`ed to the declared C++-side type, jluna will do so implicitly. 

### Executing Julia Code in Module Scope

So far, we have used `Main.safe_eval`, which evaluates its argument in `Main` scope, similarly to how any code entered into the REPL would be evaluated. Sometimes this may not be desired. Consider the following example:

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

What happened here? Instead of assigning the variable `MyModule.var`, we actually created a new variable `Main.var`, instead. This is because the line `var = 9999` was executed *in Main scope* (the function `Main.eval` was used Julia-side). 

To avoid this, we could just prefix `var` with the proper `MyModule.var`, however this may not always be the most convenient. If we want to modify multiple things in `MyModule` scope, we can instead do:

```cpp
Module my_module = Main.safe_eval("return MyModule");
my_module.safe_eval("var = 777");

Main.safe_eval("println(MyModule.var)");
```
```
777
```

Where we have explicitly declared the C++-side variable `my_module` to be of type `jluna::Module`.

### Calling Julia Functions

While executing code using `safe_eval` is convenient, it has a significant performance impact. This is, because Julia needs to first `Meta.parse` the string, then evaluate the resulting expression.

> **Julia hint**: Meta.parse takes a string and treats it as a line of code. It returns an object of type `Expr`, which can then be `eval`ed into actual machine code. To learn more about expressions, see the [official manual page about metaprogramming](https://docs.julialang.org/en/v1/manual/metaprogramming/)

Instead, users are encouraged to execute any given piece of Julia code by using a *function*. We can get a function (once) by using `save_eval("return function_name")`, then use that function over and over, at no performance cost compared to using pure Julia:

```cpp
Main.safe_eval("f(x) = x^x");
auto f = Main.safe_eval("return f");
Int64 result = f(12);
std::cout << result << std::endl;
```
```
8916100448256
```

Where we now used `auto` as `f`s type.

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

> **Julia hint**: A structtype is any type declared using `struct` or `mutable struct`

```cpp
Main.safe_eval(R"(
    mutable struct MyStruct
       _field::Int64
    end
    
    jl_instance = MyStruct(9876);
)");
auto cpp_instance = Main["jl_instance"];
```

Here, we defined a new structtype `MyStruct`, which has a single field `_field`. We then created a Julia-side variable, `jl_instance`, which we bound a newly constructed `MyStruct` to. 
To access the Julia-side `jl_instance`, we used `Main["jl_instance"]`, which we were allowed to use since `jl_instance` is a proper named variable.

We can access the field of `cpp_instance` like so:

```cpp
Int64 field_value = cpp_instance["_field"];
std::cout << field_value << std::endl;
```
```
9876
```

Note again how we manually declared the type of the C++-side variable `field_value`. We did not use `auto`.

### Mutating Julia-side Values

We can use `operator[]` to access Julia-side values, be it variables in a module or fields of a structtype. However, we can actually also *mutate* them:

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
> In the latter example, `static_cast` (and thus the conversion operator) was implicitely called. This means we have already been using `static_cast` in the previous sections, be it implicitly.
> 
> See the [official manual](https://en.cppreference.com/w/cpp/language/static_cast) for more information.

The C++-variable `instance_field` directly refers to the Julia-side variable `Main.jl_instance._field` (recall the `MyStruct` is mutable). Because of this, **if we mutate the C++-side variable, we will also mutate the corresponding Julia-side variable**:

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

> **Hint**: In C++, indices are 0-based. In Julia, they are 1-based. This can be quite confusing considering we are operating in both languages at the same time. See the [section on arrays]() for more information 

## Proxies

So far, we have learned how to do basic things like calling functions/code using jluna. In previous examples, you may have noticed that it was never explicitly said, what the `auto` in a statement like `auto cpp_var = Main["julia var"]` deduces to. To fully understand how mutating and accessing variables works, we need to talk about the most central feature of jluna: `jluna::Proxy`.

---


## Specialized Proxies: Symbols

Another specialized type of proxy is the symbol proxy. It holds a Julia-side `Base.Symbol`.

We can create a symbol proxy in the following ways:

```cpp
// new unnamed proxy
auto unnamed_symbol = Symbol("symbol_value");

// new named proxy
auto named_symbol = Main.new_symbol("symbol_var", "symbol_value")
// generates variable `symbol_var` in main scope, then assigns it :symbol_value
```

Where, unlike with general proxy, the value the proxy is pointing to is asserted to be of type `Base.Symbol`. 

### Symbol Hashing

The main additional functionality `jluna::Symbol` brings, is that of *constant time hashing*.

A hash is essentially a `UInt64` we assign to things as a label. In Julia, hashes are unique and there a no hash collisions. This means if `A != B` then `hash(A) != hash(B)` and, furthermore, if `hash(A) == hash(B)` then `A === B`.

Unlike with many other classes, `Base.Symbol` can be hashed in constant time. This is because the hash is precomputed at the time of construction. 

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

We have already seen `jluna::Module` in the introductory sections, it's time to get to know it more closely. `jluna::Module` (`Module`, henceforth) inherits publicly from `jluna::Proxy`, meaning it sports all the same functions and functionalities we discussed previously, along with a few additional ones.

#### Assign in Module

Let's say we have a module `M` in main scope that has a single variable `var`:

```cpp
jluna::safe_eval(R"(
    module M
        var = []
    end
)");

Module M = Main.safe_eval("return M");
```

We've already seen that we can modify this variable using `M.safe_eval`, however this is a fairly badly performing option, because we have to go to the `Meta.parse` and `eval` all over.

Much faster is using `Module::assign`:

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

As the name suggest, if the variable does not yet exist, it is created. If the variable does already exist, `create_or_assign` acts identical to `assign`.

#### Creating a new Variable

A convenient and highly useful function is `Module::new_*`, for example `Module::new_undef("var_name")` creates a new variable named `var_name` in the module and assigns it `undef`. The return value of this function is a named proxy pointing to that variable, making the following syntax possible:

```cpp
Main.new_undef("variable") = // any value here
```

The following `new_*` functions are available:

| jluna Name | argument type(s) | Julia-side Type of Result |
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
| `new_tuple` | `T1` , `T2`, `T3`, ... | `Tuple{T1, T2, T3, ...}` |
| `new_array<T>` | `D1`, `D2`, ..., `Dn` | `Array{T, n}` of size `D1 * D2 * ... * Dn` |

This is the most performant way to create variables new variables module scope. 

#### `import` / `using`

Additionally, `jluna::Module` provides the following two functions wrapping the `using` and `import` functionality of modules:

+ `M.import("PackageName")`
  - equivalent to calling `import PackageName` inside `M`
+ `M.add_using("PackageName")`
  - equivalent to calling `using PackageName` inside `M`


---

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

This is, because the Julia C-API is seemingly hardcoded to prevent any call of C-API functions from anywhere but master scope (the scope of the thread, `main` is executed in). For obvious reasons, this makes things quite difficult, because, **we cannot access the Julia state from within a C++-side thread**. This has nothing to do with `jluna`, it is how the C-API was designed. 

Given this, we can immediately throw out using any of the C++ `std::thread`-related multi-threading support, as well as libraries like libuv. It is important to keep this in mind, if our application already uses these frameworks, we have to take care to never execute code that interacts with Julia from within a thread. This includes any of `jluna`s functionalities, as it is, of course, build entirely on the Julia C-API.

All is not lost, however: `jluna` offers its own multi-threading framework, allowing for parallel execution of truly arbitrary C++ code - even if that code interacts with the Julia state.

### Initializing the Julia Threadpool

In Julia, we have to decide the number of threads we want to use *before startup*. 
In the Julia REPL, we would use the `-threads` (or `-t`) argument. In `jluna`, we instead give the desired threads as an argument to `jluna::initialize`:

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

If left unspecified, `jluna` will initialize Julia with exactly 1 thread. We can set the number of threads to `auto` by supplying the following constant to `jluna::initialize`:

```cpp
using namespace jluna;

initialize(JULIA_NUM_THREADS_AUTO);
// equivalent to `julia -t auto`
```

This sets the number of threads to number of local CPU threads, just like setting environment variable `JULIA_NUM_THREADS` to `auto` would do.

Note that any already existing `JULIA_NUM_THREAD` variable in the environment the `jluna` executable is run in, is ignored and overridden. We can only specify the number of threads through `jluna::initialize`.

### Spawning a Task

Owning to its status of being in-between two languages with differying vocabulary and design decisions, `jluna`s thread pool architecture borrows a bit from both C++ and Julia.

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

Note how, even though we called the Julia function `println`, the task **did not segfault**. Using `jluna`s thread pool is the only way to call C++-side functions that access the Julia state concurrently.

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

The following `jluna` features are thread-safe, as of version `0.9.0`:

+ any C++-side interaction with `jluna::Proxy`:
  - ctors / dtors
  - `.safe_call` and `operator()`
  - `operator[]`
  - `operator=`
  - `get_*`
  - etc.
+ any C++-side interaction with `jluna::Usertype<T>`
  - `implement`
  - `add_field`
  - `etc.`
+ `""_gen`
+ unsafe utilities:<br>
    - `unsafe::gc_*`
    - `unsafe::get_function`
    - `unsafe::call`
    - `unsafe::eval`, `unsafe::Expr`
    - all `box` / `unbox` calls
    
All other features should be assumed to **not** be thread-safe. 

This is most notable when using functions that modify the global Julia state, such as: 
+ `jluna::safe_call`
+ `jluna::safe_eval`
+ `Module::safe_eval`
+ `Module::assign`
+ `Module::create_or_assign`
+ `Module::get`
+ `Module::new_*`
+ etc. 
  
The user is responsible for any potential unintended interactions calling these function concurrently may cause. 

As an example for how to deal with this, let's say we have the following variable in `Main`:

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

A similar approach can be taken when trying to safely modify non-thread-safe C++-side objects. Instead of the lock we would use a C++-side `std::mutex`.

### Closing Notes

#### Interacting with `jluna::Task` from Julia

Internally, `jluna` makes accessing the Julia-state from a C++-sided, asynchronously executed function possible by wrapping it in an actual Julia-side task, then using Julias native thread pool to execute it. This has some beneficial side-effects.

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
We get a compiler error. This is, because in C++, non-specialized lambdas do not yet have a specific return-type. This allows for the users to specify `auto` for both the return and argument types of a lambda, however for `jluna`, things need to have a clear type in order to know how to allocate both the task and its futures. To still allow any lambda to be used for `ThreadPool::create`, `jluna` offers a templated version of `create` that takes a single, user-specified template argument: the return type of the lamdba:

```cpp
auto lambda = []() -> void {    // still auto
  return;
};

auto& task = ThreadPool::create<void>(lambda); // <void> specified manually
```

`jluna` now knows to implicitely bind the lambda to a `std::function<void()>` and the above code will compile and work just as expected.

#### Do **NOT** use `@threadcall`

Lastly, a warning: Julia has a macro called `@threadcall`, which purports to simply execute a `ccall` in a new thread. Internally, it actually uses the libuv thread pool for this, not the native Julia thread pool. Because the C-API is seemingly hardcoded to segfault any attempt at accessing the Julia state through any non-master C-side thread, using `@threadcall` to call arbitrary code also segfaults. Because of this, it is not recommended to use `@threadcall` in any circumstances. Instead, call `ccall` from within a proper `Base.Task`, or use `jluna`s thread pool to execute C-side code.

---













