#include <iostream>
#include <julia.h>
#include <jluna.hpp>
#include <.test/test.hpp>

using namespace jluna;
int main()
{
    Test::initialize();

    Test::test("throw if unitialized", [](){

        Test::assert_that_throws<JuliaUnitializedException>([](){throw_if_unitialized();});
    });

    jl_init();

    Test::test("catch c exception", [](){

        Test::assert_that_throws<JuliaException>([](){
            jl_eval_string("return undefined");
            forward_last_exception();
        });
    });

    Test::conclude();


}


