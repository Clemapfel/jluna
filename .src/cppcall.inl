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

#include <unordered_map>
#include <functional>

namespace jluna
{
    template<is_function_with_n_args<0> Function_t>
    unsafe::Value* register_function(Function_t f)
    {
        std::function<unsafe::Value*()> in = [f]()
        {
            return box_function_result(f);
        };
    }

}