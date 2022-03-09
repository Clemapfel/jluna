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

size_t n_reps = 5000;

int main()
{
    State::initialize();

    jl_gc_pause;

    auto* map = jl_eval_string("return Dict([Pair(rasnd(i), rand(i)) for i in 1:10])");
    auto* keys = (jl_array_t*) jl_get_nth_field(map, 1);
    auto* vals = (jl_array_t*) jl_get_nth_field(map, 2);

    jl_println((Any*) keys);

    std::map<size_t, size_t> out;
    for (size_t i = 0; i < 1000; ++i)
        out.insert({jl_unbox_uint64(jl_arrayref(keys, i)), jl_unbox_uint64(jl_arrayref(vals, i))});

    jl_gc_unpause;

    for (auto& p : out)
        std::cout << p.first << " => " << p.second << std::endl;

    return 0;

    Benchmark::initialize();

    /*
    Benchmark::run("jl_eval_string", n_reps, [](){

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

    State::safe_eval(R"(
        function f()
            out::Vector{Int64} = [0]
            for i in 2:1000
                push!(out, i + out[i-1])
            end
            return out
        end
    )");

    Benchmark::run("jl_call", n_reps, [](){

        static jl_function_t* f = jl_get_function(jl_main_module, "f");
        volatile auto* out = jl_call0(f);
    });

    Benchmark::run("jluna::call", n_reps, [](){

        static jl_function_t* f = jl_get_function(jl_main_module, "f");
        volatile auto* out = jluna::call(f);
    });

    Benchmark::run("jluna::safe_call", n_reps, [](){

        static jl_function_t* f = jl_get_function(jl_main_module, "f");
        volatile auto* out = jluna::safe_call(f);
    });

    Benchmark::run("Proxy::call", n_reps, [](){

        static jl_function_t* f = jl_get_function(jl_main_module, "f");
        static auto proxy = Proxy(f);

        volatile auto res = proxy.call();
    });

    Benchmark::run("Proxy::safe_call", n_reps, [](){

        static jl_function_t* f = jl_get_function(jl_main_module, "f");
        static auto proxy = Proxy(f);

        volatile auto res = proxy.safe_call();
    });

    Benchmark::run("C-API: allocate proxy", n_reps, []()
    {
        jl_gc_pause;
        volatile auto* res = box<std::string>(generate_string(16));
        jl_gc_unpause;
    });

    Benchmark::run("jluna: allocate unnamed proxy", n_reps, []()
    {
        volatile auto unnamed = Proxy(box<std::string>(generate_string(16)));
    });

    Benchmark::run("jluna: allocate named proxy", n_reps, []()
    {
        volatile auto named = Proxy(box<std::string>(generate_string(16)), jl_symbol("name"));
    });

    Benchmark::run("State::new_unnamed", n_reps, []()
    {
        volatile auto unnamed = State::new_unnamed_string(generate_string(16));
    });

    Benchmark::run("State::new_named", n_reps, []()
    {
        volatile auto named = State::new_named_string("name", generate_string(16));
    });

    Benchmark::run("C-API: box vector", n_reps, []()
    {
        auto vec = std::vector<size_t>();
        vec.reserve(10000);

        for (size_t i = 0; i < 10000; ++i)
            vec.push_back(generate_number<size_t>());

        jl_gc_pause;
        static jl_function_t* make_vector = jl_get_function(jl_base_module, "Vector");

        jl_array_t* out = (jl_array_t*) jl_call2(make_vector, jl_undef_initializer(), jl_box_uint64(vec.size()));

        for (size_t i = 0; i < vec.size(); ++i)
            jl_arrayset(out, jl_box_uint64(vec.at(i)), i);

        jl_gc_unpause;
    });

    Benchmark::run("jluna: box vector", n_reps, []()
    {
        auto vec = std::vector<size_t>();
        vec.reserve(10000);

        for (size_t i = 0; i < 10000; ++i)
            vec.push_back(generate_number<size_t>());

        volatile auto* res = box<std::vector<size_t>>(vec);
    });

    Benchmark::run("C-API: box primitive", n_reps, [](){

        for (size_t i = 0; i < 10000; ++i)
            volatile auto* res = jl_box_uint64(i);
    });

    Benchmark::run("jluna: box primitive", n_reps, [](){

        for (size_t i = 0; i < 10000; ++i)
            volatile auto* res = box<size_t>(i);
    });

    Benchmark::run("C-API: unbox primitive", n_reps, [](){

        for (size_t i = 0; i < 10000; ++i)
        {
            jl_gc_pause;
            auto* val = jl_box_uint64(i);
            volatile size_t res = jl_unbox_uint64(val);
            jl_gc_unpause;
        }
    });

    Benchmark::run("jluna: unbox primitive", n_reps, [](){

        for (size_t i = 0; i < 10000; ++i)
        {
            auto* val = jl_box_uint64(i);
            volatile size_t res = unbox<UInt64>(val);
        }
    });

    Benchmark::run("C-API: unbox vector", n_reps, [](){

        jl_gc_pause;
        jl_array_t* vec = (jl_array_t*) jl_eval_string("Vector{UInt64}([i for i in 1:10000])");

        std::vector<size_t> out;
        out.reserve(vec->length);

        for (size_t i = 0; i < vec->length; ++i)
            out.push_back(jl_unbox_uint64(jl_arrayref(vec, i)));

        volatile auto copy = out;

        jl_gc_unpause;
    });

    Benchmark::run("jluna: unbox vector", n_reps, [](){

        auto* vec = jl_eval_string("Vector{Int64}([i for i in 1:10000])");
        auto out = unbox<std::vector<size_t>>(vec);
    });

    Benchmark::run("C-API: box string", n_reps, [](){

        jl_gc_pause;
        auto value = generate_string(32);
        volatile auto* res = jl_alloc_string(value.size());

        for (size_t i = 0; i < value.size(); ++i)
            jl_string_data(res)[i] = value.at(i);

        jl_gc_unpause;
    });

    Benchmark::run("jluna: box string", n_reps, [](){

        volatile auto* res = box<std::string>(generate_string(32));
    });

    Benchmark::run("C-API: unbox string", n_reps, [](){

        std::stringstream str;
        str << "return \"" << generate_string(32) << "\"" << std::endl;
        auto* jl_str = jl_eval_string(str.str().c_str());
        size_t length = jl_length(jl_str);

        std::string res;
        res.reserve(length);
        res = jl_string_data(jl_str);
    });

    Benchmark::run("jluna: unbox string", n_reps, [](){

        std::stringstream str;
        str << "return \"" << generate_string(32) << "\"" << std::endl;
        auto* jl_str = jl_eval_string(str.str().c_str());
        std::string res = unbox<std::string>(jl_str);
    });
     */

    Benchmark::run("C-API: box map", n_reps, [](){

        std::map<size_t, size_t> map;
        for (size_t i = 0; i < 1000; ++i)
            map.insert({generate_number<size_t>(), generate_number<size_t>()});

        jl_gc_pause;
        static jl_function_t* make_pair = jl_get_function(jl_base_module, "Pair");
        static jl_function_t* make_map = jl_get_function(jl_base_module, "Dict");

        jl_array_t* vec = (jl_array_t*) jl_alloc_vec_any(map.size());

        {
            size_t i = 0;
            for (auto& pair : map)
                jl_arrayset(vec, jl_call2(make_pair, jl_box_uint64(pair.first), jl_box_uint64(pair.second)), i++);
        }

        volatile auto* out = jl_call1(make_map, (Any*) vec);
        jl_gc_unpause;
    });

    Benchmark::run("C-API: box map", n_reps, [](){

        std::map<size_t, size_t> map;
        for (size_t i = 0; i < 1000; ++i)
            map.insert({generate_number<size_t>(), generate_number<size_t>()});

        jl_gc_pause;
        static jl_function_t* make_pair = jl_get_function(jl_base_module, "Pair");
        static jl_function_t* make_map = jl_get_function(jl_base_module, "Dict");

        jl_array_t* vec = (jl_array_t*) jl_alloc_vec_any(map.size());

        {
            size_t i = 0;
            for (auto& pair : map)
                jl_arrayset(vec, jl_call2(make_pair, jl_box_uint64(pair.first), jl_box_uint64(pair.second)), i++);
        }

        volatile auto* out = jl_call1(make_map, (Any*) vec);
        jl_gc_unpause;
    });

    Benchmark::run("jluna: box map", n_reps, [](){

        std::map<size_t, size_t> map;
        for (size_t i = 0; i < 1000; ++i)
            map.insert({generate_number<size_t>(), generate_number<size_t>()});

        volatile auto* out = box<std::map<size_t, size_t>>(map);
    });

    Benchmark::run("C-API: unbox map", n_reps, [](){

        jl_gc_pause;

        auto* map = jl_eval_string("return Dict([Pair(rand(i), rand(i)) for i in 1:1000])");
        auto* keys = (jl_array_t*) jl_get_nth_field(map, 1);
        auto* vals = (jl_array_t*) jl_get_nth_field(map, 2);

        std::map<size_t, size_t> out;
        for (size_t i = 0; i < keys->length; ++i)
            out.insert({jl_unbox_uint64(jl_arrayref(keys, i)), jl_unbox_uint64(jl_arrayref(vals, i))});

        jl_gc_unpause;
    });

    Benchmark::run("jluna: unbox map", n_reps, [](){

        jl_gc_pause;

        auto* map = jl_eval_string("return Dict([Pair(rand(UInt64), rand(UInt64)) for i in 1:1000])");
        volatile auto out = unbox<std::map<size_t, size_t>>(map);
    });





    /*
    Benchmark::run("C-API: hash", n_reps, [](){

        auto gc = GCSentinel();

        auto name = generate_string(16);
        auto* sym = jl_symbol(name.c_str());

        volatile size_t res = sym->hash;
    });

    Benchmark::run("jluna: hash", n_reps, [](){

        auto gc = GCSentinel();

        auto name = generate_string(16);
        volatile size_t res = c_adapter::hash(name);
    });

    Benchmark::run("box lambda", n_reps, [](){

        static auto lambda = []() {
            int out = 0;

            for (size_t i = 0; i < 1000; ++i)
                if (i % 2 == 0)
                    out += i;
                else
                    out -= i;

            return out;
        };
    });
     */




    Benchmark::conclude();
    Benchmark::save();
}

void setup()
{
    for (size_t i = 0; i < 3000; ++i)
        _proxies.push_back(State::new_named_uint64(generate_string(16), generate_number(0, 10000)));
}

