// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <complex>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>

#include <julia/julia.h>

#include <include/concepts.hpp>
#include <include/typedefs.hpp>

namespace jluna
{
    /// @brief convert C++-side value to julia side value
    /// @param value: C++-side value
    /// @returns pointer to julia-side value
    template<IsJuliaValuePointer T>
    Any* box(T value);

    /// @brief box true to Bool
    template<Is<std::bool_constant<true>> T>
    Any* box(T value);

    /// @brief box false to Bool
    template<Is<std::bool_constant<false>> T>
    Any* box(T value);

    /// @brief box char to Char
    template<Is<char> T>
    Any* box(T value);

    /// @brief box uint8 to UInt8
    template<Is<uint8_t> T>
    Any* box(T value);

    /// @brief box uint16 to UInt16
    template<Is<uint16_t> T>
    Any* box(T value);

    /// @brief box uint32 to UInt32
    template<Is<uint32_t> T>
    Any* box(T value);

    /// @brief box uint64 to UInt64
    template<Is<uint64_t> T>
    Any* box(T value);

    /// @brief box int8 to Int8
    template<Is<int8_t> T>
    Any* box(T value);

    /// @brief box int16 to Int16
    template<Is<int16_t> T>
    Any* box(T value);

    /// @brief box int32 to Int32
    template<Is<int32_t> T>
    Any* box(T value);

    /// @brief box int64 to Int64
    template<Is<int64_t> T>
    Any* box(T value);

    /// @brief box float to Float32
    template<Is<float> T>
    Any* box(T value);

    /// @brief box double to Float64
    template<Is<double> T>
    Any* box(T value);

    /// @brief box string to String
    template<Is<std::string> T>
    Any* box(T value);

    /// @brief box std::complex<T> to Complex{T}
    template<typename Value_t>
    Any* box(std::complex<Value_t> value);

    /// @brief box std::vector<T> to Vector{T}
    template<typename Value_t>
    Any* box(const std::vector<Value_t>& value);

    /// @brief box std::map<T, U> to IdDict{T, U}
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<std::is_same_v<T, std::map<Key_t, Value_t>>, bool> = true>
    Any* box(T value);

    /// @brief box std::unordered_map<T, U> to Dict{T, U}
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<std::is_same_v<T, std::unordered_map<Key_t, Value_t>>, bool> = true>
    Any* box(T value);

    /// @brief box std::set<T> to Set{T}
    template<typename Value_t>
    Any* box(std::set<Value_t>& value);

    /// @brief box std::pair<T, U> to Pair{T, U}
    template<typename T,
        typename T1 = typename T::first_type,
        typename T2 = typename T::second_type,
        std::enable_if_t<std::is_same_v<T, std::pair<T1, T2>>, bool> = true>
    Any* box(T value);

    /// @brief box std::tuple<Ts...> to Tuple{Ts...}
    template<IsTuple T>
    Any* box(T value);
}

#include ".src/box.inl"