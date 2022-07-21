// 
// Copyright 2022 Clemens Cords
// Created on 17.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/typedefs.hpp>
#include <functional>

namespace jluna
{
    /// @brief make lambda available to Julia
    /// @tparam Signature: signature of lambda, C-style
    /// @tparam Lambda_t: automatically deduced
    /// @returns unsafe pointer to Julia-side function object
    template<typename Signature, typename Lambda_t>
    unsafe::Value* as_julia_function(Lambda_t lambda);

    /// @brief make function with signature () -> Return_t available to julia
    /// @tparam Return_t: return type of lambda, may be `void`
    /// @param function: lambda
    /// @returns unsafe pointer to Julia-side function object
    template<typename Return_t>
    unsafe::Value* register_function(std::function<Return_t()>);

    /// @brief make function with signature (T1) -> Return_t available to julia
    /// @tparam Return_t: return type of lambda, may be `void`
    /// @param function: lambda
    /// @returns unsafe pointer to Julia-side function object
    template<typename Return_t, typename Arg1_t>
    unsafe::Value* register_function(std::function<Return_t(Arg1_t)> f);

    /// @brief make function with signature (T1, T2) -> Return_t available to julia
    /// @tparam Return_t: return type of lambda, may be `void`
    /// @param function: lambda
    /// @returns unsafe pointer to Julia-side function object
    template<typename Return_t, typename Arg1_t, typename Arg2_t>
    unsafe::Value* register_function(std::function<Return_t(Arg1_t, Arg2_t)> f);

    /// @brief make function with signature (T1, T2, T3) -> Return_t available to julia
    /// @tparam Return_t: return type of lambda, may be `void`
    /// @param function: lambda
    /// @returns unsafe pointer to Julia-side function object
    template<typename Return_t, typename Arg1_t, typename Arg2_t, typename Arg3_t>
    unsafe::Value* register_function(std::function<Return_t(Arg1_t, Arg2_t, Arg3_t)> f);
}

#include <.src/cppcall.inl>