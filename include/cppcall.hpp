// 
// Copyright 2022 Clemens Cords
// Created on 20.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>

#include <vector>

#include <typedefs.hpp>
#include <include/concepts.hpp>

namespace jluna
{
    namespace detail
    {
        static inline size_t _internal_function_id_name = 0;

        /// @brief forward lambda returning void as jl_nothing
        /// @param func: lambda
        /// @param args: arguments
        template<typename Lambda_t, typename Return_t, typename... Args_t, std::enable_if_t<std::is_same_v<Return_t, void>, Bool> = true>
        jl_value_t* invoke_lambda(const Lambda_t* func, Args_t... args);

        /// @brief forward lambda returning jl_value_t* as jl_value_t*
        /// @param func: lambda
        /// @param args: arguments
        template<typename Lambda_t, typename Return_t, typename... Args_t, std::enable_if_t<std::is_same_v<Return_t, jl_value_t*>, Bool> = true>
        jl_value_t* invoke_lambda(const Lambda_t* func, Args_t... args);
    }

    /// @brief register lambda with signature void() or jl_value_t*()
    /// @param name: function name
    /// @param lambda
    template<LambdaType<> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda);

    /// @brief register lambda with signature void(jl_value_t*) or jl_value_t*(jl_value_t*)
    /// @param name: function name
    /// @param lambda
    template<LambdaType<jl_value_t*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda);

    /// @brief register lambda with signature void(jl_value_t*, jl_value_t*) or jl_value_t*(jl_value_t*, jl_value_t*)
    /// @param name: function name
    /// @param lambda
    template<LambdaType<jl_value_t*, jl_value_t*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda);

    /// @brief register lambda with signature void(3x jl_value_t*) or jl_value_t*(3x jl_value_t*)
    /// @param name: function name
    /// @param lambda
    template<LambdaType<jl_value_t*, jl_value_t*, jl_value_t*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda);

    /// @brief register lambda with signature void(4x jl_value_t*) or jl_value_t*(4x jl_value_t*)
    /// @param name: function name
    /// @param lambda
    template<LambdaType<jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda);

    /// @brief register lambda with signature void(std::vector<jl_value_t*>) or jl_value_t*(std::vector<jl_value_t*>)
    /// @param name: function name
    /// @param lambda
    template<LambdaType<std::vector<jl_value_t*>> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda);
}