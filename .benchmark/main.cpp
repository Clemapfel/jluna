//
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#include <jluna.hpp>
#include <.benchmark/benchmark.hpp>
#include <.benchmark/benchmark_aux.hpp>
#include <complex.h>
#include <thread>

using namespace jluna;

constexpr size_t n_reps = 20000;
std::vector<Proxy> _proxies;

int main()
{
    initialize(8);
    Benchmark::initialize();

    const auto lambda = []() -> void{
        volatile size_t sum = 0;
        for (size_t i = 0; i < 20000; ++i)
            sum = (sum + i) * i;
    };
    auto lambda_ = Main.new_undef("lambda");

    Benchmark::run_as_base("lambda_base", n_reps, [&](){

        std::vector<std::thread> threads;
        for (size_t i = 0; i < 8; ++i)
        {
            gc_pause; gc_unpause; // for equal comparison
            threads.push_back(std::thread(lambda));
        }

        for (auto& t : threads)
            t.join();
    });

    Benchmark::run("lambda_julia", n_reps, [&](){

        static auto* new_thread = jl_eval_string("(f, xs...) -> schedule(Task(f, xs...))");
        static auto* wait = unsafe::get_function(jl_base_module, "wait"_sym);

        std::vector<unsafe::Value*> threads;
        for (size_t i = 0; i < 8; ++i)
        {
            gc_pause;
            auto* boxed = box(lambda);
            threads.push_back(unsafe::call(new_thread, boxed));
            gc_unpause;
        }

        for (auto* t : threads)
            unsafe::call(wait, t);
    });

    Benchmark::conclude();
    return 0;

    Benchmark::run_as_base("unsafe: Allocate Array C-API", n_reps, [](){

        volatile auto* array = jl_alloc_array_1d(jl_apply_array_type((jl_value_t*) jl_float64_type, 1), 50000);
    });

    Benchmark::run("unsafe: Allocate Array", n_reps, [](){

        volatile auto* array = unsafe::new_array((unsafe::Value*) jl_float64_type, 50000);
    });

    Benchmark::run("unsafe: Allocate jluna::Array", n_reps, [](){

        volatile auto array = jluna::Array<Float64, 1>(jl_eval_string("return Float64[i for i in 1:50000]"));
    });

    Benchmark::run_as_base("unsafe: get_index C-API", n_reps, [](){

        static auto* array = (jl_array_t*) jl_eval_string("return [i for i in 1:10000]");

        for (size_t i = 0; i < 100; ++i)
            volatile auto* res = jl_arrayref(array, 234);
    });

    Benchmark::run("unsafe: get_index", n_reps, [](){

        static auto* array = (jl_array_t*) jl_eval_string("return [i for i in 1:10000]");

        for (size_t i = 0; i < 100; ++i)
            volatile auto* res = jl_arrayref(array, 234);
    });

    Benchmark::run_as_base("unsafe: set_index C-API", n_reps, [](){

        static auto* array = (jl_array_t*) jl_eval_string("return [i for i in 1:10000]");

        for (size_t i = 0; i < 100; ++i)
            jl_arrayset(array, jl_box_int64(12), generate_number(0, 5000));
    });

    Benchmark::run("unsafe: set_index", n_reps, [](){

        static auto* array = (jl_array_t*) jl_eval_string("return [i for i in 1:10000]");

        for (size_t i = 0; i < 100; ++i)
            unsafe::set_index(array, jl_box_int64(12), generate_number(0, 5000));
    });

    Benchmark::run_as_base("unsafe: preserve through disable", n_reps, [](){

        gc_pause;
        volatile auto* arr = jl_eval_string("return collect(1:1000)");
        gc_unpause;
    });

    Benchmark::run("unsafe: gc_preserve", n_reps, [](){

        auto* arr = jl_eval_string("return collect(1:1000)");
        auto id = unsafe::gc_preserve(arr);
        unsafe::gc_release(id);
    });

    Benchmark::run("unsafe: preserve with proxy", n_reps, [](){

        volatile auto proxy = Proxy(jl_eval_string("return collect(1:1000)"));
    });

    Benchmark::run_as_base("unsafe: get_function C-API", n_reps, []()
    {
       volatile auto* println = jl_get_function(jl_main_module, "println");
       volatile auto* println_2 = jl_get_function((jl_module_t*) jl_eval_string("Main"), "println");
    });

    Benchmark::run("unsafe: get_function", n_reps, []()
    {
       volatile auto* println = unsafe::get_function(jl_main_module, "println"_sym);
       volatile auto* println_2 = unsafe::get_function("Main"_sym, "println"_sym);
    });


    Benchmark::run("unsafe: get_function eval", n_reps, []{

        volatile auto* println = jl_eval_string("return Main.println");
        volatile auto* println_2 = jl_eval_string("return Main.println");
    });

    Benchmark::run_as_base("unsafe:: get_value C-API", n_reps, [](){

        volatile auto* _ = jl_eval_string("temp = [i for i in 1:1000]");
        volatile auto* res = jl_eval_string("return temp");
    });

    Benchmark::run("unsafe:: get_value", n_reps, [](){

        volatile auto* _ = jl_eval_string("temp = [i for i in 1:1000]");
        volatile auto* res = unsafe::get_value(jl_main_module, "temp"_sym);
    });

    Benchmark::run("unsafe:: get_value Proxy", n_reps, [](){

        volatile auto* _ = jl_eval_string("temp = [i for i in 1:1000]");
        volatile auto res = Main["temp"];
    });

    // pre-load proxy dict
    _proxies.reserve(3000);
    for (size_t i = 0; i < 3000; ++i)
        _proxies.push_back(Main.new_uint64(generate_string(16), generate_number<size_t>()));

    // ### eval code as strings

    Benchmark::run_as_base("jl_eval_string", n_reps, [](){

        auto name = generate_string(8);
        jl_eval_string((name + std::string(" = [i for i in 1:1000]")).c_str());
        jl_eval_string((name + std::string(" = undef")).c_str());
    });

    Benchmark::run("jluna::safe_eval", n_reps, [](){

        auto name = generate_string(8);
        jluna::safe_eval((name + std::string(" = [i for i in 1:1000]")).c_str());
        jluna::safe_eval((name + std::string(" = undef")).c_str());
    });

    Benchmark::run("State::eval", n_reps, [](){

        auto name = generate_string(8);
        jluna::safe_eval(name + std::string(" = [i for i in 1:1000]"));
        jluna::safe_eval(name + std::string(" = undef"));
    });

    Benchmark::run("State::safe_eval", n_reps, [](){

        auto name = generate_string(8);
        jluna::safe_eval(name + std::string(" = [i for i in 1:1000]"));
        jluna::safe_eval(name + std::string(" = undef"));
    });

    Benchmark::run("Module::eval", n_reps, [](){

        auto name = generate_string(8);
        Main.safe_eval(name + std::string(" = [i for i in 1:1000]"));
        Main.safe_eval(name + std::string(" = undef"));
    });

    Benchmark::run("Module::safe_eval", n_reps, [](){

        auto name = generate_string(8);
        Main.safe_eval(name + std::string(" = [i for i in 1:1000]"));
        Main.safe_eval(name + std::string(" = undef"));
    });

    // ### calling functions

    Main.safe_eval(R"(
        function f()
            out::Vector{Int64} = [0]
            for i in 2:1000
                push!(out, i + out[i-1])
            end
            return out
        end
    )");

    Benchmark::run_as_base("jl_call", n_reps, [](){

        static jl_function_t* f = jl_get_function(jl_main_module, "f");
        volatile auto* out = jl_call0(f);
    });
    

    Benchmark::run("jluna::unsafe::call", n_reps, [](){

        static jl_function_t* f = jl_get_function(jl_main_module, "f");
        volatile auto* out = jluna::unsafe::call(f);
    });

    Benchmark::run("jluna::safe_call", n_reps, [](){

        static jl_function_t* f = jl_get_function(jl_main_module, "f");
        volatile auto* out = jluna::safe_call(f);
    });

    Benchmark::run("Proxy::safe_call", n_reps, [](){

        static jl_function_t* f = jl_get_function(jl_main_module, "f");
        static auto proxy = Proxy(f);

        volatile auto res = proxy.safe_call();
    });

    // ### allocating values

    Benchmark::run_as_base("C-API: allocate proxy", n_reps, []()
    {
        gc_pause;
        volatile auto* res = box<std::string>(generate_string(16));
        gc_unpause;
    });
    

    Benchmark::run("jluna: allocate unnamed proxy", n_reps, []()
    {
        volatile auto unnamed = Proxy(box<std::string>(generate_string(16)));
    });

    Benchmark::run("jluna: allocate named proxy", n_reps, []()
    {
        volatile auto named = Proxy(box<std::string>(generate_string(16)), jl_symbol("name"));
    });

    Benchmark::run("State::new_unnamed", n_reps, []()
    {
        volatile auto unnamed = Main.new_string(generate_string(16));
    });

    Benchmark::run("State::new_named", n_reps, []()
    {
        volatile auto named = Main.new_string("name", generate_string(16));
    });

    _proxies.clear();

    /// ### (un)box: vector

    Benchmark::run_as_base("C-API: box vector", n_reps, []()
    {
        auto vec = std::vector<size_t>();
        vec.reserve(10000);

        for (size_t i = 0; i < 10000; ++i)
            vec.push_back(generate_number<size_t>());

        gc_pause;
        static jl_function_t* make_vector = jl_get_function(jl_base_module, "Vector");

        jl_array_t* out = (jl_array_t*) jl_call2(make_vector, Main.new_undef(""), jl_box_uint64(vec.size()));

        for (size_t i = 0; i < vec.size(); ++i)
            jl_arrayset(out, jl_box_uint64(vec.at(i)), i);

        gc_unpause;
    });

    Benchmark::run("jluna: box vector", n_reps, []()
    {
        gc_pause;
        auto vec = std::vector<size_t>();
        vec.reserve(10000);

        for (size_t i = 0; i < 10000; ++i)
            vec.push_back(generate_number<size_t>(0, 10000));

        auto* res = box<std::vector<size_t>>(vec);
        gc_unpause;
    });

    Benchmark::run_as_base("C-API: unbox vector", n_reps, [](){

        gc_pause;
        jl_array_t* vec = (jl_array_t*) jl_eval_string("Vector{UInt64}([i for i in 1:10000])");

        std::vector<size_t> out;
        out.reserve(vec->length);

        for (size_t i = 0; i < vec->length; ++i)
            out.push_back(jl_unbox_uint64(jl_arrayref(vec, i)));

        volatile auto copy = out;

        gc_unpause;
    });

    Benchmark::run("jluna: unbox vector", n_reps, [](){

        auto* vec = jl_eval_string("Vector{Int64}([i for i in 1:10000])");
        auto out = unbox<std::vector<size_t>>(vec);
    });

    /// ### (un)box: primitive

    Benchmark::run_as_base("C-API: box primitive", n_reps, [](){

        volatile auto* res = jl_box_uint64(generate_number<size_t>());
    });
    

    Benchmark::run("jluna: box primitive", n_reps * 10, [](){

        volatile auto* res = box<size_t>(generate_number<size_t>());
    });

    Benchmark::run_as_base("C-API: unbox primitive", n_reps, [](){

        {
            gc_pause;
            auto* val = jl_box_uint64(generate_number<size_t>());
            volatile size_t res = jl_unbox_uint64(val);
            gc_unpause;
        }
    });

    Benchmark::run("jluna: unbox primitive", n_reps * 10, [](){

        {
            auto* val = jl_box_uint64(generate_number<size_t>());
            volatile size_t res = unbox<UInt64>(val);
        }
    });

    /// ### (un)box: string

    Benchmark::run_as_base("C-API: box string", n_reps, [](){

        gc_pause;
        auto value = generate_string(32);
        volatile auto* res = jl_alloc_string(value.size());

        for (size_t i = 0; i < value.size(); ++i)
            jl_string_data(res)[i] = value.at(i);

        gc_unpause;
    });

    Benchmark::run("jluna: box string", n_reps, [](){

        volatile auto* res = box<std::string>(generate_string(32));
    });

    Benchmark::run_as_base("C-API: unbox string", n_reps, [](){

        std::stringstream str;
        str << "return \"" << generate_string(32) << "\"" << std::endl;
        auto* jl_str = jl_eval_string(str.str().c_str());
        size_t length = jl_string_len(jl_str);

        std::string res;
        res.reserve(length);
        res = jl_string_data(jl_str);
    });

    Benchmark::run("jluna: unbox string", n_reps, [](){

        std::stringstream str;
        str << "return \"" << generate_string(32) << "\"" << std::endl;
        auto* jl_str = jl_eval_string(str.str().c_str());
        std::string res = unbox<std::string>(jl_str);
    });

    /// ### (un)box: map

    Benchmark::run_as_base("C-API: box map", n_reps, [](){

        std::map<size_t, size_t> map;
        for (size_t i = 0; i < 1000; ++i)
            map.insert({generate_number<size_t>(), generate_number<size_t>()});

        gc_pause;
        static jl_function_t* make_pair = jl_get_function(jl_base_module, "Pair");
        static jl_function_t* make_map = jl_get_function(jl_base_module, "Dict");

        jl_array_t* vec = (jl_array_t*) jl_alloc_vec_any(map.size());

        {
            size_t i = 0;
            for (auto& pair : map)
                jl_arrayset(vec, jl_call2(make_pair, jl_box_uint64(pair.first), jl_box_uint64(pair.second)), i++);
        }

        volatile auto* out = jl_call1(make_map, (unsafe::Value*) vec);
        gc_unpause;
    });

    Benchmark::run("jluna: box map", n_reps, [](){

        std::map<size_t, size_t> map;
        for (size_t i = 0; i < 1000; ++i)
            map.insert({generate_number<size_t>(), generate_number<size_t>()});

        volatile auto* out = box<std::map<size_t, size_t>>(map);
    });

    Benchmark::run_as_base("C-API: unbox map", n_reps, [](){

        gc_pause;

        auto* map = jl_eval_string("return Dict([Pair(i, i) for i in 1:1000])");

        auto* slots = (jl_array_t*) jl_get_nth_field(map, 0);
        auto* keys = (jl_array_t*) jl_get_nth_field(map, 1);
        auto* vals = (jl_array_t*) jl_get_nth_field(map, 2);

        std::map<size_t, size_t> out;
        for (size_t i = 0; i < slots->length; ++i)
            if (jl_unbox_bool(jl_arrayref(slots, i)))
                out.insert({jl_unbox_uint64(jl_arrayref(keys, i)), jl_unbox_uint64(jl_arrayref(vals, i))});

        gc_unpause;
    });

    Benchmark::run("jluna: unbox map", n_reps, [](){

        auto* map = jl_eval_string("return Dict([Pair(i, i) for i in 1:1000])");
        volatile auto out = unbox<std::map<size_t, size_t>>(map);
    });

    /// ### (un)box: complex

    Benchmark::run_as_base("C-API: box complex", n_reps, [](){

        auto value = std::complex<float>(generate_number<float>(), generate_number<float>());

        gc_pause;
        static jl_function_t* complex = jl_get_function(jl_base_module, "Complex");
        volatile auto* out = jl_call2(complex, jl_box_float32(value.real()), jl_box_float32(value.imag()));
        gc_unpause;
    });

    Benchmark::run("jluna: box complex", n_reps, [](){

        auto value = std::complex<float>(generate_number<float>(), generate_number<float>());
        volatile auto* out = box<std::complex<float>>(value);
    });

    Benchmark::run_as_base("C-API: unbox complex", n_reps, [](){

        auto* value = jl_eval_string("return Complex(rand(), rand())");
        _Complex float as_complex = *((_Complex float*) value);
        volatile auto out = std::complex<float>(as_complex);
    });

    Benchmark::run("jluna: unbox complex", n_reps, [](){

        auto* value = jl_eval_string("return Complex(rand(), rand())");
        volatile auto out = unbox<std::complex<float>>(value);
    });

    /// ### (un)box: set

    Benchmark::run_as_base("C-API: box set", n_reps, []()
    {
        auto set = std::set<size_t>();

        for (size_t i = 0; i < 10000; ++i)
            set.insert(generate_number<size_t>());

        gc_pause;
        static jl_function_t* make_set = jl_get_function(jl_base_module, "Set");
        static jl_function_t* push = jl_get_function(jl_base_module, "push!");

        auto* out = jl_call0(make_set);

        for (size_t e : set)
            jl_call2(push, out, jl_box_uint64(e));

        gc_unpause;
    });

    Benchmark::run("jluna: box set", n_reps, []()
    {
        auto set = std::set<size_t>();

        for (size_t i = 0; i < 10000; ++i)
            set.insert(generate_number<size_t>());

        volatile auto* res = box<std::set<size_t>>(set);
    });

    Benchmark::run_as_base("C-API: unbox set", n_reps, [](){

        gc_pause;
        auto* value = jl_eval_string("Set{UInt64}([rand(UInt64) for _ in 1:100])");
        auto* dict = jl_get_nth_field(value, 0);
        auto* slots = (jl_array_t*) jl_get_nth_field(dict, 0);
        auto* keys = (jl_array_t*) jl_get_nth_field(dict, 1);

        std::set<size_t> out;

        for (size_t i = 0; i < slots->length; ++i)
            if (jl_unbox_bool(jl_arrayref(slots, i)))
                out.insert(jl_unbox_uint64(jl_arrayref(keys, i)));

        gc_unpause;
    });

    Benchmark::run("jluna: unbox set", n_reps, [](){

        auto* value = jl_eval_string("Set{UInt64}([rand(UInt64) for _ in 1:100])");
        auto out = unbox<std::set<size_t>>(value);
    });

    /// ### (un)box: pair

    Benchmark::run_as_base("C-API: unbox pair", n_reps, [](){

        gc_pause;
        auto* value = jl_eval_string("return Pair(rand(), rand())");

        volatile auto out = std::pair<float, float>();
        out.first = jl_unbox_float32(jl_get_nth_field(value, 0));
        out.second = jl_unbox_float32(jl_get_nth_field(value, 1));
        gc_unpause;
    });

    Benchmark::run("jluna: unbox pair", n_reps, [](){

        auto* value = jl_eval_string("return Pair(rand(), rand())");
        volatile auto out = unbox<std::pair<float, float>>(value);
    });

    Benchmark::run_as_base("C-API: box pair", n_reps, [](){

        gc_pause;
        auto value = std::make_pair(generate_number<float>(), generate_number<float>());
        static jl_function_t* pair = jl_get_function(jl_base_module, "Pair");

        volatile auto* out = jl_call2(pair, jl_box_float32(value.first), jl_box_float32(value.second));
        gc_unpause;
    });

    Benchmark::run("jluna: box pair", n_reps, [](){

        auto value = std::make_pair(generate_number<float>(), generate_number<float>());
        volatile auto out = box<std::pair<float, float>>(value);
    });

    /// ### (un)box: tuple

    Benchmark::run_as_base("C-API: unbox tuple", n_reps, [](){

        gc_pause;
        auto* value = jl_eval_string("return (rand(), rand(), rand(), rand())");

        auto out = std::tuple<float, float, float, float>();
        std::get<0>(out) = jl_unbox_float32(jl_get_nth_field(value, 0));
        std::get<1>(out) = jl_unbox_float32(jl_get_nth_field(value, 1));
        std::get<2>(out) = jl_unbox_float32(jl_get_nth_field(value, 2));
        std::get<3>(out) = jl_unbox_float32(jl_get_nth_field(value, 3));

        gc_unpause;
    });

    Benchmark::run("jluna: unbox tuple", n_reps, [](){

        auto* value = jl_eval_string("return (rand(), rand(), rand(), rand())");
        volatile auto out = unbox<std::tuple<float, float, float, float>>(value);
    });

    Benchmark::run_as_base("C-API: box tuple", n_reps, [](){

        gc_pause;
        auto value = std::make_tuple
        (
            generate_number<float>(),
            generate_number<float>(),
            generate_number<float>()
        );

        static jl_function_t* tuple = jl_get_function(jl_base_module, "Tuple");
        volatile auto* out = jl_call3(
            tuple,
            jl_box_float32(std::get<0>(value)),
            jl_box_float32(std::get<0>(value)),
            jl_box_float32(std::get<0>(value))
        );
        gc_unpause;
    });

    Benchmark::run("jluna: box tuple", n_reps, [](){

        auto value = std::make_tuple
        (
            generate_number<float>(),
            generate_number<float>(),
            generate_number<float>()
        );

        volatile auto* out = box<typeof(value)>(value);
    });

    /// ### hash (used for cppcall)

    Benchmark::run_as_base("C-API: hash", n_reps, [](){

        gc_pause;
        auto name = generate_string(16);
        auto* sym = jl_symbol(name.c_str());

        volatile size_t res = sym->hash;
        gc_unpause;
    });

    Benchmark::run("jluna: hash", n_reps, [](){

        gc_pause;
        auto name = generate_string(16);
        volatile size_t res = c_adapter::hash(name);
        gc_unpause;
    });

    /// ### lambda call overhead

    Benchmark::run_as_base("C-API: call lambda without box", n_reps, [](){

        static auto lambda = []() {
            int out = 0;

            for (size_t i = 0; i < 1000; ++i)
                if (i % 2 == 0)
                    out += i;
                else
                    out -= i;

            return out;
        };

        auto res = Main.new_undef(generate_string(1));
        lambda();
    });

    Benchmark::run("jluna: call lambda with box", n_reps, [](){

        static auto lambda = []() {
            int out = 0;

            for (size_t i = 0; i < 1000; ++i)
                if (i % 2 == 0)
                    out += i;
                else
                    out -= i;

            return out;
        };

        auto res = Main.new_undef(generate_string(1));
        res = lambda;
        res();
    });

    Benchmark::conclude();
    Benchmark::save();
}
