// 
// Copyright 2022 Clemens Cords
// Created on 03.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <complex>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <tuple>
#include <array>

namespace jluna
{
    namespace detail
    {
        template<typename T>
        struct as_julia_type_aux
        {};

        template<>
        struct as_julia_type_aux<bool>
        {
            static inline const std::string type_name = "Bool";
        };

        template<>
        struct as_julia_type_aux<char>
        {
            static inline const std::string type_name = "Char";
        };

        template<>
        struct as_julia_type_aux<int8_t>
        {
            static inline const std::string type_name = "Int8";
        };

        template<>
        struct as_julia_type_aux<int16_t>
        {
            static inline const std::string type_name = "Int16";
        };

        template<>
        struct as_julia_type_aux<int32_t>
        {
            static inline const std::string type_name = "Int32";
        };

        template<>
        struct as_julia_type_aux<int64_t>
        {
            static inline const std::string type_name = "Int64";
        };

        template<>
        struct as_julia_type_aux<uint8_t>
        {
            static inline const std::string type_name = "UInt8";
        };

        template<>
        struct as_julia_type_aux<uint16_t>
        {
            static inline const std::string type_name = "UInt8";
        };

        template<>
        struct as_julia_type_aux<uint32_t>
        {
            static inline const std::string type_name = "UInt32";
        };

        template<>
        struct as_julia_type_aux<uint64_t>
        {
            static inline const std::string type_name = "UInt64";
        };

        template<>
        struct as_julia_type_aux<float>
        {
            static inline const std::string type_name = "Float32";
        };

        template<>
        struct as_julia_type_aux<double>
        {
            static inline const std::string type_name = "Float64";
        };

        template<>
        struct as_julia_type_aux<std::string>
        {
            static inline const std::string type_name = "String";
        };

        template<>
        struct as_julia_type_aux<jl_value_t*>
        {
            static inline const std::string type_name = "Any";
        };

        template<>
        struct as_julia_type_aux<jl_sym_t*>
        {
            static inline const std::string type_name = "Symbol";
        };

        template<>
        struct as_julia_type_aux<jl_module_t*>
        {
            static inline const std::string type_name = "Module";
        };

        template<>
        struct as_julia_type_aux<jl_datatype_t*>
        {
            static inline const std::string type_name = "Type";
        };

        template<>
        struct as_julia_type_aux<jl_array_t*>
        {
            static inline const std::string type_name = "Array";
        };

        template<typename Value_t>
        struct as_julia_type_aux<std::complex<Value_t>>
        {
            static inline const std::string type_name = "Complex{" + as_julia_type_aux<Value_t>::type_name + "}";
        };

        template<typename Value_t>
        struct as_julia_type_aux<std::vector<Value_t>>
        {
            static inline const std::string type_name = "Vector{" + as_julia_type_aux<Value_t>::type_name + "}";
        };

        template<typename Value_t>
        struct as_julia_type_aux<std::set<Value_t>>
        {
            static inline const std::string type_name = "Set{" + as_julia_type_aux<Value_t>::type_name + "}";
        };

        template<typename Key_t, typename Value_t>
        struct as_julia_type_aux<std::unordered_map<Key_t, Value_t>>
        {
            static inline const std::string type_name =
                    "Dict{" + as_julia_type_aux<Key_t>::type_name + ", " + as_julia_type_aux<Value_t>::type_name + "}";
        };

        template<typename Key_t, typename Value_t>
        struct as_julia_type_aux<std::map<Key_t, Value_t>>
        {
            static inline const std::string type_name =
                    "IdDict{" + as_julia_type_aux<Key_t>::type_name + ", " + as_julia_type_aux<Value_t>::type_name +
                    "}";
        };

        template<typename T1, typename T2>
        struct as_julia_type_aux<std::pair<T1, T2>>
        {
            static inline const std::string type_name =
                    "Pair{" + as_julia_type_aux<T1>::type_name + ", " + as_julia_type_aux<T2>::type_name + "}";
        };

        template<typename... Ts>
        struct as_julia_type_aux<std::tuple<Ts...>>
        {
            static inline const std::string type_name = []() -> std::string {

                std::stringstream str;
                str << "Tuple{";

                size_t n = sizeof...(Ts);
                auto push = [&](const std::string& name, size_t i) {
                    str << name;
                    if (i != n)
                        str << ", ";
                };

                size_t i = 0;
                (push(as_julia_type_aux<Ts>::type_name, ++i), ...);

                str << "}";
                return str.str();
            }();
        };
    }
}