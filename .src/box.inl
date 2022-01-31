// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <include/julia_extension.hpp>

namespace jluna
{
    template<IsJuliaValuePointer T>
    Any* box(T value)
    {
        return (Any*) value;
    }

    template<Is<std::bool_constant<true>> T>
    Any* box(T value)
    {
        return jl_box_bool((bool) value);
    }

    template<Is<std::bool_constant<false>> T>
    Any* box(T value)
    {
        return jl_box_bool((bool) value);
    }
    
    template<Is<char> T>
    Any* box(T value)
    {
        auto* res = jl_box_int8((int8_t) value);
        return jl_convert("Char", res);
    }

    template<Is<uint8_t> T>
    Any* box(T value)
    {
        return jl_box_uint8((uint8_t) value);
    }

    template<Is<uint16_t> T>
    Any* box(T value)
    {
        return jl_box_uint16((uint16_t) value);
    }

    template<Is<uint32_t> T>
    Any* box(T value)
    {
        return jl_box_uint32((uint32_t) value);
    }

    template<Is<uint64_t> T>
    Any* box(T value)
    {
        return jl_box_uint64((uint64_t) value);
    }

    template<Is<int8_t> T>
    Any* box(T value)
    {
        return jl_box_int8((int8_t) value);
    }

    template<Is<int16_t> T>
    Any* box(T value)
    {
        return jl_box_int16((int16_t) value);
    }

    template<Is<int32_t> T>
    Any* box(T value)
    {
        return jl_box_int32((int32_t) value);
    }

    template<Is<int64_t> T>
    Any* box(T value)
    {
        return jl_box_int64((int64_t) value);
    }

    template<Is<float> T>
    Any* box(T value)
    {
        return jl_box_float32((float) value);
    }

    template<Is<double> T>
    Any* box(T value)
    {
        return jl_box_float64((double) value);
    }

    template<Is<std::string> T>
    Any* box(T value)
    {
        return jl_eval_string(("return \"" + value + "\"").c_str());
    }
}