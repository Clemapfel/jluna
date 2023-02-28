# Specialized Proxies: Modules

We have already used `jluna::Module` (`Module` henceforth) in limited ways before, it is now time to learn about all its features. While non-module proxies are capable of manipulation the Julia state, most of `Module`s member functions [are more optimized](benchmarks.md#performance-evaluation--summary) for this purpose and should be preferred. Correct use of `Module` is central to any application using jluna.

### Assign in Module

Let's say we have a module `M` in main scope that has a single variable `var`:

```cpp
// declare module
Main.safe_eval(R"(
    module M
        var = []
    end
)");

// access module as unnamed proxy
Module M = Main.safe_eval("return M");
```

We've already seen that we can modify this variable using `M.safe_eval`, however, this is fairly slow performance-wise. This is, because we force Julia to `Meta.parse`, then `eval` the `"return M"`.

`Module::assign` [is much faster](benchmarks.md#mutating-julia-side-variables--results):

```cpp
// works but slow:
M.safe_eval("var = [777]");

// works and super fast:
M.assign("var", 777);
```

Where the first argument of assign is the variables name, the second argument is the new desired value.

If the variable we want to assign does not exist yet, `assign` will throw an exception. Instead, we can create new variables using `create_or_assign`:

```cpp
// create variable `new_var` and assign it 777
M.create_or_assign("new_var", 777);

// print value
Base["println"](M["new_var"]);
```
```
777
```

As the name suggest, if the variable does not exist, it is created. If the variable does exist, `create_or_assign` acts identically to `assign`.

### Creating a new Variable

A convenient function is `Module::new_*`. `Module::new_undef("var_name")`, for example, creates a new variable named `var_name` in that module, assigns it the value `undef`, then returns a named proxy to that new variable.

The following `new_*` functions are available:

| jluna Name     | C++ Argument Type(s)       | Julia-side Type of Result                  |
|----------------|----------------------------|--------------------------------------------|
| `new_undef`    | `void`                     | `UndefInitializer`                         |
| `new_bool`     | `bool`                     | `Bool`                                     |
| `new_uint8`    | `UInt8`                    | `UInt8`                                    | 
| `new_uint16`   | `UInt16`                   | `UInt16`                                   |
| `new_uint32`   | `UInt32`                   | `UInt32`                                   |
| `new_uint64`   | `UInt64`                   | `UInt64`                                   | 
| `new_int8`     | `Int8`                     | `Int8`                                     |
| `new_int16`    | `Int16`                    | `Int16`                                    | 
| `new_int32`    | `Int32`                    | `Int32`                                    |
| `new_int64`    | `Int64`                    | `Int64`                                    |
| `new_float32`  | `Float32`                  | `Float32`                                  |
| `new_float64`  | `Float64`                  | `Float64`                                  |
| `new_string`   | `std::string`              | `String`                                   |
| `new_symbol`   | `std::string`              | `Symbol`                                   |
| `new_complex`  | `T`, `T`                   | `Complex{T}`                               |
| `new_vector`   | `std::vector<T>`           | `Vector{T}`                                |
| `new_dict`     | `std::map<T, U>`           | `Dict{T, U}`                               |
| `new_dict`     | `std::unordered_map<T, U>` | `Dict{T, U}`                               |
| `new_set`      | `std::set<T>`              | `Set{T}`                                   |
| `new_pair`     | `T`, `T`                   | `Pair{T, T}`                               |
| `new_tuple`    | `T1` , `T2`, `...`, `Tn`   | `Tuple{T1, T2, ..., Tn}`                   |
| `new_array<T>` | `D1`, `D2`, `...`, `Dn`    | `Array{T, n}` of size `D1 * D2 * ... * Dn` |

This is a safe way and quick way to create proxies to newly created variables in module scope.

### Import, Using, Include

Additionally, `jluna::Module` provides the following functions wrapping the `using` `import`, and `include` functions:

+ `M.import("PackageName")`
    - equivalent to calling `import PackageName` inside `M`
+ `M.add_using("PackageName")`
    - equivalent to calling `using PackageName` inside `M`
+ `M.include("path/to/file.jl")`
    - equivalent to calling `include("path/to/file.jl")` inside `M`

Where `M` is a module.