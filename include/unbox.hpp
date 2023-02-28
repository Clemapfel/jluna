// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>

#include <include/concepts.hpp>
#include <include/typedefs.hpp>
#include <include/exceptions.hpp>

namespace jluna
{
    // ###########################################################

    /// @brief unbox to unsafe::Value*
    template<is<unsafe::Value*> T>
    T unbox(unsafe::Value*);

    /// @brief forward void* as unsafe::Value*
    template<is<void*> T>
    T unbox(unsafe::Value*);
    
    /// @brief unbox to bool
    template<is<bool> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to char
    template<is<char> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to uint8
    template<is<uint8_t> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to uint16
    template<is<uint16_t> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to uint32
    template<is<uint32_t> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to uint64
    template<is<uint64_t> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to int8
    template<is<int8_t> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to int16
    template<is<int16_t> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to int32
    template<is<int32_t> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to int64
    template<is<int64_t> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to float
    template<is<float> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to double
    template<is<double> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to string
    template<is<std::string> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to complex
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, std::complex<Value_t>>, bool> = true>
    T unbox(unsafe::Value*);

    /// @brief unbox to vector
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, std::vector<Value_t>>, bool> = true>
    T unbox(unsafe::Value*);

    /// @brief unbox to map
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<std::is_same_v<T, std::map<Key_t, Value_t>>, bool> = true>
    T unbox(unsafe::Value*);

    /// @brief unbox to unordered_map
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<std::is_same_v<T, std::unordered_map<Key_t, Value_t>>, bool> = true>
    T unbox(unsafe::Value*);

    /// @brief unbox to multi_map
    template<typename T,
        typename Key_t = typename T::key_type,
        typename Value_t = typename T::mapped_type,
        std::enable_if_t<std::is_same_v<T, std::multimap<Key_t, Value_t>>, bool> = true>
    T unbox(unsafe::Value*);

    /// @brief unbox to set
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, std::set<Value_t>>, bool> = true>
    T unbox(unsafe::Value*);

    /// @brief unbox to pair
    template<is_pair T>
    T unbox(unsafe::Value*);

    /// @brief unbox to tuple
    template<is_tuple T>
    T unbox(unsafe::Value*);

    /// @brief unbox to jluna::Proxy
    class Symbol;
    template<is<Proxy> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to jluna::Symbol
    class Symbol;
    template<is<Symbol> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to jluna::Module
    class Module;
    template<is<Module> T>
    T unbox(unsafe::Value*);

    /// @brief unbox to jluna::Type
    class Type;
    template<is<Type> T>
    T unbox(unsafe::Value*);

    /// @brief unbox usertype wrapper to usertype
    template<is_usertype T>
    T unbox(unsafe::Value*);

    /// @brief unbox Base.ReentrantLock to jluna::Mutex
    class Mutex;
    template<is<Mutex> T>
    T unbox(unsafe::Value*);

    /// @concept requires a value to be unboxed from a julia-side value
    template<typename T>
    concept is_unboxable = requires(T t, jl_value_t* v)
    {
        {unbox<T>(v)};
    };
}

#include <.src/unbox.inl>