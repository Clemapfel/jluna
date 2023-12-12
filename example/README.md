# jluna Example

This is an example Julia project. First install jluna globally, making note of its include path.

If you are on non-linux, you may need to edit the following line in this projects `CMakeLists.txt`:

```cmake
set(jluna_INCLUDE_DIRS "/usr/local/include/jluna;/usr/local/include/jluna/include")
```
replace the path with wherever the jluna headers were installed too, see [here]() for more information.

To compile and run this project, call:

```shell
mkdir build
cd build
cmake ..
make
./jluna_example
```

If cmake fails to detect Julia, set `JULIA_BINDIR` to the result of 

```shell
julia -e "println(Sys.BINDIR)"
```

Executed in your bash shell.