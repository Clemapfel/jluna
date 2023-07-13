// 
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>
#include <include/exceptions.hpp>
#include <include/unsafe_utilities.hpp>
#include <include/safe_utilities.hpp>

namespace jluna::detail
{
    inline unsafe::Value* convert(unsafe::DataType* type, unsafe::Value* value)
    {
        static auto* convert = unsafe::get_function(jl_base_module, "convert"_sym);
        return jluna::safe_call(convert, type, value);
    }

    inline unsafe::DataType* array_value_type(unsafe::Array* array)
    {
        static auto* get_value_type_of_array = unsafe::get_function("jluna"_sym, "get_value_type_of_array"_sym);
        return (unsafe::DataType*) jluna::safe_call(get_value_type_of_array, array);
    }

    inline std::string to_string(unsafe::Value* value)
    {
        static auto* string = unsafe::get_function(jl_base_module, "string"_sym);
        return {jl_string_ptr(unsafe::call(string, value))};
    }

    inline void assert_type(unsafe::DataType* type_a, unsafe::DataType* type_b)
    {
        if (not (jl_subtype((jl_value_t*) type_a, (jl_value_t*) type_b) or jl_types_equal((jl_value_t*) type_a, (jl_value_t*) type_b)))
        {
            std::stringstream str;
            str << "Assertion failed: Value is of wrong type. Expected: " << detail::to_string((jl_value_t*) type_b);
            str << ", but got: " << detail::to_string((jl_value_t*) type_a) << std::endl;
            auto* exc = jl_new_struct(jl_errorexception_type, jl_alloc_string(0));
            throw jluna::JuliaException(exc, str.str());
        }
    }

    inline uint64_t tuple_length(unsafe::Value* tuple)
    {
        static auto* length = unsafe::get_function(jl_base_module, "length"_sym);
        return jl_unbox_int64(unsafe::call(length, tuple));
    }

    inline bool is_equal(unsafe::Value* a, unsafe::Value* b)
    {
        static auto* equals = unsafe::get_function(jl_base_module, "=="_sym);
        gc_pause;
        auto* res = safe_call(equals, a, b);
        auto out = a == b or jl_unbox_bool(res);
        gc_unpause;
        return out;
    }
}