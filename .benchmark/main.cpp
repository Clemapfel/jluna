//
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#include <jluna.hpp>
#include <.benchmark/benchmark.hpp>
#include <.benchmark/benchmark_aux.hpp>

using namespace jluna;

std::vector<Proxy> _proxies;
void setup();

size_t n_reps = 1000;

int main()
{
    State::initialize();
    Benchmark::initialize();

    Benchmark::run("C: eval", n_reps, [](){

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
        State::eval(name + std::string(" = [i for i in 1:1000]"));
        State::eval(name + std::string(" = undef"));
    });

    Benchmark::run("State::safe_eval", n_reps, [](){

        auto name = generate_string(8);
        State::safe_eval(name + std::string(" = [i for i in 1:1000]"));
        State::safe_eval(name + std::string(" = undef"));
    });

    Benchmark::run("Module::eval", n_reps, [](){

        auto name = generate_string(8);
        Main.eval(name + std::string(" = [i for i in 1:1000]"));
        Main.eval(name + std::string(" = undef"));
    });

    Benchmark::run("Module::safe_eval", n_reps, [](){

        auto name = generate_string(8);
        Main.safe_eval(name + std::string(" = [i for i in 1:1000]"));
        Main.safe_eval(name + std::string(" = undef"));
    });

    Benchmark::conclude();
    Benchmark::save();
}

void setup()
{
    for (size_t i = 0; i < 3000; ++i)
        _proxies.push_back(State::new_named_uint64(generate_string(16), generate_number(0, 10000)));
}

