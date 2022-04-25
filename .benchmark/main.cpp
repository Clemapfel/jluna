//
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#include <jluna.hpp>
#include <.benchmark/benchmark.hpp>
#include <.benchmark/benchmark_aux.hpp>
#include <thread>
#include <.src/gc_sentinel.hpp>

using namespace jluna;

constexpr size_t n_reps = 10000000;
std::vector<Proxy> _proxies;

void benchmark_threading()
{
    static auto task_f = [](){
        for (volatile size_t i = 0; i < 10000; i = i+1);
    };

    Benchmark::run_as_base("threading: no task", n_reps, [](){

        task_f();
    });

    Benchmark::run("threading: std::thread", n_reps, [](){

        auto t = std::thread(task_f);
        t.join();
    });

    Benchmark::run("threading: jluna::Task", n_reps, [](){

        auto t = ThreadPool::create<void()>(task_f);
        t.schedule();
        t.join();
    });
}

void benchmark_cppcall()
{
        static auto task_f = [](){
        for (volatile size_t i = 0; i < 10000; i = i+1);
    };

    Benchmark::run_as_base("base", n_reps, []() {
        task_f();
    });

    Main.create_or_assign("task_f", as_julia_function<void()>(task_f));
    static auto* run_julia_side = unsafe::get_function(jl_main_module, "task_f"_sym);

    Benchmark::run("jluna", n_reps, []() {
       volatile auto* _ = unsafe::call(run_julia_side);
    });
}


int main()
{
    initialize(4);
    Benchmark::initialize();

    // ### CALLING JULIA FUNCTIONS ###

    size_t n_reps = 10000000;

    // function to test
    Main.safe_eval(R"(
        function f()
            sum = 0;
            for i in 1:100
                sum += rand();
            end
        end
    )");

    auto* f_ptr = unsafe::get_function(jl_main_module, "f"_sym);

    // C-API
    Benchmark::run_as_base("C-API: jl_call", n_reps, [&](){

        jl_call0(f_ptr);
    });

    // unsafe
    Benchmark::run("unsafe: call", n_reps, [&](){

        unsafe::call(f_ptr);
    });

    // jluna::safe_call
    Benchmark::run("jluna::safe_call", n_reps, [&](){

        jluna::safe_call(f_ptr);
    });

    // proxy::safe_call<T>
    auto f_proxy = Proxy(f_ptr);
    Benchmark::run("Proxy::safe_call<T>", n_reps, [&](){

        f_proxy.safe_call<void>();
    });

    // proxy::operator()
    Benchmark::run("Proxy::operator()", n_reps, [&](){

        f_proxy();
    });

    Benchmark::conclude();
    Benchmark::save();
    return 0;

    // ### ACCESSING JULIA-SIDE VALUES ###

    // number of cycles
    n_reps = 1000000;

    // allocate function object Julia-side
    Main.safe_eval(R"(
        function f()
            sum = 0;
            for i in 1:100
                sum += rand();
            end
        end
    )");

    // C-API
    Benchmark::run_as_base("C-API: get", n_reps, [](){
        volatile auto* f = jl_get_function(jl_main_module, "f");
    });

    // unsafe::get_function
    Benchmark::run("unsafe: get_function", n_reps, [](){
        volatile auto* f = unsafe::get_function(jl_main_module, "f"_sym);
    });

    // unsafe::get_value
    Benchmark::run("unsafe: get_value", n_reps, [](){
        volatile auto* f = unsafe::get_value(jl_main_module, "f"_sym);
    });

    // named Proxy from unsafe
    Benchmark::run("named proxy: from unsafe", n_reps, [](){
        volatile auto* f = (unsafe::Function*) Proxy(unsafe::get_value(jl_main_module, "f"_sym), "f"_sym);
    });

    // Proxy.operator[](std::string)
    Benchmark::run("named proxy: operator[]", n_reps, [](){
        volatile auto* f = (unsafe::Function*) Main["f"];
    });

    // Main.get
    Benchmark::run("module: get", n_reps, [](){
        volatile auto* f = Main.get<unsafe::Function*>("f");
    });

    // C-API: eval
    Benchmark::run("eval: get", n_reps / 4, [](){
        volatile auto* f = (unsafe::Function*) jl_eval_string("return f");
    });

    Benchmark::conclude();
    Benchmark::save();
    return 0;
}
