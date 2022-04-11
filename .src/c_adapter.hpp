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
        /// @brief holds lambda registers via jluna
        static inline std::map<size_t, std::pair<std::function<jl_value_t*(jl_value_t*)>, size_t>> _functions = {};

        /// @brief hash lambda-side
        /// @param name
        /// @returns hash
        size_t hash(const std::string&);

        /// @brief add lambda to function register
        /// @param name
        /// @param n_args: size of tuple argument
        /// @param lambda
        void register_function(const std::string& name, size_t n_args, std::function<jl_value_t*(jl_value_t*)>&&);

        /// @brief remove lambda from function register
        /// @param name
        void unregister_function(const std::string& name);

        /// @brief call lambda by id
        /// @param id
        void call_function(size_t);

        /// @brief get number of tuples allowed for function with id
        /// @param id
        /// @returns expected tuple size
        size_t get_n_args(size_t);

        /// @brief check if function is registered
        /// @param id
        /// @returns true if registered, false otherwise
        bool is_registered(size_t id);

        /// @brief free function
        /// @param id
        void free_function(size_t);

        //
        using MutexPtr = size_t;
        MutexPtr new_mutex();
        void delete_mutex(MutexPtr);
        void lock_mutex(MutexPtr);
        void try_lock_mutex(MutexPtr);
        void unlock_mutex(MutexPtr);
    }
}

#else // exposed to juila as pure C header:

void initialize(const char*);
void call_function(size_t);
bool is_registered(size_t);
void throw_undefined_symbol(const char*);
size_t get_n_args(size_t);
size_t new_mutex();
void delete_mutex(size_t);
void lock_mutex(size_t);
void try_lock_mutex(size_t);
void unlock_mutex(size_t);

#endif