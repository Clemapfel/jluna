#include <iostream>
#include <julia.h>
#include <jluna.hpp>
#include <.test/test.hpp>
#include <include/julia_extension.hpp>

using namespace jluna;
int main()
{
    Test::initialize();

    Test::test("throw if unitialized", [](){

        Test::assert_that_throws<JuliaUninitializedException>([](){ throw_if_uninitialized();});
    });

    jl_init();

    Test::test("catch c exception", [](){

        Test::assert_that_throws<JuliaException>([](){
            jl_eval_string("return undefined");
            forward_last_exception();
        });
    });

    Test::test("jl_get_function", [](){

        auto* expected = jl_get_function(jl_base_module, "println");
        auto* got = jl_get_function("Base", "println");

        Test::assert_that(jl_is_identical(expected, got));
    });

    Test::test("call", [](){
        Test::assert_that(false);
    });

    Test::test("safe_call", [](){
        Test::assert_that(false);
    });

    Test::conclude();


}


