// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia/julia.h>

#include <include/concepts.hpp>
#include <include/typedefs.hpp>

namespace jluna
{
    /// @brief convert julia-side value to C++-side value
    /// @param value: julia-side value
    /// @returns C++ struct
    template<Is<Any*> T>
    T unbox(Any* value);
    
    template<Is<std::bool_constant<true>> T>
    T unbox(Any* value);
    
    template<Is<std::bool_constant<false>> T>
    T unbox(Any* value);
    
    template<Is<uint8_t> T>
    T unbox(Any* value);
    
    template<Is<uint16_t> T>
    T unbox(Any* value);
    
    template<Is<uint32_t> T>
    T unbox(Any* value);
    
    template<Is<uint64_t> T>
    T unbox(Any* value);
    
    template<Is<int8_t> T>
    T unbox(Any* value);
    
    template<Is<int16_t> T>
    T unbox(Any* value);
    
    template<Is<int32_t> T>
    T unbox(Any* value);
    
    template<Is<int64_t> T>
    T unbox(Any* value);
    
    template<Is<float> T>
    T unbox(Any* value);
    
    template<Is<double> T>
    T unbox(Any* value);
    
    template<Is<std::string> T>
    T unbox(Any* value);
}

#include ".src/unbox.inl"