// 
// Copyright 2022 Clemens Cords
// Created on 15.04.22 by clem (mail@clemens-cords.com)
//

#include <include/typedefs.hpp>
#include <include/unsafe_utilities.hpp>
#include <include/mutex.hpp>

namespace jluna
{
    Mutex::Mutex()
    {
        static auto* lock_t = (unsafe::DataType*) jl_eval_string("return Base.ReentrantLock");
        _value = jl_new_struct(lock_t);
        _value_id = unsafe::gc_preserve(_value);
    }

    Mutex::~Mutex()
    {
        unsafe::gc_release(_value_id);
    }

    void Mutex::lock()
    {
        static auto* lock = unsafe::get_function(jl_base_module, "lock"_sym);
        unsafe::call(lock, _value);
    }

    void Mutex::try_lock()
    {
        static auto* trylock = unsafe::get_function(jl_base_module, "trylock"_sym);
        unsafe::call(trylock, _value);
    }

    void Mutex::unlock()
    {
        static auto* unlock = unsafe::get_function(jl_base_module, "unlock"_sym);
        unsafe::call(tunlock, _value);
    }
}

