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

std::string generate_string(size_t size)
{
    static auto engine = std::mt19937(1234);
    static auto dist = std::uniform_int_distribution<char>(65, 90);

    std::string out;
    out.reserve(size);

    for (size_t i = 0; i < size; ++i)
        out.push_back(dist(engine));

    return out;
}

template<typename Number_t> requires IsNumerical<Number_t>
Number_t generate_number(
    Number_t lower = Number_t(0),
    Number_t upper = std::numeric_limits<Number_t>::max())
{
    static auto engine = std::mt19937(1234);
    static auto dist = std::uniform_int_distribution<Number_t>(lower, upper);

    return dist(engine);
}

size_t count = 1000;

void benchmark_lambda_call()
{
     auto a = Benchmark::run("Lambda: Create & Run Julia-Side", count, [&](){

        auto* res = register_unnamed_function([]() -> void {
            for (size_t i = 0; i < 100000; ++i)
                volatile size_t j = i;
        });

        jl_call0(res);
    });

    auto b = Benchmark::run("Lambda: Create & Run C++-Side", count, [&](){

        auto lambda = []() -> void {
            for (size_t i = 0; i < 100000; ++i)
                volatile size_t j = i;
        };
        lambda();
    });

    Benchmark::add_results("Lambda: Absolute Overhead",  a - b);
}

int main()
{
    using namespace jluna;
    State::initialize();

    auto test = Main["abc"];

    jl_eval_string("module M1; module M2; module M3; end end end");

    Benchmark::run("Old Proxy Eval", count, [](){

        auto name = generate_string(8);
        jl_eval_string(("M1.M2.M3.eval(:(" + name + " = \"" + generate_string(16) + "\"))").c_str());

        //auto value = Main["M1"]["M2"]["M3"][name].operator std::string();
    });

    Benchmark::run("Old Proxy Assign", count, [](){

        auto name = generate_string(8);
        jl_eval_string(("M1.M2.M3.eval(:(" + name + " = undef))").c_str());

        //Main["M1"]["M2"]["M3"].as<Module>().assign(name, generate_string(16));
    });

    Benchmark::conclude();
    Benchmark::save();
    return 0;

    /*
    Benchmark::run("Generator Native", count, [](){

        jl_eval_string(R"(
            begin
                local n = 0;
                for i in (x for x in 1:10)
                    n += i
                end
            end
        )");
    });
     */

    //jl_gc_pause;
    Benchmark::run("Generator C++-Side", count, [](){

        // 3.53912ms
        volatile size_t sum = 0;
        for (auto i : " (x for x in 1:1000)"_gen)
            sum = sum + unbox<size_t>(i);
    });
    //jl_gc_unpause;

    Benchmark::conclude();

    return 0;

    Benchmark::initialize();

    jl_eval_string("test = 1234");

    Benchmark::run("Proxy: CTOR DTOR Unnamed", count, [](){

        volatile auto* p = new Proxy(box(generate_string(1)));
        delete p;
    });

    Benchmark::run("Proxy: CTOR DTOR Named", count, [](){

        volatile auto* p = new Proxy(Main[std::string("test")]);
        delete p;
    });

    Benchmark::run("Proxy: Control", count, [](){

        volatile auto* p = box(generate_string(1));
    });






    Benchmark::conclude();
    return 0;


    /*
    Benchmark::run("box string by Eval", count, [](){

        auto str = generate_string(32);
        volatile auto* res = jl_eval_string(("return \"" + str + "\"").c_str());
    });

    Benchmark::run("box string from Symbol", count, [](){

        static jl_function_t* to_string = jl_get_function(jl_base_module, "string");
        auto str = generate_string(32);
        volatile auto* res = jl_call1(to_string, (Any*) jl_symbol(str.c_str()));
    });

    Benchmark::run("box string as C Array", count, [](){

        auto str = generate_string(32);

        volatile auto* res = jl_alloc_string(str.size());
        auto* data = jl_string_data(res);

        for (size_t i = 0; i < 32; ++i)
            data[i] = str.at(i);
    });
     */

    Benchmark::conclude();
    /*
    Benchmark::run("unbox map", count, [](){

        // target: 19.1908
        Any* in = jl_eval_string("return Dict{Int64, Int64}(Pair.(collect(1:100), collect(1:100)))");
        auto val = unbox<std::map<Int64, Int64>>(in);
    });


    Benchmark::run("unbox vector", count, [](){

        Any* vec = jl_eval_string("return collect(1:100)");
        volatile auto res = unbox<std::vector<Int64>>(vec);
    });

    /*
    Benchmark::run("forward_last_exception positive:", count, []()
    {
        jl_eval_string("try throw(AssertionError(\"abc\")) catch e throw(e) end");

        try
        {
            forward_last_exception();
        }
        catch (...)
        {}
    });

    Benchmark::run("forward_last_exception negative:", count, []()
    {
        jl_eval_string("try throw(AssertionError(\"abc\")) catch e end");

        try
        {
            forward_last_exception();
        }
        catch (...)
        {}
    });

    Benchmark::run("box set", count, [](){
        auto set = std::set<Int64>();

        for (size_t i = 0; i < 1000; ++i)
            set.insert(i);

        volatile auto* res = box(set);
        // 1.1393
    });

    Benchmark::conclude();
    return 0;

    Benchmark::run("box iddict", count, [](){

        std::map<Int64, size_t> map;
        for (size_t i = 0; i < 1000; ++i)
            map.insert({i, i});

        volatile auto* res = box(map);
    });

    Benchmark::run("box vector", count, [](){

        std::vector<size_t> in;
        in.reserve(1000);
        for (size_t i = 0; i < 1000; ++i)
            in.push_back(i);

        volatile auto* res = box(in);
    });


    /*

    std::vector<size_t> refs;
    refs.reserve(count);

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

     */

    Benchmark::conclude();
    Benchmark::save();
}

