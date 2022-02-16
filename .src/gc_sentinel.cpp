// 
// Copyright 2022 Clemens Cords
// Created on 16.02.22 by clem (mail@clemens-cords.com)
//

#include <include/gc_sentinel.hpp>
#include <julia.h>

namespace jluna
{
    GCSentinel::GCSentinel()
        : _state_before(jl_gc_is_enabled())
    {}

    GCSentinel::~GCSentinel()
    {
        jl_gc_enable(_state_before);
    }

    void GCSentinel::enable()
    {
        jl_gc_enable(true);
    }

    void GCSentinel::disable()
    {
        jl_gc_enable(false);
    }

    bool GCSentinel::is_enabled() const
    {
        return jl_gc_is_enabled();
    }
}

