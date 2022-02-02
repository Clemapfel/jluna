// 
// Copyright 2022 Clemens Cords
// Created on 03.02.22 by clem (mail@clemens-cords.com)
//

#include <complex>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <tuple>

namespace jluna
{
    template<>
    struct to_julia_type<bool>
    {
        static inline const std::string type_name = "Bool";
    };

    template<>
    struct to_julia_type<char>
    {
        static inline const std::string type_name = "Char";
    };

    template<>
    struct to_julia_type<int8_t>
    {
        static inline const std::string type_name = "Int8";
    };

    template<>
    struct to_julia_type<int16_t>
    {
        static inline const std::string type_name = "Int16";
    };

    template<>
    struct to_julia_type<int32_t>
    {
        static inline const std::string type_name = "Int32";
    };

    template<>
    struct to_julia_type<int64_t>
    {
        static inline const std::string type_name = "Int64";
    };

    template<>
    struct to_julia_type<uint8_t>
    {
        static inline const std::string type_name = "UInt8";
    };

    template<>
    struct to_julia_type<uint16_t>
    {
        static inline const std::string type_name = "UInt8";
    };

    template<>
    struct to_julia_type<uint32_t>
    {
        static inline const std::string type_name = "UInt32";
    };

    template<>
    struct to_julia_type<uint64_t>
    {
        static inline const std::string type_name = "UInt64";
    };

    template<>
    struct to_julia_type<float>
    {
        static inline const std::string type_name = "Float32";
    };

    template<>
    struct to_julia_type<double>
    {
        static inline const std::string type_name = "Float64";
    };

    template<>
    struct to_julia_type<std::string>
    {
        static inline const std::string type_name = "String";
    };

    template<>
    struct to_julia_type<Any*>
    {
        static inline const std::string type_name = "Any";
    };

    template<>
    struct to_julia_type<Symbol*>
    {
        static inline const std::string type_name = "Symbol";
    };

    template<>
    struct to_julia_type<Module*>
    {
        static inline const std::string type_name = "Module";
    };

    template<typename Value_t>
    struct to_julia_type<std::complex<Value_t>>
    {
        static inline const std::string type_name = "Complex{" + to_julia_type<Value_t>::type_name + "}";
    };

    template<typename Value_t>
    struct to_julia_type<std::vector<Value_t>>
    {
        static inline const std::string type_name = "Vector{" + to_julia_type<Value_t>::type_name + "}";
    };

    template<typename Value_t>
    struct to_julia_type<std::set<Value_t>>
    {
        static inline const std::string type_name = "Set{" + to_julia_type<Value_t>::type_name + "}";
    };

    template<typename Key_t, typename Value_t>
    struct to_julia_type<std::unordered_map<Key_t, Value_t>>
    {
        static inline const std::string type_name = "Dict{" + to_julia_type<Key_t>::type_name + ", " + to_julia_type<Value_t>::type_name + "}";
    };

    template<typename Key_t, typename Value_t>
    struct to_julia_type<std::map<Key_t, Value_t>>
    {
        static inline const std::string type_name = "IdDict{" + to_julia_type<Key_t>::type_name + ", " + to_julia_type<Value_t>::type_name + "}";
    };

    template<typename T1, typename T2>
    struct to_julia_type<std::pair<T1, T2>>
    {
        static inline const std::string type_name = "Pair{" + to_julia_type<T1>::type_name + ", " + to_julia_type<T2>::type_name + "}";
    };

    template<typename... Ts>
    struct to_julia_type<std::tuple<Ts...>>
    {
        static inline const std::string type_name = []<typename Tuple_t = std::tuple<Ts...>>(){

            std::stringstream str;
            str << "Tuple{";

            size_t n = std::tuple_size<Tuple_t>::value;
            for (size_t i = 0; i < n; ++i)
                str << to_julia_type<typename std::tuple_element<0, Tuple_t>::type>::type_name << (i != n-1 ? ", " : "");

            str << "}";
            return str.str();
        }();
    };
}