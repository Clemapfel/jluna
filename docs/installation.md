# Installation

The following is a step-by-step guide on how to install jluna and link it to your application. The [upcoming section on troubleshooting](troubleshooting.md) addresses the most common errors experienced during installation.

----------------

## Installing jluna

It is recommended to install jluna globally on our system, as it makes creating new projects and locating jluna easier.

To install jluna globally, in any public directory, henceforce assumed to be `Desktop`, run:

```shell
# in Desktop/
git clone https://github.com/clemapfel/jluna
cd jluna
mkdir build
cd build
cmake ..
sudo make install
```

Where, in the last line, `sudo` is necessary on some operating systems in order to install to the global shared library directory (usually `/usr/local` on unix)

> If error messages appear during `cmake ..` or `make install`, head to the [troubleshooting section](./troubleshooting.md).

## Testing jluna

To make sure everything works, we should test jluna after it is installed. To do this, in the same folder as `cmake ..`, we run:

```shell
# in Desktop/jluna/build
ctest --verbose
```

At the end it should say:

```
Number of tests unsuccessful: 0

Process finished with exit code 0
```

We can now move onto creating our own project using jluna

---

## Creating a User Project

If you are starting a new project wanting to use jluna, you can download the [example project template](), which will have anything needed to build and link against jluna.

To download the template, in the same directory as `jluna`, run:

```shell
# in Desktop/jluna

```




