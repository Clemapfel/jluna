// 
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#include <.benchmark/benchmark.hpp>
#include <chrono>
#include <iostream>
#include <jluna.hpp>

using namespace jluna::detail;

int main()
{
    jluna::State::initialize();

    Benchmark::initialize();

    Benchmark::run("test", 1000, []{

        throw std::invalid_argument("test");
        for (size_t i = 0; i < 10000; ++i)
           i = i;
    });

    Benchmark::conclude();

}

