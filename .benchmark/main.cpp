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
    out.reserve(size);

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

    size_t count = 10000;

    Benchmark::run("forward_last_exception positive:", count, []()
    {
        jl_eval_string("try throw(AssertionError(\"abc\")) catch e throw(e) end");

        try
        {
            forward_last_exception();
        }
        catch (...)
        {}
    });

    Benchmark::run("forward_last_exception negative:", count, []()
    {
        jl_eval_string("try throw(AssertionError(\"abc\")) catch e end");

        try
        {
            forward_last_exception();
        }
        catch (...)
        {}
    });

    Benchmark::conclude();
    return 0;

    Benchmark::run("box iddict", count, [](){

        std::map<Int64, size_t> map;
        for (size_t i = 0; i < 1000; ++i)
            map.insert({i, i});

        volatile auto* res = box(map);
    });

    Benchmark::run("box vector", count, [](){

        std::vector<size_t> in;
        in.reserve(1000);
        for (size_t i = 0; i < 1000; ++i)
            in.push_back(i);

        volatile auto* res = box(in);
    });


    /*

    std::vector<size_t> refs;
    refs.reserve(count);

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

     */

    Benchmark::conclude();
    Benchmark::save();
}

