// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#include <include/module.hpp>
#include <include/state.hpp>

namespace jluna
{
    Module::Module(jl_module_t* value)
        : Proxy((jl_value_t*) value, value->name)
    {}

    Module::Module(jl_value_t* value, std::shared_ptr<ProxyValue>& owner, jl_sym_t* name)
        : Proxy(value, owner, name)
    {}

    jl_module_t * Module::get() const
    {
        return (jl_module_t*) Proxy::operator const jl_value_t*();
    }

    Module::operator jl_module_t*()
    {
        return get();
    }

    Proxy Module::eval(const std::string& command)
    {
        throw_if_uninitialized();

        static jl_function_t* unsafe_call = jl_find_function("jluna.exception_handler", "unsafe_call");
        Any* expr = State::eval(("return quote " + command + " end").c_str());
        auto* res = jluna::call(unsafe_call, expr, (Any*) get());
        return Proxy(res, nullptr);
    }

    Proxy Module::safe_eval(const std::string& command)
    {
        throw_if_uninitialized();

        static jl_function_t* safe_call = jl_find_function("jluna.exception_handler", "safe_call");
        Any* expr = State::safe_eval(("return quote " + command + " end").c_str());
        auto* res = jluna::safe_call(safe_call, expr, (Any*) get());
        forward_last_exception();
        return Proxy(res, nullptr);
    }

    jl_sym_t* Module::get_symbol() const
    {
        get()->name;
    }

    Module Module::get_parent_module() const
    {
        return Module(get()->parent);
    }

    size_t Module::get_build_id() const
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

    int8_t Module::get_optimization_level() const
    {
        return get()->optlevel;
    }

    int8_t Module::get_compile_status() const
    {
        return get()->compile;
    }

    int8_t Module::get_type_inference_status() const
    {
        return get()->infer;
    }

    std::map<Symbol, Any*> Module::get_bindings() const
    {
        std::map<Symbol, Any*> out;

        auto htable = get()->bindings;
        for (size_t i = 0; i < htable.size / 2; ++i)
        {
            auto key = htable.table[2*i];
            auto value = htable.table[2*i+1];

            if ((size_t) key != 0x1 and (size_t) value != 0x1)
            {
                out.insert({
                    Symbol((jl_sym_t*) key),
                    (reinterpret_cast<size_t>(value) > 0x1) ?
                        (Any*) (reinterpret_cast<jl_binding_t*>(value)->value) :
                        jl_nothing
                });
            }
        }

        return out;
    }

    std::vector<Module> Module::get_usings() const
    {
        std::vector<Module> out;

        auto array = jl_main_module->usings;
        for (size_t i = 0; i < array.len; ++i)
            out.push_back(Module((jl_module_t*) array.items[i]));

        return out;
    }

    bool Module::is_defined(const std::string& name) const
    {
        static jl_function_t* isdefined = jl_get_function(jl_base_module, "isdefined");
        return jluna::safe_call(isdefined, get(), jl_symbol(name.c_str()));
    }
}
