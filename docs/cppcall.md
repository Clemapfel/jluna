## Calling C++ Functions from Julia

We've seen how to call Julia functions from C++. Despite being more of a Julia-wrapper for C++ than a C++-wrapper for Julia, in jluna, calling C++ functions from Julia is actually just as convenient and [performant](benchmarks.md#calling-c-functions-from-julia--results).

To call a C++ function, we need to assign to a Julia-side variable, a **lambda**.

> **C++ Hint**: Lambdas are C++s anonymous function objects. Before continuing with this section, it is recommended to read up on the basics of lambdas [here](https://docs.microsoft.com/en-us/cpp/cpp/lambda-expressions-in-cpp). Users are expected to know about basic syntax, trailing return types and capture clauses from this point onward.

### Creating a Function Object

Let's say we have the following, simple lambda:

```cpp
auto add = [](Int64 a, Int64 b) -> Int64 
{
    return a + b;
};
```

This function has the signature `(Int64, Int64) -> Int64`.

When interfacing with jluna, we should always manually specify the trailing return type of lambda using `->`. We should never use `auto`, either for the lambdas return- or any of the argument-types.

To make this function available to Julia, we use `as_julia_function`.
+ the argument of `as_julia_function` is a lambda or `std::function` object
+ the template argument of `as_julia_function` is the functions signature

> **C++ Hint**: `std::function` is a class able to wrap any function in a movable object. See the [official documentation](https://en.cppreference.com/w/cpp/utility/functional/function) for more details.

Because `add` has the signature `(Int64, Int64) -> Int64`, we use `as_julia_function<Int64(Int64, Int64)>`.

> **C++ Hint**: `std::function` and thus `as_julia_function` uses the C-style syntax for a functions' signature. A function with return-type `R` and argument types `T1, T2, ..., Tn` has the signature `(T1, T2, ..., Tn) -> R`, or `R(T1, T2, ..., Tn)` in C-style.

We can then assign the result of `as_julia_function` to a Julia variable like so:

```cpp
// declare lambda
auto add = [](Int64 a, Int64 b) -> Int64 
{
    return a + b;
};

// bind to Julia-side variable
Main.create_or_assign("add", as_julia_function<Int64(Int64, Int64)>(add));
```

From this point onwards, we can simply call the C++-side `add` by using the newly created Julia-side variable `Main.add`:

```cpp
Main.safe_eval("println(add(1, 3))");
```
```
4
```

The return value of `as_julia_function` is a Julia-side object. This means, we can assign it to already existing proxies, or otherwise handle it like any other Julia-side value.

The Julia function can be called with Julia- or C++-side arguments, and its return value can be directly accessed from both Julia and C++.

### Allowed Signatures

Not all function signatures are supported for `as_julia_function`. Its argument (the C++ function) can only have one of the following signatures:

```cpp
() -> T_r
(T1) -> T_r
(T1, T2) -> T_r
(T1, T2, T3) -> T_r
```

Where
+ `T_r` is `void` or [unboxable](proxies.md#-un--boxable-types)
+ `T1`, `T2`, `T3` are  [boxable](proxies.md#-un--boxable-types)

This may seem limiting at first, how could we execute arbitrary C++ code when we are only allowed to use functions with a maximum of three arguments using only (Un)Boxable types? The next sections will answer this question.

### Taking Any Number of Arguments

Let's say we want to write a function that takes any number of `String`s and concatenates them. Obviously, just 3 arguments are not enough for this. Luckily, there is a workaround. Instead of using a n-argument function, we can use a 1-argument function where the argument is a n-element vector:

```cpp
// declare lambda, jluna::Array (aka. Base.Array) as argument
auto concat_all = [](jluna::Array<std::string, 1> arg) -> std::string
{
    // append through stringstream
    std::stringstream str;
    for (std::string s : arg)
        str << s;
    
    str << std::endl;
    
    // return string
    return str.str();
};
```
> **C++ Hint**: `std::stringstream` is a stream that we can write strings into using `operator<<`. We then flush it using `std::endl`, and convert its contents to a single `std::string` using the member function `.str()`. More info about `std::stringstream` can be found in the [C++ manual](https://en.cppreference.com/w/cpp/io/basic_stringstream).

This lambda has the signature
+ `(jluna::Array<std::string, 1>) -> std::string`

Because `jluna::Array<T, N>` boxes into `Base.Array{T, N}`, Julia-side, the resulting function will have the signature

+ `(Base.Array{String, 1}) -> String`

Therefore, we move it Julia-sie using `as_julia_function` like so:

```cpp
// create new variable
Main.create_or_assign(
    "concat_all",           // variable name
    as_julia_function<      // as_julia_function call
        std::string(jluna::Array<std::string, 1>) // C++ signature
    >(concat_all)           // lambda
);
```

We can then call `concat_all` Julia-side, with any number of arguments, by wrapping the arguments in a Julia-side vector:

```cpp
Main.safe_eval(R"(
    println(concat_all(["GA", "TT", "AC", "A"]))
)");
```
```
GATTACA
```

If we want to truly call it with any number of arguments, not just a vector, we can simply do:

```cpp
// declare lambda
auto concat_all = [](jluna::Array<std::string, 1> arg) -> std::string
{
    // ...
};

// bind lambda to `concat_all_aux`
Main.create_or_assign(
    "concat_all_aux",   // now named concat_all_aux
    as_julia_function<std::string(jluna::Array<std::string, 1>)>(concat_all)
);

// create new proper Julia function `concat_all`
// that forwards its n arguments as a 
// n-sized vector to `concat_all_aux`
Main.safe_eval(R"(
    concat_all(xs::String...) = concat_all_aux(String[xs...])
)");
    
// can now be called with n arguments
Main.safe_eval(R"(
    println(concat_all("now ", "callable ", "like ", "this"))
)");
```
```
now callable like this
```
Where we renamed the object holding the C++ lambda `concat_call_aux`, then called that object using a Julia-method with signature `(::String...) -> String`, which forwards its arbitrary number of arguments as a vector to `concat_call_aux`, thus achieving the desired syntax.

If we want our lambda to take any number of *differently-typed* arguments, we can either wrap them in a `jluna::ArrayAny1d` (which has the value type `Any` and thus can contain elements of any type), or we can use a `std::tuple`, both of which are (Un)Boxable. The latter should be preferred for performance reasons.

### Using Non-Julia Objects

We now know how to work around the restriction on the number of arguments, but what about the types? Not all types are [(Un)Boxable](proxies.md#-un--boxable-types), but this does not mean we cannot use arbitrary C++ types. How? By using **captures**.

Let's say we have the following C++ class:

```cpp
// declare non-Julia class
struct NonJuliaObject
{
    // member
    Int64 _value;
    
    // ctor
    NonJuliaObject(Int64 in)
        : _value(in)
    {}
    
    // member function: doubles _value n-times
    void double_value(size_t n)
    {
        for (size_t i = 0; i < n; ++i)
            _value = 2 * _value;
    }
};

// instance the class C++-side
auto instance = NonJuliaObject(13);
```

This object is obviously not (Un)Boxable.

The naive approach to modifying `instance` would be with the following lambda:

```cpp
auto modify_instance = [](NonJuliaObject& instance, size_t n) -> void
{
    instance.double_value(n);
};
```

This lambda has the signature `(NonJuliaObject&, size_t) -> void`, which is a disallowed signature because `NonJuliaObject&` is not boxable.

Instead of handing `instance` to the lambdas function body through an argument, we can instead **forward it through its capture**:

```cpp
auto modify_instance = [instance_ref = std::ref(instance)] (size_t n) -> void
{
    instance_ref.get().double_value(n);
    return;
};
```

> **C++ Hint**: `std::ref` is used to create a [reference wrapper](https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper) around any instanced object. It is very similar to Julia's `Base.Ref` in functionality. To "unwrap" it, we use `.get()` in C++, where we would use `[]` in Julia.

Lambda syntax can get quite complicated, so let's talk through this step-by-step.

Firstly, this second lambda has the signature `(size_t) -> void`. Capture variables do not affect a lambdas signature.

Inside the capture `[]`, we have the expression `instance_ref = std::ref(instance)`. This expression creates a new variable, `instance_ref`, that will be available inside the lambdas body. We initialize `instance_ref` with `std::ref(instance)`, which creates a reference wrapper around our desired C++-side instance. A reference wrapper acts the same as a plain reference in terms of memory ownership, as long as the reference wrapper stays in scope, `instance` will too. Therefore, as long as the lambda body stays in scope, so will `instance_ref` and therefore `instance`.

Having captured `instance` through the reference wrapper, we can modify it inside our body by first unwrapping it using `.get()`, then applying whatever mutation we intend to. In our case, we are calling `double_value` with the `size_t` argument of the lambda.

After all this wrapping, we can simply:

```cpp
// declare instance
auto instance = NonJuliaObject(13);

// declare lambda
auto modify_instance = 
    [instance_ref = std::ref(instance)] (size_t n) -> void
    {
        instance_ref.get().double_value(n);
        return;
    };

// create Julia-side variable
Main.create_or_assign(
    "modify_instance",  // variable name
    as_julia_function<void(size_t)>(modify_instance)  // lambda
)

// call function Julia-side
Main.safe_eval("modify_instance(3)");

// print value of C++-side instance
std::cout << instance._value << std::endl;
```
```
104
```

The Julia-side function modified our C++-side instance, despite its type being uninterpretable to Julia.

By cleverly employing captures and collections / tuples, the restriction on what functions can be forwarded to Julia using `as_julia_function` are lifted. Any arbitrary C++ function (and thus any arbitrary C++ code) can now be executed Julia-side. Furthermore, calling C++ functions like this [introduces no overhead](benchmarks.md#calling-c-functions-from-julia--results), making this feature of jluna very powerful.
