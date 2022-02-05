# jluna: A modern Julia ⭤ C++ Wrapper (v0.5)

Julia is a beautiful language, it is well-designed and well-documented. julia C-API is also well-designed, less beautiful and much less... documented.

Heavily inspired in design and syntax by (but in no way affiliated with) the excellent Lua⭤C++ wrapper [**sol2**](https://github.com/ThePhD/sol2), `jluna` aims to fully wrap the official Julia C-API and replace it in usage in C++ projects, by making accessing julias unique strengths through C++ safe, hassle-free and just as beautiful.

---

### Table of Contents

0. [Introduction](README.md)
1. [Showcase](#showcase)<br>
2. [Features](#features)<br>
3. [Planned Features](#planned-but-not-yet-implemented)<br>
4. [Documentation](#documentation)<br>
    4.1 [🔗 Manual](./docs/manual.md)<br>
    4.2 [🔗 Installation](./docs/installation.md)<br>
    4.3 [🔗 Troubleshooting](./docs/installation.md#troubleshooting)<br>
5. [Dependencies](#dependencies)<br>
   5.1 [Julia 1.7.0+](#dependencies)<br>
   5.2 [g++10](#dependencies)<br>
   5.3 [cmake 3.19+](#dependencies)<br>
   5.4 [Linux / Mac OS](#dependencies)
6. [License](#license)
   
---

### Showcase
#### Access Julia-Side Values/Functions
```cpp
// execute arbitrary strings with exception forwarding
State::safe_script(R"(
    f(x) = x*x*x
    
    mutable struct MyStruct
        _field
        MyStruct() = new(123)
    end

    instance = MyStruct();
)");

// access and modify variables
Main["instance"]["_field"] = 456;
State::script(R"(println("instance._field is now: ", instance._field))");

// call julia-side functions with C++-side values
int result = Main["f"](12);
Base["println"](result);
```
```
instance._field is now: 456
1728
```
---
#### Multi-Dimensional Array Interface
```cpp
State::script("array = collect(1:9)");
Array<size_t, 1> cpp_array = Main["array"];

// julia style list indexing
auto sub_array = cpp_array[{6, 5, 4, 3, 2}];
Base["println"]((Any*) sub_array);

// iterable and assignable
for (auto e : cpp_array)
    e = e.operator size_t() + 10;

State::script("println(array)");
```
```
[7, 6, 5, 4, 3]
[11, 12, 13, 14, 15, 16, 17, 18, 19]
```
---
#### Call C++ Functions from Julia

```cpp
/// register lambda and bind to julia-side variable
State::new_named_undef("lambda") = [](Any* x, Any* y) -> Any*
{
    auto as_string = unbox<std::string>(x);
    std::cout << "cpp prints " << as_string << " and returns: " << std::endl;
    auto as_set = unbox<std::set<size_t>>(y);

    size_t out = 0;
    for (size_t x : as_set)
        out += x;

    return box(out);

    return jl_nothing;
};

// now callable from julia
State::safe_script(R"(
    println(Main.lambda("what julia handed it", Set([1, 2, 3, 3, 4])))  # non-c-types work!
)");
```
```
cpp prints what julia handed it and returns: 
10
```
---

### Features
Some of the many advantages `jluna` has over the C-API include:

+ expressive generic syntax
+ call C++ functions from julia using any julia-type
+ assigning C++-side proxies also mutates the corresponding variable with the same name julia-side
+ julia-side values, including temporaries, are kept safe from the garbage collector while they are in use C++-side
+ verbose exception forwarding from julia, compile-time assertions
+ wraps [most](./docs/quick_and_dirty.md#list-of-unboxables) of the relevant C++ `std` objects and types
+ multidimensional, iterable array interface with julia-style indexing
+ manual written by a human for beginners
+ inline documentation for IDEs for both C++ and Julia code 
+ freely mix `jluna` and the C-API
+ And more!

### Planned (but not yet implemented):
In order of priority, highest first:

+ `v0.6 - 0.7`: expression proxy, access to meta features via C++ including C-API-only introspection
+ `v0.7 - 0.8`: linear algebra, matrices
+ `v0.8 - 0.9`: thread-safety, parallelization
+ `v0.9 - 1.0`: 0-overhead performance version of proxies and `cppcall`
+ `v1.0+`: multiple julia worlds, save-states: restoring a previous julia state
---

## Documentation

A step-by-step introduction and reference guide intended for users is available [here](./docs/manual.md). Furthermore, all user-facing code has in-line documentation available through most IDEs (or the julia `help?` command). 

Advanced users are encouraged to check the headers (available in `jluna/include/`) for implementation details. They are formatted specifically to be easily understood by 3rd parties. 

---

## Dependencies

`jluna` aims to be as modern as is practical. It uses C++20 features extensively and aims to support the newest Julia version, rather than focusing on backwards compatibility. If you are looking for a C++ library that supports Julia 1.5 or lower, consider checking out [CxxWrap](https://github.com/JuliaInterop/CxxWrap.jl) instead.

For `jluna` you'll need:
+ [**Julia 1.7.0**](https://julialang.org/downloads/#current_stable_release) (or higher)
+ [**g++10**](https://askubuntu.com/questions/1192955/how-to-install-g-10-on-ubuntu-18-04) (or higher)
  - including `-fconcepts`
+ [**cmake 3.16**](https://cmake.org/download/) (or higher)
+ unix-based operating system

Currently, only g++10 and g++11 are supported, clang support is planned in the future.

---

## [Installation & Troubleshooting](./docs/installation.md)

A step-by-step tutorial on how to create, compile and link a new C++ Project with jluna can be found [here](./docs/installation.md).

---

## License

`jluna` is freely available for non-commercial and educational use. For use in for-profit commercial applications, please [contact the developer](https://www.clemens-cords.com/contact).

---