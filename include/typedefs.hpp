// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>
#include ".src/typedefs.inl"

namespace jluna
{
    using Bool = bool;
    using Char = uint8_t;

    using Int8 = int8_t;
    using Int16 = int16_t;
    using Int32 = int32_t;
    using Int64 = int64_t;

    using UInt8 = uint8_t;
    using UInt16 = uint16_t;
    using UInt32 = uint32_t;
    using UInt64 = uint64_t;

    using Float32 = float;
    using Float64 = double;

    using Any = jl_value_t;
    using Function = jl_function_t;
    using Symbol = jl_sym_t;
    using Module = jl_module_t;

    /// @brief constexpr transform C++ type into the julia type it will be after unboxing
    /// @tparam C++ type
    /// @returns type_name, accessible via member if type can be deduced, has no member otherwise
    template<typename T>
    struct to_julia_type
    {
        static inline const std::string type_name = detail::to_julia_type_aux<T>::type_name;
    };
}