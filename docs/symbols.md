# Specialized Proxies: Symbols

Another specialized type of proxy is the **symbol proxy**. It manages a Julia-side `Base.Symbol`.

We can create a symbol proxy in the following ways:

```cpp
// new unnamed proxy
auto unnamed_symbol = Symbol("symbol_value");

// new named proxy
auto named_symbol = Main.new_symbol("symbol_var", "symbol_value");

// from already existing proxy
auto non_symbol_proxy_managing_symbol = Main.safe_eval("return :symbol_value");
auto unnamed_symbol = non_symbol_proxy_managing_symbol.as<Symbol>();
```

Where, unlike with `jluna::Proxy`, the value the proxy is pointing to is asserted to be of type `Base.Symbol`.

### Symbol Hashing

The main additional functionality `jluna::Symbol` brings is that of **constant time hashing**.

A hash is essentially a `UInt64` we assign to things as a label. In Julia, hashes are unique and there a no hash collisions. This means if `A != B` then `hash(A) != hash(B)` and, furthermore, if `hash(A) == hash(B)` then `A === B`.

> **Julia Hint**: `(==)` checks if the value of two variables is the same. `(===)` checks whether both variables values have the exact identical location in memory - if they are the same instance.

Unlike with other classes, `Base.Symbol`s hash is pre-computed, making it much faster to hash.

We can access the hash of a symbol proxy using `.hash()`. To get the symbol as a string, we use `static_cast`:

```cpp
// create symbol
auto symbol = Symbol("abc");

// print name and hash
std::cout << "name: " << static_cast<std::string>(symbol) << std::endl;
std::cout << "hash: " << symbol.hash() << std::endl;
```
```text
name: abc
hash: 16076289990349425027
```

In most cases, it is impossible to predict which hash will be assigned to which symbol. This also means for Symbols `s1` and `s2` if `string(s1) < string(s2)` this does not mean `hash(s1) < hash(s2)`. This is very important to realize, **symbol comparison is not lexicographical comparison**.

Having taken note of this, `jluna::Symbol` provides the following comparison operators:

```cpp
/// (*this).hash() == other.hash()
bool operator==(const Symbol& other) const;

/// (*this).hash() != other.hash()
bool operator!=(const Symbol& other) const;

/// (*this).hash() < other.hash()
bool operator<(const Symbol& other) const;

/// (*this).hash() <= other.hash()
bool operator<=(const Symbol& other) const;

/// (*this).hash() >= other.hash()
bool operator>=(const Symbol& other) const;

/// (*this).hash() > other.hash()
bool operator>(const Symbol& other) const;
```

To further illustrate the way symbols are compared, consider the following example using `std::set`, which orders its elements according to their user-defined comparison operators:

```cpp
// create set
auto set = std::set<Symbol>();

// add newly constructed symbols to it
for (auto str : {"abc", "bcd", "cde", "def"})
    set.insert(Symbol(str));

// print in order
for (auto symbol : set)
    std::cout << symbol.operator std::string() << " (" << symbol.hash() << ")" << std::endl;
```
```text
cde (10387276483961993059)
bcd (11695727471843261121)
def (14299692412389864439)
abc (16076289990349425027)
```

We see that, lexicographically, the symbols are out of order. They are, however, ordered properly according to their hashes.

