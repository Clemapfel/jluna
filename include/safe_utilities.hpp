// 
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/unsafe_utilities.hpp>

namespace jluna
{
    /// @brief manually set the C-adapter path
    void set_c_adapter_path(const std::string& path);

    /// @brief initialize environment
    void initialize();

    /// @brief initialize environment from image
    /// @param absolute path to julia image
    void initialize(const std::string&);

    /// @brief call function with args, with verbose exception forwarding
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param function
    /// @param args
    /// @returns result
    template<IsJuliaValuePointer... Args_t>
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
    template<IsJuliaValuePointer... Ts>
    void println(Ts...);

    /// @brief trigger the garbage collector
    void collect_garbage();
}

#include <.src/safe_utilities.inl>