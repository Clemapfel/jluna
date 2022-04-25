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

int main()
{
    initialize(1);
    Benchmark::initialize();

    // ### JLUNA ARRAY ###
    size_t n_reps = 10000000;

    auto* allocate_array_f = (unsafe::Function*) Main.safe_eval(R"(
        function allocate_array()
            return [i for i in 1:5000]
        end

        return allocate_array;
    )");

    Benchmark::run_as_base("Allocate Array: Julia", n_reps, [&](){
        jl_call0(allocate_array_f);
    });

    Benchmark::run("Allocate Array: C-API", n_reps, [&](){

        auto* arr = (jl_array_t*) jl_alloc_array_1d((jl_value_t*) jl_int64_type, 5000);
        detail::gc_push(arr);

        for (size_t i = 1; i <= 5000; ++i)
            jl_arrayset(arr, jl_box_int64(i), i-1);

        detail::gc_pop(1);
    });

    Benchmark::run("Allocate Array: unsafe", n_reps, [&](){

        auto* arr = unsafe::new_array(Int64_t, 5000);
        detail::gc_push(arr);

        for (size_t i = 1; i <= 5000; ++i)
            unsafe::set_index(arr, unsafe::unsafe_box<Int64>(i), i);

        detail::gc_pop(1);
    });

    Benchmark::run("Allocate Array: jluna::Array", n_reps, [&](){

        auto arr = jluna::Vector<Int64>();

        for (size_t i = 1; i <= 5000; ++i)
            arr.push_back(i);
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

    //Benchmark::conclude();
    //Benchmark::save();
    //return 0;

    // ### MUTATING JULIA VALUES ###

    n_reps = 10000000;

    Main.safe_eval("x = 1234");

    // C-API
    Benchmark::run_as_base("C-API: set", n_reps, [](){

        Int64 to_box = generate_number<Int64>();
        jl_set_global(jl_main_module, "x"_sym, jl_box_int64(to_box));
    });

    // unsafe
    Benchmark::run("unsafe: set", n_reps, [](){

        Int64 to_box = generate_number<Int64>();
        unsafe::set_value(jl_main_module, "x"_sym, unsafe::unsafe_box<Int64>(to_box));
    });

    // Module::assign
    Benchmark::run("Module::assign", n_reps / 10, [](){

        Int64 to_box = generate_number<Int64>();
        Main.assign("x", to_box);
    });

    // Proxy::operator=
    auto x_proxy = Main["x"];
    Benchmark::run("Proxy::operator=", n_reps / 10, [&](){

        Int64 to_box = generate_number<Int64>();
        x_proxy = to_box;
    });

    Benchmark::conclude();
    Benchmark::save();
    return 0;

    // ### CALLING JULIA FUNCTIONS ###

    n_reps = 10000000;

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

    //Benchmark::conclude();
    //Benchmark::save();
    //return 0;

    // ### Calling C++-side Functions

    n_reps = 1000000;

    // actual function
    static auto task_f = [](){
        for (volatile size_t i = 0; i < 10000; i = i+1);
    };

    // base
    Benchmark::run_as_base("Call C++-Function in C++", n_reps, [](){
        task_f();
    });

    // move to julia, then call
    Main.create_or_assign("task_f", as_julia_function<void()>(task_f));
    auto* jl_task_f = unsafe::get_function(jl_main_module, "task_f"_sym);

    Benchmark::run("Call C++-Function in Julia", n_reps, [&](){
        jl_call0(jl_task_f);
    });

    Benchmark::conclude();
    Benchmark::save();
    return 0;
}
