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

namespace jluna
{
    template<is_function_with_n_args<0> Function_t>
    unsafe::Value* register_function(Function_t f)
    {
        auto* out = new c_adapter::lambda_0_arg([f]()
        {
            return box_function_result(f);
        });

        return c_adapter::make(out, 0);
    }

    template<is_function_with_n_args<1> Function_t>
    unsafe::Value* register_function(Function_t f)
    {
        using arg1_t = get_nth_argument_t<Function_t, 0>;
        auto* out = new c_adapter::lambda_1_arg([f](unsafe::Value* arg1)
        {
            return box_function_result(f, unbox<arg1_t>(arg1));
        });

        return c_adapter::make(out, 1);
    }

    template<is_function_with_n_args<2> Function_t>
    unsafe::Value* register_function(Function_t f)
    {
        using arg1_t = get_nth_argument_t<Function_t, 0>;
        using arg2_t = get_nth_argument_t<Function_t, 1>;

        auto* out = new c_adapter::lambda_2_arg([f](unsafe::Value* arg1, unsafe::Value* arg2)
        {
            return box_function_result(f, unbox<arg1_t>(arg1), unbox<arg2_t>(arg2));
        });

        return c_adapter::make(out, 2);
    }

    template<is_function_with_n_args<3> Function_t>
    unsafe::Value* register_function(Function_t f)
    {
        using arg1_t = get_nth_argument_t<Function_t, 0>;
        using arg2_t = get_nth_argument_t<Function_t, 1>;
        using arg3_t = get_nth_argument_t<Function_t, 2>;

        auto* out = new c_adapter::lambda_3_arg([f](unsafe::Value* arg1, unsafe::Value* arg2, unsafe::Value* arg3)
        {
            return box_function_result(f, unbox<arg1_t>(arg1), unbox<arg2_t>(arg2), unbox<arg3_t>(arg3));
        });

        return c_adapter::make(out, 3);
    }

    template<typename Function_t, std::enable_if_t<
        not is_function_with_n_args<f, 0> and
        not is_function_with_n_args<f, 1> and
        not is_function_with_n_args<f, 2> and
        not is_function_with_n_args<f, 3>
        , bool> = true>
    unsafe::Value* register_function(Function_t f)
    {
        auto* out = new c_adapter::lambda_n_arg([f](unsafe::Array* array) {



        });
    }

}