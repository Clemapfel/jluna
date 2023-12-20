# jluna: A modern Julia Wrapper for C++ (v1.0.0)


![](./header.png)

Julia is a beautiful language, it is well-designed, and well-documented. Julia's C-API is also well-designed, less beautiful, and much less... documented.<br>
jluna aims to fully wrap the official Julia C-API, replacing it in projects with C++ as the host language, by making accessing Julia's unique strengths through C++ safe, hassle-free, and just as beautiful.

---

### Table of Contents

0. [Introduction](README.md)
1. [Features](#features)<br>
2. [Showcase](#showcase)<br>
3. [Documentation](#documentation)<br>
4. [Dependencies](#dependencies)<br>
   4.1 [Julia 1.7.0+](#dependencies)<br>
   4.2 [Supported Compilers: g++, clang++, MSVC](#dependencies)<br>
   4.3 [CMake 3.12+](#dependencies)<br>
5. [Installation](#installation--troubleshooting)<br>
6. [License](#license)
7. [Authors](#credits)

---

### Features

+ expressive, generic syntax
+ create / call / assign Julia-side variables from C++
+ full exception forwarding, verbose error messages with complete stacktrace
+ `std` types & usertypes can be moved freely between Julia and C++
+ call arbitrary C++ functions from Julia
+ multidimensional, iterable array interface
+ provides a custom thread pool that, [unlike the C-API](https://clemens-cords.com/jluna/multi_threading.html), allows for concurrent interfacing with Julia
+ provides < 5% overhead functions, viable in performance-critical environments
+ complete [manual](https://clemens-cords.com/jluna/basics.html), [installation guide](https://clemens-cords.com/jluna/installation.md), [benchmark analysis](https://clemens-cords.com/jluna/benchmarks.html), inline documentation for IDEs - all written by a human
+ and more!

---

### Showcase

(If you are looking for examples showing best-practice basic usage, please instead consult the [manual](https://clemens-cords.com/jluna/basics.html))

#### Executing Julia Code

```cpp
 // execute multi-line Julia code
Main.safe_eval(R"(
     f(x) = x^x^x
     vec = Int64[1, 2, 3, 4]
 )");

// call Julia functions with C++ values
auto f = Main["f"];
std::cout << (Int64) f(3) << std::endl;

// mutate Julia-side values
Main["vec"][2] = 999;
Main.safe_eval("println(vec)");

// assign `std` objects to Julia variables
Main["vec"] = std::vector<char>{117, 118, 119, 120};
Main.safe_eval("println(vec)");
```
```
2030534587
[1, 2, 999, 4]
['u', 'v', 'w', 'x']
```

---

#### Array Interface

```cpp
// array interface
Array<Int64, 2> matrix = Main.safe_eval("return reshape([i for i in 1:(4*4)], 4, 4)");

// supports multi-dimensional indexing (and array comprehension, not shown here)
matrix.at(0, 2) = 999;
Main["println"](matrix);

// even has generator expressions!
auto generated_vector = Vector<char>("(Char(i) for i in 97:104)"_gen);
Main["println"](generated_vector);
```
```
[1 5 9 13; 2 6 10 14; 999 7 11 15; 4 8 12 16]
['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
```

---

#### Calling C++ Functions from Julia
```cpp
Main.safe_eval("cpp_function = () -> ()"); // forward declaration

// assign C++ lambda to Julia Function
Main["cpp_function"] = as_julia_function<void(std::string)>(
    [](std::string in) -> void {
        std::cout << "cpp prints: " << in << std::endl;
    }
);

// call lambda, entirely Julia-side
Main.safe_eval(R"(
  cpp_function("what_julia_hands_it")
)");
```
```
cpp prints: what_julia_hands_it
```

---

## Documentation

Documentation, including a step-by-step installation and troubleshooting guide, tutorial, and index of all functions and objects in jluna is available
[here](https://clemens-cords.com/jluna).

---

## Dependencies

jluna aims to be as modern as is practical. It uses C++20 features extensively and aims to support the newest Julia version, rather than focusing on backwards compatibility.

For jluna you'll need:
+ [**Julia 1.7.0**](https://julialang.org/downloads/#current_stable_release) (or newer)
+ [**cmake 3.12**](https://cmake.org/download/) (or newer)
+ C++ Compiler, one of
    - [g++10](https://gcc.gnu.org/) (or newer)<br>
    - [clang++-12](https://releases.llvm.org/) (or newer)<br>
    - [MSVC](https://visualstudio.microsoft.com/downloads/) 19.34 (or newer)

On Unix, g++ or clang (installed using your package manager) are recommended. <br>
On Windows, either use g++ provided by [MinGW](https://sourceforge.net/projects/mingw/) or MSVC provided by the [Visual Studio C++ build tools](https://visualstudio.microsoft.com/downloads/).

In either case, make sure the compilers' version is as stated above or newer, as jluna uses modern C++20 features extensively.

---

## Quick Installation

(If you are windows, please visit the [step-by-step documentation guide](https://clemens-cords.com/jluna/installation.html#) instead)

In any public directory, execute:

```bash
git clone https://github.com/clemapfel/jluna
cd jluna
mkdir build 
cd build
cmake .. 
sudo make install -j 8
```

If jluna fails to detect Julia, set 'JULIA_BINDIR' to the return value of `print(Sys.BINDIR)`, executed in the Julia REPL. For example, if `print(Sys.BINDIR)` returns the path `/home/Desktop/julia-1.9.3/bin`, call 

```bash
cmake .. -DJULIA_BINDIR="/home/Dekstop/julia-1.9.3/bin"
```

---

To use jluna, in your own `CMakeLists.txt`, add:

```cmake
find_package(jluna REQUIRED)
```

Then, where `my_target` is the name of **your own** CMake library or executable:

```cmake
target_compile_features(my_target PUBLIC cxx_std_20)

target_link_libraries(my_target PUBLIC 
    ${JLUNA_LIBRARIES}
    # other libraries here
)
target_include_directories(my_target PUBLIC 
    ${JLUNA_INCLUDE_DIRECTORIES}
    # other paths here
)
```

For a complete example project you can use to build off of, see [here](/example).

---

## Credits
jluna was designed and written by [Clem Cords](https://github.com/Clemapfel).

+ **March 2022**: CMake improvements by [friendlyanon](https://github.com/friendlyanon)

## Donations

jluna was created with no expectation of compensation and made available for free. Consider donating to reward past work and support the continued development of this library:

+ [GitHub Sponsors](https://github.com/sponsors/Clemapfel)
+ [PayPal](https://www.paypal.com/donate/?hosted_button_id=8KWF3JTDF8XL2)

---

## License & Citation

The current and all prior releases of jluna are supplied under MIT license, available [here](./LICENSE.txt).

If you would like to cite jluna in your academic publication, you can copy the entry in [CITATION.bib](CITATION.bib) to your [BibTeX](https://www.overleaf.com/learn/latex/Bibliography_management_with_bibtex) bibliography, then use the `\cite{jluna}` command anywhere in your [LaTeX](https://www.latex-project.org/) source code.

Thank you for your consideration,
C.
