// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>
#include <.src/typedefs.inl>

namespace jluna
{
    /// @brief 1-bit bool interpreted as 8-bit Bool julia-side
    using Bool = bool;

    /// @brief 8-bit char interpreted as 32-bit Char julia-side
    using Char = uint8_t;

    /// @brief 8-bit int interpreted as Int8 julia-side
    using Int8 = int8_t;

    /// @brief 16-bit int interpreted as Int16 julia-side
    using Int16 = int16_t;

    /// @brief 32-bit int interpreted as Int32 julia-side
    using Int32 = int32_t;

    /// @brief 64-bit int interpreted as Int64 julia-side
    using Int64 = int64_t;

    /// @brief 8-bit unsigned int interpreted as UInt8 julia-side
    using UInt8 = uint8_t;

    /// @brief 16-bit unsigned int interpreted as UInt16 julia-side
    using UInt16 = uint16_t;

    /// @brief 32-bit unsigned int interpreted as UInt32 julia-side
    using UInt32 = uint32_t;

    /// @brief 64-bit unsigned int interpreted as UInt64 julia-side
    using UInt64 = uint64_t;

    /// @brief 32-bit float interpreted as Float32 julia-side
    using Float32 = float;

    /// @brief 64-bit double interpreted as Float64 julia-side
    using Float64 = double;

    /// @brief nothing
    using Nothing = void;

    namespace unsafe
    {
        /// @brief address of julia-side memory
        using Value = jl_value_t;

        /// @brief address of julia-side function
        using Function = jl_function_t;

        /// @brief julia-side, non-proxy symbol
        using Symbol = jl_sym_t;

        /// @brief julia-side, non-proxy module
        using Module = jl_module_t;

        /// @brief julia-side expression
        using Expression = jl_expr_t;

        /// @brief julia-side array
        using Array = jl_array_t;

        /// @brief julia-side type
        using DataType = jl_datatype_t;
    }

    /// @brief constexpr transform C++ type into the julia type it will be after unboxing
    /// @tparam C++ type
    /// @returns type_name, accessible via member if type can be deduced, has no member otherwise
    template<typename T>
    struct as_julia_type
    {
        static inline const std::string type_name = detail::as_julia_type_aux<T>::type_name;
        static unsafe::DataType* type()
        {
            static jl_datatype_t* out = (jl_datatype_t*) jl_eval_string(("return " + type_name).c_str());return out;
        }
    };

    // @concept: can be converted to Julia type
    template<typename T>
    concept to_julia_type_convertable = requires(T)
    {
        as_julia_type<T>::type_name;
    };
}