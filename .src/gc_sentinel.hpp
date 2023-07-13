// 
// Copyright 2022 Clemens Cords
// Created on 24.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/concepts.hpp>

namespace jluna::detail
{
    template<is_julia_value_pointer... Ts>
    inline void gc_push(Ts... ts)
    {
        static unsafe::Function* gc_push = nullptr;

        if (gc_push == nullptr)
        {
            bool before = jl_gc_is_enabled();
            jl_gc_enable(false);
            gc_push = jl_eval_string("return jluna.gc_sentinel.gc_push");
            jl_gc_enable(true);
        }

        (jl_call1(gc_push, jl_box_voidpointer((void*) ts)), ...);
    }

    inline void gc_pop(uint64_t n = 1)
    {
        static unsafe::Function* gc_pop = nullptr;

        if (gc_pop == nullptr)
        {
            bool before = jl_gc_is_enabled();
            jl_gc_enable(false);
            gc_pop = jl_eval_string("return jluna.gc_sentinel.gc_pop");
            jl_gc_enable(true);
        }

        for (uint64_t i = 0; i < n; ++i)
            jl_call0(gc_pop);
    }

    inline unsafe::Value* gc_save(unsafe::Value* in)
    {
        gc_push(in);
        return in;
    }
}
