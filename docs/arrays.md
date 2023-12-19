# Specialized Proxies: Arrays

One of the nicest features of Julia is its array interface. Therefore, it is only natural a Julia wrapper should extend all that functionality to a C++ canvas. While we learned before that we can use a `jluna::Proxy` like an array using `operator[](size_t)`:

```cpp
auto array_proxy = Main.safe_eval("return [1, 2, 3, 4]");

Int64 third = array_proxy[2];
std::cout << third << std::endl;
```
```
3
```

This is as far at it goes for array-related functionalities of `jluna::Proxy` itself. To access the full breadth of features jluna offers, we need to use `jluna::Array`.

To transform a `jluna::Proxy` to a `jluna::Array`, we can use `.as`:

```cpp
// declare regular proxy
Proxy array_proxy = Main.safe_eval("return Int64[1, 2, 3, 4]");

// convert to array proxy
auto array = array_proxy.as<Array<Int64, 1>>();
```

or, more conveniently, we can use an implicit `static_cast`:

```cpp
Array<Int64, 1> array = Main.safe_eval("return Int64[1, 2, 3, 4]");
```

which implicitly calls `as<Array<Int64, 1>` for us.

We see that `jluna::Array<T, N>` has two template parameters:

+ `T` is the **value type** of the array
+ `N` is the **dimensionality** of the array, sometimes called the **rank**. A 1d array is a vector, a 2d array is a matrix, etc.

Both these template arguments directly correspond to the Julia-side parameters of `Base.Array{T, N}`.

Note that we have declared the Julia-side vector `[1, 2, 3, 4]` to be of value-type `Int64`. This is important, because we specified the C++-side array to also have that value-type. On construction, `jluna::Array` will check if the value types match (that is, whether for all elements `e_n` in the Julia-side array, it holds that `e_n <: T`). If they do not, an exception is thrown.

Because the match does not have to be exact (see the section on [type ordering](types.md#type-order)), we can simply declare the value type to be `Any`. This lets us move any assortment of objects into the same array. jluna provides a typedef for this:

```cpp
using ArrayAny1d = // equivalent to Base.Array<Any, 1>
using ArrayAny2d = // equivalent to Base.Array<Any, 2>
using ArrayAny3d = // equivalent to Base.Array<Any, 3>
```

> **C++ Hint**: A `typedef` or "`using` declaration" is a way to declare a type alias. It is giving a type a new name, while keeping the old name valid. This is most commonly used to simplify syntax for the convenience of the programmer.

Which, while convenient, can introduce [type-instability](https://docs.julialang.org/en/v1/manual/performance-tips/#man-performance-value-type) to a piece of code may not require it.

## Accessing Array Indices

Before continuing, we need to set up our example arrays for this section:

```cpp
// create arrays
Array<Int64, 1> array_1d = jluna::safe_eval("return Int64[1, 2, 3, 4, 5]");
Array<Int64, 2> array_2d = jluna::safe_eval("return reshape(Int64[i for i in 1:9], 3, 3)");

// print
Base["println"]("array_1d: ", array_1d);
Base["println"]("array_2d: ", array_2d);
```
```
array_1d: [1, 2, 3, 4, 5]
array_2d: [1 4 7; 2 5 8; 3 6 9]
```

Where `jluna::safe_eval` is faster version of `Main.safe_eval`, as it returns a pointer to Julia-side values, not a proxy. Because we are binding the result to a proxy  (`jluna::Array`) anyway, `jluna::safe_eval` is the better choice in this situation. For more information, visit the section on [performance optimization tips](benchmarks.md#performance-evaluation--summary).

We created two arrays, a `Base.Array{Int64, 1}` bound to the C++-side array proxy `array_1d`, as well as a `Base.Array{Int64, 2}`, bound to `array_2d`.

To get a specific element of any array, we use `at(size_t...)`:

```cpp
// access elements at index (0-based)
Int64 one_d_at_3 = array_1d.at(2);
Int64 two_d_at_2_2 = array_2d.at(1, 1);

// print
std::cout << one_d_at_3 << std::endl;
std::cout << one_d_at_2_2 << std::endl;
```
```
3
5
```

> **Note**: Since jluna version 1.0.1, `operator[]` is no longer recommended to be used with `jluna::Array`. Instead, all array indexing is done through `Array::at`. This is because top-level comma expressions in `operator[]`, such as `x[1, 2, 3]`, are [marked deprecated and soon-to-be-removed for future C++ versions](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1161r3.html).

For a 1d array, `at` takes a single argument, its **linear index** (see below). For a 2d array, `at` takes two arguments, one for each dimension. This extends to any dimensionality, for a 5d array, we would call `at` with 5 integers. All indices used for member function of `jluna::Array` are 0-based.

Bounds-checking is performed Julia side, if an array element is accessed out of bounds, a `JuliaException` will be thrown.

### Linear Indexing

While n-dimensional indexing is only available for arrays of rank 2 or higher, linear indexing is available for all arrays, regardless of rank. We can linear-index any array using `at(size_t)`:

```cpp
Array<Int64, 3> array_3d = jluna::safe_eval("return reshape(Int64[i for i in 1:(3*3*3)], 3, 3, 3)");
std::cout << (Int64) array_3.at(3) << std::endl;
```
```
4
```
Linear indexing accesses the n-th element in column-first order, just like it would in Julia (except that the index in C++ is 0-based).

### List Indexing

jluna also supports Julia-style list indexing for `at`:

```cpp
Array<Int64, 1> vector = jluna::safe_eval("return [i for i in 1:10]");
auto sub_vector = vector.at({1, 5, 2, 7});

Base["println"](sub_vector);
```
```
[2, 6, 3, 8]
```
> **C++ Hint**: Here, the syntax used, `{1, 5, 2, 7}`, is called a "brace-enclosed initializer list", which is a form of [aggregate initialization](https://en.cppreference.com/w/cpp/language/aggregate_initialization) in C++. It can be best though of as a proto-vector, the compiler will infer the vectors type and construct it for us. In our case, because `Array::at` expects a list of integer, it will interpret `{1, 5, 2, 7}` as the argument for an implicitly called constructor for that type, creating a `std::vector<size_t>`.<br>
>
> see also: [list initialization](https://en.cppreference.com/w/cpp/language/list_initialization).

### 0-base vs. 1-base

This is about as good a place as any to talk about index bases. Consider the following:

```cpp
// create vector
Array<Int64, 1> array = jluna::safe_eval("return Int64[1, 2, 3, 4, 5, 6]");

// access element through C++ function
std::cout << "cpp: " << (Int64) array.at(3) << std::endl;

// access element through Julia function
std::cout << "jl : " << (Int64) Base["getindex"](array, 3) << std::endl;
```
```
cpp: 4
jl : 3
```

C++ indices are 0-based, this means `array.at(3)` will give use the `(3 - 0)`th element, which for our vector is `4`. In Julia, indices are 1-based, meaning `getindex(array, 3)` will give us the `(3 - 1)`th element, which is `3`.

The following table illustrates how to translate C++-side indexing into Julia-side indexing:

| Rank | Julia                | jluna                         |
|------|----------------------|-------------------------------|
| 1    | `M[1]`               | `M.at(0)`                     |
| 2    | `M[1, 2]`            | `M.at(0, 1)`                  |
| 3    | `M[1, 2, 3]`         | `M.at(0, 1, 2)`               |
| Any  | `M[ [1, 13, 7] ]`    | `M.at( {0, 12, 6} )`          |
| Any  | `M[i for i in 1:10]` | `M.at("i for i in 1:10"_gen)` |
|      |                      |                               |
| *    | `M[:]`               | not available                 |

Where `_gen` is a string-literal operator that  constructs a generator expression from the code it was called with. We will learn more about them shortly.

### Iterating

The main advantage `jluna::Array` has over the C-APIs `jl_array_t` is that it is **iterable**:

```cpp
Array<Int64, 1> array = jluna::safe_eval("return Int64[1, 2, 3, 4, 5, 6]");

for (Int64 i : array)
    std::cout << i << " ";

std::cout << std::endl;
```
```
1 2 3 4 5 6 
```
> **C++ Hint**: `std::endl` adds a `\n` to the stream, then flushes it.

Note, how we manually declared the iterators type to be `Int64`.

If we declare the iterator type as `auto`, similar to how proxies work, each iterator is assignable:

```cpp
for (auto iterator : array)
    it = static_cast<Int64>(it) + 1;

Base["println"](array);
```
```
[2, 3, 4, 5, 6, 7]
```

Where we had to `static_cast` the iterator, just like we would have to do with proxies.

Mutating the iterator also mutates the underlying Julia-array, with minimal overhead.

If the array is also a named proxy, it will also modify that specific element of whatever variable the proxy is managing.

### Accessing the Size of an Array

To get the size of an array, we use `get_n_elements`:

```cpp
Array<size_t, 1> vec = jluna::safe_eval("return UInt64[i for i in 1:333]");
std::cout << vec.get_n_elements() << std::endl;
```
```
333
```

This returns the number of elements in the array, not the size along a specific dimension. If we want the latter, we instead use `Array::size`, which takes as its only argument the index of the dimension (0-based). The size of a 3d array `array_3d` along its second dimension would be accessible via `array_3d.size(1)`.

### jluna::Vector

For arrays of dimensionality 1, a special proxy called `jluna::Vector<T>` is provided. It directly inherits from `jluna::Array<T, 1>`, all of `Array`s functionalities are also available to `Vector`.

In addition, the following member functions are only available for `jluna::Vector`:

+ `insert(size_t pos, T value)`
    - insert element `value` at position `pos` (0-based)
+ `erase(size_t pos)`
    - delete the element at position `pos` (0-based)
+ `push_front(T)`
    - push element to the front, such that it is now at position 0
+ `push_back(T)`
    - push element to the back of the vector

When boxing a `jluna::Vector<T>`, the resulting Julia-side value will be of type `Base.Vector{T}`. When boxing a `jluna::Array<T, 1>`, the result will be a value of type `Base.Array{T, 1}`.

## Generator Expressions

One of Julia's most convenient features are [**generator expressions**](https://docs.julialang.org/en/v1/manual/arrays/#man-comprehensions) (also called list- or array-comprehensions). These are is a special kind of syntax that creates an iterable, in-line, lazy-eval range.

> **Hint**: A lazy-eval range is a collection, that only allocates and/or computes its actual elements when that specific element is queried. After construction, no allocation is performed until requested by the user, for example by indexing the range or iterating over it.

For example:

```julia
# in Julia

# comprehension
out = [i*i for i in 1:10 if i % 2 == 0]

# mostly equivalent to
out = Vector()
let f = i -> i*i
    for i in 1:10
        if i % 2 == 0
            push!(out, f(i))
        end
    end
end
```
> **Julia Hint**: `let` introduces a new "hard" scope, such that any variable declared using it, `f` in our case, is only available in that scope.

In Julia, we use `[]` when we want the expression to be vectorized, and `()` when we want the expression to stay an object of type `Base.Generator`. Only the latter is lazy-eval, as `[]` triggers serialization.

In jluna, we can create a generator expression using the postfix string-literal operator `_gen`:

> **C++ Hint**: A postfix string-literal operator has to have a name like `_x`, where x is an arbitrary name. We call it by appending it to the end of a C-string: `"string value here"_x`.
>
> See the [C++ documentation](https://en.cppreference.com/w/cpp/language/user_literal) for more information.

```cpp
// in Julia:
(i for i in 1:10 if i % 2 == 0)

// in cpp:
"(i for i in 1:10 if i % 2 == 0)"_gen
```
Note that, when using `_gen`, **only round brackets are allowed**. Every generator expression has to be in round brackets, they cannot be omitted or replaced with another form of brackets. Otherwise, an exception will be raised.

We can iterate through a generator expression like so:

```cpp
for (jluna::Proxy i : "(i for i in 1:10 if i % 2 == 0)"_gen)
    std::cout << static_cast<Int64>(i) << " ";
```
```
2 4 6 8 10
```

Where `i` was explicitly declared to be of type `jluna::Proxy`.

While this is convenient, we can actually use generator expressions as arguments for many member functions of arrays, just like in Julia:

```cpp
// initialize a vector from a generator expression
// this is equivalent to serializing it with [] Julia-side
auto vec = Vector<Int64>("(i*i for i in 1:99)"_gen);

// use a generator expressions as a list index
vec["(i for i in 1:99 if i < 50)"_gen];
```

This imitates Julia syntax very closely, despite C++ being a language that does not have array comprehension. 

