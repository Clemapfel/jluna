// 
// Copyright 2022 Clemens Cords
// Created on 24.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/concepts.hpp>
#include <include/unsafe_utilities.hpp>

namespace jluna
{
    /// @brief convenient object that preserves one or more pointer
    class GCSentinel
    {
        public:
            GCSentinel(size_t n)
            {
                initialize();
                jluna::safe_call(new_sentinel, jl_box_int64(n));
            }

            ~GCSentinel()
            {
                jluna::safe_call(release);
            }

            void add(unsafe::Value* ptr)
            {
                jluna::safe_call(preserve, jl_box_voidpointer((void*) ptr));
            }

        private:
            static inline unsafe::Function* new_sentinel = nullptr;
            static inline unsafe::Function* preserve = nullptr;
            static inline unsafe::Function* release = nullptr;

            void initialize()
            {
                if (new_sentinel == nullptr)
                {
                    unsafe::Module* module = (unsafe::Module*) jl_eval_string("return jluna.memory_handler");
                    new_sentinel = unsafe::get_function(module, "new_sentinel"_sym);
                    preserve = unsafe::get_function(module, "preserve"_sym);
                    release = unsafe::get_function(module, "release"_sym);
                }
            }
    };
}
