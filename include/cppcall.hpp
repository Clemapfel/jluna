// 
// Copyright 2022 Clemens Cords
// Created on 20.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>

#include <vector>

#include <include/typedefs.hpp>
#include <include/concepts.hpp>

namespace jluna
{
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
    
    /// @brief register anonymous lambda with signature void() or jl_value_t*()
    /// @param lambda
    /// @returns julia-side anonymous function wrapping lambda
    template<LambdaType<> Lambda_t>
    Function* register_unnamed_function(const Lambda_t& lambda);
    
    /// @brief register anonymous lambda with signature void(jl_value_t*) or jl_value_t*(jl_value_t*)
    /// @param lambda
    /// @returns julia-side anonymous function wrapping lambda
    template<LambdaType<jl_value_t*> Lambda_t>
    Function* register_unnamed_function(const Lambda_t& lambda);
    
    /// @brief register anonymous lambda with signature void(jl_value_t*, jl_value_t*) or jl_value_t*(jl_value_t*, jl_value_t*)
    /// @param lambda
    /// @returns julia-side anonymous function wrapping lambda
    template<LambdaType<jl_value_t*, jl_value_t*> Lambda_t>
    Function* register_unnamed_function(const Lambda_t& lambda);
    
    /// @brief register anonymous lambda with signature void(3x jl_value_t*) or jl_value_t*(3x jl_value_t*)
    /// @param lambda
    /// @returns julia-side anonymous function wrapping lambda
    template<LambdaType<jl_value_t*, jl_value_t*, jl_value_t*> Lambda_t>
    Function* register_unnamed_function(const Lambda_t& lambda);
    
    /// @brief register anonymous lambda with signature void(4x jl_value_t*) or jl_value_t*(4x jl_value_t*)
    /// @param lambda
    /// @returns julia-side anonymous function wrapping lambda
    template<LambdaType<jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*> Lambda_t>
    Function* register_unnamed_function(const Lambda_t& lambda);
    
    /// @brief register anonymous lambda with signature void(std::vector<jl_value_t*>) or jl_value_t*(std::vector<jl_value_t*>)
    /// @param lambda
    /// @returns julia-side anonymous function wrapping lambda
    template<LambdaType<std::vector<jl_value_t*>> Lambda_t>
    Function* register_unnamed_function(const Lambda_t& lambda);
}

#include <.src/cppcall.inl"