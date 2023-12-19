#include <jluna_example.hpp>

namespace jluna_example
{
    void example_function()
    {
        // example function implementation, prints "hello world" using Julia
        jluna::Main["println"]("hello world!");
    }
}