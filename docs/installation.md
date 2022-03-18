# Creating a Project with jluna

The following is a step-by-step guide on how to install jluna, and how to create our own C++ application using jluna from scratch.

### Table of Contents
1. [Installing jluna](#installing-jluna)<br>
   1.1 [Cloning from Git](#cloning-from-git)<br>
   1.2 [Configuring CMake](#configure-cmake)<br>
   1.3 [make install](#make-install)<br>
2. [Troubleshooting](#troubleshooting)<br>
    2.1 [Permission Denied](#permission-denied)<br>
    2.2 [Unable to Detect Julia Executable](#unable-to-detect-the-julia-executable)<br>
    2.3 [Found Unsuitable Version](#could-not-find-julia-found-unsuitable-version)<br>
    2.4 [Could NOT find Julia: Missing X](#could-not-find-julia-missing-x)<br>
    2.5 [Could not find `julia.h` / `jluna.hpp`](#cannot-find-juliah--jlunahpp)<br>
    2.6 [Could not find libjluna_c_adapter](#when-trying-to-initialize-jlunacppcall-cannot-find-libjluna_c_adapter)<br>
   
## Installing jluna

This section will guide users on how to install `jluna`, either globally or in a folder-contained manner.

### Cloning from Git

We first need to download `jluna`, to do this we navigate into any public folder (henceforth assumed to be `Desktop`) and execute:

```bash
git clone https://github.com/clemapfel/jluna.git
cd jluna
mkdir build
cd build
```

This will have cloned the `jluna` git repository into the folder `Desktop/jluna`. 

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

And `<path>` is the desired install path. `-DCMAKE_INSTALL_PREFIX=<path>` is optional, if it is specified manually (not recommended), keep note of this path as we will need it later.


> Window supports is experimental. This means using MSVC may work, however this is currently untested. The [cpp compiler support]() page seems to imply that MSVC 19.30 or newer is required to compile `jluna`.

Some errors may appear here, if this is the case, head to [troubleshooting](#troubleshooting).

If the command does not recognize the compiler, even though you are sure it is installed, it may be necessary to specify the full path to the compiler executable instead, like so:

```
-DCMAKE_CXX_COMPILER=/usr/bin/g++-10
```

### Make Install

Having successfully configured cmake, we now call:

```bash
# in Desktop/jluna/build
make install
```

Which will create two shared libraries `libjluna.*`, and `libjluna_c_adapter.*`, where `*` is the platform-dependent library suffix.

Now that `jluna` is installed on our system, we can link any executable or library inside any non-`jluna` `CMakeLists.txt` using:

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

Note that `jlunas` install rules also export is as a package, so it will be available through `find_package` on systems where it was installed globally like so:

```cmake
find_package(jluna REQUIRED)
```

After `find_library` or `find_package`, we link our own library like so:

```cmake
# in users own CMakeLists.txt
target_link_libraries(my_library "${jluna}" "${<julia package>}")
target_include_directories("${JLUNA_DIR}" "${<julia include directory>}")
```

Where `<julia package>` is the library/package containing the julia C-API and `<julia include directory>` is the folder containing `julia.h`.

The shared julia library location is usually 
+ `${JULIA_BINDIR}/../lib` 
  
while the julia include directory is usually 
+ `${JULIA_BINDIR}/../include/` or 
+ `${JULIA_BINDIR}/../include/julia/`

If building your library triggers linker or compiler errors, head to [troubleshoot](#troubleshooting).

---

## Troubleshooting

### Permission Denied

During `make install`, or during execution of the bash script, your OS may notify you that it was unable to write to a folder due to missing permissions. To fix this, either run `make install` as `sudo` (or as administrator on windows), or specify a different folder using `-DCMAKE_INSTALL_PREFIX` for which `jluna` or cmake does have write/read permission.

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
```

This error appears, because `jluna` was unable to locate the julia package on your system. To make `jluna` aware of the location manually, we can pass the following variable to the cmake command:

```bash
cmake .. -DJULIA_BINDIR=path/to/your/julia/bin -DCMAKE_COMPILER=g++-10
```

Where `path/to/your/julia/bin` is the path of the binary directory of your julia image. If you are unsure of its location, you can execute

```julia
println(Sys.BINDIR)
```

From inside the julia REPL.

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

This means that either `JULIA_BINDIR` was not set correctly or the directory it is pointing to is not the julia binary directory. Verify that the value of `JULIA_BINDIR` starts at root (`/` on unix and `C:/` on windows), ends in `/bin`, and that your julia image folder is uncompressed. 

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

Where `*` may be a version suffix, such as `julia-1.7.2`.

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
target_include_directories(<path to jluna>)
target_include_directories(<path to julia>)
``` 
Where 

+ `<path to jluna>` is the install path of the `jluna` shared libary, possibly specified during cmake configuration using `-DCMAKE_INSTALL_PREFIX`
+ `<path to julia>` is the location of `julia.h`, usually `${JULIA_BINDIR}/../include` or `${JULIA_BINDIR}/../include/julia`

### Cannot find libjluna_c_adapter

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

To allow the local julia state to interface with `jluna`, it needs the shared C-adapter-library to be available. During `make install`, `jluna` modifies its own code to keep track of the location of the C-adapter. If it was moved, `jluna` may no longer be able to find it.
To fix this, recompile jluna, as detailed [above](#make-install). 


The C-adapter library is always installed into the directory specified by `CMAKE_INSTALL_PREFIX`, regardless of cmake presets used. Be aware of this.

---

If your particular problem was not addressed in this section, feel free to [open an issue on GitHub](https://github.com/Clemapfel/jluna/issues).
