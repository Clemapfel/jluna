#include <iostream>
#include <julia.h>
#include <jluna.hpp>
#include <.test/test.hpp>
#include <include/julia_extension.hpp>

using namespace jluna;
int main()
{
    State::initialize();

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
        Test::assert_that_throws<JuliaException>([]() {
            call(jl_find_function("Base", "throw"), jl_eval_string("return ErrorException(\"\")"));
        });
    });

    Test::test("safe_call", [](){
        Test::assert_that_throws<JuliaException>([]() {
            safe_call(jl_find_function("Base", "throw"), jl_eval_string("return ErrorException(\"\")"));
        });
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

    //test_box_unbox("Bool", Bool(true));
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
    test_box_unbox("Complex", std::complex<double>(0, 1));

    test_box_unbox("Pair", std::pair<size_t, std::string>(12, "abc"));
    test_box_unbox("Tuple3", std::tuple<size_t, std::string, float>(12, "abc", 0.01));

    auto test_box_unbox_iterable = []<typename T>(const std::string& name, T&& value){

        Test::test("box/unbox " + name, [&value](){

            jl_value_t* boxed = box(value);
            auto unboxed = unbox<T>(boxed);

            Test::assert_that(value == unboxed);
        });
    };

    test_box_unbox_iterable("Vector", std::vector<size_t>{1, 2, 3, 4});
    test_box_unbox_iterable("IdDict", std::map<size_t, std::string>{{12, "abc"}});
    test_box_unbox_iterable("Dict", std::unordered_map<size_t, std::string>{{12, "abc"}});
    test_box_unbox_iterable("Set", std::set<size_t>{1, 2, 3, 4});

    Test::test("create_reference", []() {

        jl_eval_string(R"(
            struct StructType
                any
            end
        )");

        auto hold = Proxy(jl_eval_string("return StructType([99, 2, 3, 4])"), nullptr);
        State::collect_garbage();
        Test::assert_that(((int) hold["any"][0]) == 99);
    });

    Test::test("proxy ctor", [](){

        jl_value_t* val = jl_eval_string("return [1, 2, 3, 4]");
        auto proxy = Proxy(val, nullptr);

        Test::assert_that(jl_unbox_bool(jl_call2(jl_get_function(jl_base_module, "=="), val, (jl_value_t*) proxy)));
    });

    Test::test("proxy trivial dtor", [](){

        jl_value_t* val = jl_eval_string("return [1, 2, 3, 4]");
        size_t n = 0;
        {
            auto proxy = Proxy(val, nullptr);
            n = jl_unbox_int64(jl_eval_string("return length(jluna.memory_handler._refs.x)"));
        }

        Test::assert_that(n - jl_unbox_int64(jl_eval_string("return length(jluna.memory_handler._refs.x)")) == 2);
        // 2 bc symbol and value are registered, even for unnamed
    });

    Test::test("proxy inheritance dtor", [](){

        jl_eval_string(R"(
            struct Inner
                _field
            end

            struct Outer
                _inner::Inner
            end

            instance = Outer(Inner(true))
        )");

        std::unique_ptr<Proxy> inner;

        {
            auto outer = Main["instance"];
            inner = std::make_unique<Proxy>(outer["_inner"]["_field"]);
        }
        State::collect_garbage();

        Test::assert_that((bool) inner.get());
    });

    Test::test("proxy reject as non-vector", [](){

        jl_eval_string(R"(
            struct NonVec
                _field
            end
        )");

        auto vec = Proxy(jl_eval_string("return [1, 2, 3, 4]"), nullptr);
        auto non_vec = Proxy(jl_eval_string("return NonVec([9, 9, 9, 9])"), nullptr);

        try
        {
            Test::assert_that((int) vec[0] == 1);
        }
        catch (...)
        {
            Test::assert_that(false);
        }

        try
        {
            Test::assert_that((int) non_vec[0] == 1);
            Test::assert_that(false);   // fails if no exception thrown
        }
        catch (...)
        {}
    });

    Test::test("proxy reject as non-function", [](){

        jl_eval_string(R"(
            struct NonVec
                _field
                NonVec() = new(1)
            end

            f(xs...) = return sum([xs...])
            non_f = NonVec()
        )");

        auto func = Main["f"];
        auto non_func = Main["non_f"];

        try
        {
            Test::assert_that((int) func(1, 2, 3, 4) == 10);
        }
        catch (...)
        {
            Test::assert_that(false);
        }

        try
        {
            non_func();
            Test::assert_that(false);   // fails if no exception thrown
        }
        catch (...)
        {}
    });

    Test::test("proxy reject as non-struct", [](){

        State::safe_script(R"(
            struct NewStructType
                _field
                NewStructType() = new(true)
            end

            f(xs...) = return sum([xs...])
            instance = NewStructType()
        )");

        auto non_struct = Main["f"];
        auto is_struct = Main["instance"];

        try
        {
            Test::assert_that((bool) is_struct["_field"]);
        }
        catch (...)
        {
            Test::assert_that(false);
        }

        try
        {
            auto res = non_struct["_field"];
            Test::assert_that(false);   // fails if no exception thrown
        }
        catch (...)
        {}
    });

    Test::test("proxy fieldnames", [](){

        State::safe_script(R"(

            struct FieldStruct
                _a
                _b
                _☻
            end
        )");

        Proxy proxy = State::safe_script("return FieldStruct");
        auto names = proxy.get_field_names();

        Test::assert_that(names.at(0) == "_a" and names.at(1) == "_b" and names.at(2) == "_☻");
    });

    Test::test("proxy mutation", [](){

        jl_eval_string("variable = [1, 2, 3, 4]");

        auto mutating_proxy = Main["variable"];

        Test::assert_that(mutating_proxy.is_mutating());
        mutating_proxy[0] = 9999;

        Test::assert_that(mutating_proxy[0].operator Int64() == 9999);
        Test::assert_that(jl_unbox_int64(jl_eval_string("variable[1]")) == 9999);


        auto non_mutating_proxy = State::script("return variable");
        non_mutating_proxy = 8888;

        Test::assert_that(non_mutating_proxy.operator Int64() == 8888);
        Test::assert_that(jl_unbox_int64(jl_eval_string("variable[1]")) != 8888);
    });

    Test::test("proxy mutate unnamed member", [](){

        State::safe_script(R"(

            mutable struct UnnamedMemberStruct
                _field::Vector{Any}
            end

            vector = [['a', 'b', 'c'], ['a', 'b', 'c'], ['a', 'b', 'c']]
            instance = UnnamedMemberStruct([1, 2, 3, 4])
        )");

        auto unnamed_vector = State::safe_script("return vector");
        auto uv_a = unnamed_vector[0];
        auto uv_b = uv_a[0];
        uv_b = '?';

        Test::assert_that((char) unnamed_vector[0][0] == '?');


        auto unnamed_instance = State::safe_script("return instance");
        auto ui_a = unnamed_instance["_field"];
        auto ui_b = ui_a[0];
        ui_b = 999;

        Test::assert_that((int) unnamed_instance["_field"][0] == 999);
    });

    Test::test("proxy detach update", []()
    {
        State::safe_script(R"(

        mutable struct Detach

            _field::Int64
        end

        instance = Detach(123);
        )");

        auto proxy = Main["instance"];

        State::script("instance = 9999");

        Test::assert_that((size_t) proxy["_field"] == 123);

        proxy.update();

        Test::assert_that((size_t) proxy == 9999);
    });

    Test::test("proxy make unnamed", [](){

        State::script("var = [1, 2, 3, 4]");
        auto named = Main["var"];

        named[0] = 9999;
        Test::assert_that(State::script("return var[1]").operator int() == 9999);

        named = named.value();
        named[0] = 0;
        Test::assert_that(State::script("return var[1]").operator int() == 9999);
        Test::assert_that(named[0].operator int() == 0);
    });

    Test::test("proxy reject immutable", [](){

        auto string_proxy = State::script("return \"string\"");
        auto first = string_proxy[0];

        Test::assert_that(first.operator char() == 's');

        bool thrown = false;
        try
        {
            first = 'b';
        }
        catch (JuliaException& e)
        {
            thrown = true;
        }

        Test::assert_that(thrown);

        State::safe_script(R"(

            struct ImmutableStructType
                _field;

                ImmutableStructType() = new(123)
            end

            instance = ImmutableStructType();
        )");

        auto struct_proxy = Main["instance"];
        auto field = struct_proxy["_field"];

        Test::assert_that(field.operator int() == 123);

        thrown = false;
        try
        {
            field = 456;
        }
        catch (JuliaException& e)
        {
            thrown = true;
        }

        Test::assert_that(thrown);

    });

    /*
    Test::test("proxy cast", []() {

        State::safe_script(R"(
            symbol = Symbol("")
            array = [1, 2, 3, 4]
            type = Type
        )");

        Symbol s = Main["symbol"].as<Symbol>();
        Test::assert_that(s.operator std::string() == "");

        Array<Int64, 1> a = Main["array"].as<Array<Int64, 1>>();
        Test::assert_that((int) a.at(0) == 1);
    });
     */

    Test::conclude();
}


