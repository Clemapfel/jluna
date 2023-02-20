//
// Created by cleme on 19/02/2023.
//

#include <julia.h>

int main() 
{
    jl_init();
    auto* println = jl_get_global(jl_main_module, jl_symbol("println"));
    jl_call1(println, (jl_value_t*) jl_symbol("test"));
    return 0;
}