#include <iostream>
#include <julia.h>
#include <jluna.hpp>
#include <.test/test.hpp>
#include <include/julia_extension.hpp>

using namespace jluna;
int main()
{
    State::initialize();

    std::cout << std::is_same_v<char, uint8_t> << std::endl;
    Test::initialize();

    Test::test("catch c exception", [](){

        Test::assert_that_throws<JuliaException>([](){
            jl_eval_string("return undefined");
            forward_last_exception();
        });
    });

    Test::test("jl_find_function", [](){

        auto* expected = jl_get_function(jl_base_module, "println");
        auto* got = jl_find_function("Base", "println");

        Test::assert_that(jl_is_identical(expected, got));
    });

    Test::test("call", [](){
        Test::assert_that(false);
    });

    Test::test("safe_call", [](){
        Test::assert_that(false);
    });

    auto test_box_unbox = []<typename T>(const std::string type_name, T value)
    {
        Test::test("box/unbox " + type_name , [value]() {

            jl_value_t* boxed = box<T>(value);
            Test::assert_that(unbox<T>(boxed) == value);

            boxed = box<T>(T());
            Test::assert_that(unbox<T>(boxed) == T());
        });
    };

    test_box_unbox("Char", Char(12));
    test_box_unbox("String", std::string("abc"));
    test_box_unbox("Int8", Int8(12));
    test_box_unbox("Int16", Int16(12));
    test_box_unbox("Int32", Int32(12));
    test_box_unbox("Int64", Int64(12));
    test_box_unbox("UInt8", UInt8(12));
    test_box_unbox("UInt16", UInt16(12));
    test_box_unbox("UInt32", UInt32(12));
    test_box_unbox("UInt64", UInt64(12));
    test_box_unbox("Float32", Float32(0.01));
    test_box_unbox("Float64", Float64(0.01));

    Test::conclude();


}


