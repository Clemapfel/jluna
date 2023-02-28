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
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cassert>

namespace jluna
{
    /// @brief benchmark API, not intended for end-users
    struct Benchmark
    {
        using Duration = std::chrono::duration<long int, std::nano>;

        struct Result
        {
            Result()
                : _min(0),
                  _max(0),
                  _average(0),
                  _median(0),
                  _n_loops(-1),
                  _exception_maybe()
            {}

            Result(std::string name, Duration min, Duration max, Duration average, Duration median, size_t n_loops, std::string exception_maybe)
            : _name(name),
              _min(min),
              _max(max),
              _average(average),
              _median(median),
              _n_loops(n_loops),
              _exception_maybe(exception_maybe)
            {}
            
            std::string _name;

            Duration _min;
            Duration _max;
            Duration _average;
            Duration _median;

            float _overhead = 0;
            std::string _compared_to = "self";

            size_t _n_loops;
            std::string _exception_maybe;
        };

        static void initialize()
        {
            std::cout << "[C++][LOG] starting benchmarks...\n" << std::endl;
            _results.clear();
        }

        template<typename Lambda_t>
        static Benchmark::Result run_as_base(const std::string& name, size_t count, Lambda_t lambda, bool log = true)
        {
            _base = Benchmark::Result();
            auto res = run(name, count, lambda, log);
            _base = res;
            return res;
        }

        template<typename Lambda_t>
        static Benchmark::Result run(const std::string& name, size_t count, Lambda_t lambda, bool log = true)
        {
            if (count % 2 == 0)
                count += 1;

            collect_garbage();

            std::cout << "[C++][LOG] Running \"" << name << "\"" << std::endl;
            std::vector<Duration> runs;
            runs.reserve(count);

            // pre-initialize to avoid allocation
            auto before = Benchmark::_clock.now();
            auto after = Benchmark::_clock.now();
            size_t n = 0;

            std::string exception_maybe = "";

            try
            {
                lambda(); // for potential static allocation
                for (; n < count; ++n)
                {
                    before = Benchmark::_clock.now();
                    lambda();
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

            auto res = Result{
                    name,
                    runs.front(),
                    runs.back(),
                    avg,
                    (runs.size() > 1 ? runs.at(runs.size() / 2) : Duration::zero() + Duration(-1)),
                    n,
                    exception_maybe
            };

            static auto overhead = [](std::chrono::duration<double> a, std::chrono::duration<double> b) -> double {

                if (a.count() == 0)
                    return 0;

                return (a < b ? 1 : -1) * (abs(a - b) / a);
            };
            res._overhead = overhead(_base._median, res._median);
            if (res._overhead != 0)
                res._compared_to = _base._name;

            if (log)
                _results.push_back(res);

            std::cout << "[C++][LOG] done." << std::endl;
            return _results.back();
        }

        static void add_results(const std::string name, Benchmark::Result result)
        {
            _results.push_back(result);
        }

        static void conclude()
        {
            for (auto& res : _results)
            {
                auto min = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(res._min).count();
                auto avg = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(res._average).count();
                auto med = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(res._median).count();
                auto max = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(res._max).count();

                std::cout << "┌────────────────────────────────\n";
                std::cout << "│ " << res._name << " (" << res._n_loops << "): \n│\n";
                std::cout << "│ Min    : " << min << "ms" << std::endl;
                std::cout << "│ Average: " << avg << "ms" << std::endl;
                std::cout << "│ Max    : " << max << "ms" << std::endl;
                std::cout << "│ Median : " << med << "ms" << std::endl;
                std::cout << "│ " << std::endl;
                std::cout << "│ Overhead: " << (res._overhead * 100.f) << "%" << std::endl;

                if (res._exception_maybe != "")
                {
                    std::cout << "│ " << std::endl;
                    std::cout << "│ Exception at run " << res._n_loops << ":" << std::endl;
                    std::cout << "│ " << res._exception_maybe << std::endl;
                }

                std::cout << "└────────────────────────────────\n" << std::endl;
            }
        }

        static void save(const std::string& path = "/home/clem/Workspace/jluna/.benchmark/results/")
        {
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            std::stringstream str;
            str << std::put_time(&tm, "%d-%m-%Y_%H:%M:%S") << std::endl;


            std::ofstream file;
            auto name = path + str.str() + ".csv";
            file.open(name);

            const std::string del = ",";
            file << "name" << del << "count" << del << "min" << del << "max" << del << "average" << del << "median" << del << "overhead" << del << "compared_to" << std::endl;

            for (auto& res : _results)
            {
                auto min = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(res._min).count();
                auto avg = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(res._average).count();
                auto med = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(res._median).count();
                auto max = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(res._max).count();

                file << "\"" << res._name << "\"" << del;
                file << res._n_loops << del;
                file << min << del;
                file << max << del;
                file << avg << del;
                file << med << del;
                file << res._overhead << del;
                file << "\"" << res._compared_to << "\"" << std::endl;
            }

            file << std::endl;
            file.close();

            std::cout << "[C++][LOG] Benchmark written to " << name << std::endl;
        }

        private:
            static inline Benchmark::Result _base = Benchmark::Result();
            static inline std::chrono::steady_clock _clock = std::chrono::steady_clock();
            static inline std::vector<Result> _results = {};
    };

}