// 
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#include <.benchmark/benchmark.hpp>
#include <chrono>
#include <iostream>
#include <jluna.hpp>
#include <random>

using namespace jluna;

std::string generate_string(size_t size)
{
    static auto engine = std::mt19937(1234);
    static auto dist = std::uniform_int_distribution<char>(65, 90);

    std::string out;
    for (size_t i = 0; i < size; ++i)
        out.push_back(dist(engine));

    return out;
}

template<typename Number_t> requires IsNumerical<Number_t>
Number_t generate_number(
    Number_t lower = Number_t(0),
    Number_t upper = std::numeric_limits<Number_t>::max())
{
    static auto engine = std::mt19937(1234);
    static auto dist = std::uniform_int_distribution<Number_t>(lower, upper);

    return dist(engine);
}

int main()
{
    jluna::State::initialize();

    Benchmark::initialize();

    size_t count = 100000;


    jl_eval_string("sym_dict = Dict{UInt64, Base.RefValue(Symbol)}()");
    jl_eval_string("any_dict = Dict{UInt64, Base.RefValue(Any)}()");

    auto* sym_dict = jl_eval_string("return sym_dict");
    auto* any_dict = jl_eval_string("return any_dict");

    static jl_function_t* setindex = jl_get_function(jl_base_module, "setindex!");
    static jl_function_t* delete_ = jl_get_function(jl_base_module, "delete!");
    size_t n = 100000;
    for (size_t i = 0; i < 100000; ++i)
    {
        if (i < n/2)
        {
            jl_call3(setindex, sym_dict, (Any*) jl_symbol(generate_string(8).c_str()), jl_box_uint64(i));
            jl_call3(setindex, any_dict, (Any*) jl_symbol(generate_string(8).c_str()), jl_box_uint64(i));
        }
        else
        {
            jl_call3(setindex, sym_dict, (Any*) jl_symbol(std::to_string(-1 * i).c_str()), jl_box_uint64(i));
            jl_call3(setindex, any_dict, jl_box_int64(-1 * i), jl_box_uint64(i));
        }
    }

    Benchmark::run("setindex Dict{Any}", count, [&](){

        jl_value_t* sym = (Any*) jl_symbol(generate_string(8).c_str());
        jl_value_t* num = jl_box_uint64(generate_number<size_t>());

        static size_t i = 0;

        jl_call3(setindex, any_dict, num, jl_box_uint64(i));

        i += 1;
    });

    Benchmark::run("setindex Dict{Symbol}", count, [&](){

        jl_value_t* sym = (Any*) jl_symbol(generate_string(8).c_str());
        jl_value_t* num = jl_box_uint64(generate_number<size_t>());

        static size_t i = 0;

        jl_call3(setindex, sym_dict, sym, jl_box_uint64(i));

        i += 1;
    });

    return 0;

     Benchmark::run("setindex Dict{Any}", count, [&](){

        jl_value_t* sym = (Any*) jl_symbol(generate_string(8).c_str());
        jl_value_t* num = jl_box_uint64(generate_number<size_t>());

        static size_t i = 0;

        jl_call3(setindex, any_dict, num, jl_box_uint64(i));

        i += 1;
    });

    Benchmark::run("getindex Dict{Any}", count, [&](){

        static jl_function_t* getindex = jl_get_function(jl_base_module, "getindex");
        static size_t i = 0;
        volatile auto* res = jl_call2(getindex, any_dict, jl_box_uint64(i));
    });

    Benchmark::run("getindex Dict{Symbol}", count, [&](){

        static jl_function_t* getindex = jl_get_function(jl_base_module, "getindex");
        static size_t i = 0;
        volatile auto* res = jl_call2(getindex, sym_dict, jl_box_uint64(i));
    });

    jl_eval_string("println(length(sym_dict))");
    jl_eval_string("println(length(any_dict))");

    Benchmark::conclude();
    return 0;

    Benchmark::run("Eval: C", count, []{

        std::stringstream str;
        str << "struct " << generate_string(16) << " end";
        jl_eval_string(str.str().c_str());
    });

    Benchmark::run("Eval: jluna safe_eval no proxy", count, [](){

        std::stringstream str;
        str << "struct " << generate_string(16) << " end";

        std::stringstream wrap;
        wrap << "jluna.exception_handler.safe_call(quote " << str.str() << " end)" << std::endl;
        auto* res = jl_eval_string(str.str().c_str());

        volatile auto proxy = Proxy(res);
    });

    std::vector<size_t> refs;
    refs.reserve(count);

    Benchmark::run("State: create_reference", count, [&](){

        Any* value = box<std::string>(generate_string(16));
        refs.push_back(State::detail::create_reference(value));
        // target: 0.366586ms
    });

    Benchmark::run("State: get_reference", count, [&](){

        static Function* get_reference = jl_find_function("jluna.memory_handler", "get_reference");
        static size_t i = 0;
        volatile auto* res = jluna::safe_call(get_reference, jl_box_uint64(reinterpret_cast<size_t>(refs.at(i++))));

        // target:  0.365002ms
    });


    /*
    {
        auto unnamed_proxy = State::new_unnamed_string(generate_string(16));
        Benchmark::run("Proxy: assign unnamed", count, [&]() {
            // target: 0.368181ms
            unnamed_proxy = generate_string(16);
        });

        auto named_proxy = State::new_named_string("string_proxy", generate_string(16));
        Benchmark::run("Proxy: assign named", count, [&]() {
            // target: 1.37952ms
            named_proxy = generate_string(16);
        });
    }
     */

    Benchmark::conclude();
    Benchmark::save();
}

