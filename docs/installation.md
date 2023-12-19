# Installation

The following is a step-by-step guide of how to install jluna on our system and how to create a project that uses jluna. If an error appears at any point, we can head to the
[section on troubleshooting](troubleshooting.md) which is likely to have a solution.

---

## Installing jluna

### Unix

This section shows how to install jluna on an Linux distro or macOS. **If you are using Windows, skip to the next section**.

To install jluna, we first need to download it to our system. To do this, we call, in any public directory, henceforth assumed to be `Desktop`:

```bash
# in Desktop/
git clone https://github.com/clemapfel/jluna.git
cd jluna
mkdir build
cd build
```

We now need to compile jluna and link it against the specific Julia on our system. For most users, the following will do so automatically:

```bash
# in Desktop/jluna/build
cmake ..
sudo make install -j 8
```

Where `sudo` is required to write to the global shared library directory.

If cmake fails to locate Julia, or compiler errors appear, head to the [troubleshooting section](troubleshooting.md).

### Windows

TODO

---

## Creating a new Project using jluna

If we are creating a completely new project, we can download the example project template to make starting out easy. It contains everything needed to create a new C++ executable or library using jluna.

To download the template, navigate to the folder you want your project to be located in, henceforth assumed to be `Workspace`

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



---

### Docker

A community member has created an example project for jluna using Docker. See [here](https://github.com/Clemapfel/jluna/issues/51) for more information.