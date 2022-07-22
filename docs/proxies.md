# Proxies

So far, this manual has intentionally obfuscated what type `auto` resolves to in a statement like this:

```cpp
// declare variable
jluna::safe_eval("new_var = 1234");

// access variable (not variables value)
auto new_var = Main["new_var"];
```

Indeed, `new_var` is not an integer, it is deduced to be a `jluna::Proxy`. To fully understand how this class works, we first need to discuss how / if memory is shared between the C++ and Julia state.

### Shared & Separate Memory

By default, most memory allocated C++-side is incompatible with Julia. For example, a C++-side variable of type `char` is an 8-bit number, while a Julia-side `Char` has 32 bits. If we were to directly exchange memory between states, the value of the char would be corrupted. For more complex classes, for example comparing `std::set` to `Base.set`, the memory layout is even more different, making directly sharing memory impossible.

Because of this, it is best to assume that we have to **reformat** any C++-side memory before moving it Julia-side.

In jluna, the **inverse is not true**. We do not have to reformat Julia-side memory to interact with it C++-side. This is the entire purpose of the library, and it is made possible through `jluna::Proxy`.

`jluna::Proxy` is a proxy, or stand-in, of arbitrary Julia-side memory. Internally, it simply holds a pointer to whatever memory it is managing. Copying the proxy copies this pointer, not the Julia-side memory. Moving the proxy does not affect the Julia-side memory. If we want to actually change the Julia-side memory being managed, we use the proxies member functions.

### Constructing a Proxy

We rarely construct proxies from raw pointers (see the [section on `unsafe` usage](unsafe.md#the-unsafe-library)), instead, a proxy will be the return value of a member function of another proxy.

How can we make a proxy if we need a proxy to do so? We've done so already, jluna offers many pre-initialized proxies, including those for modules `Main`, `Base` and `Core`. We can use these proxies to generate new proxies for us.

To create a proxy from a Julia-side value, we use `Main.get("return x")`, where x is a value or a variable (though only the value of that variable will be returned):

```cpp
auto proxy = Main.safe_eval("return 1234");
```

To reiterate, the resulting proxy, `proxy`, **does not contain the Julia-side value `1234`**. It only keeps a pointer to wherever `1234` is located.

We can actually access the raw pointer using `static_cast<unsafe::Value*>`, where `unsafe::Value*` is the type of pointer pointing to arbitrary Julia-side memory:

```cpp
std::cout << static_cast<unsafe::Value*>(proxy) << std::endl;
```
```
0x7f3a9e5cd8d0
```

(this pointer will, of course, be different anytime `1234` is allocated)

As long as `proxy` stays in scope, `1234` cannot be deallocated by the garbage collector. This is, despite the fact that there is no Julia-side reference to it.

> **C++ Hint**: The term "going out of scope" or "staying in scope" is used to refer to whether a values' [destructor](https://en.cppreference.com/w/cpp/language/destructor) has yet been called, or not been called respectively.

> **Julia Hint**: In pure Julia, any value that is not the value of a named variable, inside a collection, or referenced by a `Base.Ref`, is free to be garbage collected at any point. `jluna::Proxy` prevents this.

When using `proxy` with Julia-functions, it behaves exactly like whatever memory it is pointing to:

```cpp
// call `println(1234^2)` entirely Julia-side
Base["println"](Base["^"](proxy, 2));
```
```
1522756
```

Here, we're using the Julia-side function `Base.println` to print the result of `(^)(x, 2)` where `x` is the proxies values.

> **Julia Hint**: In Julia, an infix operator that has the symbol `x`, has the function definition
> ```julia
> (x)(<arguments>) = <body>
> ```
> The braces signal to Julia that whatever symbol is in-between them, is an infix operator. Only certain characters can be used as infix operators, see [here](https://discourse.julialang.org/t/is-not-an-operator/20221).

No memory was moved during this call. We called a Julia-side function (`println` and `(^)`) with a Julia-side value `1234`. Other than having to access the proxies in the first place, the actual computation is exactly the same as if it was done in only Julia.

### Changing a Proxies Value

The most central feature of jluna is that of **changing a proxies value**. If we assign a proxy a C++-side value, jluna will move this value Julia-side, then set the proxies pointer to that newly allocated memory:

```cpp
// create proxy to "string_value"
auto proxy = Main.safe_eval("return \"string_value\"");

// print value of proxy
Base["println"](proxy);

// print type of proxy
Base["println"](Base["typeof"](proxy));

// assign a proxy a C++ vector
proxy = std::vector<Int64>{4, 5, 6};

// print again after mutation
Base["println"](proxy);
Base["println"](Base["typeof"](proxy));
```
```
string_value
String
[4, 5, 6]
Vector{Int64}
```

We initialized a proxy, `proxy`, with the value of a Julia-side string `"string_value"`. Its type is, of course, `Base.String`. We then assigned `proxy` a C++ `std::vector`. This updates the proxies value. It is now pointing to `[4, 5, 6]`, which is of type `Vector{Int64}` and located entirely Julia-side.

Here, `proxy` is **not** pointing to the C++-side vector. jluna has implicitly converted the C++-side `std::vector<Int64>` to a Julia-side `Base.Vector{Int64}`, creating a deepcopy Julia-side (we will learn [later](unsafe.md#the-unsafe-library) how to avoid this copying behavior, if desired). It has even accurately deduced the type of the resulting vector, based on the declared type and value-type of the C++-side vector.

> **Hint**: Let `std::vector<T> x`, then `x`s type is `std::vector<T>`, `x`s value-type is `T`

What about the other way around? Recall that `proxy` is now pointing to a Julia-side `Int64[4, 5, 6]`. We can actually do the following:

```cpp
// assign Proxy to a C++ vector
std::vector<UInt64> cpp_vector = proxy;

// print the C++-side elements
for (UInt64 i : cpp_vector)
    std::cout << i << " ";
```
```
4 5 6 
```
Where we have used the proxy as the right-hand side of an assignment.
> **Hint**: An assignment is any line of code of the form `x = y`. `x` is called the "left-hand expression", `y` is called the "right-hand expression", owing to their respective positions relative to the `=`.

During execution, jluna has moved the Julia-side value. `proxy` is pointing to (`Int64[4, 5, 6]`), to C++, potentially converting its memory layout such that it can be assigned to a now fully C++-side `std::vector<UInt64>`.

We can sure a conversion took place, because we changed value-types. Julia-side, the value-type was `Int64`, C++-side it is now `UInt64`. jluna has detected this discrepancy and, during assignment of the C++-side `std::vector`, implicitly converted all elements of the Julia-side `Array` from `Int64` to `UInt64`.

### Boxing & Unboxing

#### Boxing

The process of moving a C++-side value to Julia is called **boxing**. We **box** a value by assigning it to a proxy, that is:
+ the proxy is the left-hand expression of the assignment
+ the value to be boxed is the right-hand expression of the assignment

```cpp
<Type Here> value = // ...
jluna::Proxy proxy = value;
```
#### Unboxing

The process of moving a Julia-side value to C++ is called **unboxing**. We **unbox** a proxy (and thus a Julia-side value), by assigning it to a non-proxy C++-side variable. That is:

+ the C++-side variable is the left-hand expression of the assignment
+ the proxy is the right-hand expression of the assignment

```cpp
jluna::Proxy proxy = // ...
<Type Here> value = proxy;
```

These concepts are important to understand, as they are central to moving values between the Julia- and C++-state.

In summary:
+ moving a value C++ -> Julia is called **boxing**
+ moving a value Julia -> C++ is called **unboxing**

We perform either using the assignment operator of `jluna::Proxy`, though we will [later](#boxing--unboxing) explore other, more performant, but less safe ways do to the same.

### (Un)Boxable Types

Not all types can be boxed and/or unboxed. A type that can be boxed is called a **Boxable**. A type that can be unboxed is called **Unboxable**. jluna offers two concepts `is_boxable` and `is_unboxable` that represent these properties.

> **C++ Hint**: A concept is a [new feature of C++20](https://en.cppreference.com/w/cpp/language/constraints). It is used like so:
> ```cpp
> template<//...
> concept is_boxable = //...
> 
> // jluna function:
> template<is_boxable T>
> void do_something(T value);
> ```
> Because we specified the template argument of `do_something` to be a `is_boxable`, at compile time, C++ will evaluate this condition for the template argument type. If `do_something` was called with a value of a type that is not boxable, a compiler error will be raised.

A type that is both boxable and unboxable is called **(Un)Boxable**. This is an important conceptualization, because:

+ we can only use **Boxables** as the **right-hand expression** of a proxy-assignment
+ we can only use **Unboxables** as the **left-hand expression** of an assignment.

Of course, an (Un)Boxables can be used on either side, making it possible to freely move them between states.

Out-of-the-box, the following types are all (Un)Boxable:

```cpp
// cpp type (unboxed)    // Julia-side type (boxed)
bool                     <=> Bool
char                     <=> Char
int8_t                   <=> Int8
int16_t                  <=> Int16
int32_t                  <=> Int32
int64_t                  <=> Int64
uint8_t                  <=> UInt8
uint16_t                 <=> UInt16
uint32_t                 <=> UInt32
uint64_t                 <=> UInt64
float                    <=> Float32
double                   <=> Float64
        
nullptr_t                <=> Nothing
void*                    <=> Ptr{Cvoid}

std::string              <=> String
std::string              <=> Symbol
std::complex<T>          <=> Complex{T}    //[1]
std::vector<T>           <=> Vector{T}     //[1]
std::array<T, R>         <=> Vector{T}     //[1]
std::pair<T, U>          <=> Pair{T, U}    //[1]
std::tuple<Ts...>        <=> Tuple{Ts...}  //[1]
std::map<T, U>           <=> Dict{T, U}    //[1]
std::unordered_map<T, U> <=> Dict{T, U}    //[1]
std::set<T>              <=> Set{T, U}     //[1]

jluna::Proxy             <=> /* value-type deduced during runtime */
jluna::Symbol            <=> Symbol
jluna::Type              <=> Type
jluna::Array<T, R>       <=> Array{T, R}   //[1][2]
jluna::Vector<T>         <=> Vector{T}     //[1]
jluna::JuliaException    <=> Exception
jluna::Mutex             <=> Base.ReentrantLock

// [1] where T, U are also (Un)Boxable
// [2] where R is the rank of the array
        
std::function<TR()>           <=> jluna.UnnamedFunction{0} //[3]
std::function<TR(T1)>         <=> jluna.UnnamedFunction{1} //[3]
std::function<TR(T1, T2)>     <=> jluna.UnnamedFunction{2} //[3]
std::function<TR(T1, T2, T3)> <=> jluna.UnnamedFunction{3} //[3]
        
// [3] where TR, T1, T2, T3 are also (Un)Boxable

Usertype<T>::original_type   <=> T //[4]
        
// [4] where T is an arbitrary C++ type

unsafe::Value*          <=> /* value-type deduced during runtime */
unsafe::Module*         <=> Module
unsafe::Function*       <=> /* value-type deduced during runtime */
unsafe::Symbol*         <=> Symbol
unsafe::Expression*     <=> Expr
unsafe::Array*          <=> Array<T, R> //[5][6]
unsafe::DataType*       <=> Type
        
// [5] where T is an arbitrary Julia type
// [6] where R is the rank of the array
```

There are a lot of things on this list that we have not discussed yet. It was considered important to have an exhaustive list at this point, as it will be valuable when referencing back to this section.

Most relevant `std::` types are supported out-of-the-box. All jluna types that reference Julia-side objects can be unboxed into their corresponding Julia-side values, just like any Julia-side value can be managed by their corresponding proxy.

If we are unsure of what a particular C++ type will be boxed to, we can use `to_julia_type<T>`. This template meta function has two points of interaction.

> **C++ Hint**: A template meta function is an advanced technique, where a structs template arguments, [partial specialization](https://en.cppreference.com/w/cpp/language/partial_specialization), [concepts](https://en.cppreference.com/w/cpp/language/constraints) and [SFINAE](https://en.cppreference.com/w/cpp/language/sfinae) are used to add function-like compile-time behavior to it. For end-users, all that is needed is to know how to access any particular return value, as most template meta functions are not actual `std::function`s.

We can access the name of the Julia type, any particular type `T` will be boxed to, using `as_julia_function<T>::type_name`:

```cpp
auto name = as_julia_type<
    std::pair<
        std::complex<Float32>,
        std::string
    >
  >::type_name;

std::cout << name << std::endl;
```
```
Pair{Complex{Float32}, String}
```

Similarly, we can get the actual Julia-side Type object, `T` will be boxed to, using `as_julia_function<T>::type()`.


### Named & Unnamed Proxies

We've seen before that we can mutate Julia-side variable with proxies. To understand how this works, we need to be aware of the fact that there are two types of proxies in jluna: **named** and **unnamed**.

+ A **unnamed proxy** manages a Julia-side **value**
+ A **named proxy** manages a Julia-side **variable**

Here, "manage" means that, whatever the proxy points to, is safe from the garbage collector. Furthermore, when the proxy is mutated, the object being managed is mutated at the same time.

#### Unnamed Proxies

We create an unnamed proxy using `Module::safe_eval("return x")`:

```cpp
auto unnamed_proxy = Main.safe_eval("return [7, 7, 7]");
```

We've seen this type of proxy before, if we assign it a value:

```cpp
auto unnamed_proxy = std::vector<std::string>{"abc", "def"};
```

That value will be boxed and moved to Julia, after which the proxy will point to that value.

If we have a Julia-side variable `jl_var`:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct proxy
auto unnamed_proxy = Main.safe_eval("return jl_var");
```

Then, using `Main.safe_eval("return` will **only forward its value** to the unnamed proxy. If we modify the unnamed proxy again, the variable will not be modified:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// forward value to proxy
auto unnamed_proxy = Main.safe_eval("return jl_var");

// reassign proxy
unnamed_proxy = 1234;

// print proxy value
Base["println"]("cpp: ", unnamed_proxy);

// print Julia-side variable value
Main.safe_eval(R"(println("jl : ", jl_var))");
```
```
cpp: 1234
jl : [7, 7, 7]
```

The proxy has changed the value it is pointing to, the value of `jl_var` remains unchanged. All unnamed proxies exhibit this behavior.

### Named Proxy

We construct a named proxy using `operator[]`:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct proxy
auto named_proxy = Main["jl_var"];
```

This proxy behaves exactly the same as an unnamed proxy, except, **when the named proxy is mutated, it will mutate the corresponding Julia-side variable**:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct proxy
auto named_proxy = Main["jl_var"];

// reassign proxy
named_proxy = 1234;

// print proxy value
Base["println"]("cpp: ", named_proxy);

// print Julia-side variable value
Main.safe_eval(R"(println("jl : ", jl_var))");
```
```
cpp : 1234
jl  : 1234
```

The value the proxy is pointing to, **and** the value of the variable `jl_var` has changed. This is because a named proxy remembers the name of the variable it was constructed from. If we modify the proxy, it will also modify the variable. Only named proxies exhibit this behavior.

In summary:

+ We construct an **unnamed proxy** using `Module.safe_eval("return <value>")`. Mutating an unnamed proxy will mutate its value, but it will **not** mutate any Julia-side variables.
  <br><br>
+ We construct a **named proxy** using `Module["<variable name>"]`. Mutating a named proxy will mutate its value **and** mutate its corresponding Julia-side variable.

### Proxy: Additional Member Functions

We can check which (if any) variable a proxy is managing using `get_name`:

```cpp
// declare variable
Main.safe_eval("jl_var = [7, 7, 7]");

// construct named & unnamed proxy
auto named_proxy = Main["jl_var"];
auto unnamed_proxy = Main.safe_eval("return jl_var");

// print name
std::cout << "named  : " << named_proxy.get_name() << std::endl;
std::cout << "unnamed: " << unnamed_proxy.get_name() << std::endl;
```
```
named  : jl_var
unnamed: <unnamed proxy #104>
```

We see that the named proxy indeed manages `Main.jl_var`. The unnamed proxy does not manage any variable, its name is an internal id.

If we just want to know whether a proxy is mutating (named), we can use `is_mutating()`, which returns a `bool`:

```cpp
std::cout << "named  : " << named_proxy.is_mutating() << std::endl;
std::cout << "unnamed: " << unnamed_proxy.is_mutating() << std::endl;
```
```
named  : 1
unnamed: 0
```

### Detached Proxies

Consider the following:

```cpp
// create proxy to variable in main
Main.safe_eval("x = [1, 2, 3]");
auto x_proxy = Main["x"];

// print value of proxy
Base["println"]("before: ", x_proxy);

// update x without the proxy
Main.safe_eval("x = -15");

// print value of proxy again
Base["println"]("after : ", x_proxy);
```
```
before: [1, 2, 3]
after : [1, 2, 3]
```

Here, we created a vector with the value `[1, 2, 3]`, bound to `Main.x`. We created a proxy, `x_proxy`, managing that variable. When then updated the value of `Main.x` to `-15`, **without the proxy**. Printing the value of the proxy again, we see that it still points to `[1, 2, 3]`.

This proxy is now in a **detached state**. Despite being a named proxy, its value does not correspond to the value of its variable.

This is an artifact of how the proxy manages the memory it is pointing to. For performance reasons, the pointer only gets updated when mutation happens through the proxy. Because of this, if we modify the value of a variable a named proxy manages completely Julia-side, **we need to update that proxy** to tell it that the value changed. For this, jluna provides `Proxy::upate()`:

```cpp
Main.safe_eval("x = [1, 2, 3]");
auto x_proxy = Main["x"];

// print current value
Base["println"]("before: ", x_proxy);

// update x
Main.safe_eval("x = -15");

// update proxy
x_proxy.update();

// print again
Base["println"]("after : ", x_proxy);
```
```
before: [1, 2, 3]
after : -15
```

A simple `.update()` makes a named proxy query the current state of its variable, updating its value pointer and releasing whatever other value it managed before, such that it can now be collected by the garbage collector.

### Making a Named Proxy Unnamed

Sometimes we want to make a named proxy unnamed. It is not recommended to first generate a named proxy via a function that returns one, then make it unnamed. Rather, we should always try to just generate it unnamed in the first place.

In any case, if we already have a named proxy, we can create a new unnamed proxy pointing to a deepcopy of the same underlying value using `.as_unnamed()`:

```cpp
// new variable
Main.safe_eval("x = 911");

// create named proxy
auto named = Main["x"];

// generate unnamed proxy using as_unnamed
auto from_named = named.as_unnamed();

// print name and value
std::cout << "named  : " << static_cast<Int64>(named) << " " << named.get_name() << std::endl;
std::cout << "unnamed: " << static_cast<Int64>(from_named) << " " << from_named.get_name() << std::endl;
```
```
named  : 911 x
unnamed: 911 <unnamed proxy #123>
```

Again, this should be used sparingly, as it invokes a deepcopy.

### Implications

With our newly acquired knowledge of named and unnamed proxies, we can investigate code from the previous sections more closely:

```cpp
// declare field an instance
Main.safe_eval(R"(
    mutable struct MyStruct
       _field::Int64
    end
    
    jl_instance = MyStruct(9876);
)");

// mutate jl_instance._field
Main["jl_instance"]["_field"] = 666;
```
This code snippet from the section on mutating Julia-side values uses a named proxy to update the field of `jl_instance`.

Because we are using `operator[]`, `Main["jl_instance"]` returns a named proxy managing the Julia-side variable `Main.jl_instance`. Calling `["_field"]` on this result creates another named proxy, this time managing `Main.jl_instance._field`. We know that mutating a named proxy mutates its corresponding variable, thus, assigning the resulting proxy the C++-side value `666`, it is first boxed, then assigned to `Main.jl_instance._field`.

In this example, the actual proxies were temporary - they only stayed in scope for the duration of one line.

This syntax is highly convenient, we have used it frequently already:

```cpp
Base["println"]("hello julia");
```

In this expression, we create a named proxy to the Julia-side variable `Base.println`. We then use the proxies call operator ,`operator()`, to call the proxy as a function with the argument `"hello julia"`, which is a C++-side `std::string`. jluna implicitly boxes it to a Julia-side `String`, which is then used as the argument for calling the Julia-side `println`.

### Proxy Inheritance

Many of jlunas classes not named `jluna::Proxy` are actually still proxies, because they **inherit** from `jluna::Proxy`.

> **C++ Hint**: Inheritance is not the same as subtyping in Julia. If class `A` inherits from class `B`, then `A` has access to all non-private member and member functions of `B`. `A` can furthermore be `static_cast` to `B`, and any pointer to `A` can be directly treated as if it was a pointer to `B`.

Therefore, any of these classes provide the same functionalities of proxies, along with some additional ones. The next few sections will introduce `jluna::Proxies` "children", all of which have a more specialized purpose. It is important to keep in mind that both the concepts of named and unnamed proxies and their implied behavior also apply to all of these classes.
