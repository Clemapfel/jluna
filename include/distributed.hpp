// 
// Copyright 2022 Clemens Cords
// Created on 7/26/22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/safe_utilities.hpp>

namespace jluna
{
    using NodeID = size_t;

    std::vector<NodeID> add_process(size_t n);
    void remove_process(NodeID id);

    size_t get_n_procs();

    void safe_eval_everywhere(const std::string&, unsafe::Module* module = jl_main_module);
    void safe_eval_at(NodeID node_id, const std::string&, unsafe::Module* module = jl_main_module);

    void safe_eval_file_everywhere(const std::string& path);
    void safe_eval_file_at(NodeID node_const, std::string& path);

    template<is_julia_value_pointer... Args_t>
    unsafe::Value* safe_call_everywhere(unsafe::Function* function, Args_t... args);

    template<is_julia_value_pointer... Args_t>
    unsafe::Value* safe_call_at(unsafe::Function* function, Args_t... args);
}