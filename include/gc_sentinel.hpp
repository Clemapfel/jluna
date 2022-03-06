// 
// Copyright 2022 Clemens Cords
// Created on 01.03.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>

namespace jluna
{
    /// @brief convenient, lock-like class that protects from the garbage collector while it is in scope
    struct GCSentinel
    {
        private:
            bool _before;

        public:
            /// @brief ctor, from this point onwards, the GC is disabled
            GCSentinel()
            {
                _before = jl_gc_is_enabled();
                jl_gc_enable(false);
            }

            /// @brief dtor, after this is called, the GC state is returned to what it was previously
            ~GCSentinel()
            {
                jl_gc_enable(_before);
            }
    };
}