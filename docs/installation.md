# Creating a Project with jluna

The following is a step-by-step guide to creating our own application using `jluna`.

### Table of Contents
1. [Creating the Project](#creating-the-project)
2. [Setting JULIA_PATH](#setting-julia_path)
3. [Recompiling `jluna`](#building-jluna)
4. [Linking `jluna`](#linking-jluna)<br>
    4.1 [main.cpp](#linking-jluna)<br>
    4.2 [CMakeLists.txt](#linking-jluna)<br>
5. [Troubleshooting](#troubleshooting)<br>
    5.1 [CMake: Cannot determine location of julia image](#cannot-determine-location-of-julia-image)<br>
    5.2 [CMake: Cannot find julia.h / libjulia.so](#cannot-find-juliah--libjuliaso)<br>
    5.3 [C++: Cannot find <julia.h> / <jluna.hpp>](#cannot-find-juliah--jlunahpp)<br>
    5.4 [C++: State::initialize fails](#stateinitialize-fails)<br>

### Creating the Project

First, we create our workspace directory. For the remainder of this section, this will be assumed to be `~/my_project`. We now execute:

```bash
cd ~/my_project
git clone https://github.com/Clemapfel/jluna.git
```

This adds the folder `jluna/` to our directory. We now need to recompile `jluna`.

### Setting JULIA_PATH

To tell `jluna` where to find julia, we need to set an environment variable `JULIA_PATH`, which contains the path to the local julia image we'll use for jluna. We set it in bash using:

```bash
export JULIA_PATH=$(julia -e "println(joinpath(Sys.BINDIR, \"..\"))")
```

Here, we're calling the julia REPL inline to output the the global variable `Sys.BINDIR`, which contains the path to the currently used julia binary. We verify `JULIA_PATH` was set correctly by using:

```bash
echo $JULIA_PATH
```
```
/home/user/Applications/julia/bin/..
```
Of course, this path will be different for each user. Note the prefix `/` which designates an absolute path starting at root and that there is no post-fix `/`. If `JULIA_PATH` reports an empty string, it may be because the `julia` command is not available on a system level. If this is the case, we can simply call the above command from within the julia REPL

```julia
println(joinpath(Sys.BINDIR, ".."))
```
```
/path/to/your/julia/bin/..
```

And copy-paste the resulting output to assign `JULIA_PATH` in bash like so:

```bash
export JULIA_PATH=/path/to/your/julia/bin/..
```

We can now continue to compiling `jluna` using cmake.

### Building `jluna`

We navigate into `my_project/jluna/` and create a new build directory, then call cmake from within that directory:

```bash
cd ~/my_project/jluna
mkdir build
cd build
cmake -D CMAKE_CXX_COMPILER=g++-10 ..
make
```

The following warnings may appear:

```
(...)
/home/user/Applications/julia/bin/../include/julia/julia_locks.h:72:32: warning: ‘++’ expression of ‘volatile’-qualified type is deprecated [-Wvolatile]
   72 |         jl_current_task->ptls->defer_signal++;  \
(...)
```

This is because the official julia header `julia.h` is slightly out of date and is unrelated to `jluna`s codebase. `jluna` itself should report no warnings or errors. If this is not the case, head to [troubleshooting](#troubleshooting).

We verify everything works by running `JLUNA_TEST` which we just compiled:

```bash
# in ~/my_project/jluna/build
./JLUNA_TEST
```

A lot of output will appear. At the very end it should show: 

```
Number of tests unsuccessful: 0
```

We have now compiled and verified jluna and are left with a shiny new `libjluna.so` and `libjluna_c_adapter.so` in `my_project/jluna/`. We can optionally clean up the build files using:

```bash
# in ~/my_project/jluna/build
cd ..
rm -r build
```

### Linking `jluna`

We now need to create our own application. First we create a `main.cpp`:

```bash
cd ~/my_project
gedit main.cpp
```

This will open a GUI editor, if `gedit` is unavailable, any other editor (`vim`, `nano`, `emacs`, etc.) can be used. 

We replace the contents of `main.cpp` with the following:

```cpp
#include <jluna.hpp>

using namespace jluna;

int main()
{
    State::initialize();
    Base["println"]("hello julia");
    State::shutdown();
}
```

and safe. 

To compile our project, we again use cmake. We first create `CMakeLists.txt`:

```bash
# in ~/my_project/
gedit CMakeLists.txt
```

And replace its contents with the following:

```cmake
cmake_minimum_required(VERSION 3.16)

# name of our project
project(MyProject)

# cmake and cpp settings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")
set(CMAKE_CXX_STANDARD 20)

# build type
set(CMAKE_BUILD_TYPE Debug)

# julia
set(JULIA_LIB "$ENV{JULIA_PATH}/lib/libjulia.so")

# find jluna and jluna_c_adapter
find_library(JLUNA_LIB REQUIRED NAMES libjluna.so PATHS "${CMAKE_SOURCE_DIR}/jluna/")
find_library(JLUNA_C_ADAPTER_LIB REQUIRED NAMES libjluna_c_adapter.so PATHS "${CMAKE_SOURCE_DIR}/jluna/")

# include directories needed
include_directories("${CMAKE_SOURCE_DIR}/jluna")
include_directories("$ENV{JULIA_PATH}/include/julia")

# add our executable
add_executable(MY_EXECUTABLE ${CMAKE_SOURCE_DIR}/main.cpp)

# link executable with jluna, jluna_c_adapter and julia
target_link_libraries(MY_EXECUTABLE ${JLUNA_LIB} ${JLUNA_C_ADAPTER_LIB} ${JULIA_LIB})
```

We again close and safe, then create our own build folder and run cmake, just like we did with `jluna` before

```bash
# in ~/my_projects
mkdir build
cd build
cmake -D CMAKE_CXX_COMPILER=g++-10 ..
make
```

If errors appear, be sure `JULIA_PATH` is still set correctly as it is needed to find `julia.h`. 

After compiliation succeeded, the directory should now have the following layout:

```
my_project/
    CMakeLists.txt
    main.cpp
    jluna/
        libjluna.so
        libjluna_c_adapter.so
        jluna.hpp
        (...)
    build/
        MY_EXECUTABLE
        (...)
```
Where names with a `/` suffix are folders.

We can now run our application using:

```bash
# in ~/my_project/build
./MY_EXECUTABLE
```
```
[JULIA][LOG] initialization successfull.
hello julia
```

`State::initialize()` may fail, if this is the case, head to [troublshooting](#troubleshooting). Otherwise, we are done and can now start developing our own application with the aid of julia and `jluna`.

## Troubleshooting

### Cannot determine location of julia image

When compiling `jluna`, the following error may occurr:

```
 Cannot determine location of julia image.  Before running cmake, please
  manually set the environment variable JULIA_PATH using

      export JULIA_PATH=/path/to/your/.../julia

  If you are unsure of the location of your julia image, you can access the
  path from within the julia REPL using

      println(joinpath(Sys.BINDIR, ".."))

  For more information, visit
  https://github.com/Clemapfel/jluna/blob/master/README.md#troubleshooting
```

This is because `JULIA_PATH` is not properly set. Repeat the section on [setting `JULIA_PATH`](#setting-julia_path) to resolve this issue.

###  Cannot find julia.h / libjulia.so

When building `jluna`, the following warnings may appear:

```
CMake Warning at CMakeLists.txt:37 (message):
  Cannot find library header julia.h in (...)
```
```
CMake Warning at CMakeLists.txt:37 (message):
  Cannot find library header julia.h in (...)
```

This means verification of the directory specified through `JULIA_PATH` failed. We can make sure the path is correct by verifying:

+ the path is an absolute path starting at root
+ the path has a prefix `/`
+ the path has no suffix `/`
+ the path is the same as reported in the julia REPL

If all of these are true, it may be that your julia image is corrupted or compressed. The julia folder should have the following layout:

```
julia/
    bin/
       julia 
    include/
        julia/
            julia.h
            (...)
    lib/
        libjulia.so
        (...)
    
    (...)
```
Where names with a suffix `/` are folders. If your julia folder looks different, redownload the latest official release [here](https://julialang.org/downloads/#current_stable_release) and reassign `JULIA_PATH` accordingly.

### Cannot find <julia.h> / <jluna.hpp>

If the following error may appear when compiling your library:

```
fatal error: julia.h: No such file or directory
    3 | #include <julia.h>
      |          ^~~~~~~~~
compilation terminated.
```

or, similarly:

```
fatal error: jluna.hpp: No such file or directory
    3 | #include <jluna.hpp>
      |          ^~~~~~~~~~~
compilation terminated.
```

It means that the include-paths were not properly declared in cmake. Make sure the following lines are present in your `CMakeLists.txt`:

```
include_directories("${CMAKE_SOURCE_DIR}/jluna")
include_directories("$ENV{JULIA_PATH}/include/julia")
``` 

Furthermore, make sure 

+ `jluna/` is located in `my_project/`, right next to `my_project/main.cpp`
+ `JULIA_PATH` is set correctly (see above)

### State::initialize fails

#### AssertionError("jluna requires julia v1.7.0 or higher")

`jluna` asserts the correct version of julia is present on initialization. If the following exception occurs when calling `State::initialize`:

```
terminate called after throwing an instance of 'jluna::JuliaException'
  what():  [JULIA][EXCEPTION] AssertionError("jluna requires julia v1.7.0 or higher, but v1.7.1 was detected. Please download the latest julia release at https://julialang.org/downloads/#current_stable_release, set JULIA_PATH accordingly, then recompile jluna using cmake. For more information, visit https://github.com/Clemapfel/jluna/blob/master/README.md#troubleshooting")

signal (6): Aborted
in expression starting at none:0
(...)
```

It means that your julia version is out of date. [Download the latest version](https://julialang.org/downloads/#current_stable_release), set `JULIA_PATH` to point to it, then recompile `jluna` as outlined [above](#building-jluna).

### ERROR: could not load library 

When calling `State::initialize`, julias C-API may report an error of the following type:

```
ERROR: could not load library "(...)/lib/julia/sys.so"
(...)/lib/julia/sys.so: cannot open shared object file: No such file or directory
```

This means julia is not installed on a system level or that `JULIA_PATH` was not set correctly. If the former is true, replace `State::initialize()` with its overload:

```cpp
State::initialize("/path/to/(...)/julia/bin")
```
Where `path/to/(...)` is replaced with the absolute path to your image of julia. This will tell the C-API to load julia from that image, rather from the default system image.



















