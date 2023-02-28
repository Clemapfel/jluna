## Usertypes

So far, we were only able to move (Un)Boxables to and from Julia. In some applications, this can be quite limiting. To address this, jluna provides a user-interface for making **any C++ type (Un)Boxable**.

> **Hint**: A usertype is any type not defined by the standard library or jluna itself.

---

### Usertype Interface

Consider the following C++ class:

```cpp
// class representing color in the RGBA system
struct RGBA
{
    float _red;     // red component, in [0,1]
    float _green;   // green component, in [0, 1]
    float _blue;    // blue component, in [0, 1]
    float _alpha;   // transparency, in [0, 1]
    
    // construct
    RGBA(float r, float g, float b)
        : _red(r), _green(g), _blue(b), _alpha(1)
    {}
};
```

While it may be possible to manually translate this class into a Julia-side `NamedTuple`, this is rarely the best option. For more complex classes, this is often not possible at all. To make classes like this (Un)Boxable, we use `jluna::Usertype<T>`, the **usertype interface**.

### Step 1: Making the Type Compliant

For a type `T` to be manageable by `Usertype<T>`, it needs to be **default constructable**. `RGBA` currently has no default constructor, so we need to add it:

> **C++ Hint**: The default constructor of type T is `T()`. It can sometimes be declared as `T() = default`, see [here](https://en.cppreference.com/w/cpp/language/default_constructor).

```cpp
struct RGBA
{
    float _red;
    float _green;
    float _blue;
    float _alpha;
    
    RGBA(float r, float g, float b)
        : _red(r), _green(g), _blue(b), _alpha(1)
    {}
    
    // added default ctor
    RGBA() 
        : _red(0), _green(0), _blue(0), _alpha(1)
    {}
};
```

If the type `T` is not default constructable, a static assertion is raised at compile time.

### Step 2: Enabling the Interface

To make jluna aware that we will be using the usertype interface for `RGBA`, we need to **enable it at compile time**. To do this, we use the `set_usertype_enabled` macro, executed in non-block scope.

> **C++ Hint**: Non-block scope (also called "global scope") is any scope that is not inside a namespace, function, struct, or block. As an example, `int main()` has to be declared in global scope.

```cpp
struct RGBA
{
    /* ... */
};

// enable usertype at compile time
set_usertype_enabled(RGBA); 
```

This sets up `Usertype<T>` for us. Among other things, it declares the Julia-side name of `RGBA`. This name is that same as the C++ side name, `"RGBA"` in our case.

### Step 3: Adding Property Routines

To add a property for `RGBA`s `_red`, we use the following function (at runtime):

> **Julia Hint**: In usage, both properties and fields have the exact same syntax. In C++, we would call these "members". For this section, all three terms will be used interchangeably.

```cpp
Usertype<RGBA>::add_property<Float32>(     // template argument
    "_red_jl",                             // field name
    [](RGBA& in) -> Float32 {              // boxing routine
        return in._red;
    },
    [](RGBA& out Float32 red_jl) -> void { // unboxing routine
        out._red = red_jl;
    }
);
```

This call has a lot going on, so it's best to investigate it closely.

Firstly, we have the template argument, `Float32`. This decides the Julia-side type of the Julia-side instances field.

The first argument is the Julia-side instances' fields name. Usually, we want this name to be the same as C++-side, `_red`, but to avoid confusion for this section only, the C++-side field is called `_red` while the corresponding Julia-side field is `_red_jl`.

The second argument of `add_property` is called the **boxing routine**. This function always has the signature `(T&) -> Property_t`, where `T` is the usertype-manage type (`RGBA` for us) and `Property_t` is the type of the field (`Float32`).
The boxing routine governs what value to assign the corresponding Julia-side field. In our case, it takes a C++-side instance of `RGBA`, accesses the value that instances `_red`, then assigns it to the Julia-side instances' `_red_jl`.

The third argument is optional, it is called the **unboxing routine**. It always has the signature `(T&, Property_t) -> void`. When a Julia-side instance of `RGBA` is moved back C++-side, the unboxing routine governs what value the now C++-sides `RGBA` fields `_red` will be assigned. If left unspecified, the value will be the value set by the default constructor. In our case, we assign `_red` the value of `_red_jl`, which is the second argument of the unboxing routine.

In summary:

+ the template argument governs the Julia-side type of the field
+ the first argument is the name of the Julia-side field
+ the boxing routine decides what value the Julia-side field will be assigned when moving the object C++ -> Julia
+ the unboxing routine decides what value the C++-side field will be assigned when moving the object Julia -> C++

Now that we know how to add fields, we can do so for `_green`, `_blue` and `_alpha`:

```cpp
// in namespace scope
struct RGBA
{
    float _red;
    float _green;
    float _blue;
    float _alpha;
    
    RGBA(float r, float g, float b)
        : _red(r), _green(g), _blue(b), _alpha(1)
    {}
    
    RGBA() 
        : _red(0), _green(0), _blue(0), _alpha(1)
    {}
};
set_usertype_enabled(RGBA);

// ###

// in function scope, i.e. inside main

// add field _red
Usertype<RGBA>::add_property<float>(
    "_red", // now named `_red`, not `_red_jl`
    [](RGBA& in) -> float {return in._red;},
    [](RGBA& out, float in) -> void {out._red = in;}
);

// add field _green
Usertype<RGBA>::add_property<float>(
    "_green",
    [](RGBA& in) -> float {return in._green;},
    [](RGBA& out, float in) -> void {out._green = in;}
);

// add field _blue
Usertype<RGBA>::add_property<float>(
    "_blue",
    [](RGBA& in) -> float {return in._blue;},
    [](RGBA& out, float in) -> void {out._blue = in;}
);

// add field _alpha
Usertype<RGBA>::add_property<float>(
    "_alpha",
    [](RGBA& in) -> float {return in._alpha;},
    [](RGBA& out, float in) -> void {out._alpha = in;}
);
```

Note that, now, the Julia-side field is actually called `_red`, which is better style than the `_red_jl` we used only for clarity.

To illustrate that properties do not have to directly correspond with members of the C++ class, we'll add another Julia-side-only field that represents the `value` component from the [HSV color system](https://en.wikipedia.org/wiki/HSL_and_HSV) (sometimes also called "lightness"). It is defined as the maximum of red, green and blue:

```cpp
// add Julia-only field _value
Usertype<RGBA>::add_property<float>(
    "_value",
    [](RGBA& in) -> float {
        float max = 0;
        for (auto v : {in._red, in._green, in._blue})
            max = std::max(v, max);
        return max;
    }
);
```

We leave the unboxing routine for `_value` unspecified, because the C++-side instance does not have any corresponding field to assign to.

### Step 4: Implementing the Type

Having added all properties to the usertype interface, we make the Julia state aware of the interface by calling:

```cpp
// in main
Usertype<RGBA>::implement();
```

This creates a new Julia-side type that has the architecture we just gave it. For end-users, this happens automatically.

Internally, the following expression is assembled and evaluated:

```julia
mutable struct RGBA
    _red::Float32
    _green::Float32
    _blue::Float32
    _alpha::Float32
    _value::Float32
    RGBA() = new(0.0f0, 0.0f0, 0.0f0, 1.0f0, 0.0f0)
end
```

We see that jluna assembled a mutable structtype, whose field names and types are as specified. Even the order in which we called `add_property` for specific names is preserved. This becomes important for the default constructor (a constructor that takes no arguments). The default values for each of the types' fields, are those of an unmodified, default-initialized instance of `T` (`RGBA()` in our case). This is why the type needs to be default constructable.

If we want the type to be implemented in a different module, we can specify this module (as a `jluna::Module`) as an argument to `Usertype<T>::implement`.

If we desire additional constructors, we can simply add them as external constructors in the same scope the usertype was `implement`ed in:

```cpp
// add additional, external constructor
Main.safe_eval(R"(
    function RGBA(r::Float32, g::Float32, b::Float32, a::Float32) ::RGBA
        out = RGBA()
        out._red = r
        out._green = g
        out._blue = b
        out._alpha = a
        out._value = max(r, g, b, a)

        return out
    end
)");
```

### Step 5: Usage

After `Usertype<RGBAB>::implement()`, we can use `RGBA` just like any other (Un)Boxable type:

```cpp
// create new variable and assign it a RGBA
Main.create_or_assign("jl_rgba", RGBA(1, 0, 1));

// print value of that variable
Main.safe_eval("println(jl_rgba);");

// print fieldnames of the Julia-side type
Main.safe_eval("println(fieldnames(RGBA))");
```
```text
RGBA(1.0f0, 0.0f0, 1.0f0, 1.0f0)
(:_red, :_green, :_blue, :_alpha, :_value)
```
> **Julia Hint**: `Base.fieldnames` takes a type (not an instance of a type) and returns the symbols of a types fields, in order.

We see that now, `Main.RGBA` is a proper Julia type and `jl_rgba` got the correct values according to each field's boxing / unboxing routine.

The same applies when moving `Main.RGBA` from Julia to C++:

```cpp
// create a Julia-side RGBA and assign it to a C++-side RGBA
RGBA cpp_rgba = Main.safe_eval("return RGBA(0.5, 0.5, 0.3, 1.0)");

// print member values
std::cout << cpp_rgba._red << " ";
std::cout << cpp_rgba._green << " ";
std::cout << cpp_rgba._blue << " ";
```
```
0.5 0.5 0.3
```

### Example Summary

This section was quite complicated, a fully working `main.cpp` replicating this `RGBA` example can be found [here](https://github.com/Clemapfel/jluna/blob/master/docs/rgba_example.cpp). Users are encouraged to play with it, to further their understanding of the usertype interface.

### Usertype: Additional Member Functions

In addition to functions used for steps outlined in this section, `Usertype<T>` offers the following additional members / member functions:

+ `Usertype<T>::original_type`
    - typedef equal to `T`
+ `is_enabled()`
    - was `set_usertype_enabled` called for `T`
+ `get_name()`
    - get the Julia side name of `T` after unboxing
+ `is_implemented()`
    - was implement called at least once for this `T`

Lastly, after `implement` was called, the  `as_julia_type<Usertype<T>>` template meta function will work, just like it would for other (Un)Boxables.
