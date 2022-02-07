// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#include <include/module.hpp>

namespace jluna
{
    Module::Module(jl_module_t* value)
        : Proxy((jl_value_t*) value, value->name)
    {}

    Module::Module(jl_module_t* value, std::shared_ptr<ProxyValue>& owner)
        : Proxy((jl_value_t*) value, owner, value->name)
    {}

    jl_module_t * Module::get() const
    {
        return (jl_module_t*) Proxy::operator const jl_value_t*();
    }

    Module::operator jl_module_t*()
    {
        return get();
    }

    jl_sym_t* Module::name() const
    {
        get()->name;
    }

    Module Module::parent() const
    {
        return Module(get()->parent);
    }

    size_t Module::build_id() const
    {
        return get()->build_id;
    }

    std::pair<size_t, size_t> Module::get_uuid() const
    {
        return std::pair<size_t, size_t>(get()->uuid.hi, get()->uuid.lo);
    }

    bool Module::is_top_module() const
    {
        return get()->istopmod;
    }

    int8_t Module::optimization_level() const
    {
        return get()->optlevel;
    }

    int8_t Module::compile_status() const
    {
        return get()->compile;
    }

    std::map<Symbol, Any*> Module::bindings() const
    {
        std::map<Symbol, Any*> out;

        TODO: check to cast to jl_binding_t

        auto htable = get()->bindings;
        for (size_t i = 0; i < htable.size / 2; ++i)
        {
            auto key = htable.table[2*i];
            auto value = htable.table[2*i+1];

            if ((size_t) key != 0x1)
            {
                out.push_back(Symbol((jl_sym_t*) key));
            }
        }

        return out;
    }

    std::vector<Module> Module::usings() const
    {
        std::vector<Module> out;

        auto array = jl_main_module->usings;
        for (size_t i = 0; i < array.len; ++i)
            out.push_back(Module((jl_module_t*) array.items[i]));

        return out;
    }
}
