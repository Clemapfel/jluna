// 
// Copyright 2022 Clemens Cords
// Created on 25.02.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>

namespace jluna
{
    template<typename T>
    UserTypeNotFullyInitializedException<T>::UserTypeNotFullyInitializedException()
    {}
    
    template<typename T>
    const char * UserTypeNotFullyInitializedException<T>::what() const noexcept
    {
        std::stringstream str;
        str << "[C++][EXCEPTION] UserType interface for this type has not yet been fully specified, make sure that the following actions were performed:" << std::endl;
        str << "\ta) UserType<T>::set_name was used to specify the name" << std::endl;
        str << "\tb) UserType<T>::set_boxing_routine was used to implement the boxing routine" << std::endl;
        str << "\tc) UserType<T>::set_unboxing_routine was used to implement the unboxing routine" << std::endl;
        str << "\td) after all of the above, UserType<T>::implement was called exactly once" << std::endl;
        return str.str().c_str();
    }
    
    template<typename T>
    void UserType<T>::pre_initialize()
    {
        if (_pre_initialized)
            return;

        throw_if_uninitialized();
        jl_gc_pause;
        _template = Proxy(jl_eval_string("return jluna.usertype.new_usertype(:unitialized)"));
        jl_gc_unpause;

        _pre_initialized = true;
    }

    template<typename T>
    void UserType<T>::set_name(const std::string& name)
    {
        pre_initialize();
        jl_gc_pause;
        static jl_function_t* setfield = jl_get_function(jl_base_module, "setfield!");
        jluna::safe_call(setfield, _template, jl_symbol("_name"), jl_symbol(name.c_str()));
        jl_gc_unpause;
        _name_set = true;
    }

    template<typename T>
    std::string UserType<T>::get_name()
    {
        pre_initialize();
        return _template["_name"].operator std::string();
    }

    template<typename T>
    void UserType<T>::set_mutable(bool b)
    {
        pre_initialize();
        _template["_is_mutable"] = b;
    }

    template<typename T>
    bool UserType<T>::is_mutable()
    {
        pre_initialize();
        return _template["_is_mutable"];
    }

    template<typename T>
    void UserType<T>::add_field(const std::string& name, const std::string& type, std::function<Any*(T&)> box_get, std::function<void(T&, Any*)> unbox_set)
    {
        pre_initialize();
        _mapping.insert({name, {box_get, unbox_set, Symbol(type)}});
    }

    template<typename T>
    void UserType<T>::add_field(const std::string& name, Type type, std::function<Any*(T&)> box_get, std::function<void(T&, Any*)> unbox_set)
    {
        pre_initialize();
        _mapping.insert({name, {box_get, unbox_set, type.get_symbol()}});
    }

    template<typename T>
    Any* UserType<T>::box(T in)
    {
        if (not _implemented)
            throw UserTypeNotFullyInitializedException<T>();

        jl_gc_pause;
        static jl_function_t* add_field = jl_find_function("jluna.usertype", "add_field!");

        for (auto& pair : _mapping)
            jluna::safe_call(
                add_field,
                _template,
                jl_symbol(pair.first.c_str()),                  // name
                jl_symbol(std::get<2>(pair.second).c_str()),    // typename
                std::get<0>(pair.second)(in)                    // getted value
            );

        auto* out = jl_call1(_implemented_type, _template);
        forward_last_exception();
        jl_gc_unpause;
        return out;
    }

    template<typename T>
    T UserType<T>::unbox(Any* in)
    {
        if (not _implemented)
            throw UserTypeNotFullyInitializedException<T>();

        jl_gc_pause;
        static jl_function_t* getfield = jl_get_function(jl_base_module, "getfield");

        auto out = T(); //todo enforce this by concept

        for (auto pair : _mapping)
            std::get<1>(pair.second)(out, jluna::safe_call(getfield, in, jl_symbol(pair.first)));

        return out;
    }

    template<typename T>
    void UserType<T>::add_parameter(const std::string& name, Type upper_bound, Type lower_bound)
    {
        pre_initialize();
        jl_gc_pause;
        static jl_function_t* add_parameter = jl_find_function("jluna.usertype", "add_parameter!");
        jluna::safe_call(add_parameter,_template, jl_symbol(name.c_str()), upper_bound, lower_bound);
        jl_gc_unpause;
    }

    template<typename T>
    Type UserType<T>::implement(Module module)
    {
        pre_initialize();

        if (not is_initialized())
            throw UserTypeNotFullyInitializedException<T>();
        
        jl_gc_pause;
        static jl_function_t* add_field = jl_find_function("jluna.usertype", "add_field!");

        for (auto& pair : _mapping)
            jluna::safe_call(
                add_field,
                _template,
                jl_symbol(pair.first.c_str()),                  // name
                jl_symbol(std::get<2>(pair.second).c_str())     // typename
            );

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

    template<typename T>
    bool UserType<T>::is_initialized()
    {
        return _name_set;
    }
}