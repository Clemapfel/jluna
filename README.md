# jluna: A modern Julia ⭤ C++ Wrapper (v0.5)

Julia is a beautiful language, it is well-designed and well-documented. julia C-API is also well-designed, less beautiful and much less... documented.

Heavily inspired in design and syntax by (but in no way affiliated with) the excellent Lua⭤C++ wrapper [**sol2**](https://github.com/ThePhD/sol2), `jluna` aims to fully wrap the official Julia C-API and replace it in usage in C++ projects by making accessing julia unique strengths through C++ safe, hassle-free and just as beautiful.

---

### Table of Contents

0. [Introduction](README.md)
1. [Showcase](#showcase)<br>
2. [Features](#features)<br>
3. [Planned Features](#planned-but-not-yet-implemented)<br>
4. [Documentation](#documentation)<br>
    4.1 [Manual](./docs/docs.md)<br>
    4.2 [Quick & Dirty Overview](#documentation)<br>
5. [Dependencies](#dependencies)<br>
   5.1 [Julia 1.7.0+](#dependencies)<br>
   5.2 [g++10](#dependencies)<br>
   5.3 [cmake 3.19+](#dependencies)<br>
   5.4 [Linux / Mac OS](#dependencies)
6. [License](#license)
7. [Installation](#installation)<br>
  7.1 [Step-by-Step Guide](#installation)<br>
  7.2 [Troubleshooting](#troubleshooting)<br>
   
---

## Showcase

```cpp
using namespace jluna;

// one-line initialization and setup
State::initialize();

// run arbitrary code with exception forwarding
State::safe_script(R"(
    array3d = reshape(collect(1:(3*3*3)), 3, 3, 3)
    function(xs...) = 


    mutable struct Holder
        _array_field::Array{Int64, 3}
        _vector_field::Vector{String}
    
        Holder() = new(reshape(collect(1:(3*3*3)), 3, 3, 3), Vector{String}())
    end
    
    instance = Holder();
)");

// access and mutate variables
Array<Int64, 3> array = Main["instance"]["_array_field"];
array.at(0, 1, 2) = 9999;

// std:: objects are supported out-of-the-box
Main["instance"]["_vector_field"] = std::vector<std::string>{"string", "string", "string"};

// call julia-side functions with C++-side arguments
auto println = State::script("return Base.println");
println(Main["instance"]);

// call c++-side functions julia-side arguments
State::register_function("cpp_print", [](jl_value_t* in) -> jl_value_t* {
   
    std::cout << "cpp called" << std::endl;
    
    auto as_vector = unbox<jluna::Vector<size_t>>(in);
    for (auto e : as_vector)
        e = ((size_t)) e + 1
                
    return as_vector;
});
State::safe_script("println(cppcall(:cpp_print, [1, 2, 3, 4]))");
```
```
Holder([1 4 7; 2 5 8; 3 6 9;;; 10 13 16; 11 14 17; 12 15 18;;; 19 9999 25; 20 23 26; 21 24 27], ["string", "string", "string"])

cpp called
[2, 3, 4, 5]
```
---

### Features
Some of the many advantages `jluna` has over the C-API include:

+ expressive generic syntax
+ call C++ functions from julia using any julia-type
+ assigning C++-side proxies also mutates the corresponding variable with the same name Julia-side
+ Julia-side values, including temporaries, are kept safe from the garbage collector while they are in use C++-side
+ verbose exception forwarding from Julia, compile-time assertions
+ wraps [most](./docs/quick_and_dirty.md#list-of-unboxables) of the relevant C++ `std` objects and types
+ multidimensional, iterable array interface with Julia-style indexing
+ fully documented, including inline documentation for IDEs for both C++ and Julia code
+ mixing the C-API and `jluna` works no problem
+ And more!

### Planned (but not yet implemented):
In order of priority, highest first:

+ `v0.6`: expression proxy, access to meta features via C++
+ `v0.7`: creating new modules and datatypes with member-access completely C++-Side
+ `v0.8`: thread-safe `cppcall` and proxy-data read/write
+ `v0.9`: No-Overhead performance version of proxies and `cppcall`
+ `v1.0`: multiple julia states, save-states: restoring a previous julia state
---

## Documentation

A step-by-step introduction and reference guide intended for users is available [here](./docs/docs.md). Furthermore, all user-facing code has in-line documentation available through most IDEs (or the julia `help?` command). 

Advanced users are encouraged to check the headers (available in `jluna/include`) for implementation details. They are formatted specifically to be easily understood by 3rd parties. 

---

## Dependencies

`jluna` aims to be as modern as is practical. It uses C++20 features extensively and aims to support the newest Julia version, rather than focusing on backwards compatibility. If you are looking for a C++ library that supports Julia 1.5 or lower, consider checking out [CxxWrap](https://github.com/JuliaInterop/CxxWrap.jl) instead.

For `jluna` you'll need:
+ [**Julia 1.7.0**](https://julialang.org/downloads/#current_stable_release) (or higher)
+ [**g++10**](https://askubuntu.com/questions/1192955/how-to-install-g-10-on-ubuntu-18-04) (or higher)
  - including `-fconcepts`
+ [**cmake 3.16**](https://cmake.org/download/) (or higher)
+ unix-based operating system

Currently, only g++10 is supported, clang support is planned in the future.

---

## Installation & Troubleshooting

A step-by-step tutorial on how to create a new C++ project, then compile and link jluna can be found [here](./docs/installation.md).

---

## License

`jluna` is freely available for non-commercial and educational use. For use in for-profit commercial applications, please [contact the developer](https://www.clemens-cords.com/contact).

---