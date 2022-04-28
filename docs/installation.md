# Installation & Troubleshooting

The following is a step-by-step guide on how to install jluna and link it to your application. The troubleshooting section addresses the most commorn errors experienced during installation.

### Table of Contents
1. [Installing jluna](#installing-jluna)<br>
    1.1 [Cloning from Git](#cloning-from-git)<br>
    1.2 [Configuring CMake](#configure-cmake)<br>
    1.3 [make install & test](#make-install--test)<br>
    1.4 [Linking](#linking)<br>
    1.5 [Example: Hello World](#example-hello-world)<br>
2. [Troubleshooting](#troubleshooting)<br>
    2.1 [Permission Denied](#permission-denied)<br>
    2.2 [Unable to Detect Julia Executable](#unable-to-detect-the-julia-executable)<br>
    2.3 [Found Unsuitable Version](#found-unsuitable-version)<br>
    2.4 [Could NOT find Julia: Missing X](#could-not-find-julia-missing-x)<br>
    2.5 [Could not find `julia.h` / `jluna.hpp`](#cannot-find-juliah--jlunahpp)<br>
    2.6 [Could not find libjluna_c_adapter](#cannot-find-libjluna_c_adapter)<br>
    2.7 [error: `concept` does not name a type](#error-concept-does-not-name-a-type)<br>
    2.8 [Segmentation fault in expression starting at none:0](#segmentation-fault-in-expression-starting-at-none0)<br>
   
## Installing jluna

This section will guide users on how to install jluna, either globally or in a folder-contained manner.

### Cloning from Git

First, jluna needs to be downloaded. To do this, navigate into any public folder (henceforth assumed to be `Desktop`) and execute:

```bash
git clone https://github.com/clemapfel/jluna.git
cd jluna
mkdir build
cd build
```

This will have cloned the jluna git repository into the folder `Desktop/jluna`. 

### Configure CMake

Before building, cmake needs to be configured. You can do so using:

```bash
# in Desktop/jluna/build
cmake .. -DCMAKE_CXX_COMPILER=<compiler> -DCMAKE_INSTALL_PREFIX=<path>
```

Where `<compiler>` is one of:
+ `g++-10`
+ `g++-11`
+ `clang++-12`

and `<path>` is the desired install path, usually `/usr/local` on unix, `C:/Program Files/jluna` on Windows. Keep track of this path, as you may need it later.

> Window supports is experimental. This means using MSVC may work, however this is currently untested. The [cpp compiler support]() page seems to imply that MSVC 19.30 or newer is required to compile jluna.

Some errors may appear here, if this is the case, head to [troubleshooting](#troubleshooting).

### Make Install & Test

Having successfully configured cmake, call:

```bash
# in Desktop/jluna/build
make install
```

Which will create two shared libraries `libjluna.*`, and `libjluna_c_adapter.*`, where `*` is the platform-dependent library suffix.

You can then verify everything is working correctly by calling

```bash
ctest --verbose
```

This will print a number of lines to the console. Make sure that at the very end, it says:

```
1: 
1: Number of tests unsuccessful: 0
1/1 Test #1: jluna_test .......................   Passed    4.65 sec

100% tests passed, 0 tests failed out of 1
```

### Linking

Now that jluna is installed on your system, your application can access it using:

```cmake
# in users own CMakeLists.txt
find_library(jluna REQUIRED)
```

If the above fails, you may need to manually specify, which directory jluna was installed into, using:

```cmake
# in users own CMakeLists.txt
find_library(jluna REQUIRED 
    NAMES jluna
    PATHS <install path>
)
```

Where `<install path>` is the path specified as `-DCMAKE_INSTALL_PREFIX` during configuration before.

Note that `jlunas` install rules also export is as a package, so it will be available through `find_package` on systems where it was installed globally:

```cmake
find_package(jluna REQUIRED)
```

After `find_library` or `find_package`, link your own library or executable to jluna like so:

```cmake
# in your own CMakeLists.txt
target_link_libraries(<target> PRIVATE 
    "${jluna}" 
    "${<julia package>}"
)
target_include_directories(<target> PRIVATE 
    "${<jluna include directory>}" 
    "${<julia include directory>}"
)
target_compile_features(<target> PRIVATE cxx_std_20)
```

Where 
+ `<target>` is the identifier of your library / executable
+ `<jluna include directory>` is the folder containing `jluna.hpp`
+ `<julia package>` is the library/package containing the julia C-API
+ `<julia include directory>` is the folder containing `julia.h`.

The shared julia library location is usually 
+ `${JULIA_BINDIR}/../lib` 
  
while the julia include directory is usually 
+ `${JULIA_BINDIR}/../include/` or 
+ `${JULIA_BINDIR}/../include/julia/`

If building your library triggers linker or compiler errors, head to [troubleshooting](#troubleshooting).

### Example: Hello World

A basic example main could be the following:

```cpp
#include <jluna.hpp>

using namespace jluna;
int main()
{
    initialize();
    Base["println"]("hello julia");
    
    return 0;
}
```

Where `jluna.hpp` already includes `julia.h`.

To learn how to use more of jlunas features, please consult the [manual](./manual.md).

---

## Troubleshooting

### Permission Denied

During `make install`, your OS may notify you that it was unable to write to a folder due to missing permissions. To fix this, either run `make install` as `sudo` (or as administrator on Windows), or specify a different folder (using `-DCMAKE_INSTALL_PREFIX` )for which jluna or cmake does have write/read permission.

### Unable to detect the Julia executable

When calling:

```bash
# in ~/Desktop/jluna/build
cmake .. #-DCMAKE_CXX_COMPILER=<compiler> -DCMAKE_INSTALL_PREFIX=<path>
```

(Where the commented out arguments are set as detailed in the [section on configuring cmake](#configure-cmake).)

You may encounter the following error:

```
CMake Error at cmake/find/FindJulia.cmake:5 (message):
  Unable to detect the julia executable.  Make sure JULIA_BINDIR is set
  correctly.
```

This error appears, because jluna was unable to locate the julia package on your system. To make jluna aware of the location manually, you can pass the following variable to the cmake command:

```bash
cmake .. -DJULIA_BINDIR=path/to/your/julia/bin #-DCMAKE_CXX_COMPILER=<compiler> -DCMAKE_INSTALL_PREFIX=<path>
```

Where `path/to/your/julia/bin` is the path of the binary directory of your julia image. If you are unsure of its location, you can execute

```julia
# in julia
println(Sys.BINDIR)
```

From inside the julia REPL, which will print the correct directory to the console.

### Found unsuitable version

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

This means that either `JULIA_BINDIR` was not set correctly, or the directory it is pointing to is not the julia binary directory. Verify that the value of `JULIA_BINDIR` starts at root (`/` on unix and `C:/` on Windows), ends in `/bin`, and that your julia image folder is uncompressed. 

Make sure the folder `JULIA_BINDIR` points to, has the following layout:

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

Where 
+ `*` may be a version suffix, such as `julia-1.7.2`
+ `libjulia.so` may have a different file extension on windows

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

This means the `include_directories` in your `CMakeLists.txt` were set improperly. Make sure the following lines are present in your `CMakeLists.txt`:

```
target_include_directories(<your target> PRIVATE
    "<path to jluna>" 
    "<path to julia>"
)
``` 
Where 

+ `<your target>` is the build target, a library or executable
+ `<path to jluna>` is the install path of the jluna shared library, as specified via `CMAKE_INSTALL_PREFIX` during [configuration](#configure-cmake)
+ `<path to julia>` is the location of `julia.h`, usually `${JULIA_BINDIR}/../include` or `${JULIA_BINDIR}/../include/julia`

See the [official CMake documentation](https://cmake.org/cmake/help/latest/command/target_include_directories.html) for more information.

### Cannot find libjluna_c_adapter

When calling

```cpp
jluna::initialize()
```

The following error may appear:

```
AssertionError: when trying to initialize jluna.cppcall: cannot find /home/Desktop/jluna/libjluna_c_adapter.so
Stacktrace:
 [1] verify_library()
   @ Main.jluna.cppcall ./none:951
 [2] top-level scope
   @ none:2
[JULIA][ERROR] initialization failed.
terminate called after throwing an instance of 'jluna::JuliaException'
  what():  [JULIA][EXCEPTION] AssertionError("[JULIA][ERROR] initialization failed.")

signal (6): Aborted
```

To allow the local julia state to interface with jluna, it needs the shared C-adapter-library to be available. During `make install`, jluna modifies its own code to keep track of the location of the C-adapter. If it was moved, jluna may no longer be able to find it.
To fix this, recompile jluna, as detailed [above](#make-install). 

The C-adapter library is always installed into the directory specified by `CMAKE_INSTALL_PREFIX`, regardless of cmake presets used. Be aware of this.

> **HACK**: Some IDEs and modern versions of cmake may override `CMAKE_INSTALL_PREFIX` between the time of configuration and build. As a hacky fix (March 2022), you can override the C-adapter shared library location manually, **before calling `jluna::initialize`**, using `jluna::set_c_adapter_path`:
> ```cpp
> int main()
> {
>   jluna::set_c_adapter_path("C:/actual/path/to/jluna_c_adapter.dll")
>   jluna::initialize();
>   (...)
> ```
> Where `jluna_c_adapter.dll` may have a differing pre- or suffix, depending on your system.

### error: `concept` does not name a type

When compiling a target that includes jluna, the following compiler error may occur:

```
/home/clem/Workspace/jluna/include/typedefs.hpp:90:5: error: ‘concept’ does not name a type
   90 |     concept to_julia_type_convertable = requires(T)
      |     ^~~~~~~
```

This indicates that you have not configured your compiler to utilize C++20, or your compiler is out of date. After verifying you are using `g++-10`, `g++-11`, `clang++-12` or `MSVC 19.32`, make sure the following line is present in your `CMakeLists.txt`:

```cmake
target_compile_features(<your target> PRIVATE cxx_std_20)
```

Where `<your target>` is the name of your compile target, such as an executable or library. See the [official cmake documentation](https://cmake.org/cmake/help/latest/command/target_compile_features.html), for more information.


### Segmentation fault in expression starting at none:0

When calling `jluna::initialize`, or any other jluna function, the following error may occur:

```cpp
[JULIA][LOG] initialization successful (1 thread(s)).

signal (11): Segmentation fault
in expression starting at none:0
Allocations: 1619712 (Pool: 1618782; Big: 930); GC: 1

Process finished with exit code 139 (interrupted by signal 11: SIGSEGV)
``` 

Where the above is the entirety of the console output. This error means that you tried to access jluna or the Julia C-API from inside a C-side thread that was not master (the thread `main` is executed in). Unrelated to jluna, the C-API disallows this. It will always trigger a crash when accessed concurrently. Please read the [multi-threading section of the manual](./manual.md#multi-threading) for more information.

---

If your particular problem was not addressed in this section, feel free to [open an issue on GitHub](https://github.com/Clemapfel/jluna/issues).