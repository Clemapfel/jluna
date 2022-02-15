// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <include/julia_extension.hpp>
#include <include/cppcall.hpp>

#include <iostream>


namespace jluna
{
    template<IsJuliaValuePointer T>
    Any* box(T value)
    {
        return (Any*) value;
    }

    template<Is<bool> T>
    Any* box(T value)
    {
        return jl_box_bool(value);
    }

    template<Is<std::bool_constant<true>> T>
    Any* box(T value)
    {
        return jl_box_bool((bool) value);
    }

    template<Is<std::bool_constant<false>> T>
    Any* box(T value)
    {
        return jl_box_bool((bool) value);
    }
    
    template<Is<char> T>
    Any* box(T value)
    {
        auto* res = jl_box_int8((int8_t) value);
        return jl_convert("Char", res);
    }

    template<Is<uint8_t> T>
    Any* box(T value)
    {
        return jl_box_uint8((uint8_t) value);
    }

    template<Is<uint16_t> T>
    Any* box(T value)
    {
        return jl_box_uint16((uint16_t) value);
    }

    template<Is<uint32_t> T>
    Any* box(T value)
    {
        return jl_box_uint32((uint32_t) value);
    }

    template<Is<uint64_t> T>
    Any* box(T value)
    {
        return jl_box_uint64((uint64_t) value);
    }

    template<Is<int8_t> T>
    Any* box(T value)
    {
        return jl_box_int8((int8_t) value);
    }

    template<Is<int16_t> T>
    Any* box(T value)
    {
        return jl_box_int16((int16_t) value);
    }

    template<Is<int32_t> T>
    Any* box(T value)
    {
        return jl_box_int32((int32_t) value);
    }

    template<Is<int64_t> T>
    Any* box(T value)
    {
        return jl_box_int64((int64_t) value);
    }

    template<Is<float> T>
    Any* box(T value)
    {
        return jl_box_float32((float) value);
    }

    template<Is<double> T>
    Any* box(T value)
    {
        return jl_box_float64((double) value);
    }

    template<Is<std::string> T>
    Any* box(T value)
    {
        return jl_eval_string(("return \"" + value + "\"").c_str());
    }

    template<Is<const char*> T>
    Any* box(T value)
    {
        return box<std::string>(std::string(value));
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::complex<Value_t>>, bool>>
    Any* box(T value)
    {
        static jl_function_t* complex = jl_find_function("jluna", "make_complex");
        return safe_call(complex, box<Value_t>(value.real()), box<Value_t>(value.imag()));
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::vector<Value_t>>, bool>>
    Any* box(const T& value)
    {
        static jl_function_t* vector = jl_get_function(jl_base_module, "Vector");

        auto before = jl_gc_is_enabled();
        jl_gc_enable(false);

        auto* res = (jl_array_t*) jl_call2(vector, jl_undef_initializer(), jl_box_uint64(value.size()));

        for (size_t i = 0; i < value.size(); ++i)
            jl_arrayset(res, box(value.at(i)), i);

        jl_gc_enable(before);
        return (Any*) res;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<std::is_same_v<T, std::multimap<Key_t, Value_t>>, bool>>
    Any* box(T value)
    {
        static jl_function_t* iddict = jl_get_function(jl_base_module, "IdDict");
        static jl_function_t* make_pair = jl_get_function(jl_base_module, "Pair");

        auto before = jl_gc_is_enabled();
        jl_gc_enable(false);

        std::vector<jl_value_t*> pairs;
        pairs.reserve(value.size());

        for (auto& pair : value)
            pairs.push_back(jl_call2(make_pair, box<Key_t>(pair.first), box<Value_t>(pair.second)));

        auto* res = jl_call(iddict, pairs.data(), pairs.size());

        jl_gc_enable(before);
        return res;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<
            std::is_same_v<T, std::unordered_map<Key_t, Value_t>> or
            std::is_same_v<T, std::map<Key_t, Value_t>>,
            bool>>
    Any* box(T value)
    {
        static jl_function_t* dict = jl_get_function(jl_base_module, "Dict");
        static jl_function_t* make_pair = jl_get_function(jl_base_module, "Pair");

        auto before = jl_gc_is_enabled();
        jl_gc_enable(false);

        std::vector<jl_value_t*> pairs;
        pairs.reserve(value.size());

        for (auto& pair : value)
            pairs.push_back(jl_call2(make_pair, box<Key_t>(pair.first), box<Value_t>(pair.second)));

        jl_gc_enable(before);

        return jl_call(dict, pairs.data(), pairs.size());
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::set<Value_t>>, bool>>
    Any* box(const T& value)
    {
        static jl_function_t* vector = jl_get_function(jl_base_module, "Vector");
        static jl_function_t* set = jl_get_function(jl_base_module, "Set");

        auto before = jl_gc_is_enabled();
        jl_gc_enable(false);
        auto* res = (jl_array_t*) jl_call2(vector, jl_undef_initializer(), jl_box_uint64(value.size()));

        size_t i = 0;
        for (const auto& s : value)
        {
            jl_arrayset(res, box<Value_t>(s), i);
            i += 1;
        }

        jl_gc_enable(before);
        return jl_call1(set, (Any*) res);
    }

    template<typename T, typename T1, typename T2, std::enable_if_t<std::is_same_v<T, std::pair<T1, T2>>, bool>>
    Any* box(T value)
    {
        static jl_function_t* pair = jl_get_function(jl_base_module, "Pair");
        return jl_call3(pair, box<T1>(value.first), box<T2>(value.second));
    }

    template<IsTuple T>
    Any* box(T value)
    {
        static jl_function_t* tuple = jl_get_function(jl_core_module, "tuple");

        std::vector<jl_value_t*> args;
        args.reserve(std::tuple_size_v<T>);

        std::apply([&](auto... elements) {
            (args.push_back(box<decltype(elements)>(elements)), ...);
        }, value);

        return jl_call(tuple, args.data(), args.size());
    }

    template<LambdaType<> T>
    Any* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<Any*> T>
    Any* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<Any*, Any*> T>
    Any* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<Any*, Any*, Any*> T>
    Any* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<Any*, Any*, Any*, Any*> T>
    Any* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<std::vector<Any*>> T>
    Any* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }
}