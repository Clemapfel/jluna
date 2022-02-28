// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia/julia.h>

#include <include/concepts.hpp>
#include <include/typedefs.hpp>
#include <include/exceptions.hpp>

namespace jluna
{
    /// @brief convert julia-side value to C++-side value
    /// @param value: pointer to julia-side memory of arbitrary type
    /// @returns C++-side object by value

    // ###########################################################

    /// @brief unbox to Any*
    template<Is<Any*> T>
    T unbox(Any* value);
    
    /// @brief unbox to bool
    template<Is<bool> T>
    T unbox(Any* value);

    /// @brief unbox to char
    template<Is<char> T>
    T unbox(Any* value);

    /// @brief unbox to uint8
    template<Is<uint8_t> T>
    T unbox(Any* value);

    /// @brief unbox to uint16
    template<Is<uint16_t> T>
    T unbox(Any* value);

    /// @brief unbox to uint32
    template<Is<uint32_t> T>
    T unbox(Any* value);

    /// @brief unbox to uint64
    template<Is<uint64_t> T>
    T unbox(Any* value);

    /// @brief unbox to int8
    template<Is<int8_t> T>
    T unbox(Any* value);

    /// @brief unbox to int16
    template<Is<int16_t> T>
    T unbox(Any* value);

    /// @brief unbox to int32
    template<Is<int32_t> T>
    T unbox(Any* value);

    /// @brief unbox to int64
    template<Is<int64_t> T>
    T unbox(Any* value);

    /// @brief unbox to float
    template<Is<float> T>
    T unbox(Any* value);

    /// @brief unbox to double
    template<Is<double> T>
    T unbox(Any* value);

    /// @brief unbox to string
    template<Is<std::string> T>
    T unbox(Any* value);

    /// @brief unbox to complex
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, std::complex<Value_t>>, bool> = true>
    T unbox(Any* value);

    /// @brief unbox to vector
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, std::vector<Value_t>>, bool> = true>
    T unbox(Any* value);

    /// @brief unbox to map
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<std::is_same_v<T, std::map<Key_t, Value_t>>, bool> = true>
    T unbox(Any* value);

    /// @brief unbox to unordered_map
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<std::is_same_v<T, std::unordered_map<Key_t, Value_t>>, bool> = true>
    T unbox(Any* value);

    /// @brief unbox to multi_map
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<std::is_same_v<T, std::multimap<Key_t, Value_t>>, bool> = true>
    T unbox(Any* value);

    /// @brief unbox to set
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, std::set<Value_t>>, bool> = true>
    T unbox(Any* value);

    /// @brief unbox to pair
    template<IsPair T>
    T unbox(Any* value);

    /// @brief unbox to tuple
    template<IsTuple T>
    T unbox(Any* value);

    /// @brief unbox to jluna::Proxy
    class Symbol;
    template<Is<Proxy> T>
    T unbox(Any* value);

    /// @brief unbox to jluna::Symbol
    class Symbol;
    template<Is<Symbol> T>
    T unbox(Any* value);

    /// @brief unbox to jluna::Module
    class Module;
    template<Is<Module> T>
    T unbox(Any* value);

    /// @brief unbox to jluna::Type
    class Type;
    template<Is<Type> T>
    T unbox(Any* value);

    /// @brief unbox usertype wrapper to usertype
    template<IsUsertype T>
    T unbox(Any* value);

    /// @concept requires a value to be unboxed from a julia-side value
    template<typename T>
    concept Unboxable = requires(T t, jl_value_t* v)
    {
        {unbox<T>(v)};
    };
}

#include ".src/unbox.inl"