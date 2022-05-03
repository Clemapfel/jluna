// 
// Copyright 2022 Clemens Cords
// Created on 17.04.22 by clem (mail@clemens-cords.com)
//

#include <include/typedefs.hpp>
#include <include/mutex.hpp>
#include <include/concepts.hpp>
#include <include/unsafe_utilities.hpp>
#include <include/safe_utilities.hpp>
#include <include/box.hpp>
#include <include/proxy.hpp>
#include <.src/c_adapter.hpp>

#include <unordered_map>
#include <functional>

#pragma once

namespace jluna
{
    template<typename Return_t>
    unsafe::Value* register_function(std::function<Return_t()> f)
    {
        gc_pause;
        auto* out = new c_adapter::lambda_0_arg([f]() -> unsafe::Value*
        {
            return box_function_result(f);
        });

        auto* res = c_adapter::make(out, 0);
        gc_unpause;
        return res;
    }

    template<typename Return_t, typename Arg1_t>
    unsafe::Value* register_function(std::function<Return_t(Arg1_t)> f)
    {
        gc_pause;
        auto* out = new c_adapter::lambda_1_arg([f](unsafe::Value* arg1)
        {
            return box_function_result(f, unbox<Arg1_t>(arg1));
        });

        auto* res = c_adapter::make(out, 1);
        gc_unpause;
        return res;
    }

    template<typename Return_t, typename Arg1_t, typename Arg2_t>
    unsafe::Value* register_function(std::function<Return_t(Arg1_t, Arg2_t)> f)
    {
        gc_pause;
        auto* out = new c_adapter::lambda_2_arg([f](unsafe::Value* arg1, unsafe::Value* arg2)
        {
            return box_function_result(f, unbox<Arg1_t>(arg1), unbox<Arg2_t>(arg2));
        });

        auto* res = c_adapter::make(out, 2);
        gc_unpause;
        return res;
    }

    template<typename Return_t, typename Arg1_t, typename Arg2_t, typename Arg3_t>
    unsafe::Value* register_function(std::function<Return_t(Arg1_t, Arg2_t, Arg3_t)> f)
    {
        gc_pause;
        auto* out = new c_adapter::lambda_3_arg([f](unsafe::Value* arg1, unsafe::Value* arg2, unsafe::Value* arg3)
        {
            return box_function_result(f, unbox<Arg1_t>(arg1), unbox<Arg2_t>(arg2), unbox<Arg3_t>(arg3));
        });

        auto* res = c_adapter::make(out, 3);
        gc_unpause;
        return res;
    }

    template<typename Function_t, typename _>
    unsafe::Value* as_julia_function(_ lambda)
    {
        return register_function(std::function<Function_t>(lambda));
    }
}