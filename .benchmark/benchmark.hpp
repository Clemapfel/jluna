// 
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <chrono>
#include <map>
#include <string>
#include <iostream>
#include <vector>

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
            const Duration _average;
            const Duration _median;

            const size_t _n_loops;
            const std::string _exception_maybe;
        };

        static void initialize()
        {
            std::cout << "starting benchmarks...\n" << std::endl;
            _results.clear();
        }

        template<typename Lambda_t, typename... Args_t>
        static void run(const std::string& name, size_t count, Lambda_t lambda, Args_t... args)
        {
            if (count % 2 == 0)
                count += 1;

            std::vector<Duration> runs;
            runs.reserve(count);

            // pre-initialize to avoid allocation
            auto before = Benchmark::_clock.now();
            auto after = Benchmark::_clock.now();
            size_t n = 0;

            std::string exception_maybe = "";

            try
            {
                for (; n < count; ++n)
                {
                    before = Benchmark::_clock.now();
                    lambda(args...);
                    after = Benchmark::_clock.now();

                    runs.push_back(after - before);
                }
            }
            catch (std::exception& e)
            {
                exception_maybe = e.what();
            }

            std::sort(runs.begin(), runs.end());
            auto avg = Duration::zero();
            for (auto& r : runs)
                avg += r;

            avg = avg / (runs.size() != 0 ? runs.size() : 1);

            _results.insert({name,
                Result{
                    runs.front(),
                    runs.back(),
                    avg,
                    (runs.size() > 1 ? runs.at(runs.size() / 2) : Duration::zero() + Duration(-1)),
                    n,
                    exception_maybe
            }});
        }

        static void conclude()
        {
            for (auto& pair : _results)
            {
                auto min = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(pair.second._min).count();
                auto avg = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(pair.second._average).count();
                auto med = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(pair.second._median).count();
                auto max = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(pair.second._max).count();

                std::cout << "┌────────────────────────────────\n";
                std::cout << "│ " << pair.first << " (" << pair.second._n_loops << "): \n│\n";
                std::cout << "│ Min    : " << min << "ms" << std::endl;
                std::cout << "│ Average: " << avg << "ms" << std::endl;
                std::cout << "│ Median : " << med << "ms" << std::endl;
                std::cout << "│ Max    : " << max << "ms" << std::endl;

                if (pair.second._exception_maybe != "")
                {
                    std::cout << "│ " << std::endl;
                    std::cout << "│ Exception at run " << pair.second._n_loops << ":" << std::endl;
                    std::cout << "│ " << pair.second._exception_maybe << std::endl;
                }

                std::cout << "└────────────────────────────────\n\n" << std::endl;
            }
        }

        private:
            static inline std::chrono::steady_clock _clock = std::chrono::steady_clock();
            static inline std::map<std::string, Result> _results = {};
    };

}