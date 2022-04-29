// 
// Copyright 2022 Clemens Cords
// Created on 21.04.22 by clem (mail@clemens-cords.com)
//

#include <.src/common.hpp>

namespace jluna
{
    template<is<jluna::Mutex> T>
    unsafe::Value* box(T in)
    {
        return in.operator unsafe::Value*();
    }

    template<is<jluna::Mutex> T>
    T unbox(unsafe::Value* in)
    {
        jluna::detail::assert_type(
            (unsafe::DataType*) jl_typeof(in),
            (unsafe::DataType*) jl_eval_string("return Base.ReentrantLock")
        );

        return Mutex(in);
    }
}