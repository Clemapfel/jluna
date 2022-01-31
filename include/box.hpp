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
    /// @brief convert C++-side value to julia side value
    /// @param value: C++-side value
    /// @returns pointer to julia-side value
    template<IsJuliaValuePointer T>
    Any* box(T value);

    template<Is<std::bool_constant<true>> T>
    Any* box(T value);

    template<Is<std::bool_constant<false>> T>
    Any* box(T value);

    template<Is<char> T>
    Any* box(T value);

    template<Is<uint8_t> T>
    Any* box(T value);

    template<Is<uint16_t> T>
    Any* box(T value);

    template<Is<uint32_t> T>
    Any* box(T value);

    template<Is<uint64_t> T>
    Any* box(T value);

    template<Is<int8_t> T>
    Any* box(T value);

    template<Is<int16_t> T>
    Any* box(T value);

    template<Is<int32_t> T>
    Any* box(T value);

    template<Is<int64_t> T>
    Any* box(T value);

    template<Is<float> T>
    Any* box(T value);

    template<Is<double> T>
    Any* box(T value);

    template<Is<std::string> T>
    Any* box(T value);


}

#include ".src/box.inl"