# Creating a Project with jluna

The following is a step-by-step guide to creating an application using `jluna` from scratch. 


To install jluna globally, go into any public folder (henceforth assumed to be `Desktop`) and execute:

```bash
git clone https://github.com/clemapfel/jluna.git
cd jluna
mkdir build
cd build
```

This will have cloned the `jluna` git repository into a folder called `Desktop/jluna`. We know execute:

```bash
# in Desktop/jluna/build
cmake .. -DCMAKE_CXX_COMPILER=g++-10
```
At this point, some errors may appear. If this is the case, head to [troubleshooting](#troubleshooting). We manually specify the C++ compiler here. It can be any of:
+ `g++-10`
+ `g++-11`
+ `clang++-12`

If the command does not recognize the compiler, even though you are are sure it is installed, it may be necessary to specify the full path to the compiler executable, e.g: `-DCMAKE_CXX_COMPILER=/usr/bin/g++-10`.

After having configured cmake, we can now run: 

```bash
# in Desktop/jluna/build
make install
```

This will install jluna into the default shared library folder (usually `/usr/local` (on unix)). If we do not want jluna to be install there, we can manually specify `-DCMAKE_INSTALL_PREFIX`:

```bash
# in Desktop/jluna/build
make install -DCMAKE_INSTALL_PREFIX





