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
    {
        jl_assert_type((Any*) value, jl_module_type);
    }

    Module::Module(Proxy* owner)
        : Proxy(*owner)
    {
        jl_assert_type(owner->operator Any*(), jl_module_type);
    }

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
        jl_gc_pause;
        auto* res = jl_call2(unsafe_call, jl_quote(command.c_str()), (Any*) get());
        jl_gc_unpause;
        return Proxy(res);
    }

    Proxy Module::safe_eval(const std::string& command)
    {
        jluna::throw_if_uninitialized();

        static jl_function_t* safe_call = jl_find_function("jluna.exception_handler", "safe_call");
        static jl_function_t* has_exception_occurred = jl_find_function("jluna.exception_handler", "has_exception_occurred");

        jl_gc_pause;
        auto* result = jl_call2(safe_call, jl_quote(command.c_str()), (Any*) get());

        if (jl_exception_occurred() or jl_unbox_bool(jl_call0(has_exception_occurred)))
        {
            std::cerr << "exception in jluna::State::safe_eval for expression:\n\"" << command << "\"\n" << std::endl;
            forward_last_exception();
        }
        jl_gc_unpause;
        return Proxy(result, nullptr);
    }

    jl_sym_t* Module::get_symbol() const
    {
        return get()->name;
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

    std::map<Symbol, Proxy> Module::get_bindings() const
    {
        static jl_function_t* get_names = jl_find_function("jluna", "get_names");
        return unbox<std::map<Symbol, Proxy>>(jluna::safe_call(get_names, get()));
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
