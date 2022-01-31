// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    template<Is<Any*> T>
    T unbox(Any* in)
    {
        return in;
    }

    template<Is<std::bool_constant<true>> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Bool", value);
        jl_unbox_bool(res);
    }

    template<Is<std::bool_constant<false>> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Bool", value);
        jl_unbox_bool(res);
    }

    template<Is<uint8_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("UInt8", value);
        jl_unbox_uint8(res);
    }

    template<Is<uint16_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("UInt16", value);
        jl_unbox_uint16(res);
    }

    template<Is<uint32_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("UInt32", value);
        jl_unbox_uint32(res);
    }

    template<Is<uint64_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("UInt64", value);
        jl_unbox_uint64(res);
    }

    template<Is<int8_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Int8", value);
        jl_unbox_int8(res);
    }

    template<Is<int16_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Int16", value);
        jl_unbox_int16(res);
    }

    template<Is<int32_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Int32", value);
        jl_unbox_int32(res);
    }

    template<Is<int64_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Int64", value);
        jl_unbox_int64(res);
    }

    template<Is<float> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Float32", value);
        jl_unbox_float32(res);
    }

    template<Is<double> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Float64", value);
        jl_unbox_float64(res);
    }

    template<Is<std::string> T>
    T unbox(Any* value)
    {
        return jl_to_string(value);
    }
}