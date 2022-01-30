#include <iostream>
#include <include/julia/julia.h>

int main()
{
    jl_init();

    jl_eval_string("println(\"hello world\")");
}
