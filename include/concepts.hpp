// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <type_traits>
#include <tuple>

namespace jluna
{
    /// @concept: reinterpretable to jl_value_t*
    template<typename T>
    concept IsJuliaValuePointer =
        std::is_same_v<T, jl_value_t*> or
        std::is_same_v<T, jl_module_t*> or
        std::is_same_v<T, jl_function_t*> or
        std::is_same_v<T, jl_sym_t*>;

    /// @concept: wrapper for std::is_same_v
    template<typename T, typename U>
    concept Is = std::is_same<T, U>::value;

    /// @concept: is tuple but not pair
    template<typename T>
    concept IsTuple = std::tuple_size<T>::value != 2;
}