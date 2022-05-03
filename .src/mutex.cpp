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
        gc_pause;
        static auto* new_lock = unsafe::get_function("jluna"_sym, "new_lock"_sym);
        _value = unsafe::call(new_lock);
        _value_id = unsafe::gc_preserve(_value);
        gc_unpause;
    }

    Mutex::Mutex(unsafe::Value* lock)
    {
        _value = lock;
        _value_id = unsafe::gc_preserve(_value);
    }

    Mutex::~Mutex()
    {
        unsafe::gc_release(_value_id);
    }

    Mutex::operator unsafe::Value*()
    {
        return _value;
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
        unsafe::call(unlock, _value);
    }

    bool Mutex::is_locked() const
    {
        gc_pause;
        static auto* islocked = unsafe::get_function(jl_base_module, "islocked"_sym);
        auto out = jl_unbox_bool(unsafe::call(islocked, _value));
        gc_unpause;
        return out;
    }
}

