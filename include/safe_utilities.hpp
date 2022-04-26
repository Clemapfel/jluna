// 
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/unsafe_utilities.hpp>

namespace jluna
{
    class Proxy;

    /// @brief manually set the C-adapter path
    void set_c_adapter_path(const std::string& path);

    /// @brief constant, equivalent to -t auto
    constexpr size_t JULIA_NUM_THREADS_AUTO = 0;

    /// @brief initialize environment
    /// @param number_of_threads: JULIA_NUM_THREADS set before initialization. If 0, set to "auto"
    void initialize(size_t n_threads = 1, bool suppress_log = false);

    /// @brief initialize environment from image
    /// @param absolute path to julia image
    /// @param number_of_threads: JULIA_NUM_THREADS set before initialization. If 0, set to "auto"
    void initialize(const std::string&, size_t n_threads = 1, bool suppress_log = false);

    /// @brief call function with args, with verbose exception forwarding
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param function
    /// @param args
    /// @returns result
    template<is_julia_value_pointer... Args_t>
    unsafe::Value* safe_call(unsafe::Function* function, Args_t... args);

    /// @brief evaluate string with exception forwarding
    /// @param string
    /// @returns result
    unsafe::Value* safe_eval(const std::string&, unsafe::Module* module = jl_main_module);

    /// @brief execute file
    /// @param path to file
    /// @returns proxy to result, if any
    unsafe::Value* safe_eval_file(const std::string& path, unsafe::Module* module = jl_main_module);

    /// @brief call julia-side println on values
    /// @param values
    template<is_julia_value_pointer... Ts>
    void println(Ts...);

    /// @brief get julia-side undef
    /// @eturns value of type UndefInitializer, does not need to be preserved
    unsafe::Value* undef();

    /// @brief get julia-side nothing
    /// @returns value of type Nothing, does not need to be preserved
    unsafe::Value* nothing();

    /// @brief get julia-side
    /// @returns value of type Missing, does not need to be preserved
    unsafe::Value* missing();

    /// @brief trigger the garbage collector
    void collect_garbage();
}

#include <.src/safe_utilities.inl>