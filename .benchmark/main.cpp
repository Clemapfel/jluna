//
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#include <jluna.hpp>
#include <.benchmark/benchmark.hpp>
#include <.benchmark/benchmark_aux.hpp>
#include <thread>
#include <future>
#include <queue>

using namespace jluna;

int main()
{
    initialize(1);

    // ### ACCESSING JULIA-SIDE VALUES ###

    // number of cycles
    size_t n_reps = 1000000;

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
    Benchmark::run_as_base("C-API: get_function", n_reps, [](){
        volatile auto* f = jl_get_function(jl_main_module, "f");
    });

    Benchmark::run_as_base("C-API: get_global", n_reps, [](){
        volatile auto* f = jl_get_global(jl_main_module, "f"_sym);
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

    // ### MUTATING JULIA VALUES ###

    n_reps = 10000000;

    Main.safe_eval("x = 1234");

    // C-API
    Benchmark::run_as_base("C-API: set", n_reps, [](){

        auto to_box = generate_number<Int64>();
        jl_set_global(jl_main_module, "x"_sym, jl_box_int64(to_box));
    });

    // unsafe
    Benchmark::run("unsafe: set", n_reps, [](){

        auto to_box = generate_number<Int64>();
        unsafe::set_value(jl_main_module, "x"_sym, unsafe::unsafe_box<Int64>(to_box));
    });

    // Module::assign
    Benchmark::run("Module::assign", n_reps / 10, [](){

        auto to_box = generate_number<Int64>();
        Main.assign("x", to_box);
    });

    // Proxy::operator=
    auto x_proxy = Main["x"];
    Benchmark::run("Proxy::operator=", n_reps / 10, [&](){

        auto to_box = generate_number<Int64>();
        x_proxy = to_box;
    });

    //Benchmark::conclude();
    //Benchmark::save();
    //return 0;

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

    //Benchmark::conclude();
    //Benchmark::save();
    //return 0;

    // ### JLUNA ARRAY ###
    n_reps = 1000000;

    // pre-run to avoid artifacting
    Benchmark::run_as_base("Allocate Array: C-API", 1000, [&](){

        auto* arr = (jl_array_t*) jl_alloc_array_1d(jl_apply_array_type((jl_value_t*) jl_int64_type, 1), 0);
        jl_array_sizehint(arr, 100);

        for (size_t i = 1; i <= 100; ++i)
        {
            jl_array_grow_end(arr, 1);
            jl_arrayset(arr, jl_box_int64(generate_number<Int64>()), i);
        }

        jl_gc_collect(JL_GC_AUTO);
    });

    Benchmark::run_as_base("Allocate Array: C-API", n_reps, [&](){

        auto* arr = (jl_array_t*) jl_alloc_array_1d(jl_apply_array_type((jl_value_t*) jl_int64_type, 1), 0);
        jl_array_sizehint(arr, 100);

        for (size_t i = 1; i <= 100; ++i)
        {
            jl_array_grow_end(arr, 1);
            jl_arrayset(arr, jl_box_int64(generate_number<Int64>()), i);
        }

        jl_gc_collect(JL_GC_AUTO);
    });

    Benchmark::run("Allocate Array: unsafe", n_reps, [](){

        auto* arr = unsafe::new_array(Int64_t, 0);
        unsafe::sizehint(arr, 100);

        for (size_t i = 1; i <= 100; ++i)
            unsafe::push_back(arr, unsafe::unsafe_box<Int64>(generate_number<Int64>()));

        jl_gc_collect(JL_GC_AUTO);
    });

    Benchmark::run("Allocate Array: jluna::Array", n_reps, [&](){

        auto arr = jluna::Vector<Int64>();
        arr.reserve(100);

        for (size_t i = 1; i <= 100; ++i)
            arr.push_back(generate_number<Int64>());

        jl_gc_collect(JL_GC_AUTO);
    });

    //Benchmark::conclude();
    //Benchmark::save();
    //return 0;

    // ### JLUNA TASK ###

    // setup 1-thread threapool
    static std::mutex queue_mutex;
    static auto queue_lock = std::unique_lock(queue_mutex, std::defer_lock);
    static std::condition_variable queue_cv;

    static auto queue = std::queue<std::packaged_task<void()>>();
    static auto shutdown = false;

    static std::mutex master_mutex;
    static auto master_lock = std::unique_lock(queue_mutex, std::defer_lock);
    static std::condition_variable master_cv;

    // worker thread
    static auto thread = std::thread([](){

        while (true)
        {
            queue_cv.wait(queue_lock, []() -> bool {
                return not queue.empty() or shutdown;
            });

            if (shutdown)
                return;

            auto task = std::move(queue.front());
            queue.pop();
            task();

            master_cv.notify_all();
        }
    });

    // task
    std::function<void()> task = []() {
        size_t sum = 0;
        for (volatile auto i = 0; i < 100; ++i)
            sum += generate_number<Int64>();

        return sum;
    };

    n_reps = 10000000;

    // run task using jluna::Task
    Benchmark::run_as_base("threading: jluna::Task", n_reps, [&]()
    {
        auto t = ThreadPool::create<void()>(task);
        t.schedule();
        t.join();
    });

    // run task using std::thread
    Benchmark::run("threading: std::thread", n_reps, [&]()
    {
        queue.emplace(task);
        queue_cv.notify_all();
        master_cv.wait(master_lock, [](){
            return queue.empty();
        });
    });

    shutdown = true;
    thread.detach();
    queue_cv.notify_all();

    Benchmark::conclude();
    return 0;
}
