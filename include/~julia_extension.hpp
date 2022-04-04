// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>

#include <string>
#include <iostream>

#include <include/exceptions.hpp>

extern "C"
{
    /// @brief convert any to string julia-side
    /// @param value
    /// @returns c-string
    inline const char* detail::to_string(unsafe::Value* value)
    {
        if (value == nullptr)
            return "nothing";

        static jl_function_t* tostring = jl_get_function(jl_base_module, "string");
        return jl_string_data(jl_call1(tostring, value));
    }

    /// @brief get verbose type name
    /// @param value
    /// @returns result of println(typeof(value))
    inline const char* jl_verbose_typeof_str(unsafe::Value* value)
    {
        return detail::to_string(jl_typeof(value));
    }

    /// @brief get function
    /// @param module_name
    /// @param function_name
    /// @returns function ptr
    inline jl_function_t* jl_find_function(const char* module_name, const char* function_name)
    {
        return jl_get_function((jl_module_t*) jl_eval_string(("return " + std::string(module_name)).c_str()), function_name);
    }

    /// @brief wraps (===) operator
    /// @param a
    /// @param b
    /// @returns bool
    inline bool jl_is_identical(unsafe::Value* a, unsafe::Value* b)
    {
        static jl_function_t* triple_equal = jl_get_function(jl_base_module, "===");
        return jl_call2(triple_equal, a, b);
    }

    /// @brief wraps (==) operator
    /// @param a
    /// @param b
    /// @returns bool
    inline bool jl_is_equal(unsafe::Value* a, unsafe::Value* b)
    {
        static jl_function_t* double_equal = jl_get_function(jl_base_module, "==");
        return jl_call2(double_equal, a, b);
    }

    /// @brief wraps convert(Type, Value)
    /// @param type_name
    /// @param value
    /// @returns julia-side value after conversion
    inline unsafe::Value* detail::convert(unsafe::DataType* type, unsafe::Value* value)
    {
        static jl_function_t* convert = jl_get_function(jl_base_module, "convert");
        return jl_call2(convert, (unsafe::Value*) type, value);
    }

    /// @brief wraps convert(Type, Value) with verbose exception forwarding
    /// @param type_name
    /// @param value
    /// @returns julia-side value after conversion
    inline unsafe::Value* jl_try_convert(unsafe::DataType* type, unsafe::Value* value)
    {
        static jl_function_t* convert = jl_get_function(jl_base_module, "convert");
        if (jl_isa(value, (unsafe::Value*) type))
            return value;
        else
            return jluna::safe_call(convert, (unsafe::Value*) type, value);
    }

    /// @brief throw error if value is not of type named
    /// @param value
    /// @param types_name
    inline void jl_assert_type(unsafe::DataType* type_a, unsafe::DataType* type_b)
    {
        if (not (jl_subtype((unsafe::Value*) type_a, (unsafe::Value*) type_b) or jl_types_equal((unsafe::Value*) type_a, (unsafe::Value*) type_b)))
        {
            std::stringstream str;
            str << "Assertion failed: Value is of wrong type. Expected: " << detail::to_string((unsafe::Value*) type_b);
            str << ", but got: " << detail::to_string((unsafe::Value*) type_a) << std::endl;
            auto* exc = jl_new_struct(jl_errorexception_type, jl_alloc_string(0));
            throw jluna::JuliaException(exc, str.str());
        }
    }

    //

    /// @brief get value of reference
    /// @param reference
    /// @returns value
    inline unsafe::Value* jl_ref_value(unsafe::Value* reference)
    {
        jl_function_t* get_reference_value = jl_get_function((jl_module_t*) jl_eval_string("return Main.jluna"), "get_reference_value");
        return jl_call1(get_reference_value, reference);
    }

    /// @brief invoke deepcopy
    /// @param in
    /// @returns deep copy
    inline unsafe::Value* jl_deepcopy(unsafe::Value* in)
    {
        static jl_function_t* deepcopy = jl_get_function(jl_base_module, "deepcopy");
        return jl_call1(deepcopy, in);
    }

    /// @brief wrap undef
    /// @returns value
    inline unsafe::Value* jl_undef_initializer()
    {
        static jl_function_t* undef_initializer = jl_get_function(jl_base_module, "UndefInitializer");
        return jl_call0(undef_initializer);
    }

    /// @brief get nth element of tuple
    /// @param tuple
    /// @param index, 0-based
    /// @returns value
    inline unsafe::Value* jl_get_nth_field(unsafe::Value* tuple, size_t n)
    {
        return jl_get_nth_field(tuple, n);
    }

    /// @brief get length of tuple
    /// @param tuple
    /// @returns length
    inline size_t jl_tuple_len(unsafe::Value* tuple)
    {
        static jl_function_t* length = jl_get_function(jl_base_module, "length");
        return jl_unbox_int64(jl_call1(length, tuple));
    }

    /// @brief hash julia-side by first converting to symbol
    /// @param string
    /// @returns hash
    inline size_t jl_hash(const char* str)
    {
        static jl_function_t* hash = jl_get_function(jl_base_module, "hash");
        return jl_unbox_uint64(jl_call1(hash, (unsafe::Value*) jl_symbol(str)));
    }

    /// @brief get length of any
    /// @param args
    /// @returns length as int64
    inline size_t jl_length(unsafe::Value* value)
    {
        static jl_function_t* length = jl_get_function(jl_base_module, "length");
        return jl_unbox_int64(jl_call1(length, value));
    }

    /// @brief return string as expression
    inline unsafe::Value* jl_quote(const char* in)
    {
        const std::string a = "quote ";
        const std::string b = " end";
        return jl_eval_string((a + in + b).c_str());
    }

    /// @brief get value type of array
    inline unsafe::Value* jl_array_value_t(unsafe::Array* value)
    {
        static jl_function_t* get_array_value_type = jl_get_function((jl_module_t*) jl_eval_string("jluna"), "get_array_value_type");
        return jl_call1(get_array_value_type, (unsafe::Value*) value);
    }

    /// @brief print
    inline void jl_println(unsafe::Value* in)
    {
        static jl_function_t* println = jl_get_function(jl_base_module, "println");
        jl_call1(println, in);
    }

    /// @brief julia-side sizeof, in bits
    inline int64_t jl_sizeof(unsafe::Value* in)
    {
        static jl_function_t* sizeof_ = jl_get_function(jl_base_module, "sizeof");
        return jl_unbox_int64(jl_call1(sizeof_, in)) * 8;
    }

    /// @brief pause gc and save current state
    #define jl_gc_pause bool _b_e_f_o_r_e_ = jl_gc_is_enabled(); if (_b_e_f_o_r_e_) jl_gc_enable(false);
                             // weird naming to avoid potential name-collision when used in C++

    /// @brief restore previously saved state
    #define jl_gc_unpause jl_gc_enable(_b_e_f_o_r_e_);
}