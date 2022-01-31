// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia/julia.h>

#include <string>

#include <include/exceptions.hpp>

extern "C"
{
    /// @brief convert any to string julia-side
    /// @param value
    /// @returns c-string
    const char* jl_to_string(jl_value_t* value)
    {
        static jl_function_t* tostring = jl_get_function(jl_base_module, "string");
        return jl_string_data(jl_call1(tostring, value));
    }

    /// @brief get function
    /// @param module_name
    /// @param function_name
    /// @returns function ptr
    jl_function_t* jl_find_function(const char* module_name, const char* function_name)
    {
        return jl_get_function((jl_module_t*) jl_eval_string(("return " + std::string(module_name)).c_str()), function_name);
    }

    /// @brief wraps (===) operator
    /// @param a
    /// @param b
    /// @returns bool
    bool jl_is_identical(jl_value_t* a, jl_value_t* b)
    {
        static jl_function_t* triple_equal = jl_get_function(jl_base_module, "===");
        return jl_call2(triple_equal, a, b);
    }

    /// @brief wraps (==) operator
    /// @param a
    /// @param b
    /// @returns bool
    bool jl_is_equal(jl_value_t* a, jl_value_t* b)
    {
        static jl_function_t* double_equal = jl_get_function(jl_base_module, "==");
        return jl_call2(double_equal, a, b);
    }

    /// @brief wraps convert(Type, Value)
    /// @param type_name
    /// @param value
    /// @returns julia-side value after conversion
    jl_value_t* jl_convert(const char* type, jl_value_t* value)
    {
        static jl_function_t* convert = jl_get_function(jl_base_module, "convert");
        return jl_call2(convert, jl_eval_string(("return " + std::string(type)).c_str()), value);
    }

    jl_value_t* jl_try_convert(const char* type, jl_value_t* value)
    {
        static jl_function_t* convert = jl_get_function(jl_base_module, "convert");
        return jluna::safe_call(convert, jl_eval_string(("return " + std::string(type)).c_str()), value);
    }
}