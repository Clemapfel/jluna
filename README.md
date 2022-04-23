# jluna: A modern julia <-> C++ Wrapper (v0.8.6)

![](./header.png)

Julia is a beautiful language, it is well-designed, and well-documented. Julias C-API is also well-designed, less beautiful, and much less... documented.<br>
Heavily inspired in design and syntax by (but in no way affiliated with) the excellent Lua ⭤ C++ wrapper [**sol3**](https://github.com/ThePhD/sol2), jluna aims to fully wrap the official julia C-API, replacing it in projects with C++ as the host language, by making accessing julias unique strengths through C++ safe, hassle-free, and just as beautiful.

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

# TODO

---

### Features
+ expressive, generic syntax
+ create / call / mutate Julia-side values from C++
+ thread-safe, provides a custom threadpool that, [unlike the C-API](./docs/manual.md/#multi-threading), allows safe interfacing with Julia
+ `std::` types & usertypes can be moved freely between Julia and C++
+ call arbitrary C++ functions from Julia
+ multi-dimensional, iterable, 0-overhead array interface
+ **fast**: target overheads of ~5%, compared to the C-API
+ **safe**: full exception forwarding and verbose error messages
+ **well-documented**: extensive manual, installation guide, written by a human
+ and more!

### Planned (but not yet implemented):

jluna is feature complete as of 0.9.0. The library will continue to be supported. If no major issues appear, 0.9 will be upgraded to 1.0 in Winter 2022.

---

## Documentation

A verbose, step-by-step introduction and manual is available [here](./docs/manual.md). This manual is written for people less familiar with C++ and/or Julia, providing non-jluna related guidance where necessary.

Furthermore, all user-facing code has in-line documentation, available through most IDEs (or the julia `help?` command). 

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


> A step-by-step guide intended for users unfamiliar with cmake is available [here](./docs/installation.md).

Execute, in any public directory

```bash
git clone https://github.com/Clemapfel/jluna
cd jluna
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=<C++ Compiler>
make install
```

Where 
+ `<C++ Compiler>` is one of `g++-10`, `g++-11`, `clang++-12`

Afterwards, you can make jluna available to your library using 

```cmake
# inside your own CMakeLists.txt
find_library(jluna REQUIRED)
target_link_libraries(<Your Library> PRIVATE
    "${jluna}" 
    "${<Julia>}")
```
Where 
+ `<Julia>` is the Julia Library (usually available in `"${JULIA_BINDIR}/../lib"`)
+ `<Your Library>` is the name of your library or executable

If errors appear at any point, head to [troubleshooting](./docs/installation.md#troubleshooting).

---

## License

The current and all prior releases of jluna are supplied under MIT license, available [here](./LICENSE.txt).

I would like to ask people using this library in commercial or university settings, to disclose their usage of jluna in some small way (for example, at the end of the credits or via a citation) and to make clear the origin of the work (for example by linking this github page). Unlike the text in `LICENSE.txt`, this is not a legally binding condition, only a personal request by me, the developer.

For collaboration or further questions, feel free to [contact me](https://www.clemens-cords.com/contact).

Thank you for your consideration,
C.

---

## Credits
jluna was designed and implemented by [Clem Cords](https://github.com/Clemapfel).

#### March 2022:<br>
+ cmake improvements by [friendlyanon](https://github.com/friendlyanon)

---
