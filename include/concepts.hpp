// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>
#include <type_traits>
#include <tuple>
#include <functional>

namespace jluna
{
    /// @concept: can be reinterpret-cast to jl_value_t*
    template<typename T>
    concept IsJuliaValuePointer =
        std::is_same_v<T, jl_value_t*> or
        std::is_same_v<T, jl_module_t*> or
        std::is_same_v<T, jl_function_t*> or
        std::is_same_v<T, jl_sym_t*>;

    /// @concept is primitive number type
    template<typename T>
    concept IsNumerical =
        std::is_same_v<T, float> or
        std::is_same_v<T, double> or
        std::is_same_v<T, char> or
        std::is_same_v<T, uint8_t> or
        std::is_same_v<T, uint16_t> or
        std::is_same_v<T, uint32_t> or
        std::is_same_v<T, uint64_t> or
        std::is_same_v<T, uint8_t> or
        std::is_same_v<T, uint16_t> or
        std::is_same_v<T, uint32_t> or
        std::is_same_v<T, uint64_t>;

    /// @concept: wrapper for std::is_same_v
    template<typename T, typename U>
    concept Is = std::is_same<T, U>::value;

    /// @concept: is tuple but not pair
    template<typename T>
    concept IsTuple = std::tuple_size<T>::value != 2;

    /// @concept: is iterable
    template<typename T>
    concept Iterable = requires(T t)
    {
        {t.begin()};
        {t.end()};
        typename T::value_type;
    };

    class Proxy;

    /// @concept describes lambda with signature (Args_t...) -> T
    template<typename T, typename... Args_t>
    concept LambdaType = std::is_invocable<T, Args_t...>::value and not std::is_base_of<Proxy, T>::value;
}