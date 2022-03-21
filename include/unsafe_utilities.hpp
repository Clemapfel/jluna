// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/typedefs.hpp>

namespace jluna::unsafe
{
    /// @brief access function in module
    /// @param module
    /// @param name: function name
    /// @returns function pointer or nullptr if not found
    unsafe::Function* find_function(unsafe::Module* module, unsafe::Symbol* name);

    /// @brief access function by module name
    /// @param module_name
    /// @param function_name
    /// @returns function pointer or nullptr if not found
    unsafe::Function* find_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name);

    /// @brief call function with args, with brief exception forwarding
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param function
    /// @param args
    /// @returns result
    template<typename... Args_t>
    unsafe::Value* call(unsafe::Function* function, Args_t... args);
}

#include <.src/unsafe_utilities.inl>