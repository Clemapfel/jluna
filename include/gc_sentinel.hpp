// 
// Copyright 2022 Clemens Cords
// Created on 16.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

namespace jluna
{
    /// @brief lock-like object that keeps any values allocated while it is in scope safe from the garbage collector
    class GCSentinel
    {
        public:
            /// @brief ctor
            GCSentinel();

            /// @brief dtor, once this is called the sentinel seizes its protective behavior
            ~GCSentinel();

            /// @brief enable, from this point no garbage will be collected. If already enabled, does nothing
            void enable();

            /// @brief disable, from this point the garbage collector is free to collect. If already disabled, does nothing
            void disable();

            /// @brief is enabled
            /// @returns bool
            bool is_enabled() const;

        private:
            bool _state_before;
    };

}