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

#include <include/julia_wrapper.hpp>

#include <include/concepts.hpp>
#include <include/typedefs.hpp>

namespace jluna
{
    // ###########################################################

    /// @brief box identity
    template<is_julia_value_pointer T>
    unsafe::Value* box(T value);

    /// @brief box nullptr to nothing
    template<is<std::nullptr_t> T>
    unsafe::Value* box(T value);

    /// @brief box void* to Ptr{Cvoid}
    template<is<void*> T>
    unsafe::Value* box(T value);

    /// @brief box identity
    template<is<bool> T>
    unsafe::Value* box(T value);

    /// @brief box true to Bool
    template<is<std::bool_constant<true>> T>
    unsafe::Value* box(T value);

    /// @brief box false to Bool
    template<is<std::bool_constant<false>> T>
    unsafe::Value* box(T value);

    /// @brief box char to Char
    template<is<char> T>
    unsafe::Value* box(T value);

    /// @brief box uint8 to UInt8
    template<is<uint8_t> T>
    unsafe::Value* box(T value);

    /// @brief box uint16 to UInt16
    template<is<uint16_t> T>
    unsafe::Value* box(T value);

    /// @brief box uint32 to UInt32
    template<is<uint32_t> T>
    unsafe::Value* box(T value);

    /// @brief box uint64 to UInt64
    template<is<uint64_t> T>
    unsafe::Value* box(T value);

    /// @brief box int8 to Int8
    template<is<int8_t> T>
    unsafe::Value* box(T value);

    /// @brief box int16 to Int16
    template<is<int16_t> T>
    unsafe::Value* box(T value);

    /// @brief box int32 to Int32
    template<is<int32_t> T>
    unsafe::Value* box(T value);

    /// @brief box int64 to Int64
    template<is<int64_t> T>
    unsafe::Value* box(T value);

    /// @brief box float to Float32
    template<is<float> T>
    unsafe::Value* box(T value);

    /// @brief box double to Float64
    template<is<double> T>
    unsafe::Value* box(T value);

    /// @brief box string to String
    template<is<std::string> T>
    unsafe::Value* box(T value);

    /// @brief box c-string to String
    template<is<const char*> T>
    unsafe::Value* box(T value);

    /// @brief box std::complex<T> to Complex{T}
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, std::complex<Value_t>>, bool> = true>
    unsafe::Value* box(T value);

    /// @brief box std::vector<T> to Vector{T}
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, std::vector<Value_t>>, bool> = true>
    unsafe::Value* box(const T& value);

    /// @brief box std::multimap<T, U> to IdDict{T, U}
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<std::is_same_v<T, std::multimap<Key_t, Value_t>>, bool> = true>
    unsafe::Value* box(const T& value);

    /// @brief box std::unordered_map<T, U> to Dict{T, U}
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<
            std::is_same_v<T, std::unordered_map<Key_t, Value_t>> or
            std::is_same_v<T, std::map<Key_t, Value_t>>, bool> = true>
    unsafe::Value* box(const T& value);

    /// @brief box std::set<T> to Set{T}
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, std::set<Value_t>>, bool> = true>
    unsafe::Value* box(const T& value);

    /// @brief box std::pair<T, U> to Pair{T, U}
    template<typename T,
        typename T1 = typename T::first_type,
        typename T2 = typename T::second_type,
        std::enable_if_t<std::is_same_v<T, std::pair<T1, T2>>, bool> = true>
    unsafe::Value* box(const T& value);

    /// @brief box std::tuple<Ts...> to Tuple{Ts...}
    template<is_tuple T>
    unsafe::Value* box(const T& value);

    /// @brief box jluna::Proxy to Value
    class Proxy;
    template<is<Proxy> T>
    unsafe::Value* box(T);

    /// @brief box jluna::Symbol to Symbol
    class Symbol;
    template<is<Symbol> T>
    unsafe::Value* box(T);

    /// @brief box jluna::Module to Module
    class Module;
    template<is<Module> T>
    unsafe::Value* box(T);

    /// @brief unbox jluna::Type to Type
    class Type;
    template<is<Type> T>
    unsafe::Value* box(T);

    /// @brief box usertype wrapper to usertype
    template<is_usertype T>
    unsafe::Value* box(T);

    /// @brief box mutex to Base.ReentrantLock
    class Mutex;
    template<is<Mutex> T>
    unsafe::Value* box(T);

    /// @brief requires a value to be boxable into a julia-side value
    template<typename T>
    concept is_boxable = requires(T t)
    {
        box(t);
    } or requires(T t)
    {
        (unsafe::Value*) t;
    };

    /// @brief forward function result
    /// @param function
    /// @param args
    /// @returns jl_nothing if return_type is void, boxed value otherwise
    template<typename Function_t, typename... Args_t, std::enable_if_t<std::is_void_v<std::invoke_result_t<Function_t, Args_t...>>, bool> = true>
    static unsafe::Value* box_function_result(Function_t f, Args_t... args);
    template<typename Function_t, typename... Args_t, std::enable_if_t<not std::is_void_v<std::invoke_result_t<Function_t, Args_t...>>, bool> = true>
    static unsafe::Value* box_function_result(Function_t f, Args_t... args);
}

#include <.src/box.inl>