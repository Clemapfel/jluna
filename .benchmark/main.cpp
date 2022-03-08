//
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#include <.benchmark/benchmark.hpp>
#include <chrono>
#include <iostream>
#include <jluna.hpp>
#include <random>
#include <include/generator_expression.hpp>

using namespace jluna;

int main()
{
    Benchmark::initialize();



    Benchmark::conclude();
    Benchmark::save();
}

