## Specialized Proxies: Types

We've seen specialized module-, symbol- and array-proxies. jluna has a fourth kind of proxy, `jluna::Type`, which wraps all of Julia's varied introspection functionalities in one class.

> **Hint**: [Introspection](https://en.wikipedia.org/wiki/Type_introspection) is the act of gaining information about the language itself, such as properties of types and functions. While possible, introspection in C++ can be quite cumbersome. Julia, on the other hand, was build from the ground up with it in mind.

While some overlap is present, `jluna::Type` is not a direct equivalent of `Base.Type{T}`, even though it is asserted to manage an object of type `T` such that `T isa Type`. It just provides more functions than are available using only Julia's `Base`.

---

### Constructing a Type

There are multiple ways to construct a type proxy:

```cpp
// get type of proxy
auto proxy = Main.safe_eval("return " + /* ... */);
auto type_proy = general_proxy.get_type();

// implicit cast
Type type_proxy = Main.safe_eval("return Base.Vector");

// result of `Base.typeof`
Type type = Base["typeof"](/* ... */);
```

Most often, we will want to know what type a variable is an instance of. For this we can either use `Base.typeof`, binding the result to an explicitly declared `jluna::Type`, or we can use`jluna::Proxy::get_type`.

### Base Types

For most types in `Base`, jluna offers a pre-defined type proxy in `jluna::` namespace, similar to the `Main` and `Base` module proxies.

The following types are available this way:

| jluna Constant Name  | Julia-side Name       |
|----------------------|-----------------------|
| `AbstractArray_t`    | `AbstractArray{T, N}` |
| `AbstractChar_t`     | `AbstractChar`        |
| `AbstractFloat_t`    | `AbstractFloat`       |
| `AbstractString_t`   | `AbstractString`      |
| `Any_t`              | `Any`                 |
| `Array_t`            | `Array{T, N}`         |
| `Bool_t`             | `Bool`                |
| `Char_t`             | `Char`                |
| `DataType_t`         | `DataType`            |
| `DenseArray_t`       | `DenseArray{T, N}`    |
| `Exception_t`        | `Exception`           |
| `Expr_t`             | `Expr`                |
| `Float16_t`          | `Float16`             |
| `Float32_t`          | `Float32`             |
| `Float64_t`          | `Float64`             |
| `Function_t`         | `Function`            |
| `GlobalRef_t`        | `GlobalRef`           |
| `IO_t`               | `IO`                  |
| `Int8_t`             | `Int8`                |
| `Int16_t`            | `Int16`               |
| `Int32_t`            | `Int32`               |
| `Int64_t`            | `Int64`               |
| `Int128_t`           | `Int128`              |
| `Integer_t`          | `Integer`             |
| `UInt8_t`            | `UInt8`               |
| `UInt16_t`           | `UInt16`              |
| `UInt32_t`           | `UInt32`              |
| `UInt64_t`           | `UInt64`              |
| `UInt128_t`          | `UInt128`             |
| `Unsigned_t`         | `Unsigned`            |
| `Signed_t`           | `Signed`              |
| `LineNumberNode_t`   | `LineNumberNode`      |
| `Method_t`           | `Method`              |
| `Module_t`           | `Module`              |
| `NTuple_t`           | `NTuple{T, N}`        |
| `NamedTuple_t`       | `NamedTuple`          |
| `Nothing_t`          | `Nothing`             |
| `Number_t`           | `Number`              |
| `Pair_t`             | `Pair{T, U}`          |
| `Ptr_t`              | `Ptr{T}`              |
| `QuoteNode_t`        | `QuoteNode`           |
| `Real_t`             | `Real`                |
| `Ref_t`              | `Ref{T}`              |
| `String_t`           | `String`              |
| `Symbol_t`           | `Symbol`              |
| `Task_t`             | `Task`                |
| `Tuple_t`            | `Tuple{T...}`         |
| `Type_t`             | `Type{T}`             |
| `TypeVar_t`          | `TypeVar`             |
| `UndefInitializer_t` | `UndefInitializer`    |
| `Union_t`            | `Union{T...}`         |
| `UnionAll_t`         | `UnionAlll`           |
| `VecElement_t`       | `VecElement{T}`       |
| `WeakRef_t`          | `WeakRef`             |

Where `T`, `U` are arbitrary types, `N` is an Integer

### Type Order

Julia types can be ordered. To conceptualize this, the relation of types is best thought of as a directed graph. Each node of the graph is a type, each edge is directed, where, if the edge goes from type `A` to type `B`, then `B <: A`. That is, `B` is a subtype of `A`, or, equivalently, `A >: B`: `A` is a supertype of `B`.

This relational nature is heavily used in Julia's multiple dispatch and type inference, for now, it gives us a way to put types in relation to each other. `jluna::Type` offers multiple functions for this:

```cpp
// (*this) <: other
bool is_subtype_of(const Type& other) const;

// (*this) >: other
bool is_supertype_of(const Type& other) const;

// (*this) <: other && (*this) >: other
bool is_same_as(const Type&) const;
```

Because we can assign an order to types in this way, `jluna::Type` also provides proper boolean comparison operators:

```cpp
// (*this) <: other
bool operator<(const Type& other) const;

// other >: (*this)
bool operator>(const Type& other) const;

// (*this) === other
bool operator==(const Type& other) const;

// !((*this) === other)
bool operator!=(const Type& other) const;
```

This ordering becomes relevant when talking about `TypeVar`s.

> **Julia Hint**: `Base.TypeVar` is a class that represents a not-yet-defined type, such as a parameter for a struct. It has a lower bound `lb` and upper bound `ub`, where, for all types `t` represented by the `TypeVar`, it holds that `lb <: t <: ub`.


`TypeVar`s can be thought of as a sub-graph of the type-graph. An unrestricted types upper bound is `Any`, while its lower bound is `Union{}`. A declaration like `T where {T <: Integer}` restricts the upper bound to `Integer`. Any type that is "lower" along the sub-graph originating at `Integer`, `Int64` for example, can still bind to `T`. This is useful to keep in mind.

### Type Info

When gathering information about types, it is important to understand the difference between a types **fields**, its **parameters**, its **properties** and its **methods**. Consider the following type declaration:

```julia
# in Julia
mutable struct MyType{T <: Integer, U}
    _field1
    _field2::T
    _field3::U
    
    function MyType(a::T, b::U) where {T, U}
        return new{T, U}(undef, a, b)
    end
end
```

This type is a parametric type, it++i has two **parameters** called `T` and `U`. `T`s upper bound is `Integer` while `U` is unrestricted, its upper bound is `Any`.

`MyType` has 3 **fields**:
+ `_field1` which is unrestricted
+ `_field2` which is declared as type `T`, thus it's upper bound is also `Integer`
+ `_field3` which is declared as type `U`, however because `U` is unrestricted, `_field3` is too

The type has 1 **method**, `MyType(::T, ::U)`, which is its constructor.

This type furthermore has the following properties:

```julia
# in Julia
println(propertynames(MyType.body.body))
```
```
(:name, :super, :parameters, :types, :instance, :layout, :size, :hash, :flags)
```

Properties are often reserved for "hidden" members, such as those containing internal implementation details.

### Parameters

We can access the name and upper type bounds of the parameters of a type using `jluna::Type::get_parameters`:

```cpp
// declare type
State::safe_eval(R"(
    mutable struct MyType{T <: Integer, U}
        _field1
        _field2::T
        _field3::U

        function MyType(a::T, b::U) where {T, U}
            return new{T, U}(undef, a, b)
        end
    end
)");

// access type as Type proxy
Type my_type = Main["MyType"];

// get parameters
std::vector<std::pair<Symbol, Type>> parameters = my_type.get_parameters();

// print elements of vector with Julia-style pair syntax
for (auto& pair : parameters)
    std::cout << pair.first.operator std::string() << " => " << pair.second.operator std::string() << std::endl;
```
```
T => Integer
U => Any
```
`get_parameters` returns a vector of pairs where:
+ `.first` is a symbol that is the name of the corresponding parameter
+ `.second` is the parameters upper type bound, as a `jluna::Type`

In case of `T`, the upper bound is `Base.Integer`, because we restricted it as such in the declaration. For `U`, there is no restriction, which is why its upper bound is the default: `Any`.

We can retrieve the number of parameters directly using `get_n_parameters()`. This saves allocating the vector of pairs.

### Fields

We can access the fields of a type in a similar way, using `jluna::Type::get_fields`:

```cpp
// declare type
State::safe_eval(R"(
    mutable struct MyType{T <: Integer, U}
        _field1
        _field2::T
        _field3::U

        function MyType(a::T, b::U) where {T, U}
            return new{T, U}(undef, a, b)
        end
    end
)");

// access type as Type proxy
Type my_type = Main["MyType"];

// get fields as vector
std::vector<std::pair<Symbol, Type>> fields = my_type.get_fields();

// print
for (auto& pair : fields)
    std::cout << pair.first.operator std::string() << " => " << pair.second.operator std::string() << std::endl;
```
```
_field1 => Any
_field2 => Integer
_field3 => Any
```
We, again, get a vector of pairs where `.first` is the name, `.second` is the upper type bound of the corresponding field.

If we actually want the value of a field, we need use `operator[]` on a `jluna::Proxy` that is an instance of that type, not the type itself.

While less useful, we can access a types methods using the Julia-side `Base.methods`. Similarly, to access a types properties, `Base.propertynames` and `Base.getproperty` can be used.


### Type Classification

To classify a type means to evaluate a condition based on a types attributes, in order to get information about how similar or different clusters of types are. jluna offers a number of classifications, some of which are available as part of the Julia standard library, some of which are not. This section will list all offered by jluna, along with their meaning:

+ `is_primitive`: was the type declared using the keyword `primitive`
    ```cpp
    Bool_t.is_primitive()    // true
    Module_t.is_primitive(); // false
    ```
+ `is_struct_type`: was the type declared using the keyword `struct`
    ```cpp
    Bool_t.is_struct_type()    // false
    Module_t.is_struct_type(); // true
    ```
+ `is_declared_mutable`: was the type declared using `mutable`
    ```cpp
    Bool_t.is_declared_mutable()    // false
    Module_t.is_declared_mutable(); // true
    ```
+ `is_isbits`: is the type a [`isbits` type](https://docs.julialang.org/en/v1/base/base/#Base.isbits), meaning it is immutable and contains no references to other values that are not also isbits or primitives
    ```cpp
    Bool_t.is_declared_mutable()    // true
    Module_t.is_declared_mutable(); // false
    ```
+ `is_singleton`: a type `T` is a [singleton](https://docs.julialang.org/en/v1/base/base/#Base.issingletontype) iff:
    - `T` is immutable and a structtype
    - for types `A`, `B` if `A <: B` and `B <: A` then `A === B`
    ```cpp
    State.eval("struct Singleton end");
    Type singleton_type = Main["Singleton"];
    singleton_type.is_singleton(); // true
    ```
+ `is_abstract_type`: was the type declared using the `abstract` keyword
    ```cpp
    Float32_t.is_abstract_type()          // false
    AbstractFloat_t.is_abstract_type();   // true
    ```
+ `is_abstract_ref_type`: is the type a reference whose value type is an abstract type
    ```cpp
    Type(jluna::safe_eval("return Ref{AbstractFloat}")).is_abstract_ref_type(); //true
    Type(jluna::safe_eval("return Ref{AbstractFloat}(Float32(1))")).is_abstract_ref_type(); // false
    ```
+ `is_typename(T)`: is the `.name` property of the type equal to `Base.typename(T)`. Can be thought of as "is the type a T"
    ```cpp
    // is the type an Array:
    Type(State::safe_eval("return Array")).is_typename("Array"); // true
    Type(State::safe_eval("return Array{Integer, 3}")).is_typename("Array"); // also true
    ```

### "Unrolling" Types

There is a subtle difference between how jluna evaluates properties of a type and how pure Julia does. Consider the following:

```julia
# in Julia
function is_array_type(type::Type) 
    return getproperty(type, :name) == Base.typename(Array)
end

println(is_array_type(Base.Array))
```

Some may expect this to print `true`, however, this is not the case. It actually prints `false`.

This is because `Base.Array` is a parametric type. `typeof(Array)` is actually `UnionAll`, which does not have a property `:name`:

```julia
# in Julia
println(propertynames(Array))
```
```
(:var, :body)
```
To access the property `:name`, we need to first **unroll** the type, meaning we need to specialize all its parameters. Once we do so, it seizes to be a `UnionAll`, gaining the properties expected of a `Base.Type`:

```julia
# in Julia

# access type
type = Array
    
# specialize fully
while (hasproperty(type, :body))
    type = type.body
end

# print type after unrolling
println(type)

# print propertynames of unrolled type
println(propertynames(type))
```
```text
Array{T, N}
(:name, :super, :parameters, :types, :instance, :layout, :size, :hash, :flags)
```

Once fully unrolled, we have access to the properties necessary for introspection. jluna does this unrolling automatically for all types initialized by `jluna::initialize` (see the [previous sections list](#base-types)).

If desired, we can fully specialize a user-initialized type manually using the member function `.unroll()`. Without this, many of the introspection features will be unavailable.
