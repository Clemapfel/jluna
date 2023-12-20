# Installation

The following is a step-by-step guide of how to install jluna on our system and how to create a project that uses jluna. If an error appears at any point, we can head to the
[section on troubleshooting](troubleshooting.md), which is likely to have a solution.

---

## Installing CMake

Jluna uses CMake for compilation and installation and git for source distribution. If either is not already present on our system, we need to install them.

### Debian, Ubuntu, Linux Mint

On debian-like linux distributions, we run (in any public directory):

```bash
sudo apt-get install cmake git
```

### Fedora

On fedora, we run:

```bash
sudo dnf install cmake git
```

### macOS

On apple devices, we need install [homebrew](https://brew.sh/), then, in any public directory:

```bash
sudo brew install cmake git
```

### Windows

On windows, we open the console, then:

```shell
winget install cmake
winget install Git.Git
```

---

## Installing jluna

### Linux, macOS

This section will show how to install jluna on any Linux distribution or macOS. **If you are using Windows, skip to the next section**.

Firstly, the jluna sources need to be downloaded to our system. To do this, we call, in any public directory, henceforth assumed to be `Desktop`:

```bash
# in Desktop/
git clone https://github.com/clemapfel/jluna.git
cd jluna
mkdir build
cd build
```

We now need to compile jluna and link it against the specific Julia package on our system. For most users, the following will do so automatically:

```bash
# in Desktop/jluna/build
cmake ..
sudo make install -j 8
```

Where `sudo` is required to write to the global shared library directory.

If CMake fails to locate Julia, we need to make sure the `julia` command is available in the same console we called `cmake ..` from. If the command is not available, we should add the path of the Julia executable to our `PATH` variable, see [here](https://www.digitalocean.com/community/tutorials/how-to-install-julia-programming-language-on-ubuntu-22-04#step-2-adding-julia-to-your-path) for more information.

If jluna is still unable to find Julia, or compiler errors appear, head to [troubleshooting section](troubleshooting.md).

### Windows

#### Using the Console

Run, in any public directory, henceforth assumed to be `Desktop`:

```bash
# in Desktop
git clone https://github.com/clemapfel/jluna.git
cd jluna
mkdir build
cd build
```

To build jluna, call

```bash
# in Desktop\jluna\build
cmake ..
cmake --build . -j 8
```

The `julia` command needs to be available from the same console `cmake ..` was called. If this is not the case, make sure you followed the [official instructions](https://julialang.org/downloads/platform/#windows) when installing Julia such that is installed globally.

#### Using Visual Studio

To install jluna using Visual Studio (VS), start VS, then choose "Clone a repository", enter the URL `https://github.com/clemapfel/jluna`, then choose "clone".

Let VS initialize the CMake environment, then, choose `Build > Install`. VS should automatically detect everything needed and do a system-wide install.

---

## Creating a new Project using jluna

If we are just starting out and want to create a fresh project, it's easiest to download the **example project template**. It contains everything needed to create a new C++ executable (or library) that uses jluna, on any operating system.

To download the template, navigate to the folder we want our project to be located in, henceforth assumed to be `Workspace`

```bash
# in Workspace
git clone https://github.com/jluna
cp -r jluna/example jluna_example
rm -r jluna
```

This will create a folder `jluna_example` in the `Workspace` directory.

> **Tip**: If we want to use an IDE such as Visual Studio or CLion, simply opening `jluna_example/CMakeLists.txt` in that IDE should be the only thing that is needed to start a new project

To compile our new project manually, we do:

```bash
# in Workspace
cd jluna_example
mkdir build
cd build
cmake ..
make
```

This will deposit an executable `jluna_example` into `Workspace/jluna_example/build`. To run it, we simply do:

```bash
# in Workspace/jluna_example/build
./jluna_example
```
```
[JULIA][LOG] initialization successful (2 thread(s)).
hello world!
```

We can modify this project freely, see the source code comments in `jluna_example/CMakeLists.txt`, `jluna_example/jluna_example.hpp`, and `jluna_example/main.cpp`. 

---

## Adding jluna to an existing project

### CMake

If we already have an established project and we want to add jluna to its set of tools, we need to modify our own projects `CMakeLists.txt`, henceforth assumed to be located at `existing_project/CMakeLists.txt`. 

After installing jluna, it can be located using the `find_package` command:

```cmake
# in existing_project/CMakeLists.txt
find_package(jluna REQUIRED)
```

This command locates jluna and sets two cmake variables:

+ `JLUNA_LIBRARIES` contains the path to jluna and Julia shared libraries
+ `JLUNA_INCLUDE_DIRECTORIES` contains the path to the jluna and Julia headers

We can then link against jluna and Julia like so:

```cmake
# set cpp standard to c++20
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# add jluna and Julia include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${JLUNA_INCLUDE_DIRECTORIES}
    # other include directories here
)

# link against jluna and Julia
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${JLUNA_LIBRARIES}
    # other libraries here
)
```

where `PROJECT_NAME` is a cmake variable with the name of our existing cmake target.

To import jluna functionality into C++, we simply add `#include <jluna.hpp>` to our already existing headers.

---

### Meson

While jluna requires CMake for installation, projects using jluna are not limited to CMake. If we would like to use [meson](https://mesonbuild.com/), we can access jluna and Julia like so:

```meson
cpp_compiler = meson.get_compiler('cpp')

# find jluna
jluna = dependency('jluna',
    required: true,
    version: '>=1.1.0',
    method: 'cmake'
)
jluna_include_directories = include_directories(jluna.get_variable('JLUNA_INCLUDE_DIRECTORIES'))

# find julia
julia = cpp_compiler.find_library('julia',
    dirs: jluna.get_variable('JULIA_LIBRARY_PATH'),
    required: true
)
```

We can then link against both by adding them to the [dependencies](https://mesonbuild.com/Reference-manual_returned_exe.html) key of a build target:

```meson
target = executable(target_name,
    sources: project_sources,
    dependencies: [jluna, julia],
    include_directories: jluna_include_directories
)
```

Where `target_name` is the name of our executable (or library) target, `project_sources` are the headers and source files of our own project, not those of jluna.

---

### Docker

A community member has created an example project for jluna using Docker. See [here](https://github.com/Clemapfel/jluna/issues/51) for more information.