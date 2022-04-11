// 
// Copyright 2022 Clemens Cords
// Created on 11.04.22 by clem (mail@clemens-cords.com)
//

#include <include/mutex.hpp>

namespace jluna
{
    Mutex::Mutex()
        : cpp_mutex()
    {
        static auto* new_lock = (unsafe::Function*) jl_eval_string("__jluna_new_lock() = Base.ReentrantLock()");
        jl_mutex = jl_call0(new_lock);
        jl_mutex_id = unsafe::gc_preserve(jl_mutex);
    }

    Mutex::~Mutex()
    {
        unsafe::gc_release(jl_mutex_id);
    }

    Mutex::operator _jl_value_t*()
    {
        return jl_mutex;
    }

    void Mutex::lock()
    {
        static auto* lock = unsafe::get_function(jl_base_module, "lock"_sym);

        //lock_lock.lock();
        jl_call1(lock, jl_mutex);
        cpp_mutex.lock();
        //lock_lock.unlock();
    }

    void Mutex::unlock()
    {
        static auto* unlock = unsafe::get_function(jl_base_module, "unlock"_sym);

        //lock_lock.lock();
        jl_call1(unlock, jl_mutex);
        cpp_mutex.unlock();
        //lock_lock.unlock();
    }
}

