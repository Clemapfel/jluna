// 
// Copyright 2022 Clemens Cords
// Created on 25.02.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>

namespace jluna
{
    template<typename T>
    struct as_julia_type<Usertype<T>>
    {
        static inline const std::string type_name = usertype_enabled<T>::name;
    };

    template<typename T>
    void Usertype<T>::initialize()
    {
        throw_if_uninitialized();
        _name = std::make_unique<Symbol>(get_name());
    }

    template<typename T>
    template<typename Field_t>
    void Usertype<T>::add_property(
        const std::string& name,
        std::function<Field_t(T&)> box_get,
        std::function<void(T&, Field_t)> unbox_set)
    {
        if (_name.get() == nullptr)
            initialize();

        auto symbol = Symbol(name);

        if (_mapping.find(name) == _mapping.end())
            _fieldnames_in_order.push_back(symbol);

        _mapping.insert({symbol, {
            [box_get](T& instance) -> unsafe::Value* {
                return jluna::box<Field_t>(box_get(instance));
            },
            [unbox_set](T& instance, unsafe::Value* value) -> void {
                unbox_set(instance, jluna::unbox<Field_t>(value));
            },
            Type((jl_datatype_t*) jl_eval_string(as_julia_type<Field_t>::type_name.c_str()))
        }});
    }

    template<typename T>
    std::string Usertype<T>::get_name()
    {
        return usertype_enabled<T>::name;
    }

    template<typename T>
    bool Usertype<T>::is_enabled()
    {
        return usertype_enabled<T>::value;
    }

    template<typename T>
    void Usertype<T>::implement(unsafe::Module* module)
    {
        if (_name.get() == nullptr)
            initialize();

        gc_pause;
        static jl_function_t* implement = unsafe::get_function("jluna"_sym, "implement"_sym);
        static jl_function_t* new_proxy = unsafe::get_function("jluna"_sym, "new_proxy"_sym);
        static jl_function_t* setfield = jl_get_function(jl_base_module, "setindex!");

        auto default_instance = T();
        auto* template_proxy = jluna::safe_call(new_proxy, _name->operator unsafe::Value*());

        for (auto& field_name : _fieldnames_in_order)
            jluna::safe_call(setfield, template_proxy, (unsafe::Value*) std::get<0>(_mapping.at(field_name))(default_instance), (unsafe::Value*) field_name);

        _type = std::make_unique<Type>((jl_datatype_t*) jluna::safe_call(implement, template_proxy, module));
        _implemented = true;
        gc_unpause;
    }

    template<typename T>
    bool Usertype<T>::is_implemented()
    {
        return _implemented;
    }

    template<typename T>
    unsafe::Value* Usertype<T>::box(T& in)
    {
        if (not _implemented)
            implement();

        gc_pause;
        static jl_function_t* setfield = jl_get_function(jl_base_module, "setfield!");

        unsafe::Value* out = jl_call0(_type->operator unsafe::Value*());

        for (auto& pair : _mapping)
            jluna::safe_call(setfield, out, (unsafe::Value*) pair.first, std::get<0>(pair.second)(in));

        gc_unpause;
        return out;
    }

    template<typename T>
    T Usertype<T>::unbox(unsafe::Value* in)
    {
        if (not _implemented)
            implement();

        gc_pause;
        static jl_function_t* getfield = jl_get_function(jl_base_module, "getfield");

        auto out = T();

        for (auto& pair : _mapping)
            std::get<1>(pair.second)(out, jluna::safe_call(getfield, in, (unsafe::Value*) pair.first));

        gc_unpause;
        return out;
    }

    template<is_usertype T>
    T unbox(unsafe::Value* in)
    {
       return Usertype<T>::unbox(in);
    }

    template<is_usertype T>
    unsafe::Value* box(T in)
    {
        return Usertype<T>::box(in);
    }
}