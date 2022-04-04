// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#include <include/module.hpp>
#include <include/safe_utilities.hpp>

namespace jluna
{
    Module::Module(jl_module_t* value)
        : Proxy((unsafe::Value*) value, value->name)
    {
        jl_assert_type((unsafe::DataType*) jl_typeof((unsafe::Value*) value), jl_module_type);
    }

    Module::Module(Proxy* owner)
        : Proxy(*owner)
    {
        jl_assert_type((unsafe::DataType*) jl_typeof(owner->operator unsafe::Value*()), jl_module_type);
    }

    jl_module_t * Module::value() const
    {
        return (jl_module_t*) Proxy::operator const unsafe::Value*();
    }

    Module::operator jl_module_t*()
    {
        return value();
    }

    bool Module::is_defined(const std::string& name) const
    {
        static jl_function_t* isdefined = jl_get_function(jl_base_module, "isdefined");
        return jluna::safe_call(isdefined, value(), jl_symbol(name.c_str()));
    }

    Proxy Module::safe_eval(const std::string& code)
    {
        return jluna::safe_eval(code, value());
    }

    Proxy Module::safe_eval_file(const std::string& path)
    {
        return jluna::safe_eval_file(path, value());
    }
}
