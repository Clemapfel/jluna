#include <iostream>
#include <ptrhash.h>
#include <thread>
#include <chrono>

#include <jluna.hpp>
#include "test.hpp"

using namespace jluna;
using namespace jluna::detail;

struct NonJuliaType
{
    std::vector<uint64_t> _field;
};
set_usertype_enabled(NonJuliaType);
make_usertype_implicitly_convertible(NonJuliaType);

#include <thread>

int main()
{
    initialize(2, false, "/home/clem/Workspace/jluna/build/libjluna.so"); //, false, "/home/clem/Workspace/jluna/cmake-build-debug/libjluna.so");

    Test::initialize();
    Test::test("c_adapter found", [](){

        auto a = safe_eval("return jluna.cppcall.verify_library()");
        Test::assert_that(jl_unbox_bool(a));
    });

    Test::test("unsafe: gc_push / gc_pop", [](){

        auto* value = jl_eval_string("return [123, 434, 342]");
        gc_push(value);

        collect_garbage();

        auto after = unbox<std::vector<uint64_t>>(value);
        Test::assert_that(after.at(2) == 342);

        gc_pop(1);
    });

    Test::test("unsafe: gc", []() {

        auto* value = jl_eval_string("return [123, 434, 342]");
        auto id = unsafe::gc_preserve(value);

        collect_garbage();

        auto after = unbox<std::vector<uint64_t>>(value);
        Test::assert_that(after.at(2) == 342);

        unsafe::gc_release(id);
    });

    Test::test("unsafe: _sym", []() {

        using namespace unsafe;

        auto* symbol = jl_eval_string("return Symbol(\"test\")");
        uint64_t symbol_id = unsafe::gc_preserve(symbol);
        Test::assert_that("test"_sym == (jl_sym_t*) symbol);
        unsafe::gc_release(symbol_id);
    });

    Test::test("as_julia_pointer", [](){

        auto* ptr = jl_eval_string("return 1234");
        uint64_t true_value = (uint64_t) ptr;

        Test::assert_that(true_value == unbox<UInt64>(detail::convert((unsafe::DataType*) UInt64_t, as_julia_pointer(ptr))));
    });

    Test::test("unsafe: get_function", []() {

        using namespace unsafe;

        Main.safe_eval(R"(
            module __M1
                function f()
                    return 1234;
                end
            end
        )");

        auto* f_true = jl_eval_string("return __M1.f");
        auto* f_a = unsafe::get_function("__M1"_sym, "f"_sym);
        auto* m = (jl_module_t*) jl_eval_string("return __M1");
        auto* f_b = unsafe::get_function(m, "f"_sym);

        Test::assert_that(f_true == f_a);
        Test::assert_that(f_true == f_b);
    });

    Test::test("unsafe: Expr & eval", []() {

        using namespace unsafe;

        auto* expr_true = (jl_expr_t*) jl_eval_string("Expr(:call, :+, Int64(100), Int64(100))");
        auto expr_true_id = unsafe::gc_preserve(expr_true);
        auto* expr = unsafe::Expr("call"_sym, "+"_sym, jl_box_int64(100), jl_box_int64(100));
        auto expr_id = unsafe::gc_preserve(expr);

        Test::assert_that(jluna::detail::is_equal((unsafe::Value*) expr_true, (unsafe::Value*) expr));
        Test::assert_that(unsafe::eval(expr) == jl_box_int64(200));

        unsafe::gc_release(expr_true_id);
        unsafe::gc_release(expr_id);
    });

    Test::test("unsafe: get/set value", []() {

        using namespace unsafe;
        auto* res = (unsafe::Value*) Main.safe_eval(R"(
            module __M2
                value = 1234
            end
            return __M2.value;
        )");

        auto* m = (unsafe::Module*) unsafe::get_value(jl_main_module, "__M2"_sym);
        auto* get = unsafe::get_value(m, "value"_sym);
        Test::assert_that(unbox<Int64>(get) == unbox<Int64>(res));

        unsafe::set_value(m, "value"_sym, jl_box_int64(4567));
        Test::assert_that(jl_unbox_bool(jl_eval_string("return __M2.value == 4567")));
    });

    Test::test("unsafe: get_field", []() {

        using namespace unsafe;
        auto* instance = (unsafe::Value*) Main.safe_eval(R"(
            struct get_field_inner
                _member::Int64
                get_field_inner() = new(1234)
            end

            struct get_field_outer
                _member::get_field_inner
                get_field_outer() = new(get_field_inner())
            end

            return get_field_outer()
        )");

        auto instance_id = gc_preserve(instance);

        auto* member = unsafe::get_field(unsafe::get_field(instance, "_member"_sym), "_member"_sym);
        auto* member_true = jl_get_nth_field(jl_get_nth_field(instance, 0), 0);

        Test::assert_that(jluna::detail::is_equal(member, member_true));
        gc_release(instance_id);
    });

    Test::test("unsafe: set_field", []() {

        using namespace unsafe;
        auto* instance = (unsafe::Value*) Main.safe_eval(R"(
            mutable struct set_field_inner
                _member::Int64
                set_field_inner() = new(1234)
            end

            struct set_field_outer
                _member::get_field_inner
                set_field_outer() = new(get_field_inner())
            end

            return set_field_outer()
        )");

        unsafe::set_field(jl_get_nth_field(instance, 0), "_member"_sym, jl_box_uint64(4567));
        Test::assert_that(
        jluna::detail::is_equal(jl_get_nth_field(jl_get_nth_field(instance, 0), 0), jl_box_uint64(1234)));
    });

    Test::test("unsafe: call", []() {

        using namespace unsafe;

        auto* range = unsafe::call(unsafe::get_function(jl_base_module, "range"_sym), jl_box_int64(1),
                                   jl_box_int64(10));
        auto* sum = unsafe::call(unsafe::get_function(jl_base_module, "sum"_sym), range);
        Test::assert_that(jl_unbox_int64(sum) == 55);
    });

    Test::test("unsafe: new_array", []() {

        using namespace unsafe;

        std::vector<uint64_t> gc_ids;

        auto* one_d = new_array(Float64_t, 15);
        gc_ids.push_back(unsafe::gc_preserve(one_d));
        Test::assert_that(jl_array_len(one_d) == 15);
        Test::assert_that(jl_array_ndims(one_d) == 1);

        auto* two_d = new_array(Float64_t, 15, 15);
        gc_ids.push_back(unsafe::gc_preserve(two_d));
        Test::assert_that(jl_array_len(two_d) == 15 * 15);
        Test::assert_that(jl_array_ndims(two_d) == 2);

        auto* six_d = new_array(Float64_t, 15, 15, 15, 15, 15, 15);
        gc_ids.push_back(unsafe::gc_preserve(six_d));
        Test::assert_that(jl_array_len(six_d) == pow(15, 6));
        Test::assert_that(jl_array_ndims(six_d) == 6);

        for (uint64_t id : gc_ids)
            unsafe::gc_release(id);
    });

    Test::test("unsafe: new_array_from_data", []() {

        using namespace unsafe;

        std::vector<uint64_t> gc_ids;

        std::vector<uint64_t> one_d_vec;
        for (uint64_t i = 0; i < 10; ++i)
            one_d_vec.push_back(i);

        auto* one_d = unsafe::new_array_from_data(UInt64_t, one_d_vec.data(), one_d_vec.size());
        gc_ids.push_back(unsafe::gc_preserve(one_d));
        Test::assert_that(jl_unbox_uint64(jl_arrayref(one_d, 9)) == 9);

        std::vector<uint64_t> two_d_vec;
        for (uint64_t i = 0; i < 25; ++i)
            two_d_vec.push_back(i);

        auto* two_d = unsafe::new_array_from_data(UInt64_t, two_d_vec.data(), 5, 5);
        gc_ids.push_back(unsafe::gc_preserve(two_d));
        Test::assert_that(jl_unbox_uint64(jl_arrayref(two_d, 9)) == 9);

        std::vector<uint64_t> four_d_vec;
        for (uint64_t i = 0; i < 4 * 4 * 4 * 4; ++i)
            four_d_vec.push_back(i);

        auto* four_d = unsafe::new_array_from_data(UInt64_t, four_d_vec.data(), 4, 4, 4, 4);
        gc_ids.push_back(unsafe::gc_preserve(four_d));
        Test::assert_that(jl_unbox_uint64(jl_arrayref(four_d, 9)) == 9);

        for (uint64_t id : gc_ids)
            unsafe::gc_release(id);
    });

    Test::test("unsafe: override_array", []() {

        jl_array_t * arr01 = (jl_array_t*) jl_eval_string("return Int64[i for i in 1:25]");
        uint64_t id_1 = unsafe::gc_preserve(arr01);
        jl_array_t * arr02 = (jl_array_t*) jl_eval_string("return Int64[i for i in 25:(25+25)]");
        uint64_t id_2 = unsafe::gc_preserve(arr02);

        unsafe::override_array(arr02, arr01);
        for (uint64_t i = 0; i < 25; ++i)
            Test::assert_that(jl_unbox_int64(jl_arrayref(arr02, i)) == i + 1);

        unsafe::gc_release(id_1);
        unsafe::gc_release(id_2);
    });

    Test::test("unsafe: swap_array_data", []() {

        jl_array_t * arr01 = (jl_array_t*) jl_eval_string("return Int64[i for i in 1:25]");
        uint64_t id_1 = unsafe::gc_preserve(arr01);
        jl_array_t * arr02 = (jl_array_t*) jl_eval_string("return Int64[i for i in -1:-1:-25]");
        uint64_t id_2 = unsafe::gc_preserve(arr02);

        unsafe::swap_array_data(arr01, arr02);
        for (uint64_t i = 0; i < 25; ++i)
        {
            Test::assert_that(jl_unbox_int64(jl_arrayref(arr01, i)) == -(static_cast<Int64>(i + 1)));
            Test::assert_that(jl_unbox_int64(jl_arrayref(arr02, i)) == +(static_cast<Int64>(i + 1)));
        }

        unsafe::gc_release(id_1);
        unsafe::gc_release(id_2);
    });

    Test::test("unsafe: set_array_data", []() {

        jl_array_t * arr01 = (jl_array_t*) jl_eval_string("return Int64[i for i in 1:25]");
        uint64_t id = unsafe::gc_preserve(arr01);

        std::vector<Int64> new_data;
        for (uint64_t i = 0; i < 30; ++i)
            new_data.push_back(i + 43);

        unsafe::set_array_data(arr01, new_data.data(), new_data.size());

        for (uint64_t i = 0; i < new_data.size(); ++i)
            Test::assert_that(jl_unbox_int64(jl_arrayref(arr01, i)) == new_data.at(i));

        unsafe::gc_release(id);
    });

    Test::test("unsafe: resize_array: reshape", []()
    {
        gc_pause;
        jl_array_t * arr = (jl_array_t*) jl_eval_string("return [i for i in 1:(5*5*5)]");
        uint64_t id1 = unsafe::gc_preserve(arr);

        unsafe::resize_array(arr, 5, 5, 5);
        Test::assert_that(jl_array_ndims(arr) == 3);

        arr = (jl_array_t*) jl_eval_string("return [i for i in 1:(5*5)]");
        uint64_t id2 = unsafe::gc_preserve(arr);
        unsafe::resize_array(arr, 5, 5);
        Test::assert_that(jl_array_ndims(arr) == 2);

        arr = (jl_array_t*) jl_eval_string("return reshape([i for i in 1:(5*5)], 5, 5)");
        uint64_t id3 = unsafe::gc_preserve(arr);
        unsafe::resize_array(arr, 25);
        Test::assert_that(jl_array_ndims(arr) == 1);

        for (uint64_t id : {id1, id2, id3})
            unsafe::gc_release(id);

        gc_unpause;
    });

    Test::test("unsafe: resize_array: grow", []() {

        jl_array_t * arr = (jl_array_t*) jl_eval_string("return [i for i in 1:(5*5*5)]");
        uint64_t id1 = unsafe::gc_preserve(arr);
        unsafe::resize_array(arr, 5 * 5);
        Test::assert_that(jl_array_len(arr) == 5 * 5);
        unsafe::resize_array(arr, 5 * 5 * 5);
        Test::assert_that(jl_array_len(arr) == 5 * 5 * 5);

        arr = (jl_array_t*) jl_eval_string("return reshape([i for i in 1:(5*5)], 5, 5)");
        uint64_t id2 = unsafe::gc_preserve(arr);
        unsafe::resize_array(arr, 4, 4);
        Test::assert_that(jl_array_len(arr) == 4 * 4);
        unsafe::resize_array(arr, 6, 6);
        Test::assert_that(jl_array_len(arr) == 6 * 6);

        for (uint64_t id : {id1, id2})
            unsafe::gc_release(id);
    });

    Test::test("unsafe: array: get_index", []() {

        jl_array_t * arr = (jl_array_t*) jl_eval_string("return Int64[i for i in 1:25]");
        uint64_t id1 = unsafe::gc_preserve(arr);

        for (uint64_t i = 0; i < 25; ++i)
            Test::assert_that(jl_unbox_int64(unsafe::get_index(arr, i)) == i + 1);

        static unsafe::Function* getindex = unsafe::get_function(jl_base_module, "getindex"_sym);

        arr = (jl_array_t*) jl_eval_string("return reshape(Int64[i for i in 1:(6*8)], 6, 8)");
        uint64_t id2 = unsafe::gc_preserve(arr);
        Test::assert_that(jl_unbox_int64(unsafe::call(getindex, arr, jl_box_int64(4), jl_box_int64(4))) ==
                          jl_unbox_int64(unsafe::get_index(arr, 3, 3)));

        arr = (jl_array_t*) jl_eval_string("return reshape(Int64[i for i in 1:(3*4*5*6)], 3, 4, 5, 6)");
        uint64_t id3 = unsafe::gc_preserve(arr);
        Test::assert_that(jl_unbox_int64(unsafe::get_index(arr, 2, 2, 2, 2)) == jl_unbox_int64(
        unsafe::call(getindex, arr, jl_box_int64(3), jl_box_int64(3), jl_box_int64(3), jl_box_int64(3))));

        for (uint64_t id : {id1, id2, id3})
            unsafe::gc_release(id);
    });

    Test::test("unsafe: get_array_size", []() {

        jl_array_t * arr = (jl_array_t*) jl_eval_string("return Int64[i for i in 1:25]");
        uint64_t id1 = unsafe::gc_preserve(arr);
        Test::assert_that(unsafe::get_array_size(arr) == 25);

        arr = (jl_array_t*) jl_eval_string("return reshape(Int64[i for i in 1:(3*5*4)], 3, 5, 4)");
        uint64_t id2 = unsafe::gc_preserve(arr);

        Test::assert_that(unsafe::get_array_size(arr, 0) == 3);
        Test::assert_that(unsafe::get_array_size(arr, 1) == 5);
        Test::assert_that(unsafe::get_array_size(arr, 2) == 4);

        for (uint64_t id : {id1, id2})
            unsafe::gc_release(id);
    });

    Test::test("catch c exception", []() {

        Test::assert_that_throws<JuliaException>([]() {
            jl_eval_string("return undefined");
            forward_last_exception();
        });
    });

    Test::test("safe_call", []() {
        Test::assert_that_throws<JuliaException>([]() {
            safe_call(unsafe::get_function(jl_base_module, "throw"_sym), jl_eval_string("return ErrorException(\"\")"));
        });
    });

    Test::test("safe_eval", []() {
        Test::assert_that_throws<JuliaException>([]() {
            safe_eval("throw(ErrorException(\"abc\"))");
        });
    });

    auto test_box_unbox = []<typename T>(const std::string type_name, T value) {
        Test::test("box/unbox " + type_name, [value]() {

            jl_value_t * boxed = box<T>(value);
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

    test_box_unbox("Pair", std::pair<uint64_t, std::string>(12, "abc"));
    test_box_unbox("Tuple3", std::tuple<uint64_t, std::string, float>(uint64_t(12), "abc", 0.01f));
    test_box_unbox("Symbol", jluna::Symbol("abc"));

    auto test_box_unbox_iterable = []<typename T>(const std::string& name, T&& value) {

        Test::test("box/unbox " + name, [&value]() {

            jl_value_t * boxed = box<T>(value);
            auto unboxed = unbox<T>(boxed);

            Test::assert_that(value == unboxed);
        });
    };

    test_box_unbox_iterable("Vector", std::vector<uint64_t>{1, 2, 3, 4});
    test_box_unbox_iterable("Dict", std::map<uint64_t, std::string>{{12, "abc"}});
    test_box_unbox_iterable("Dict", std::unordered_map<uint64_t, std::string>{{12, "abc"}});
    test_box_unbox_iterable("Set", std::set<uint64_t>{1, 2, 3, 4});

    Test::test("make_new_named_undef", []() {

        auto undef = Main.new_undef("name");
        Test::assert_that(undef == jl_eval_string("return undef"));
    });

    Test::test("make_new_named_int", []() {

        int8_t value = 1;
        auto res = Main.new_int8("test", value);
        Test::assert_that(res.operator int8_t() == value);

        res = Main.new_int16("test", int16_t(value));
        Test::assert_that(res.operator int16_t() == value);

        res = Main.new_int32("test", int32_t(value));
        Test::assert_that(res.operator int32_t() == value);

        res = Main.new_int64("test", int64_t(value));
        Test::assert_that(res.operator int64_t() == value);

        res = Main.new_uint8("test", uint8_t(value));
        Test::assert_that(res.operator uint8_t() == value);

        res = Main.new_uint16("test", uint16_t(value));
        Test::assert_that(res.operator uint16_t() == value);

        res = Main.new_uint32("test", uint32_t(value));
        Test::assert_that(res.operator uint32_t() == value);

        res = Main.new_uint64("test", uint64_t(value));
        Test::assert_that(res.operator uint64_t() == value);
    });

    Test::test("make_new_named_float", []() {

        float value = 1;
        auto res = Main.new_float32("test", value);
        Test::assert_that(res.operator float() == value);

        res = Main.new_float64("test", value);
        Test::assert_that(res.operator double() == value);

        //jl_eval_string("test = undef");
    });

    Test::test("make_new_named_vector", []() {

        std::vector<uint64_t> value = {1, 2, 3};
        auto res = Main.new_vector("test", value);
        Test::assert_that(res.operator std::vector<uint64_t>() == value);

        //jl_eval_string("test = undef");
    });

    Test::test("make_new_named_set", []() {

        std::set<uint64_t> value = {1, 2, 3};
        auto res = Main.new_set("test", value);
        Test::assert_that(res.operator std::set<uint64_t>() == value);

        //jl_eval_string("test = undef");
    });

    Test::test("make_new_named_dict", []() {

        std::unordered_map<uint64_t, std::string> value = {{2, "abc"}};
        auto res = Main.new_dict("test", value);
        Test::assert_that(res.operator std::unordered_map<uint64_t, std::string>() == value);

        std::map<uint64_t, std::string> value2 = {{2, "abc"}};
        auto res2 = Main.new_dict("test", value);
        Test::assert_that(res2.operator std::map<uint64_t, std::string>() == value2);
        //jl_eval_string("test = undef");
    });

    Test::test("make_new_pair", []() {

        std::pair<uint64_t, std::string> value = {2, "abc"};
        auto res = Main.new_pair("test", value.first, value.second);
        Test::assert_that(res.operator std::pair<uint64_t, std::string>() == value);

        //jl_eval_string("test = undef");
    });

    Test::test("make_new_tuple", []() {

        std::tuple<uint64_t, std::string, int> value = {2, "abc", -1};
        auto res = Main.new_tuple("test", std::get<0>(value), std::get<1>(value), std::get<2>(value));
        Test::assert_that(res.operator std::tuple<uint64_t, std::string, int>() == value);

        //jl_eval_string("test = undef");
    });

    Test::test("create_reference", []() {

        Main.safe_eval(R"(
            struct StructType
                any
            end
        )");

        auto hold = Proxy(jl_eval_string("return StructType([99, 2, 3, 4])"), nullptr);
        collect_garbage();
        Test::assert_that(((int) hold["any"][0]) == 99);
    });

    Test::test("proxy ctor", []() {

        jl_value_t * val = jl_eval_string("return [1, 2, 3, 4]");
        auto proxy = Proxy(val, nullptr);

        Test::assert_that(jl_unbox_bool(jl_call2(jl_get_function(jl_base_module, "=="), val, (jl_value_t*) proxy)));
    });

    Test::test("proxy trivial dtor", []() {

        jl_value_t * val = jl_eval_string("return [1, 2, 3, 4]");
        uint64_t n = 0;
        {
            auto proxy = Proxy(val, nullptr);
            n = jl_unbox_int64(jl_eval_string("return length(jluna.memory_handler._refs.x)"));
        }

        Test::assert_that(n - jl_unbox_int64(jl_eval_string("return length(jluna.memory_handler._refs.x)")) == 2);
        // 2 bc symbol and value are registered, even for unnamed
    });

    Test::test("proxy inheritance dtor", []() {

        Main.safe_eval(R"(
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
        collect_garbage();

        Test::assert_that((bool) inner.get());
    });

    Test::test("proxy reject as non-vector", []() {

        Main.safe_eval(R"(
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
        {
        }
    });

    Test::test("proxy reject as non-function", []() {

        Main.safe_eval(R"(
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
        {
        }
    });

    Test::test("proxy reject as non-struct", []() {

        Main.safe_eval(R"(
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
        {
        }
    });

    Test::test("proxy fieldnames", []() {

        Main.safe_eval(R"(

            struct FieldStruct
                _a
                _b
                _☻
            end
        )");

        Proxy proxy = Main.safe_eval("return FieldStruct");
        auto names = proxy.get_field_names();

        Test::assert_that(names.at(0) == "_a" and names.at(1) == "_b" and names.at(2) == "_☻");
    });

    Test::test("proxy mutation", []() {

        Main.safe_eval("variable = [1, 2, 3, 4]");

        auto mutating_proxy = Main["variable"];

        Test::assert_that(mutating_proxy.is_mutating());
        mutating_proxy[0] = 9999;

        Test::assert_that(mutating_proxy[0].operator Int64() == 9999);
        Test::assert_that(jl_unbox_int64(jl_eval_string("variable[1]")) == 9999);


        auto non_mutating_proxy = Main.safe_eval("return variable");
        non_mutating_proxy = 8888;

        Test::assert_that(non_mutating_proxy.operator Int64() == 8888);
        Test::assert_that(jl_unbox_int64(jl_eval_string("variable[1]")) != 8888);
    });

    Test::test("proxy mutate unnamed member", []() {

        Main.safe_eval(R"(

            mutable struct UnnamedMemberStruct
                _field::Vector{Any}
            end

            vector = [['a', 'b', 'c'], ['a', 'b', 'c'], ['a', 'b', 'c']]
            instance = UnnamedMemberStruct([1, 2, 3, 4])
        )");

        auto unnamed_vector = Main.safe_eval("return vector");
        auto uv_a = unnamed_vector[0];
        auto uv_b = uv_a[0];
        uv_b = '?';

        Test::assert_that((char) unnamed_vector[0][0] == '?');

        auto unnamed_instance = Main.safe_eval("return instance");
        auto ui_a = unnamed_instance["_field"];
        auto ui_b = ui_a[0];
        ui_b = 999;

        Test::assert_that((int) unnamed_instance["_field"][0] == 999);
    });

    Test::test("proxy detach update", []() {
        Main.safe_eval(R"(

        mutable struct Detach

            _field::Int64
        end

        instance = Detach(123);
        )");

        auto proxy = Main["instance"];

        Main.safe_eval("instance = 9999");

        Test::assert_that((uint64_t) proxy["_field"] == 123);

        proxy.update();

        Test::assert_that((uint64_t) proxy == 9999);
    });

    Test::test("proxy make unnamed", []() {

        jluna::safe_eval("var = [1, 2, 3, 4]");
        auto named = Main["var"];

        named[0] = 9999;
        Test::assert_that(Main.safe_eval("return var[1]").operator int() == 9999);

        named = named.as_unnamed();
        named[0] = 0;
        Test::assert_that(Main.safe_eval("return var[1]").operator int() == 9999);
        Test::assert_that(named[0].operator int() == 0);
    });

    Test::test("proxy reject immutable", []() {

        auto string_proxy = Main.safe_eval("return \"string\"");
        auto first = string_proxy[0];

        Test::assert_that(first.operator char() == 's');

        bool thrown = false;
        try
        {
            first = 'b';
        }
        catch (JuliaException&)
        {
            thrown = true;
        }

        Test::assert_that(thrown);

        Main.safe_eval(R"(

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
        catch (JuliaException&)
        {
            thrown = true;
        }

        Test::assert_that(thrown);

    });

    Test::test("proxy cast", []() {

        Main.safe_eval(R"(
            symbol = Symbol("")
            array = [1, 2, 3, 4]
            type = Type
        )");

        //Symbol s = Main["symbol"].as<Symbol>();
        //Test::assert_that(s.operator std::string() == "");

        jluna::Array<Int64, 1> a = Main["array"].as<jluna::Array<Int64, 1>>();
        Test::assert_that((int) a.at(0) == 1);
    });

    Test::test("array: ctor", []() {

        Main.safe_eval("vector = Int32[999, 2, 3, 4, 5]");
        Vector<int> vec = Main["vector"];

        Test::assert_that(vec.at(0).operator int() == 999);

        vec = Vector<int>({1234, 1, 1, 2});

        Test::assert_that(vec.at(0).operator int() == 1234);

        vec = Vector<int>();

        Test::assert_that(vec.get_n_elements() == 0);
    });

    Test::test("array: range index", []() {

        auto vec = jluna::Array<Int64, 1>(Main.safe_eval("return collect(Int64(1):100)"), nullptr);

        const auto subvec = vec.at({12, 19, 99, 2});

        Test::assert_that(subvec.at(0) == 13 and subvec.at(1) == 20 and subvec.at(2) == 100 and subvec.at(3) == 3);
    });

    Test::test("array: reject wrong type", []() {
        //Test::assert_that(false);
        try
        {
            jluna::Array<uint64_t, 1> arr = jluna::safe_eval(R"(return ["abc", "def"])");
            arr.at(0) = "string";
        }
        catch (...)
        {}
    });

    Test::test("array: front/back", []() {
        Main.safe_eval("vector = [999, 2, 3, 4, 666]");
        ArrayAny1d vec = Main["vector"];
        Test::assert_that(vec.front().operator int() == 999 and vec.back().operator int() == 666);
    });

    Test::test("array: empty", []() {
        Main.safe_eval("vector = []");
        ArrayAny1d vec = Main["vector"];
        Test::assert_that(vec.empty());
    });

    Test::test("array: Nd at", []() {

        Main.safe_eval("array = reshape(collect(1:27), 3, 3, 3)");
        ArrayAny3d vec = Main["array"];

        static auto getindex = [&](uint64_t a, uint64_t b, uint64_t c) -> uint64_t {
            jl_function_t* _getindex = jl_get_function(jl_base_module, "getindex");
            std::vector<jl_value_t*> args;
            args.push_back((jl_value_t*) vec);
            args.push_back(jl_box_uint64(a));
            args.push_back(jl_box_uint64(b));
            args.push_back(jl_box_uint64(c));

            return jl_unbox_uint64(jl_call(_getindex, args.data(), args.size()));
        };

        Test::assert_that(getindex(1, 2, 3) == static_cast<uint64_t>(vec.at(0, 1, 2)));
        Test::assert_that(getindex(3, 3, 3) == static_cast<uint64_t>(vec.at(2, 2, 2)));
    });

    Test::test("array: out of range", []() {
        Main.safe_eval("array = reshape(collect(1:27), 3, 3, 3)");
        ArrayAny3d arr = Main["array"];

        static auto test = [&](uint64_t a, uint64_t b, uint64_t c) -> bool {
            try
            {
                arr.at(a, b, c);
            }
            catch (...)
            {
                return true;
            }

            return false;
        };

        Test::assert_that(test(-1, 2, 2));
        Test::assert_that(test(3, 2, 2));
        Test::assert_that(test(2, 3, 2));
        Test::assert_that(test(2, 2, 3));
    });

    Test::test("array_iterator: +/-", []() {

        Main.safe_eval("array = reshape(collect(1:27), 3, 3, 3)");
        ArrayAny3d arr = Main["array"];

        auto it = arr.begin();
        it++;
        Test::assert_that((int) it == 2);

        it = arr.end();
        it--;
        Test::assert_that((int) it == 27);
    });

    Test::test("array_iterator: on owner reassignment", []() {

        // check behavior if owner proxy gets reassigned during iteration
        // unlike proxy, should segfault

        Main.safe_eval("vec = collect(1:20)");
        ArrayAny1d arr = Main["vec"];
        auto it = arr.begin();
        while (it++ != arr.end());

        Main.safe_eval("vec = [1, 2, 3, 4, 5]");

        bool thrown = false;
        try
        {
            uint64_t as = *it;
        }
        catch (jluna::JuliaException&)
        {
            thrown = true;
        }

        Test::assert_that(thrown);
    });

    Test::test("array_iterator: comparison", []() {
        Main.safe_eval("array = reshape(collect(1:27), 3, 3, 3)");
        ArrayAny3d arr = Main["array"];

        auto a = arr.begin();
        auto b = arr.begin();
        auto c = arr.end();

        Test::assert_that(a == b and a != c);
        b++;
        Test::assert_that(a != b and b != c);
    });

    Test::test("array_iterator: cast to value", []() {

        Main.safe_eval("array = reshape(collect(1:27), 3, 3, 3)");
        ArrayAny3d arr = Main["array"];

        bool thrown = false;
        try
        {
            arr.begin().operator Module();
        }
        catch (...)
        {
            thrown = true;
        }

        Test::assert_that(thrown);
        Test::assert_that(arr.begin().operator std::string() == "1");
        Test::assert_that(arr.begin().operator Int64() == 1);
    });

    Test::test("array_iterator: cast to proxy", []() {

        Main.safe_eval("array = reshape(collect(1:27), 3, 3, 3)");
        ArrayAny3d arr = Main["array"];

        auto it = arr.at(0, 0, 0);
        Proxy as_proxy = it;
        as_proxy = 999;
        Test::assert_that(Main.safe_eval("return array")[0].operator int() == 999);
    });

    Test::test("array: comprehension", []() {

        auto arr = jluna::Array<Int64, 1>(jl_eval_string("return collect(0:10)"));
        auto vec = arr.at("(i for i in 0:9 if i % 2 == 0)"_gen);

        for (auto it : vec)
            Test::assert_that(it.operator int() % 2 == 0);

        auto new_vec = Vector<Int64>("(i for i in 0:9 if i % 2 == 0)"_gen);

        for (auto it : new_vec)
            Test::assert_that(it.operator int() % 2 == 0);
    });

    Test::test("vector: insert", []() {

        Main.safe_eval("vector = UInt64[1, 2, 3, 4]");
        Vector<uint64_t> vec = Main["vector"];

        vec.insert(0, 16);
        Test::assert_that((int) vec.at(0) == 16);
    });

    Test::test("vector: erase", []() {

        Main.safe_eval("vector = UInt64[1, 99, 3, 4]");
        Vector<uint64_t> vec = Main["vector"];

        vec.erase(0);
        Test::assert_that((int) vec.at(0) == 99 and vec.get_n_elements() == 3);
    });

    Test::test("vector: append", []() {
        Main.safe_eval("vector = UInt64[1, 1, 1, 1]");
        Vector<uint64_t> vec = Main["vector"];

        vec.push_front(999);
        vec.push_back(666);
        Test::assert_that(vec.get_n_elements() == 6 and vec.front<int>() == 999 and vec.back<int>() == 666);
    });

    Test::test("C: call success", []() {

        Main.create_or_assign("test", as_julia_function<Int64(Int64)>([](Int64 in) {
            return in + 11;
        }));

        Test::assert_that(Main.safe_eval("test(100) == 111").operator bool());
    });

    Test::test("C: forward exception", []() {

        Main.create_or_assign("test", as_julia_function<Nothing()>([]() -> void {

            throw std::out_of_range("123");
        }));

        bool thrown = false;

        try
        {
            Main.safe_eval("cppcall(:test)");
        }
        catch (...)
        {
            thrown = true;
        }
    });

    Test::test("C: reject wrong-sized tuple", []() {

        Main.create_or_assign("zero", as_julia_function<void()>([]() -> void {}));
        Main.create_or_assign("one", as_julia_function<void(jl_value_t*)>([](jl_value_t*) -> void {}));
        Main.create_or_assign("two", as_julia_function<void(jl_value_t*, jl_value_t*)>([](jl_value_t*, jl_value_t*) -> void {}));
        Main.create_or_assign("three", as_julia_function<void(jl_value_t*, jl_value_t*, jl_value_t*)>([](jl_value_t*, jl_value_t*, jl_value_t*) -> void {}));

        for (std::string e : {"one", "two", "three"})
        {
            bool thrown = false;
            try
            {
                if (e == "zero")
                    Main.safe_eval("cppcall(:" + e + ", 123)");
                else
                    Main.safe_eval("cppcall(:" + e + ")");
            }
            catch (JuliaException&)
            {
                thrown = true;
            }

            Test::assert_that(thrown);
        }
    });

    Test::test("Symbol: CTOR", []() {

        auto proxy = jluna::Symbol("abc");

        Test::assert_that(proxy.get_type() == Symbol_t);
        Test::assert_that(proxy.operator std::string() == "abc");
    });

    Test::test("Symbol: Hash", []() {

        auto proxy = jluna::Symbol("abc");
        Test::assert_that(proxy.hash() == (uint64_t) Base["hash"]((unsafe::Value*) proxy));
    });

    Test::test("Type: CTOR", []() {

        auto type = Type(jl_nothing_type);
        Test::assert_that(type.operator _jl_datatype_t*() == jl_nothing_type);

        type = as_julia_type<std::vector<uint64_t>>::type();
    });

    auto test_type = []<typename T>(Type& a, T b) {

        std::string name = "Type Constant: ";
        name += jluna::detail::to_string((unsafe::Value*) a);

        Test::test(name, [&]() {
            return a.operator jl_datatype_t*() == reinterpret_cast<jl_datatype_t*>(b);
        });
    };

    test_type(AbstractArray_t, jl_abstractarray_type);
    test_type(AbstractChar_t, jl_eval_string("return AbstractChar"));
    test_type(AbstractFloat_t, jl_eval_string("return AbstractFloat"));
    test_type(AbstractString_t, jl_abstractstring_type);
    test_type(Any_t, jl_any_type);
    test_type(Array_t, jl_array_type);
    test_type(Bool_t, jl_bool_type);
    test_type(Char_t, jl_char_type);
    test_type(DataType_t, jl_datatype_type);
    test_type(DenseArray_t, jl_densearray_type);
    test_type(Exception_t, jl_eval_string("return Exception"));
    test_type(Expr_t, jl_expr_type);
    test_type(Float16_t, jl_float16_type);
    test_type(Float32_t, jl_float32_type);
    test_type(Float64_t, jl_float64_type);
    test_type(Function_t, jl_function_type);
    test_type(GlobalRef_t, jl_globalref_type);
    test_type(IO_t, jl_eval_string("return IO"));
    test_type(Int128_t, jl_eval_string("return Int128_t"));
    test_type(Int16_t, jl_int16_type);
    test_type(Int32_t, jl_int32_type);
    test_type(Int64_t, jl_int64_type);
    test_type(Integer_t, jl_eval_string("return Integer"));
    test_type(LineNumberNode_t, jl_linenumbernode_type);
    test_type(Method_t, jl_method_type);
    test_type(Module_t, jl_module_type);
    test_type(NTuple_t, jl_eval_string("return NTuple"));
    test_type(NamedTuple_t, jl_namedtuple_type);
    test_type(Nothing_t, jl_nothing_type);
    test_type(Number_t, jl_number_type);
    test_type(Pair_t, jl_pair_type);
    test_type(Ptr_t, jl_pointer_type);
    test_type(QuoteNode_t, jl_quotenode_type);
    test_type(Real_t, jl_eval_string("return Real"));
    test_type(Ref_t, jl_ref_type);
    test_type(Signed_t, jl_signed_type);
    test_type(String_t, jl_string_type);
    test_type(Symbol_t, jl_symbol_type);
    test_type(Task_t, jl_task_type);
    test_type(Tuple_t, jl_tuple_type);
    test_type(Type_t, jl_type_type);
    test_type(TypeVar_t, jl_eval_string("return TypeVar"));
    test_type(UInt128_t, jl_eval_string("return UInt128"));
    test_type(UInt16_t, jl_uint16_type);
    test_type(UInt32_t, jl_uint32_type);
    test_type(UInt64_t, jl_uint64_type);
    test_type(UndefInitializer_t, jl_eval_string("return UndefInitializer"));
    test_type(Union_t, jl_eval_string("return Union"));
    test_type(UnionAll_t, jl_unionall_type);
    test_type(Unsigned_t, jl_eval_string("return Unsigned"));
    test_type(VecElement_t, jl_eval_string("return VecElement"));
    test_type(WeakRef_t, jl_weakref_type);

    Test::test("Type: (<:)", []() {
        Test::assert_that(Char_t.is_subtype_of(Char_t));
        Test::assert_that(Char_t.is_subtype_of(AbstractChar_t));
        Test::assert_that(Char_t.is_supertype_of(Char_t));
        Test::assert_that(AbstractChar_t.is_supertype_of(Char_t));
        Test::assert_that(Char_t.is_same_as(Char_t));
        Test::assert_that(Char_t == Type(jl_char_type));
        Test::assert_that(not(Char_t != Type(jl_char_type)));
    });

    Test::test("Type: get_symbol", []() {
        Test::assert_that(AbstractChar_t.get_symbol().operator jl_sym_t*() == jl_symbol("AbstractChar"));
    });

    Test::test("Type: get_parameters", []() {

        Main.safe_eval(R"(
            struct aiszdbla{T <: Integer, U}
            end
        )");

        Type type = Main["aiszdbla"];
        Test::assert_that(type.get_n_parameters() == 2);

        auto params = type.get_parameters();

        Test::assert_that(params.at(0).first == jluna::Symbol("T"));
        Test::assert_that(params.at(0).second == Integer_t);
        Test::assert_that(params.at(1).first == jluna::Symbol("U"));
        Test::assert_that(params.at(1).second == Any_t);
    });

    Test::test("Type: get_fields", []() {

        Main.safe_eval(R"(
            struct aslaiu
                _a::Integer
                _b
            end
        )");

        Type type = Main["aslaiu"];
        Test::assert_that(type.get_n_fields() == 2);

        auto fields = type.get_fields();

        Test::assert_that(fields.at(0).first == jluna::Symbol("_a"));
        Test::assert_that(fields.at(0).second == Integer_t);
        Test::assert_that(fields.at(1).first == jluna::Symbol("_b"));
        Test::assert_that(fields.at(1).second == Any_t);
    });

    Test::test("Type: is_primitive", []() {

        Main.safe_eval("primitive type PrimitiveType 16 end");
        Type t = Main["PrimitiveType"];
        Test::assert_that(t.is_primitive());
        Test::assert_that(not Array_t.is_primitive());
    });

    Test::test("Type: is_struct_type", []() {
        Main.safe_eval("struct asidblasiu end");
        Type t = Main["asidblasiu"];
        Test::assert_that(t.is_struct_type());
        Test::assert_that(not Bool_t.is_struct_type());
    });

    Test::test("Type: is_declared_mutable", []() {
        Main.safe_eval("mutable struct asdbas end");
        Type t = Main["asdbas"];
        Test::assert_that(t.is_declared_mutable());
        Test::assert_that(not Bool_t.is_declared_mutable());
    });

    Test::test("Type: is_isbits", []() {
        Test::assert_that(Bool_t.is_isbits());
        Test::assert_that(not Module_t.is_isbits());
    });

    Test::test("Type: is_singleton", []() {
        Main.safe_eval("struct asdiubasldiba end");
        Type t = Main["asdiubasldiba"];
        Test::assert_that(t.is_singleton());
        Test::assert_that(not Module_t.is_singleton());
    });

    Test::test("Type: is_abstract_type", []() {
        Test::assert_that(AbstractChar_t.is_abstract_type());
        Test::assert_that(not Bool_t.is_abstract_type());
    });

    Test::test("Type: is_abstract_ref_type", []() {

        auto t = Type(Main.safe_eval("return Ref{AbstractFloat}")).is_abstract_ref_type();
        Test::assert_that(AbstractChar_t.is_abstract_type());
        Test::assert_that(not Bool_t.is_abstract_type());
    });

    Test::test("Type: is_typename", []() {

        Test::assert_that(Array_t.typename_is("Array"));
        Test::assert_that(Array_t.typename_is(Array_t));
    });


    Test::test("Generator Expression", []() {

        Test::assert_that_throws<std::invalid_argument>([]() {
            volatile auto gen = "for i in 1:10 i = i end"_gen;
        });

        Test::assert_that_throws<JuliaException>([]() {
            volatile auto gen = "aas a0 ()d"_gen;
        });

        auto gen = "(x for x in 1:10)"_gen;

        uint64_t i = 1;
        for (auto e : gen)
        {
            Test::assert_that(unbox<Int32>(e) == i);
            i += 1;
        }
    });

    Test::test("Usertype: enable", []() {

        Test::assert_that(Usertype<NonJuliaType>::get_name() == "NonJuliaType");
        Test::assert_that(Usertype<NonJuliaType>::is_enabled());
    });

    Test::test("Usertype: add property", []() {

        Usertype<NonJuliaType>::add_property<std::vector<uint64_t>>(
        "_field",
        [](NonJuliaType& in) -> std::vector<uint64_t> {
            return in._field;
        }
        );

        Usertype<NonJuliaType>::add_property<std::vector<uint64_t>>(
        "_field",
        [](NonJuliaType& in) -> std::vector<uint64_t> {
            return in._field;
        },
        [](NonJuliaType& out, std::vector<uint64_t> in) -> void {
            out._field = in;
        }
        );
    });

    Test::test("Usertype: implement", []() {
        Usertype<NonJuliaType>::implement();
        Usertype<NonJuliaType>::implement();

        Test::assert_that(Usertype<NonJuliaType>::is_implemented());
    });

    Test::test("Usertype: box/unbox", []() {

        gc_pause;
        auto instance = NonJuliaType{{123, 34556, 12321}};

        auto* res01 = Usertype<NonJuliaType>::box(instance);
        auto* res02 = box<NonJuliaType>(instance);

        Test::assert_that(jluna::detail::is_equal(jl_get_nth_field(res01, 0), jl_get_nth_field(res02, 0)));

        auto backres01 = Usertype<NonJuliaType>::unbox(res01);
        auto backres02 = unbox<NonJuliaType>(res02);

        Test::assert_that(backres01._field.size() == backres02._field.size());
        gc_unpause;
    });

    Test::test("jluna::Mutex", [](){

        auto mutex = jluna::Mutex();

        Test::assert_that(not mutex.is_locked());
        mutex.lock();
        Test::assert_that(mutex.is_locked());
        mutex.unlock();
        Test::assert_that(not mutex.is_locked());
    });

    Test::test("Task<T>: schedule/join", []()
    {
        auto task = ThreadPool::create<uint64_t()>([]() -> uint64_t {
            return 4;
        });

        Test::assert_that(not task.is_running());
        task.schedule();
        task.join();
        Test::assert_that(task.is_done());
        Test::assert_that(task.result().get().value() == 4);
    });

    Test::test("Task<T>: move ctor", []()
    {
        std::vector<Task<uint64_t>> task_a;
        task_a.push_back(ThreadPool::create<uint64_t()>([]() -> uint64_t {
            return 4;
        }));

        std::vector<Task<uint64_t>> task_b;
        task_b.emplace_back(std::move(task_a.back()));

        task_a.back().schedule();
        task_b.back().schedule();

        task_a.back().join();
        task_b.back().join();
    });

    Test::test("Task<T>: access task proxy", []()
    {
        auto lambda = []() -> uint64_t {return 4;};
        auto task = ThreadPool::create<uint64_t()>(lambda);
        auto task_proxy = Proxy(static_cast<unsafe::Value*>(task));

        Test::assert_that((bool)task_proxy["sticky"] == false);
    });

    Test::test("Task<void>: schedule/join", []()
    {
        auto task = ThreadPool::create<void()>([]() {});

        Test::assert_that(not task.is_running());
        task.schedule();
        task.join();
        Test::assert_that(task.is_done());
    });

    Test::test("Task<void>: move ctor", []()
    {
        std::vector<Task<void>> task_a;
        task_a.push_back(ThreadPool::create<void()>([]() {}));

        std::vector<Task<void>> task_b;
        task_b.emplace_back(std::move(task_a.back()));

        task_a.back().schedule();
        task_b.back().schedule();

        task_a.back().join();
        task_b.back().join();
    });

    Test::test("Task<void>: access task proxy", []()
    {
        auto lambda = []() {};
        auto task = ThreadPool::create<void()>(lambda);
        auto task_proxy = Proxy(static_cast<unsafe::Value*>(task));

        Test::assert_that((bool)task_proxy["sticky"] == false);
    });

    return Test::conclude() ? 0 : 1;
}

