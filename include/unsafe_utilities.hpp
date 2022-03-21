// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/typedefs.hpp>
#include <include/concepts.hpp>

namespace jluna::unsafe
{
    /// @brief string suffix operator to create a symbol from a string
    unsafe::Symbol* operator""_sym(const char*, size_t);

    /// @brief access function in module
    /// @param module
    /// @param name: function name
    /// @returns function pointer or nullptr if not found
    unsafe::Function* get_function(unsafe::Module* module, unsafe::Symbol* name);

    /// @brief access function by module name
    /// @param module_name
    /// @param function_name
    /// @returns function pointer or nullptr if not found
    unsafe::Function* get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name);

    /// @brief call function with args, with brief exception forwarding
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param function
    /// @param args
    /// @returns result
    template<IsReinterpretableTo<unsafe::Value*>... Args_t>
    unsafe::Value* call(unsafe::Function* function, Args_t... args);

    /// @brief ctor julia-side expression
    /// @param symbol
    /// @params arguments (optional)
    /// @returns pointer to julia-side expression
    template<IsReinterpretableTo<unsafe::Value*>... Args_t>
    unsafe::Expression* Expr(unsafe::Symbol*, Args_t...);

    /// @brief eval expression in module scope
    unsafe::Value* eval(unsafe::Expression*, unsafe::Module* = jl_main_module);

    /// @brief get julia-side value by variable name
    /// @param module
    /// @param name: variable name
    /// @returns pointer to value
    unsafe::Value* get_value(unsafe::Module* module, unsafe::Symbol* name);

    /// @brief set julia-side value by variable name
    /// @param module
    /// @param name: variable name
    /// @param value: new value
    /// @returns pointer variable after assignment, null if failed
    unsafe::Value* set_value(unsafe::Module* module, unsafe::Symbol* name, unsafe::Value* value);
}

#include <.src/unsafe_utilities.inl>