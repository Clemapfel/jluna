// 
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <chrono>
#include <map>
#include <string>
#include <iostream>

namespace jluna::detail
{
    /// @brief benchmark API, not intended for end-users
    struct Benchmark
    {
        using Duration = std::chrono::duration<long int, std::nano>;

        struct Result
        {
            const Duration _min;
            const Duration _max;
            const Duration _avg;

            const size_t _n_loops;
        };

        static void initialize()
        {
            std::cout << "starting benchmarks...\n" << std::endl;
            _results.clear();
        }

        template<typename Lambda_t, typename... Args_t>
        static void run(const std::string& name, size_t count, Lambda_t lambda, Args_t... args)
        {
            auto min = Duration::max();
            auto max = Duration::min();
            auto avg = Duration::zero();
            
            size_t n = 0;

            // pre-initialize
            auto before = Benchmark::_clock.now();
            auto after = Benchmark::_clock.now();

            try
            {
                for (; n < count; ++n)
                {
                    before = Benchmark::_clock.now();
                    lambda(args...);
                    after = Benchmark::_clock.now();

                    auto duration = after - before;
                    if (duration < min)
                        min = duration;

                    if (duration > max)
                        max = duration;

                    avg += duration;
                }

                _results.insert({name, Result{min, max, avg / n, n}});
            }
            catch (std::exception& e)
            {
                std::cerr << "Exception in " << name << " at run " << n << std::endl;
                throw e;
            }
        }

        static void conclude()
        {
            for (auto& pair : _results)
            {
                float min = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(pair.second._min).count();
                float avg = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(pair.second._avg).count();
                float max = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(pair.second._max).count();

                std::cout << "__________________________________\n";
                std::cout << "| " << pair.first << ": \n|\n";
                std::cout << "| Min: " << min << "ms" << std::endl;
                std::cout << "| Avg: " << avg << "ms" << std::endl;
                std::cout << "| Max: " << max << "ms" << std::endl;
                std::cout << "|_________________________________\n\n" << std::endl;
            }
        }

        private:
            static inline std::chrono::steady_clock _clock = std::chrono::steady_clock();
            static inline std::map<std::string, Result> _results = {};
    };

}