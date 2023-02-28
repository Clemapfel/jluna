## Troubleshooting

Several errors may arise during compilation, linking, or regular usage. A list of the most common issue, and how 
to solve them, is provided here.

------------------

### Permission Denied

During `make install`, your OS may notify you that it was unable to write to a folder due to missing permissions. To fix this, either run `make install` as `sudo` (or as administrator on Windows), or specify a different folder (using `-DCMAKE_INSTALL_PREFIX`) for which jluna or cmake does have write/read permission.

---

### Unable to detect the Julia executable

When calling:

```bash
# in ~/Desktop/jluna/build
cmake .. #-DCMAKE_CXX_COMPILER=<compiler> -DCMAKE_INSTALL_PREFIX=<path>
```

(Where the commented out arguments are set as detailed in the [section on configuring cmake](installation.md#configure-cmake).)

You may encounter the following error:

```text
CMake Error at cmake/find/FindJulia.cmake:5 (message):
  Unable to detect the Julia executable.  Make sure JULIA_BINDIR is set
  correctly.
```

This error appears, because jluna was unable to locate the Julia package on your system. To make jluna aware of the location manually, you can pass the following variable to the cmake command:

```bash
cmake .. -DJULIA_BINDIR=path/to/your/julia/bin #-DCMAKE_CXX_COMPILER=<compiler> -DCMAKE_INSTALL_PREFIX=<path>
```

Where `path/to/your/julia/bin` is the path of the binary directory of your Julia image. If you are unsure of its location, you can execute

```julia
# in Julia
println(Sys.BINDIR)
```

From inside the Julia REPL, which will print the correct directory to the console.

---

### Found unsuitable version

During the cmake configuration step, the following error may appear:

```text
CMake Error at /home/(...)/FindPackageHandleStandardArgs.cmake:218 (message):
  Could NOT find Julia: Found unsuitable version "1.5.1", but required is at
  least "1.7.0" (found /home/clem/Applications/julia/lib/libjulia.so)
```

Where `1.5.1` could instead be any version before `1.7.0`. This means your Julia version is out of date, either update it through your packet manager or download the latest version [here](https://julialang.org/downloads/), install it, then make sure `JULIA_BINDIR` is pointing to the newer version.

---

### Could NOT find Julia (missing: X)

Where X can be any of :
+ `JULIA_LIBRARY`
+ `JULIA_EXECUTABLE`
+ `JULIA_BINDIR`
+ `JULIA_INCLUDE_DIR`

This means that either `JULIA_BINDIR` was not set correctly, or the directory it is pointing to is not the Julia binary directory. Verify that the value of `JULIA_BINDIR` starts at root (`/` on unix and `C:/` on Windows), ends in `/bin`, and that your julia image folder is uncompressed.

Make sure the folder `JULIA_BINDIR` points to, has the following layout:

```text
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
+ `libjulia.so` may have a different file extension on Windows

---

### Cannot find <julia.h> / <jluna.hpp>

The following error may appear when compiling your library:

```text
fatal error: julia.h: No such file or directory
    3 | #include <julia.h>
      |          ^~~~~~~~~
compilation terminated.
```

or, similarly:

```text
fatal error: jluna.hpp: No such file or directory
    3 | #include <jluna.hpp>
      |          ^~~~~~~~~~~
compilation terminated.
```

This means the `include_directories` in your `CMakeLists.txt` were set improperly. Make sure the following lines are present in your `CMakeLists.txt`:

```text
target_include_directories(<your target> PRIVATE
    "<path to jluna>" 
    "<path to julia>"
)
``` 
Where

+ `<your target>` is the build target, a library or executable
+ `<path to jluna>` is the installation path of the jluna shared library, as specified via `CMAKE_INSTALL_PREFIX` during [configuration](installation.md#configure-cmake)
+ `<path to julia>` is the location of `julia.h`, usually `${JULIA_BINDIR}/../include` or `${JULIA_BINDIR}/../include/julia`

See the [official CMake documentation](https://cmake.org/cmake/help/latest/command/target_include_directories.html) for more information.

---

### Cannot find libjluna

When calling

```cpp
jluna::initialize()
```

The following error may appear:

```text
AssertionError: when trying to initialize jluna.cppcall: cannot find /home/Desktop/jluna/libjluna.so
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

To allow the jluna initialized Julia state to interact with the library, the Julia part of jluna needs to know where to
find the jluna shared library, `libjluna.so`, you compiled earlier. If this error occurs, it means Julia was unable to
do so. To address this, you can manually specify the path to the shared library during `jluna::initialize`:

```cpp
// initialize with manually set path
jluna::initialize(
    1,        // number of threads
    false,    // should logging be disabled
    "/path/to/libjluna.so" // path to jluna shared library
);
```

Where `/path/to/libjluna.so` is the absolute path to the jluna shared library. Note that, on Windows, the library may
instead be named `libjluna.lib`, `jluna.dll`, etc., depending on your cmake environment. If you are unsure of where to
find the library, it will be installed into the directory of `CMAKE_INSTALL_PREFIX` specified [earlier](installation.md#configure-cmake).

---

### error: `concept` does not name a type

When compiling a target that includes jluna, the following compiler error may occur:

```text
/home/clem/Workspace/jluna/include/typedefs.hpp:90:5: error: ‘concept’ does not name a type
   90 |     concept to_julia_type_convertable = requires(T)
      |     ^~~~~~~
```

This indicates that you have not configured your compiler to utilize C++20, or your compiler is out of date. After verifying you are using `g++-10` (or newer), `clang++-12` (or newer) or `MSVC 19.32` (or newer), make sure the following line is present in your `CMakeLists.txt`:

```cmake
target_compile_features(<your target> PRIVATE cxx_std_20)
```

Where `<your target>` is the name of your compile target, such as an executable or library. See the [official cmake documentation](https://cmake.org/cmake/help/latest/command/target_compile_features.html), for more information.

---

### Segmentation fault in expression starting at none:0

When calling `jluna::initialize`, or any other jluna function, the following error may occur:

```text
[JULIA][LOG] initialization successful (1 thread(s)).

signal (11): Segmentation fault
in expression starting at none:0
Allocations: 1619712 (Pool: 1618782; Big: 930); GC: 1

Process finished with exit code 139 (interrupted by signal 11: SIGSEGV)
``` 

Where the above is the entirety of the console output. This error means that you tried to access jluna or the Julia C-API from inside a C-side thread that was not master (the thread `main` is executed in). Unrelated to jluna, the C-API disallows this. It will always trigger a crash when accessed concurrently. Please read the [multi-threading section of the manual](multi_threading.md) for more information.

---

### Warning: copy relocation against non-copyable protected symbol

When running your executable that was linked with jluna, the following warning may appear at runtime:

```
warning: copy relocation against non-copyable protected symbol `jl_nothing' in `/lib64/libjulia.so.1'
```

Where `jl_nothing` may be another symbol. This warning is triggered by newer versions of clang and gcc and does not indicate a problem. You can silence it by adding the following to *your* `CMakeLists.txt`:

```cmake
target_compile_options(<your_target> PRIVATE "-fpic")
```

Where `<your_target>` is your own executable or library using jluna, not jluna itself. If your target already has compile options specified, simply append `-fpic` at the end.

For more information, see https://github.com/Clemapfel/jluna/issues/40.

---

If your particular problem was not addressed in this section, feel free to [open an issue on GitHub](https://github.com/Clemapfel/jluna/issues).
