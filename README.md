# jluna: A modern julia <-> C++ Wrapper (v0.9.0)

![](./header.png)

Julia is a beautiful language, it is well-designed, and well-documented. Julias C-API is also well-designed, less beautiful, and much less... documented.<br>
Heavily inspired in design and syntax by (but in no way affiliated with) the excellent Lua <-> C++ wrapper [**sol3**](https://github.com/ThePhD/sol2), jluna aims to fully wrap the official julia C-API, replacing it in projects with C++ as the host language, by making accessing julias unique strengths through C++ safe, hassle-free, and just as beautiful.

---

### Table of Contents

0. [Introduction](README.md)
1. [Showcase](#showcase)<br>
2. [Features](#features)<br>
3. [Planned Features](#planned-but-not-yet-implemented)<br>
4. [Documentation](#documentation)<br>
    4.1 [Manual](./docs/manual.md)<br>
    4.2 [Installation](./docs/installation.md)<br>
    4.3 [Troubleshooting](./docs/installation.md#troubleshooting)<br>
5. [Dependencies](#dependencies)<br>
   5.1 [julia 1.7.0+](#dependencies)<br>
   5.2 [Supported Compilers: gcc10, gcc11, clang12](#dependencies)<br>
   5.3 [cmake 3.12+](#dependencies)<br>
6. [License](#license)
7. [Authors](#credits)
   
---

### Showcase

(If you are looking for examples showing basic usage, please instead consult the [manual](./docs/manual.md).)

#### Executing Julia Code

```cpp
// call Julia functions
Main.safe_eval("f(x) = x^x^x");

auto f = Main["f"];
std::cout << f(12) << std::endl;

// mutate Julia values
Main.safe_eval("vec = [1, 2, 3, 4]");
Main["vec"][2] = 999;

// assign Julia values with `std::` objects
Main["vec"] = std::vector<size_t>{8, 7, 6, 5};
Main.safe_eval("println(vec); println(typeof(vec));");
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

// even generator expressions
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

// can now be used in julia
Main.safe_eval(R"(
    julia_side = cpp_function(["does", "it", "work", "with", [1, 2, 3], "?"]);
    println(typeof(julia_side))
    println(julia_side)
)");
```
```
String
does it work with [1, 2, 3] ? 
```
---

#### Multi-Threading

```cpp
// initialize with 8 threads
initialize(8);

for (size_t i = 0; i < 4; ++i) {

// thread behavior: write current threadpool thread id to to_write_to
std::vector<size_t> to_write_to;

// custom synchronization primitive
auto mutex = jluna::Mutex();
 
auto task_function = [&]() -> void
{
    // lock for synchronization
    mutex.lock();

    // write id of ThreadPool thread that executes this line
    to_write_to.push_back(ThreadPool::thread_id());

    // unlock
    mutex.unlock();
};

// create tasks and store them until they are finished
std::vector<Task<void>> tasks;

for (size_t i = 0; i < 2 * ThreadPool::n_threads(); ++i)
{
    // create task
    tasks.push_back(ThreadPool::create<void()>(task_function));

    // then schedule
    tasks.back().schedule();
}

// wait for all tasks to finish
for (auto task : tasks)
    task.join();

// print
for (auto id : to_write_to)
    std::cout << id << " ";
std::cout << std::endl;
}

// result shows which threads of the threadpool executed which task
// this order is decided by the CPU scheduler; each run, it is different
```
```
8 7 8 7 8 7 8 2 7 2 8 1 6 5 3 4 
8 7 8 8 7 2 5 8 4 8 3 8 5 6 3 6 
6 4 6 4 4 6 6 6 6 6 6 3 4 4 6 4 
3 5 3 8 8 8 8 5 3 6 7 8 6 3 5 5 
```
---

### Features
+ expressive, generic syntax
+ create / call / mutate Julia-side values from C++
+ thread-safe, provides a custom thread pool that, [unlike the C-API](./docs/manual.md/#multi-threading), allows safe concurrent interfacing with Julia
+ `std::` types & usertypes can be moved freely between Julia and C++
+ call arbitrary C++ functions from Julia
+ multi-dimensional, iterable, low-overhead array interface
+ provides < 5% overhead functions for performance-critical environments
+ full exception forwarding and verbose error messages
+ extensive manual, installation guide, written by a human
+ and more!

### Planned (but not yet implemented):

jluna is feature complete as of 0.9.0. The library will continue to be supported. If no major issues or feature request come up, 0.9 will be upgraded to 1.0 in Winter 2022.

---

## Documentation

A verbose, step-by-step introduction and manual is available [here](./docs/manual.md). This manual is written for people less familiar with C++ and/or Julia, providing non-jluna related guidance where necessary.

Furthermore, all user-facing code has in-line documentation, available through most IDEs. 

Advanced users are encouraged to check the headers (available in `jluna/include/`) for implementation details. They are formatted specifically to be easily understood by 3rd parties. 

---

## Dependencies

jluna aims to be as modern as is practical. It uses C++20 features extensively and aims to support the newest julia version, rather than focusing on backwards compatibility. 

For jluna you'll need:
+ [**Julia 1.7.0**](https://julialang.org/downloads/#current_stable_release) (or higher)
+ [**cmake 3.12**](https://cmake.org/download/) (or higher)
+ C++ Compiler (see below)

Currently [**g++10**](https://askubuntu.com/questions/1192955/how-to-install-g-10-on-ubuntu-18-04), [**g++11**](https://lindevs.com/install-g-on-ubuntu/) and [**clang++-12**](https://linux-packages.com/ubuntu-focal-fossa/package/clang-12) are fully supported. `g++-11` is the primary compiler used for development of jluna and is thus recommended. `MSVC 19.32` seems to work, however stability remains untested.

---

## [Installation & Troubleshooting](./docs/installation.md)

> A step-by-step guide is available [here](./docs/installation.md)

Execute, in your bash console, in any public directory:

```bash
git clone https://github.com/Clemapfel/jluna
cd jluna
mkdir build
cd build
export 
cmake .. -DJULIA_BINDIR=$(julia -e "println(Sys.BINDIR)") -DCMAKE_CXX_COMPILER=<C++ Compiler> -DCMAKE_INSTALL_PREFIX=<install directory>
make install
ctest --verbose
```

Where 
+ `<C++ Compiler>` is one of `g++-10`, `g++-11`, `clang++-12`
+ `<install directory>` is the desired install directory, usually `/usr/local` on unix, `C:/Program Files/jluna` on windows

Afterwards, you can make jluna available to your library using 

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
+ `<install directory>` is the directory specified via `-DCMAKE_INSTALL_PREFIX=` before
+ `<julia>` is the Julia shared library (usually available in `"${JULIA_BINDIR}/../lib"`)
+ `<your library>` is the name of your library or executable

If any step of this does not work for you, please follow the [installation guide](./docs/installation.md) instead.

---

## License

The current and all prior releases of jluna are supplied under MIT license, available [here](./LICENSE.txt).

I would like to ask people using this library in commercial or university settings, to disclose their usage of jluna in some small way (for example, at the end of the credits or via a citation) and to make clear the origin of the work (for example by linking this github page). Unlike the text in `LICENSE.txt`, this is not a legally binding condition, only a personal request by me, the developer.

Thank you for your consideration,
C.

---

## Credits
jluna was designed and implemented by [Clem Cords](https://github.com/Clemapfel).

#### March 2022:<br>
+ cmake improvements by [friendlyanon](https://github.com/friendlyanon)

---
