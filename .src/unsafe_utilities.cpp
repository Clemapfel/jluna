// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#include <include/unsafe_utilities.hpp>

namespace jluna::unsafe
{
    unsafe::Function* get_function(unsafe::Module* module, unsafe::Symbol* name)
    {
        static jl_function_t* eval = jl_get_function(jl_base_module, "eval");
        return unsafe::call(eval, module, name);
    }

    unsafe::Function* get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name)
    {
        static jl_function_t* eval = jl_get_function(jl_base_module, "eval");
        return unsafe::call(eval, unsafe::call(eval, jl_main_module, module_name), function_name);
    }

    unsafe::Symbol* operator""_sym(const char* str, size_t)
    {
        return jl_symbol(str);
    }

    unsafe::Value* eval(unsafe::Expression* expr, unsafe::Module* module)
    {
        static unsafe::Function* base_eval = get_function(jl_base_module, "eval"_sym);
        return call(base_eval, module, expr);
    }

    unsafe::Value* get_value(unsafe::Module* module, unsafe::Symbol* name)
    {
        static unsafe::Function* eval = get_function(jl_base_module, "eval"_sym);
        return call(eval, module, name);
    }

    unsafe::Value* set_value(unsafe::Module* module, unsafe::Symbol* name, unsafe::Value* value)
    {
        return eval(Expr("="_sym, name, value), module);
    }

}

