# Creating a Project with jluna

The following is a step-by-step guide to creating an application using `jluna` from scratch. 

### Table of Contents

## Installing jluna

To install jluna globally, we navigate into any public folder (henceforth assumed to be `Desktop`) and execute:

```bash
git clone https://github.com/clemapfel/jluna.git
cd jluna
mkdir build
cd build
```

This will have cloned the `jluna` git repository into a folder called `Desktop/jluna`. We know call:

```bash
# in Desktop/jluna/build
cmake .. -DCMAKE_CXX_COMPILER=g++-10
make install
```
At this point, some errors may appear. If this is the case, head to [troubleshooting](#troubleshooting). 

We manually specify the C++ compiler here. It can be any of:
+ `g++-10`
+ `g++-11`
+ `clang++-12`

> Note that window supports is experimental. This means specifying `MSVC` may work, however this is untested. The [cpp compiler support]() list seems to imply that MSVC 19.30 or newer is required to compile `jluna`.

If the command does not recognize the compiler, even though you are are sure it is installed, it may be necessary to specify the full path to the compiler executable, e.g: `-DCMAKE_CXX_COMPILER=/usr/bin/g++-10`.

These commands will install jluna into the default shared library folder (usually `/usr/local` (on unix)). If we want the shared libraries to appear somewhere else, we can instead specify the directory like so:

```bash
# in Desktop/jluna/build
cmake .. -DCMAKE_CXX_COMPILER=g++-10 -DCMAKE_INSTALL_PREFIX=~/our/custom/path
make install
```

Having installed `jluna`, two shared libraries `libjluna.so` and `libjluna_c_adapter.so` have now appeared in the install directories. Advanced users can now make jluna available from withing their `CMakeLists.txt` by using:

```cmake
find_package(jluna REQUIRES NAMES libjluna.so HINTS /path/to/jluna)
```

Where `/path/to/jluna` is the directory jluna was installed in the previous step specified through `CMAKE_INSTALL_PREFIX`.

Note that on windows, instead of `libjluna.so`, one of `libjluna.lib`, `libjluna.dll` or `libjluna.dll.a` should be used.

## Troubleshooting

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
