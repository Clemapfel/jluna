// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>

#include <string>

#include "exceptions.hpp"

extern "C"
{
    /// @brief convert any to string julia-side
    /// @param value
    /// @returns c-string
    inline const char* jl_to_string(jl_value_t* value)
    {
        if (value == nullptr)
            return "nothing";

        static jl_function_t* tostring = jl_get_function(jl_base_module, "string");
        return jl_string_data(jl_call1(tostring, value));
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
    inline bool jl_is_identical(jl_value_t* a, jl_value_t* b)
    {
        static jl_function_t* triple_equal = jl_get_function(jl_base_module, "===");
        return jl_call2(triple_equal, a, b);
    }

    /// @brief wraps (==) operator
    /// @param a
    /// @param b
    /// @returns bool
    inline bool jl_is_equal(jl_value_t* a, jl_value_t* b)
    {
        static jl_function_t* double_equal = jl_get_function(jl_base_module, "==");
        return jl_call2(double_equal, a, b);
    }

    /// @brief wraps convert(Type, Value)
    /// @param type_name
    /// @param value
    /// @returns julia-side value after conversion
    inline jl_value_t* jl_convert(jl_datatype_t* type, jl_value_t* value)
    {
        static jl_function_t* convert = jl_get_function(jl_base_module, "convert");
        return jl_call2(convert, (jl_value_t*) type, value);
    }

    /// @brief wraps convert(Type, Value) with verbose exception forwarding
    /// @param type_name
    /// @param value
    /// @returns julia-side value after conversion
    inline jl_value_t* jl_try_convert(jl_datatype_t* type, jl_value_t* value)
    {
        static jl_function_t* convert = jl_get_function(jl_base_module, "convert");
        return jluna::safe_call(convert, (jl_value_t*) type, value);
    }

    /// @brief throw error if value is not of type named
    /// @param value
    /// @param types_name
    inline void jl_assert_type(jl_value_t* value, jl_datatype_t* type)
    {
        static jl_function_t* assert_isa = jl_find_function("jluna", "assert_isa");
        jluna::safe_call(assert_isa, value, type);
    }

    /// @brief get value of reference
    /// @param reference
    /// @returns value
    inline jl_value_t* jl_ref_value(jl_value_t* reference)
    {
        jl_function_t* get_reference_value = jl_get_function((jl_module_t*) jl_eval_string("return Main.jluna"), "get_reference_value");
        return jl_call1(get_reference_value, reference);
    }

    /// @brief invoke deepcopy
    /// @param in
    /// @returns deep copy
    inline jl_value_t* jl_deepcopy(jl_value_t* in)
    {
        static jl_function_t* deepcopy = jl_get_function(jl_base_module, "deepcopy");
        return jl_call1(deepcopy, in);
    }

    /// @brief wrap undef
    /// @returns value
    inline jl_value_t* jl_undef_initializer()
    {
        static jl_function_t* undef_initializer = jl_get_function(jl_base_module, "UndefInitializer");
        return jl_call0(undef_initializer);
    }

    /// @brief get nth element of tuple
    /// @param tuple
    /// @param index, 0-based
    /// @returns value
    inline jl_value_t* jl_tupleref(jl_value_t* tuple, size_t n)
    {
        static jl_function_t* get = jl_get_function(jl_base_module, "get");
        return jl_call3(get, tuple, jl_box_uint64(n + 1), jl_undef_initializer());
    }

    /// @brief get length of tuple
    /// @param tuple
    /// @returns length
    inline size_t jl_tuple_len(jl_value_t* tuple)
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
        return jl_unbox_uint64(jl_call1(hash, (jl_value_t*) jl_symbol(str)));
    }

    /// @brief get length of any
    /// @param args
    /// @returns length as int64
    inline size_t jl_length(jl_value_t* value)
    {
        static jl_function_t* length = jl_get_function(jl_base_module, "length");
        return jl_unbox_int64(jl_call1(length, value));
    }

    /// @brief return string as expression
    inline jl_value_t* jl_quote(const char* in)
    {
        const std::string a = "quote ";
        const std::string b = " end";
        return jl_eval_string((a + in + b).c_str());
    }

    /// @brief pause gc and save current state
    #define jl_gc_pause bool _b_e_f_o_r_e_ = jl_gc_is_enabled(); jl_gc_enable(false);
                             // weird naming to avoid potential name-collision when used in C++

    /// @brief restore previously saved state
    #define jl_gc_unpause jl_gc_enable(_b_e_f_o_r_e_);
}
