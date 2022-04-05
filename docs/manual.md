# jluna: Manual & Tutorial

This document will go over most of jlunas features, explaining most of them in detail. The manual is structured in a way that ramps up in complexity, first guiding users on how to accomplish basic tasks such as calling julia code or transferring values between states, then introduce more elaborate features such as custom usertypes, asynchronous programming and performance optimization.

This manual assumes that all users are familiar with the basics of Julia, if this is not the case, it may be necessary to first work through the excellent [official manual](https://docs.julialang.org/en/v1/), to at least get an overview of basic principles of the language.

jluna makes extensive use of newer C++20 features, as well as high-level techniques, which may provide a challenge for users only familiar with Julia and/or C. To address this, most example code will have a hint section like so:

> **Hint**: Quotation blocks like these are reserved for additional information or explanations of C++ and/or Julia features, providing some guidance for users who are unfamiliar with any particular feature of either language.

---

## Table of Contents

---

### Installation

This manual does not go over installing jluna. A step-by-step tutorial on how to install and create our own application using jluna is available [here](./installation.md).

---

### Initializating the Julia State

Before any Julia or jluna functionality can be accessed, we need to *initialize the Julia state*. This sets up the Julia environment, as well as load most of jlunas static functionalities. 

We do so using `jluna::initialize`:

```cpp
// main.cpp
#include <jluna.hpp>

using namespace jluna;

int main()
{
    initialize();
    
    // our application here
    
    return 0;
}
```
> **C++ Hint**: `using namespace jluna;` makes it, such that, in the scope the statement is declared, we do not have to prefix all jluna functions/object with their namespace: `jluna::`. Because of this, we can write `initialize()`, rather than `jluna::initialize()`.

Where `#include <jluna.hpp>` makes all of jlunas functionality available to the user.

---