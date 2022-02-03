// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <include/julia_extension.hpp>
#include <include/cppcall.hpp>

namespace jluna
{
    template<IsJuliaValuePointer T>
    Any* box(T value)
    {
        return (Any*) value;
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
        return box(std::string(value));
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::complex<Value_t>>, bool>>
    Any* box(T value)
    {
        static jl_function_t* complex = jl_find_function("jluna", "make_complex");
        return safe_call(complex, box(value.real()), box(value.imag()));
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::vector<Value_t>>, bool>>
    Any* box(const T& value)
    {
        static jl_function_t* vector = jl_find_function("jluna", "make_vector");

        std::vector<jl_value_t*> args;
        args.reserve(value.size());

        for (auto& v : value)
            args.push_back(box(v));

        auto* res = jl_call(vector, args.data(), args.size());
        forward_last_exception();
        return res;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<std::is_same_v<T, std::map<Key_t, Value_t>>, bool>>
    Any* box(T value)
    {
        static jl_function_t* iddict = jl_get_function(jl_base_module, "IdDict");

        std::vector<jl_value_t*> args;
        args.reserve(value.size());

        for (const std::pair<Key_t, Value_t>& pair : value)
            args.push_back(box(pair));

        auto* res = jl_call(iddict, args.data(), args.size());
        forward_last_exception();
        return res;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<std::is_same_v<T, std::unordered_map<Key_t, Value_t>>, bool>>
    Any* box(T value)
    {
        static jl_function_t* dict = jl_get_function(jl_base_module, "Dict");

        std::vector<jl_value_t*> args;
        args.reserve(value.size());

        for (const std::pair<Key_t, Value_t>& pair : value)
            args.push_back(box(pair));

        auto* res = jl_call(dict, args.data(), args.size());
        forward_last_exception();
        return res;
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::set<Value_t>>, bool>>
    Any* box(const T& value)
    {
        static jl_function_t* make_set = jl_find_function("jluna", "make_set");

        std::vector<jl_value_t*> args;
        args.reserve(value.size());

        for (const auto& t : value)
            args.push_back(box(t));

        auto* res = jl_call(make_set, args.data(), args.size());
        forward_last_exception();
        return res;
    }

    template<typename T, typename T1, typename T2, std::enable_if_t<std::is_same_v<T, std::pair<T1, T2>>, bool>>
    Any* box(T value)
    {
        static jl_function_t* pair = jl_find_function("jluna", "make_pair");
        return safe_call(pair, box(value.first), box(value.second));
    }

    template<IsTuple T>
    Any* box(T value)
    {
        static jl_function_t* tuple = jl_get_function(jl_core_module, "tuple");

        std::vector<jl_value_t*> args;
        args.reserve(std::tuple_size_v<T>);

        std::apply([&](auto&&... elements) {
            (args.push_back(box(elements)), ...);
        }, value);

        auto* res = jl_call(tuple, args.data(), args.size());
        forward_last_exception();
        return res;
    }

    template<LambdaType<> T>
    Any* box(T lambda)
    {
        return register_unnamed_function(lambda);
    }

    template<LambdaType<Any*> T>
    Any* box(T lambda)
    {
        return register_unnamed_function(lambda);
    }

    template<LambdaType<Any*, Any*> T>
    Any* box(T lambda)
    {
        return register_unnamed_function(lambda);
    }

    template<LambdaType<Any*, Any*, Any*> T>
    Any* box(T lambda)
    {
        return register_unnamed_function(lambda);
    }


    template<LambdaType<Any*, Any*, Any*, Any*> T>
    Any* box(T lambda)
    {
        return register_unnamed_function(lambda);
    }

    template<LambdaType<std::vector<Any*>> T>
    Any* box(T lambda)
    {
        return register_unnamed_function(lambda);
    }


}