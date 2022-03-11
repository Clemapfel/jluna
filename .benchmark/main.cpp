//
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

#include <jluna.hpp>
#include <.benchmark/benchmark.hpp>
#include <.benchmark/benchmark_aux.hpp>
#include <complex.h>

using namespace jluna;

std::vector<Proxy> _proxies;
void setup();

size_t n_reps = 100;

int main()
{
    State::initialize();
    Benchmark::initialize();

    Benchmark::run_as_base("C-API: unbox vector", n_reps, [](){

        jl_gc_pause;
        jl_array_t* vec = (jl_array_t*) jl_eval_string("Vector{UInt64}([UInt64(i) for i in 1:100000])");

        std::vector<size_t> out;
        out.reserve(vec->length);

        for (size_t i = 0; i < vec->length; ++i)
            out.push_back(jl_unbox_uint64(jl_arrayref(vec, i)));

        volatile auto copy = out;

        jl_gc_unpause;
    });

    Benchmark::run("jluna: unbox vector", n_reps, [](){

        jl_gc_pause;
        jl_array_t* vec = (jl_array_t*) jl_eval_string("Vector{UInt64}([UInt64(i) for i in 1:100000])");
        auto out = unbox<std::vector<size_t>>((Any*) vec);
        jl_gc_unpause;
    });

    Benchmark::conclude();
    return 0;


    // pre-load proxy dict
    for (size_t i = 0; i < 3000; ++i)
        _proxies.push_back(State::new_named_uint64(generate_string(16), generate_number(0, 10000)));

    // ### eval code as strings

    Benchmark::run_as_base("jl_eval_string", n_reps, [](){

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

    // ### calling functions

    State::safe_eval(R"(
        function f()
            out::Vector{Int64} = [0]
            for i in 2:1000
                push!(out, i + out[i-1])
            end
            return out
        end
    )");

    Benchmark::run_as_base("jl_call", n_reps, [](){

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

    // ### allocating values

    Benchmark::run_as_base("C-API: allocate proxy", n_reps, []()
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

    _proxies.clear();

    /// ### (un)box: vector
    
    Benchmark::run_as_base("C-API: box vector", n_reps, []()
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

    Benchmark::run_as_base("C-API: unbox vector", n_reps, [](){

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

    /// ### (un)box: primitive

    Benchmark::run_as_base("C-API: box primitive", n_reps, [](){

        for (size_t i = 0; i < 10000; ++i)
            volatile auto* res = jl_box_uint64(i);
    });
    

    Benchmark::run("jluna: box primitive", n_reps, [](){

        for (size_t i = 0; i < 10000; ++i)
            volatile auto* res = box<size_t>(i);
    });

    Benchmark::run_as_base("C-API: unbox primitive", n_reps, [](){

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

    /// ### (un)box: string

    Benchmark::run_as_base("C-API: box string", n_reps, [](){

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

    Benchmark::run_as_base("C-API: unbox string", n_reps, [](){

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

    /// ### (un)box: map

    Benchmark::run_as_base("C-API: box map", n_reps, [](){

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

    Benchmark::run_as_base("C-API: unbox map", n_reps, [](){

        jl_gc_pause;

        auto* map = jl_eval_string("return Dict([Pair(i, i) for i in 1:1000])");

        auto* slots = (jl_array_t*) jl_get_nth_field(map, 0);
        auto* keys = (jl_array_t*) jl_get_nth_field(map, 1);
        auto* vals = (jl_array_t*) jl_get_nth_field(map, 2);

        std::map<size_t, size_t> out;
        for (size_t i = 0; i < slots->length; ++i)
            if (jl_unbox_bool(jl_arrayref(slots, i)))
                out.insert({jl_unbox_uint64(jl_arrayref(keys, i)), jl_unbox_uint64(jl_arrayref(vals, i))});

        jl_gc_unpause;
    });

    Benchmark::run("jluna: unbox map", n_reps, [](){

        auto* map = jl_eval_string("return Dict([Pair(i, i) for i in 1:1000])");
        volatile auto out = unbox<std::map<size_t, size_t>>(map);
    });

    /// ### (un)box: complex

    Benchmark::run_as_base("C-API: box complex", n_reps, [](){

        auto value = std::complex<float>(generate_number<float>(), generate_number<float>());

        jl_gc_pause;
        static jl_function_t* complex = jl_get_function(jl_base_module, "Complex");
        volatile auto* out = jl_call2(complex, jl_box_float32(value.real()), jl_box_float32(value.imag()));
        jl_gc_unpause;
    });

    Benchmark::run("jluna: box complex", n_reps, [](){

        auto value = std::complex<float>(generate_number<float>(), generate_number<float>());
        volatile auto* out = box<std::complex<float>>(value);
    });

    Benchmark::run_as_base("C-API: unbox complex", n_reps, [](){

        auto* value = jl_eval_string("return Complex(rand(), rand())");
        _Complex float as_complex = *((_Complex float*) value);
        volatile auto out = std::complex<float>(as_complex);
    });

    Benchmark::run("jluna: unbox complex", n_reps, [](){

        auto* value = jl_eval_string("return Complex(rand(), rand())");
        volatile auto out = unbox<std::complex<float>>(value);
    });

    /// ### (un)box: set

    Benchmark::run_as_base("C-API: box set", n_reps, []()
    {
        auto set = std::set<size_t>();

        for (size_t i = 0; i < 10000; ++i)
            set.insert(generate_number<size_t>());

        jl_gc_pause;
        static jl_function_t* make_set = jl_get_function(jl_base_module, "Set");
        static jl_function_t* push = jl_get_function(jl_base_module, "push!");

        auto* out = jl_call0(make_set);

        for (size_t e : set)
            jl_call2(push, out, jl_box_uint64(e));

        jl_gc_unpause;
    });

    Benchmark::run("jluna: box set", n_reps, []()
    {
        auto set = std::set<size_t>();

        for (size_t i = 0; i < 10000; ++i)
            set.insert(generate_number<size_t>());

        volatile auto* res = box<std::set<size_t>>(set);
    });

    Benchmark::run_as_base("C-API: unbox set", n_reps, [](){

        jl_gc_pause;
        auto* value = jl_eval_string("Set{UInt64}([rand(UInt64) for _ in 1:100])");
        auto* dict = jl_get_nth_field(value, 0);
        auto* slots = (jl_array_t*) jl_get_nth_field(dict, 0);
        auto* keys = (jl_array_t*) jl_get_nth_field(dict, 1);

        std::set<size_t> out;

        for (size_t i = 0; i < slots->length; ++i)
            if (jl_unbox_bool(jl_arrayref(slots, i)))
                out.insert(jl_unbox_uint64(jl_arrayref(keys, i)));

        jl_gc_unpause;
    });

    Benchmark::run("jluna: unbox set", n_reps, [](){

        auto* value = jl_eval_string("Set{UInt64}([rand(UInt64) for _ in 1:100])");
        auto out = unbox<std::set<size_t>>(value);
    });

    /// ### (un)box: pair

    Benchmark::run_as_base("C-API: unbox pair", n_reps, [](){

        jl_gc_pause;
        auto* value = jl_eval_string("return Pair(rand(), rand())");

        volatile auto out = std::pair<float, float>();
        out.first = jl_unbox_float32(jl_get_nth_field(value, 0));
        out.second = jl_unbox_float32(jl_get_nth_field(value, 1));
        jl_gc_unpause;
    });

    Benchmark::run("jluna: unbox pair", n_reps, [](){

        auto* value = jl_eval_string("return Pair(rand(), rand())");
        volatile auto out = unbox<std::pair<float, float>>(value);
    });

    Benchmark::run_as_base("C-API: box pair", n_reps, [](){

        jl_gc_pause;
        auto value = std::make_pair(generate_number<float>(), generate_number<float>());
        static jl_function_t* pair = jl_get_function(jl_base_module, "Pair");

        volatile auto* out = jl_call2(pair, jl_box_float32(value.first), jl_box_float32(value.second));
        jl_gc_unpause;
    });

    Benchmark::run("jluna: box pair", n_reps, [](){

        auto value = std::make_pair(generate_number<float>(), generate_number<float>());
        volatile auto out = box<std::pair<float, float>>(value);
    });

    /// ### (un)box: tuple

    Benchmark::run_as_base("C-API: unbox tuple", n_reps, [](){

        jl_gc_pause;
        auto* value = jl_eval_string("return (rand(), rand(), rand(), rand())");

        auto out = std::tuple<float, float, float, float>();
        std::get<0>(out) = jl_unbox_float32(jl_get_nth_field(value, 0));
        std::get<1>(out) = jl_unbox_float32(jl_get_nth_field(value, 1));
        std::get<2>(out) = jl_unbox_float32(jl_get_nth_field(value, 2));
        std::get<3>(out) = jl_unbox_float32(jl_get_nth_field(value, 3));

        jl_gc_unpause;
    });

    Benchmark::run("jluna: unbox tuple", n_reps, [](){

        auto* value = jl_eval_string("return (rand(), rand(), rand(), rand())");
        volatile auto out = unbox<std::tuple<float, float, float, float>>(value);
    });

    Benchmark::run_as_base("C-API: box tuple", n_reps, [](){

        jl_gc_pause;
        auto value = std::make_tuple
        (
            generate_number<float>(),
            generate_number<float>(),
            generate_number<float>()
        );

        static jl_function_t* tuple = jl_get_function(jl_base_module, "Tuple");
        volatile auto* out = jl_call3(
            tuple,
            jl_box_float32(std::get<0>(value)),
            jl_box_float32(std::get<0>(value)),
            jl_box_float32(std::get<0>(value))
        );
        jl_gc_unpause;
    });

    Benchmark::run("jluna: box tuple", n_reps, [](){

        auto value = std::make_tuple
        (
            generate_number<float>(),
            generate_number<float>(),
            generate_number<float>()
        );

        volatile auto* out = box<typeof(value)>(value);
    });

    /// ### hash (used for cppcall)

    Benchmark::run_as_base("C-API: hash", n_reps, [](){

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

    /// ### lambda call overhead

    Benchmark::run_as_base("C-API: call lambda without box", n_reps, [](){

        static auto lambda = []() {
            int out = 0;

            for (size_t i = 0; i < 1000; ++i)
                if (i % 2 == 0)
                    out += i;
                else
                    out -= i;

            return out;
        };

        auto res = State::new_named_undef(generate_string(1));
        lambda();
    });

    Benchmark::run("jluna: call lambda with box", n_reps, [](){

        static auto lambda = []() {
            int out = 0;

            for (size_t i = 0; i < 1000; ++i)
                if (i % 2 == 0)
                    out += i;
                else
                    out -= i;

            return out;
        };

        auto res = State::new_named_undef(generate_string(1));
        res = lambda;
        res();
    });

    Benchmark::conclude();
    Benchmark::save();
}
