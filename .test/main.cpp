#include <iostream>
#include <julia.h>
#include <jluna.hpp>

using namespace jluna;
int main()
{
    jl_init();

    jl_eval_string("throw undef");
    forward_last_exception();
}
