# Creating a Project with jluna

The following is a step-by-step guide on how to install jluna, and how to create our own C++ application using jluna from scratch.

### Table of Contents
1. [Installing jluna](#installing-jluna)<br>
   1.1 [Cloning from Git](#cloning-from-git)<br>
   1.2 [Configuring CMake](#configure-cmake)<br>
   1.3 [make install](#make-install)<br>
2. [Creating a new Project](#creating-a-project-with-jluna)<br>
   2.1 [Project Folder](#project-folder)<br>
   2.2 [Cloning jluna](#cloning-jluna)<br>
   2.3 [Building jluna](#building-jluna)<br>
   2.4 [main.cpp](#our-maincpp)<br>
   2.5 [CMakelists.txt](#our-cmakeliststxt)<br>
   2.6 [FindJulia.cmake](#findjuliacmake)<br>
   2.7 [Building our Application](#building-our-application)<br>
3. [Troubleshooting](#troubleshooting)<br>
    3.1 [Permission Denied](#permission-denied)<br>
    3.2 [Unable to Detect Julia Executable](#unable-to-detect-the-julia-executable)<br>
    3.3 [Found Unsuitable Version](#could-not-find-julia-found-unsuitable-version)<br>
    3.4 [Could NOT find Julia: Missing X](#could-not-find-julia-missing-x)<br>
    3.5 [Could not find `julia.h` / `jluna.hpp`](#cannot-find-juliah--jlunahpp)<br>
    3.6 [Could not find libjluna_c_adapter](#when-trying-to-initialize-jlunacppcall-cannot-find-libjluna_c_adapter)<br>
   
## Installing jluna

This section will guide users on how to install jluna, either so it is globally available to all applications, or so it is localized to only a specific folder.

### Cloning from Git

We navigate into any public folder (henceforth assumed to be `Desktop`) and execute:

```bash
git clone https://github.com/clemapfel/jluna.git
cd jluna
mkdir build
cd build
```

This will have cloned the `jluna` git repository into a folder called `Desktop/jluna`. 

### Configure CMake

We now call:

```bash
# in Desktop/jluna/build
cmake .. -DCMAKE_CXX_COMPILER=<compiler> -DCMAKE_INSTALL_PREFIX=<path>
```

Where `<compiler>` is one of:
+ `g++-10`
+ `g++-11`
+ `clang++-12`

> Window supports is experimental. This means specifying `MSVC` may work, however this is untested. The [cpp compiler support]() list seems to imply that MSVC 19.30 or newer is required to compile `jluna`.

And `<path>` is the desired install path. `-DCMAKE_INSTALL_PREFIX=<path>` is optional, if it is specified manually, keep note of this path as we will need it later.

Some errors may appear here, if this is the case, head to [troubleshooting](#troubleshooting).

If the command does not recognize the compiler, even though you are sure it is installed, it may be necessary to specify the full path to the compiler executable, instead. For example:

```
-DCMAKE_CXX_COMPILER=/usr/bin/g++-10
```

### Make Install

Having successfully configured cmake, we now call:

```bash
# in Desktop/jluna/build
make install
```

Which will create two shared libraries `libjluna.*`, and `libjluna_c_adapter.*` in the install directories, where `*` is platform dependent, usually `.so` on unix and `.lib` or `.dll` on windows. 

We can now link our own executables or libraries using:

```cmake
# in users own CMakeLists.txt
find_library(jluna REQUIRED 
    NAMES jluna
)
```

If a custom install directory was specified, we need to make cmake aware of this:

```cmake
# in users own CMakeLists.txt
find_library(jluna REQUIRED 
    NAMES jluna
    PATHS <install path>
)
```

Where `<install path>` is the path specified as `-DCMAKE_INSTALL_PREFIX` during configuration before. 

If `jluna` is still not found using `find_library`, we may need to manually specify the shared library suffixes like so:

```cpp
set(CMAKE_FIND_LIBRARY_SUFFIXES_WIN32 ".lib;.dll;.dll.a;")
set(CMAKE_FIND_LIBRARY_SUFFIXES_UNIX ".so")
```

After `find_library`, we link our own library (here assumed to be named `my_library`), using:

```cmake
# in users own CMakeLists.txt
target_link_libraries(my_library ${jluna} ${<julia package>})
target_include_directories(${JLUNA_DIR} ${<julia include directory>})
```

Where `<julia package>` is the package containing the julia C-API and `<julia include directory>` is the folder containing `julia.h`, usually `${JULIA_BINDIR}/../include` or `${JULIA_BINDIR}/../include/julia`.

A simple, template 'CMakeLists.txt' is available [here](../install/resources/CMakeLists.txt)

---

## Creating a New Project

This section will guide users unfamiliar with cmake or C++ through the process of creating a working hello-world executable using julia and `jluna`.

> Note that, on unix-like systems, this process can be automated using `jluna/install/init.sh`. See the here](../README.md#creating-a-new-project-from-scratch)

### Project Folder

First, we need to create our project folder. This will be assumed to be `~/Desktop/MyProject`:

```bash
cd ~/Desktop/
mkdir MyProject
```

### Cloning jluna

We now need to clone jluna and install it locally:

```bash
# in ~/Desktop/MyProject
git clone https://github.com/clemapfel/jluna.git
```

### Building jluna

For `jluna` to know where to look for our julia executable, we need to set `JULIA_BINDIR`. On some system, this environment variable is set automatically during the install of julia. If this is not the case, we need to manually set it.

First we access the julia binary directory using:

```bash
# in ~/Desktop/MyProject
julia -e "println(Sys.BINDIR)"
```
```
~/path/to/our/julia-1.7.2/bin
```

Where the output of this call different for each user. 
 
Taking note of the julia binary path, we now build jluna

```bash
# in ~/Desktop/MyProject
cd jluna
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=clang++-12 -DCMAKE_INSTALL_PREFIX=~/Desktop/MyProject/jluna/build -DJULIA_BINDIR=~/path/to/our/julia-1.7.2/bin
make
```

Where `-DJULIA_BINDIR` is set to the output of `julia -e "println(Sys.BINDIR)"`, and `-DCMAKE_INSTALL_PREFIX` is set to MyProject/jluna/build.

This will compile jluna, leaving `libjluna_c_adapter` nad `libjluna` in `MyProject/jluna/build`. 

If errors appear at any point during configuration or make, head to [troubleshooting](#troubleshooting).

### Our main.cpp

We need C++ code for our own application to call, because of this we create `main.cpp` in the folder `~Desktop/MyProject/`:

```bash
cd ~/Desktop/MyProject
gedit main.cpp
```

Where `gedit` can be replaced with any common text editor, such as `vim`, `emacs`, `Notepad++`, etc..

We replace the contents of `main.cpp` with:

```cpp
#include <jluna.hpp>

using namespace jluna;

int main()
{
    State::initialize();
    Base["println"]("Your project is setup and working!");
}
```

Then safe and close the file.

### Our CMakeLists.txt

We will be building our own project with cmake again, so we first create a `CMakeLists.txt` in `~/Desktop/MyProject`:

```bash
# in ~/Deskopt/MyProject
gedit CMakeLists.txt
```
And replace it's content with the following:

```cmake
cmake_minimum_required(VERSION 3.12)

# project name and version
project(MyProject VERSION 0.0.0 LANGUAGES CXX)

# make FindJulia available to our cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/find")

# locate the julia package
find_package(Julia 1.7.0 REQUIRED)

# locate the jluna library
find_library(jluna REQUIRED
    NAMES libjluna.so libjluna.dll.a libjluna.dll libjluna.lib
    HINTS ${CMAKE_SOURCE_DIR}/jluna ${CMAKE_SOURCE_DIR}/jluna/build)

# declare our executable
add_executable(my_executable ${CMAKE_SOURCE_DIR}/main.cpp)

# link jluna and julia
target_link_libraries(my_executable PRIVATE
    ${jluna}
    "$<BUILD_INTERFACE:Julia::Julia>"
)

# set it to C++20
target_compile_features(my_executable PUBLIC cxx_std_20)

# add the jluna and julia directory as include directories
target_include_directories(my_executable PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>"
    ${CMAKE_SOURCE_DIR}/jluna
    ${JULIA_INCLUDE_DIR}
)
```

This `CMakeLists.txt` finds both Julia and `jluna` for us, then links them with our executable, and adds the include directory for `julia.h` and `jluna.hpp` to our paths.

### FindJulia.cmake

To find julia, we need a `FindJulia.cmake`, which is provided by `jluna`. First, we create the the corresponding file in `MyProject/cmake/find`:

```bash
# in ~/Desktop/MyProject
mkdir cmake
cd cmake
mkdir find
cd find
gedit FindJulia.cmake
```

And replace its content with [this](../install/resources/FindJulia.cmake).

It is not necessary to understand what this file does, just know that by calling 

```cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/find")
```

It allows us to `find_package` julia, which we need to link our executable with.

## Building our Application

We're almost done! We lastly add the `build` folder to our own project:

```bash
# in ~Desktop/MyProject/
mkdir build
```

Our project directory should now look like this:

```
MyProject/
    main.cpp
    CMakeLists.txt
    build/
    cmake/
        find/
            FindJulia.cmake
    jluna
        jluna.hpp
        (...)
        include/
            (...)
        build/
            libjluna.*
            libjluna_c_adapter.*
```

Where `*` is one of `.so`, `.dll`, `.dll.a`, `.lib`.

We are now ready to build our application:

```bash
# in ~/Desktop/MyProject
cd Build
cmake .. -DCMAKE_CXX_COMPILER=clang++-12 -DJULIA_BINDIR=~/path/to/our/julia-1.7.2/bin
```

Where `DJULIA_BINDIR` was set to the same path, that we used [when building jluna](#building-jluna).

We can now call

```bash
# in ~/Desktop/MyProject
make 
```

Which will compile our application and leave us with `my_executable` which we can now execute:

```bash
# in ~/Desktop/MyProject
./my_executable
```
```
[JULIA][LOG] initialization successfull.
hello julia
```

---

## Troubleshooting

### Permission Denied

During `make install` or execution the bash script, your OS may notify you that it was unable to write to a folder due to missing permissions. Either run `make install` as `sudo` (or as administrator on windows), or specify a different folder for which basic processes have write permission.

### Unable to detect the Julia executable

When calling:

```bash
# in Desktop/jluna/build
cmake .. -DCMAKE_COMPILER=g++-10 # or other compiler
```

You may encounter the following error:

```
CMake Error at cmake/find/FindJulia.cmake:5 (message):
  Unable to detect the julia executable.  Make sure JULIA_BINDIR is set
  correctly.

  For more information, visit
  https://github.com/Clemapfel/jluna/blob/master/docs/installation.md
Call Stack (most recent call first):
  cmake/find/FindJulia.cmake:11 (julia_bail_if_false)
  CMakeLists.txt:20 (find_package)


-- Configuring incomplete, errors occurred!
```

This error appears because jluna was unable to locate the julia package on your system. To make jluna aware of the location manually, we can pass the following variable to the cmake command:

```bash
cmake .. -DJULIA_BINDIR=path/to/your/julia/bin -DCMAKE_COMPILER=g++-10
```
This is the path to he binary directory of your julia image. If you are unsure of it's location, you can execute

```julia
println(Sys.BINDIR)
```

From inside the julia REPL.

### Could NOT find Julia: Found unsuitable version

During the cmake configuration step, the following error may appear:

```
CMake Error at /home/(...)/FindPackageHandleStandardArgs.cmake:218 (message):
  Could NOT find Julia: Found unsuitable version "1.5.1", but required is at
  least "1.7.0" (found /home/clem/Applications/julia/lib/libjulia.so)
```

Where `1.5.1` could instead be any version before `1.7.0`. This means your julia version is out of date, either update it through your packet manager or download the latest version [here](https://julialang.org/downloads/), install it, then make sure `JULIA_BINDIR` is pointing to the newer version.

### Could NOT find Julia (missing: X)

Where X can be any of :
+ `JULIA_LIBRARY` 
+ `JULIA_EXECUTABLE` 
+ `JULIA_BINDIR` 
+ `JULIA_INCLUDE_DIR`

This means that either `JULIA_BINDIR` was not set correctly or the directory it is pointing to is not the julia binary dir (a path always ending in `/bin`), or your julia image is compressed. Make sure the folder `JULIA_BINDIR` points to has the following layout:

```
julia*/
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

Then set `JULIA_BINDIR` to `/path/to/.../julia/bin`.

### Cannot find <julia.h> / <jluna.hpp>

The following error may appear when compiling your library:

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

This means the `include_directories` in cmake were set improperly. Make sure the following lines are present in your `CMakeLists.txt`:

```
find_package(Julia)
find_package(jluna)
``` 

### When trying to initialize jluna.cppcall: cannot find libjluna_c_adapter

When calling

```cpp
State::initialize()
```

The following error may appear:

```
AssertionError: when trying to initialize jluna.cppcall: cannot find /home/Desktop/jluna/libjluna_c_adapter.so
Stacktrace:
 [1] verify_library()
   @ Main.jluna._cppcall ./none:951
 [2] top-level scope
   @ none:2
[JULIA][ERROR] initialization failed.
terminate called after throwing an instance of 'jluna::JuliaException'
  what():  [JULIA][EXCEPTION] AssertionError("[JULIA][ERROR] initialization failed.")

signal (6): Aborted
```

To allow the local julia state to interface with jluna, it needs the shared c-adapter-library to be available. During `make install`, the following line should have appeared:

```
[LOG] writing libjluna_c_adapter to globally available directory "/home/Desktop/jluna/libjluna_c_adapter.so"
```
Make sure the c_adapter shared library is still in that directory, if it is not, recompile jluna. The C-adapter library is installed into the directory specified by `CMAKE_INSTALL_PREFIX` and needs to remain there in order for `jluna` to work.
