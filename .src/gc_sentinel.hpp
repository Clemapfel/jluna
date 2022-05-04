// 
// Copyright 2022 Clemens Cords
// Created on 24.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/concepts.hpp>
#include <include/unsafe_utilities.hpp>

namespace jluna
{
    namespace detail
    {
        inline size_t _gc_stack_size = 0;
    }

    template<is_julia_value_pointer... Ts>
    inline void gc_push(Ts... ts)
    {
        auto before = jl_gc_is_enabled();
        jl_gc_enable(false);
        static unsafe::Function* gc_push = nullptr;

        if (gc_push == nullptr)
        {
            bool before = jl_gc_is_enabled();
            jl_gc_enable(false);
            gc_push = jl_eval_string("return jluna.gc_sentinel.gc_push");
            jl_gc_enable(true);
        }

        (jl_call1(gc_push, jl_box_voidpointer((void*) ts)), ...);
        jl_gc_enable(before);

        detail::_gc_stack_size += sizeof...(Ts);
    }

    inline void gc_pop(size_t n = 1)
    {
        auto before = jl_gc_is_enabled();
        jl_gc_enable(false);
        static unsafe::Function* gc_pop = nullptr;

        if (gc_pop == nullptr)
        {
            bool before = jl_gc_is_enabled();
            jl_gc_enable(false);
            gc_pop = jl_eval_string("return jluna.gc_sentinel.gc_pop");
            jl_gc_enable(true);
        }

        for (size_t i = 0; i < n; ++i)
            jl_call0(gc_pop);

        jl_gc_enable(before);

        detail::_gc_stack_size -= n;
    }

    template<typename T>
    inline T gc_save(T in)
    {
        gc_push(in);
        return in;
    }
}

#define gc_store size_t __gc_stack_size__ = jluna::detail::_gc_stack_size;
#define gc_restore if (__gc_stack_size__ > jluna::detail::_gc_stack_size) gc_pop(__gc_stack_size__ - jluna::detail::_gc_stack_size);
