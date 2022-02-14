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

int main()
{
    jluna::State::initialize();

    Benchmark::initialize();

    size_t count = 1000;

    /*
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

        static Function* create_reference = jl_find_function("jluna.memory_handler", "create_reference");

        Any* value = box<std::string>(generate_string(16));
        refs.push_back(jl_unbox_uint64(jluna::safe_call(create_reference, value)));

        // target: 0.366586ms
    });

    Benchmark::run("State: get_reference", count, [&](){

        static Function* get_reference = jl_find_function("jluna.memory_handler", "get_reference");
        static size_t i = 0;

        Any* value = box<std::string>(generate_string(16));
        volatile auto* res = jluna::safe_call(get_reference, jl_box_uint64(reinterpret_cast<size_t>(refs.at(i++))));

        // target:  0.365002ms
    });
     */

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

    Benchmark::conclude();
    Benchmark::save();
}

