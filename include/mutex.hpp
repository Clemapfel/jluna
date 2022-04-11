// 
// Copyright 2022 Clemens Cords
// Created on 11.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/unsafe_utilities.hpp>

#include <mutex>

namespace jluna
{
    class Mutex
    {
        public:
            Mutex();
            ~Mutex();

            void lock();
            void unlock();
            void try_lock();

            operator unsafe::Value*();

        private:
            size_t jl_mutex_id;
            unsafe::Value* jl_mutex;
            std::mutex cpp_mutex;

            std::mutex lock_lock = std::mutex();
    };
}