//
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/unsafe_utilities.hpp>

namespace jluna
{
    class Proxy;

    /// @brief constant, equivalent to `-t auto`
    constexpr uint64_t JULIA_NUM_THREADS_AUTO = 0;

    /// @brief initialize environment from image
    /// @param n_threads: number of threads to initialize the julia threadpool with. Default: 1
    /// @param suppress_log: should logging be disabled. Default: No
    /// @param jluna_shared_library_path: absolute path that is the location of libjluna.so. Leave empty to use default path
    /// @param julia_bindir: absolute path that is the location of the julia image. Leave empty to use default path
    /// @param image_path: the path of a system image file (*.so), a non-absolute path is interpreted as relative to julia_bindir
    void initialize(
        uint64_t n_threads = 1,
        bool suppress_log = false,
        const std::string& jluna_shared_library_path = "",
        const std::string& julia_bindir = "",
        const std::string& image_path = ""
    );

    /// @brief call function with args, with verbose exception forwarding
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param function: function
    /// @param args: arguments, either need to be already Julia-side or boxable
    /// @returns result
    template<is_julia_value_pointer... Args_t>
    unsafe::Value* safe_call(unsafe::Function* function, Args_t...);

    /// @brief evaluate string with exception forwarding
    /// @param string: code as string
    /// @param module: module to eval the code in, `Main` by default
    /// @returns result
    unsafe::Value* safe_eval(const std::string&, unsafe::Module* module = jl_main_module);

    /// @brief execute file
    /// @param path: absolute path to file
    /// @param module: module to eval the file in, `Main` by default
    /// @returns proxy to result, if any
    unsafe::Value* safe_eval_file(const std::string& path, unsafe::Module* module = jl_main_module);

    /// @brief forward value as Ptr{T}
    /// @tparam any type castable to unsafe::Value*
    /// @param value: value
    /// @returns Julia-side pointer object
    unsafe::Value* as_julia_pointer(unsafe::Value*);

    /// @brief call julia-side println on values
    /// @param values: values, if Julia-side, no operation will take place, if C++-side, values will be boxed before being printed
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
