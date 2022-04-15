// 
// Copyright 2022 Clemens Cords
// Created on 15.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

namespace jluna
{
    /// @brief thread-safe proxy for Base.ReentrantLock
    class Mutex
    {
        public:
            /// @brief ctor
            Mutex();

            /// @brief dtor
            ~Mutex();

            /// @brief stall until locking is possible
            void lock();

            /// @brief free lock
            void unlock();

            /// @brief lock if possible, otherwise return and continue
            void try_lock();

            /// @brief get julia-side Base.ReentrantLock
            operator unsafe::Value*();

        private:
            unsafe::Value* _value;
            size_t _value_id;
    };
}