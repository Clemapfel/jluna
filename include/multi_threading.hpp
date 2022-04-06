// 
// Copyright 2022 Clemens Cords
// Created on 06.04.22 by clem (mail@clemens-cords.com)
//

#include <include/unsafe_utilities.hpp>

#pragma once

namespace jluna
{
    template<typename Lambda_t>
    size_t schedule_task(Lambda_t lambda)
    {
        auto* julia_func = box(lambda);
        static auto* task = unsafe::get_function((unsafe::Module*) jl_eval_string("return Base.Threads"), "")
    }
}