// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#include <include/unsafe_utilities.hpp>

namespace jluna::unsafe
{
    unsafe::Function* find_function(unsafe::Module* module, unsafe::Symbol* name)
    {
        static jl_function_t* eval = jl_get_function(jl_base_module, "eval");
        return unsafe::call(eval, module, name);
    }

    unsafe::Function* find_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name)
    {
        static jl_function_t* eval = jl_get_function(jl_base_module, "eval");
        return unsafe::call(eval, unsafe::call(eval, jl_main_module, module_name), function_name);
    }
}

