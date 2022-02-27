// 
// Copyright 2022 Clemens Cords
// Created on 25.02.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>

namespace jluna
{
    template<typename T>
    UserType<T>::UserType(const std::string& name)
    {
        jl_gc_pause;
        static jl_function_t* new_usertype = jl_find_function("jluna.usertype", "new_usertype");
        _template = Proxy(jluna::safe_call(new_usertype, jl_symbol(name.c_str())));
        jl_gc_unpause;
        set_name(name);
    }

    template<typename T>
    void UserType<T>::set_name(const std::string& name)
    {
        _template["_name"] = Symbol(name);
    }

    template<typename T>
    std::string UserType<T>::get_name()
    {
        return _template["_name"].operator std::string();
    }

    template<typename T>
    void UserType<T>::set_mutable(bool b)
    {
        _template["_is_mutable"] = b;
    }

    template<typename T>
    bool UserType<T>::is_mutable()
    {
        return _template["_is_mutable"];
    }

    template<typename T>
    void UserType<T>::add_field(const std::string& name, const std::string& type, std::function<Any*(T&)> box_get, std::function<void(T&, Any*)> unbox_set)
    {
        jl_gc_pause;

        static jl_function_t* add_field = jl_find_function("jluna.usertype", "add_field!");
        auto res = (*(_mapping.insert({name, {type, box_get, unbox_set}})).first);
        jluna::safe_call(add_field, _template, jl_symbol(res.first.c_str()), jl_symbol(std::get<0>(res.second).c_str()));
        jl_gc_unpause;
    }

    template<typename T>
    Any* UserType<T>::box(T in)
    {
        jl_gc_pause;
        static jl_function_t* set_field = jl_find_function("jluna.usertype", "set_field!");

        for (auto& pair : _mapping)
            jluna::safe_call(
                set_field,
                _template,
                jl_symbol(pair.first.c_str()),
                std::get<1>(pair.second)(in)
            );

        auto* out = jl_call1(_implemented_type, _template);
        forward_last_exception();
        jl_gc_unpause;
        return out;
    }

    template<typename T>
    T UserType<T>::unbox(Any* in)
    {
        jl_gc_pause;
        static jl_function_t* getfield = jl_get_function(jl_base_module, "getfield");

        auto out = T();

        for (auto pair : _mapping)
            std::get<2>(pair.second)(out, jluna::safe_call(getfield, in, jl_symbol(pair.first)));

        jl_gc_unpause;
        return out;
    }

    template<typename T>
    void UserType<T>::add_parameter(const std::string& name, Type upper_bound, Type lower_bound)
    {
        jl_gc_pause;
        static jl_function_t* add_parameter = jl_find_function("jluna.usertype", "add_parameter!");
        jluna::safe_call(add_parameter,_template, jl_symbol(name.c_str()), upper_bound, lower_bound);
        jl_gc_unpause;
    }

    template<typename T>
    Type UserType<T>::implement(Module module)
    {
        jl_gc_pause;

        static jl_function_t* implement = jl_find_function("jluna.usertype", "implement");
        _implemented_type = jluna::safe_call(implement, _template, module);
        _implemented = true;
        jl_gc_unpause;

        return Type((jl_datatype_t*) _implemented_type);
    }

    template<typename T>
    bool UserType<T>::is_implemented()
    {
        return _implemented;
    }
}