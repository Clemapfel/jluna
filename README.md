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
```cpp
#include <jluna.hpp>
using namespace jluna;

// initialize julia and jluna 
State::initialize();

// execute arbitrary strings with exception forwarding
State::safe_script(R"(
    f(x) = x*x*x
)");

// call julia-side functions with C++-side values
int result = Main["f"](12);
Base["println"](result);
```
```
1728
```
---
```cpp
// fully iterable, assignable array interface
Array<size_t, 3> cpp_array3d = Main["array3d"];
for (auto& e : cpp_array3d)
    e = e.operator size_t() + 1; 

State::script("println(array3d)");
```

// compatible with many C++ objects, including lambdas
Main["f"] = [](Any* x) -> void {
    std::cout << "cpp lambda called " << unbox<std::string>(x) << std::endl;
};
State::script("f(\"completely julia-side\")");
```
```
1728
cpp lambda called completely julia-side
```

---

### Features
Some of the many advantages `jluna` has over the C-API include:

+ expressive generic syntax
+ call C++ functions from julia using any julia-type
+ assigning C++-side proxies also mutates the corresponding variable with the same name julia-side
+ julia-side values, including temporaries, are kept safe from the garbage collector while they are in use C++-side
+ verbose exception forwarding from Julia, compile-time assertions
+ wraps [most](./docs/quick_and_dirty.md#list-of-unboxables) of the relevant C++ `std` objects and types
+ multidimensional, iterable array interface with Julia-style indexing
+ human-written manual, including inline documentation for IDEs for both C++ and Julia code
+ freely mix `jluna` and the C-API
+ And more!

### Planned (but not yet implemented):
In order of priority, highest first:

+ `v0.6`: expression proxy, access to meta features via C++
+ `v0.7`: creating new modules and datatypes completely C++-Side
+ `v0.8`: thread-safe `cppcall`, parallelization
+ `v0.9`: 0-overhead performance version of proxies and `cppcall`
+ `v1.0`: multiple julia states, save-states: restoring a previous julia state
---

## Documentation

A step-by-step introduction and reference guide intended for users is available [here](./docs/manual.md). Furthermore, all user-facing code has in-line documentation available through most IDEs (or the julia `help?` command). 

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

A step-by-step tutorial on how to create a new C++ project, compile and link jluna can be found [here](./docs/installation.md).

---

## License

`jluna` is freely available for non-commercial and educational use. For use in for-profit commercial applications, please [contact the developer](https://www.clemens-cords.com/contact).

---