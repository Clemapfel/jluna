# Installation

The following is a step-by-step guide on how to install jluna and link it to your application. The [upcoming section on troubleshooting](troubleshooting.md) addresses the most common errors experienced during installation.

----------------

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
+ `g++-10` (or newer)
+ `clang++-12` (or newer)
+ `MSVC 19.30` (or newer)

and `<path>` is the desired install path, usually `/usr/local` on unix, `C:/Program Files/jluna` on Windows. Keep track of this path, as you may need it later.

Some errors may appear here, if this is the case, head to [troubleshooting](troubleshooting.md).

### Make Install & Test

Having successfully configured cmake, call:

```bash
# in Desktop/jluna/build
make install
```

Which will create the shared libraries `libjluna.*`, where `*` is the platform-dependent library suffix.

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
find_library(jluna REQUIRED NAMES jluna)
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

If building your library triggers linker or compiler errors, head to [troubleshooting](troubleshooting.md).

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

To learn how to use more of jlunas features, continue on to the [manual](basics.md).