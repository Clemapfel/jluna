// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <string>

#include <include/typedefs.hpp>

namespace jluna
{
    class Proxy;
}

namespace jluna::State
{
    /// @brief initialize environment
    void initialize();

    /// @brief initialize environment
    /// @param absolute path to julia image
    void initialize(const std::string&);

    /// @brief execute line of code, evaluated in Main
    /// @param command
    /// @returns proxy to result, if any
    /// @exceptions if an error occurs julia-side it will be ignore and the result of the call will be undefined
    Proxy script(const std::string&) noexcept;

    /// @brief execute line of code with exception handling
    /// @param command
    /// @returns proxy to result, if any
    /// @exceptions if an error occurs julia-side a JuliaException will be thrown
    Proxy safe_script(const std::string&);

    /// @brief access a value, equivalent to unbox<T>(jl_eval_string("return " + name))
    /// @tparam T: type to be unboxed to
    /// @param full name of the value, e.g. Main.variable._field[0]
    /// @returns T
    /// @exceptions if an error occurs julia-side, a JuliaException will be thrown
    template<typename T>
    T safe_return(const std::string& full_name);

    /// @brief calls script("$variable_name = undef") in Main, useful for creating, then assigning new variables inline
    /// @param variable_name: exact name of variable
    /// @returns named proxy to newly created value
    Proxy new_undef(const std::string& variable_name);

    /// @brief trigger the garbage collector
    void collect_garbage();

    /// @brief activate/deactivate garbage collector
    void set_garbage_collector_enabled(bool);

    namespace detail
    {
        constexpr char _id_marker = '#';

        size_t create_reference(Any* in);
        Any* get_reference(size_t key);
        void free_reference(size_t key);
    }
}