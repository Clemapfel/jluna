// 
// Copyright 2022 Clemens Cords
// Created on 25.02.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>

namespace jluna
{
    template<typename T>
    struct to_julia_type<Usertype<T>>
    {
        inline static std::string type_name = "<USERTYPE_NAME_UNINITIALIZED>";
    };

    template<typename T>
    UsertypeNotEnabledException<T>::UsertypeNotEnabledException()
        : _msg("[C++][Exception] usertype interface for this type was not yet enabled. C Usertype<T>::enable(const std::string&) to instance the interface")
    {}

    template<typename T>
    const char * UsertypeNotEnabledException<T>::what() const noexcept
    {
        return _msg.c_str();
    }

    template<typename T>
    void Usertype<T>::enable(const std::string& name)
    {
        _enabled = true;
        _name = std::make_unique<Symbol>(name);
        to_julia_type<Usertype<T>>::type_name = name;
    }

    template<typename T>
    bool Usertype<T>::is_enabled()
    {
        return _enabled;
    }

    template<typename T>
    template<typename Field_t>
    void Usertype<T>::add_property(
        const std::string& name,
        std::function<Field_t(T&)> box_get,
        std::function<void(T&, Field_t)> unbox_set)
    {
        auto symbol = Symbol(name);

        if (_mapping.find(name) == _mapping.end())
            _fieldnames_in_order.push_back(symbol);

        _mapping.insert({symbol, {
            [box_get](T& instance) -> Any* {
                return jluna::box<Field_t>(box_get(instance));
            },
            [unbox_set](T& instance, Any* value) -> void {
                unbox_set(instance, jluna::unbox<Field_t>(value));
            },
            Type((jl_datatype_t*) jl_eval_string(to_julia_type<Field_t>::type_name.c_str()))
        }});
    }

    template<typename T>
    void Usertype<T>::implement(Module module)
    {
        jl_gc_pause;
        static jl_function_t* implement = jl_find_function("jluna", "implement");
        static jl_function_t* new_proxy = jl_find_function("jluna", "new_proxy");
        static jl_function_t* setfield = jl_get_function(jl_base_module, "setindex!");

        auto default_instance = T();
        auto* template_proxy = jluna::safe_call(new_proxy, _name->operator Any*());

        for (auto& field_name : _fieldnames_in_order)
            jluna::safe_call(setfield, template_proxy, std::get<0>(_mapping.at(field_name))(default_instance), field_name);

        _type = std::make_unique<Type>((jl_datatype_t*) jluna::safe_call(implement, template_proxy));
        _implemented = true;
        jl_gc_unpause;
    }

    template<typename T>
    Any* Usertype<T>::box(T& in)
    {
        if (not _enabled)
            throw UsertypeNotEnabledException<T>();

        if (not _implemented)
            implement();

        jl_gc_pause;
        static jl_function_t* setfield = jl_get_function(jl_base_module, "setfield!");

        Any* out = jl_call0(_type->operator Any*());

        for (auto& pair : _mapping)
            jluna::safe_call(setfield, out, pair.first, std::get<0>(pair.second)(in));

        jl_gc_unpause;
        return out;
    }

    template<typename T>
    T Usertype<T>::unbox(Any* in)
    {
        if (not _enabled)
            throw UsertypeNotEnabledException<T>();

        if (not _implemented)
            implement();

        jl_gc_pause;
        static jl_function_t* getfield = jl_get_function(jl_base_module, "getfield");

        auto out = T();

        for (auto& pair : _mapping)
            std::get<1>(pair.second)(out, jluna::safe_call(getfield, in, pair.first));

        jl_gc_unpause;
        return out;
    }

    template<IsUsertype T>
    T unbox(Any* in)
    {
        if (not Usertype<T>::is_enabled())
            throw UsertypeNotEnabledException<T>();

        return Usertype<T>::unbox(in);
    }

    template<IsUsertype T>
    Any* box(T in)
    {
        if (not Usertype<T>::is_enabled())
            throw UsertypeNotEnabledException<T>();

        return Usertype<T>::box(in);
    }
}