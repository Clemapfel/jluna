// 
// Copyright 2022 Clemens Cords
// Created on 24.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/concepts.hpp>

namespace jluna
{
    /// @brief object that preserves arbitrary julia pointer
    class GCSentinel
    {
        public:
            /// @brief initialize
            /// @param n: maximum number of preserved values
            GCSentinel(size_t n)
            {
                initialize();
                jl_call1(new_sentinel, jl_box_int64(n));
            }

            /// @brief destroy, releases all preserved values
            ~GCSentinel()
            {
                jl_call0(release);
                jl_gc_safepoint();
            }

            /// @brief preserve value
            /// @param julia pointer
            void add(unsafe::Value* ptr)
            {
                jl_call1(preserve, jl_box_voidpointer((void*) ptr));
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
                    new_sentinel = jl_get_function(module, "new_sentinel");
                    preserve = jl_get_function(module, "new_sentinel");
                    release = jl_get_function(module, "new_sentinel");
                }
            }
    };

    /// @brief setup a gc sentinel, ready to preseve N values
    #define gc_save_pool_start(N) auto* __gc__ = new jluna::GCSentinel(N);

    /// @brief add value to gc sentinel
    #define gc_save(x) __gc__->add((unsafe::Value*) x);

    /// @brief destroy gc sentinel
    #define gc_save_pool_end delete __gc__;
}
