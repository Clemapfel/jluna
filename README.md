# jluna: A modern Julia <-> C++ Wrapper (v1.0.0)


![](./header.png)

Julia is a beautiful language, it is well-designed, and well-documented. Julia's C-API is also well-designed, less beautiful, and much less... documented.<br>
jluna aims to fully wrap the official Julia C-API, replacing it in projects with C++ as the host language, by making accessing Julia's unique strengths through C++ safe, hassle-free, and just as beautiful.

---

### Table of Contents

0. [Introduction](README.md)
1. [Showcase](#showcase)<br>
2. [Features](#features)<br>
3. [Planned Features](#planned-but-not-yet-implemented)<br>
4. [Documentation](#documentation)<br>
5. [Dependencies](#dependencies)<br>
   5.1 [julia 1.7.0+](#dependencies)<br>
   5.2 [Supported Compilers: gcc10, gcc11, clang12](#dependencies)<br>
   5.3 [cmake 3.12+](#dependencies)<br>
6. [License](#license)
7. [Authors](#credits)

---

### Showcase

(If you are looking for examples showing basic usage, please instead consult the [manual](https://clemens-cords.com/jluna/basics.html).)

#### Executing Julia Code

```cpp
// call Julia functions with C++ Values
Main.safe_eval("f(x) = x^x^x");

auto f = Main["f"];
std::cout << (Int64) f(3) << std::endl;

// mutate Julia values
Main.safe_eval("vec = Int64[1, 2, 3, 4]");
Main["vec"][2] = 999;
Main.safe_eval(R"(print(typeof(vec), " ", vec, "\n"))");

// assign Julia values with `std::` objects
Main["vec"] = std::vector<char>{117, 118, 119, 120};
Main.safe_eval(R"(print(typeof(vec), " ", vec, "\n"))");
```
```
2030534587
Vector{Int64} [1, 2, 999, 4]
Vector{Char} ['u', 'v', 'w', 'x']
```
---

#### Array Interface

```cpp
// custom array class
jluna::Array<Int64, 2> matrix = Main.safe_eval("return reshape([i for i in 1:(4*4)], 4, 4)");
Base["println"](matrix);

// multi-dimensional indices
matrix[0, 2] = 999;
Base["println"](matrix);

// iterable
for (auto element : matrix)
element = static_cast<Int64>(element) + 1;
Base["println"](matrix);

// supports list indexing
auto list_vector = matrix[{1, 7, 4, 5, 3}];
Base["println"](list_vector);

// even has generator expressions!
auto generated_vector = VectorAny("(Char(i) for i in 97:107)"_gen);
Base["println"](generated_vector);
```
---

#### Calling C++ Functions in Julia

```cpp
// C++ function: concatenate all elements of input array
auto cpp_function = [](jluna::ArrayAny1d array_in) -> std::string
{
std::stringstream str;
for (auto e : array_in)
str << static_cast<std::string>(e) << " ";

return str.str();
};

// bind to Julia variable
Main.create_or_assign("cpp_function", as_julia_function<std::string(ArrayAny1d)>(cpp_function));

// can now be used in Julia
Main.safe_eval(R"(
    print(cpp_function(["does ", "it ", "work ", "with ", [1, 2, 3], " ?"]))
)");
```
```
does it work with [1, 2, 3] ? 
```
---

#### Multi-Threading

```cpp
// initialize with 8 threads
initialize(8);

// custom synchronization primitive 
// that works in both Julia and C++
auto mutex = jluna::Mutex();

// thread behavior: write current thread id to vector
std::vector<size_t> to_write_to;
auto task_function = [&]() -> void
{
mutex.lock();
to_write_to.push_back(ThreadPool::thread_id());
mutex.unlock();
};

// create tasks and store them until they are finished
std::vector<Task<void>> tasks;
for (size_t i = 0; i < 2 * ThreadPool::n_threads(); ++i)
{
// spawn task
tasks.push_back(ThreadPool::create<void()>(task_function));
tasks.back().schedule();
}

// wait for all tasks to finish
for (auto& task : tasks)
task.join();

// print
for (auto id : to_write_to)
std::cout << id << " ";
std::cout << std::endl;

// result shows which threads of the threadpool executed which task
// this order is decided by the CPU scheduler; each run, it is different
```
```
8 7 8 7 8 7 8 2 7 2 8 1 6 5 3 4 
```
---

### Features

+ expressive, generic syntax
+ create / call / assign Julia-side variables from C++
+ thread-safe, provides a custom thread pool that, [unlike the C-API](https://clemens-cords.com/jluna/multi_threading.html), allows for concurrent interfacing with Julia
+ `std::` types & usertypes can be moved freely between Julia and C++
+ call arbitrary C++ functions from Julia
+ multidimensional, iterable array interface
+ provides < 5% overhead functions, viable in performance-critical environments
+ full exception forwarding, verbose error messages
+ complete [manual](https://clemens-cords.com/jluna/basics.html), [installation guide](https://clemens-cords.com/jluna/installation.md), [benchmark analysis](https://clemens-cords.com/jluna/benchmarks.html), inline documentation for IDEs - all written by a human
+ and more!

## Long-Term Support

jluna entered version 1.0.0 in February 2023. While feature complete, the following areas would benefit from additional development:
+ **Windows Support**: Currently, the library should compile on a Windows machine with a sufficiently new compiler, stability remains untested, mostly due to the fact the developer does not currently have access to a Windows workstation
+ **Foreign Thread Support**: With Julia 1.9.0 foreign thread support [seems to have been implemented](https://github.com/JuliaLang/Julia/pull/45447), however, until 1.9.0 becomes the stable release, jluna will not utilize this new feature

---

## Documentation

Documentation, including a step-by-step installation and troubleshooting guide, tutorial, and index of all functions and objects in jluna is available
[here](https://clemens-cords.com/jluna).

---

## Dependencies

jluna aims to be as modern as is practical. It uses C++20 features extensively and aims to support the newest stable Julia version, rather than focusing on backwards compatibility.

For jluna you'll need:
+ [**Julia 1.7.0**](https://julialang.org/downloads/#current_stable_release) (or higher)
+ [**cmake 3.12**](https://cmake.org/download/) (or higher)
+ C++ Compiler (see below)

Currently, [g++10](https://gcc.gnu.org/) (or newer) and [clang-12](https://releases.llvm.org/) (or newer) fully supported. `g++-11` is the primary compiler used for development of jluna and is thus recommended. `MSVC 19.32` (or newer) seems to work, but stability remains untested.

---

## [Installation & Troubleshooting](https://clemens-cords.com/jluna/installation.html)

> A step-by-step guide is available [here](https://clemens-cords.com/jluna/installation.html). It is recommended that you follow this guide, instead of the highly abridged version below.

Execute, in your bash console, in any public directory:

```bash
git clone https://github.com/Clemapfel/jluna
cd jluna
mkdir build
cd build
```
```
cmake .. -DJULIA_BINDIR=$(julia -e "println(Sys.BINDIR)") -DCMAKE_CXX_COMPILER=<C++ Compiler> -DCMAKE_INSTALL_PREFIX=<install directory>
```
Where
+ `<C++ Compiler>` is one of `g++-10`, `g++-11`, `clang++-12`
+ `<install directory>` is the desired install directory, usually `/usr/local` on unix, `C:/Program Files/jluna` on Windows

Then:
```
make install
ctest --verbose
```

Afterward, you can make jluna available to your library using

```cmake
# inside your own CMakeLists.txt
find_library(jluna REQUIRED 
    NAMES jluna
    PATHS <install directory>
)
target_link_libraries(<your library> PRIVATE
    "${jluna}" 
    "${<julia>}")
```
Where
+ `<install directory>` is the directory specified via `-DCMAKE_INSTALL_PREFIX`
+ `<julia>` is the Julia shared library (usually available in `"${JULIA_BINDIR}/../lib"`)
+ `<your library>` is the name of your library or executable

If any step of this does not work for you, please follow the [installation guide](https://clemens-cords.com/jluna/installation.html) instead.

---

## License

The current and all prior releases of jluna are supplied under MIT license, available [here](./LICENSE.txt).

I would like to ask people using this library in commercial or university settings, to disclose their usage of jluna in some small way (for example, at the end of the credits or via a citation) and to make clear the origin of the work (for example by linking this GitHub page). Unlike the text in `LICENSE.txt`, this is not a legally binding condition, only a personal request by me, the developer.

Thank you for your consideration,
C.

---

## Credits
jluna was designed and written by [Clem Cords](https://github.com/Clemapfel).

#### March 2022:<br>
+ cmake improvements by [friendlyanon](https://github.com/friendlyanon)

---
