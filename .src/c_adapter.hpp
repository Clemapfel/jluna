// 
// Copyright 2022 Clemens Cords
// Created on 19.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <stddef.h>

#ifdef __cplusplus

#include <map>
#include <include/julia_wrapper.hpp>
#include <functional>
#include <string>
#include <mutex>

extern "C"
{
    /// @brief c-compatible interface, only intended for internal use
    namespace jluna::c_adapter
    {
        //size_t invoke_1_arg(size_t id, size_t )

        /// TODO
        size_t invoke(size_t function_ptr);
        void test(jl_value_t*);
    }
}

#else // exposed to juila as pure C header:

size_t invoke(size_t);
void test(jl_value_t*);

#endif