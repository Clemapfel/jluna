// 
// Copyright 2022 Clemens Cords
// Created on 25.02.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>

namespace jluna
{
    template<typename T>
    template<typename Lambda_t>
    UserType<T>::UserType(const std::string& name, Lambda_t lambda)
    {
        static jl_function_t* new_usertype = jl_find_function("jluna.usertype", "new_usertype");

        jl_gc_pause;
        _template = Proxy(jluna::safe_call(new_usertype, jl_symbol(name.c_str())), nullptr);
        jl_gc_unpause;
    }

    template<typename T>
    void UserType<T>::add_field(const std::string& name, Type type, Any* initial_value)
    {
        jl_gc_pause;
        static jl_function_t* add_field = jl_find_function("jluna.usertype", "add_field!");
        jluna::safe_call(add_field, _template, jl_symbol(name.c_str()), initial_value);
        jl_gc_unpause;
    }

    template<typename T>
    template<typename U, std::enable_if_t<not std::is_same_v<U, Any*>, Bool>>
    void UserType<T>::add_field(const std::string& name, Type type, U initial_value)
    {
        UserType<T>::add_field(name, type, box<U>(initial_value));
    }

    template<typename T>
    void UserType<T>::add_const_field(const std::string& name, Type type, Any* initial_value)
    {
        jl_gc_pause;
        static jl_function_t* add_const_field = jl_find_function("jluna.usertype", "add_const_field!");
        jluna::safe_call(add_const_field, _template, jl_symbol(name.c_str()), initial_value);
        jl_gc_unpause;
    }

    template<typename T>
    template<typename U, std::enable_if_t<not std::is_same_v<U, Any*>, Bool>>
    void UserType<T>::add_const_field(const std::string& name, Type type, U initial_value)
    {
        UserType<T>::add_const_field(name, type, box<U>(initial_value));
    }

    template<typename T>
    template<typename Lambda_t, std::enable_if_t<not std::is_same_v<Lambda_t, Any*>, Bool>>
    void UserType<T>::add_function(const std::string& name, Lambda_t lambda)
    {
        UserType<T>::add_function(name, box<Lambda_t>(lambda));
    }

    template<typename T>
    void UserType<T>::add_function(const std::string& name, Any* f)
    {
        jl_gc_pause;
        static jl_function_t* add_member_function = jl_find_function("jluna.usertype", "add_member_function!");
        jluna::safe_call(add_member_function, f);
        jl_gc_unpause;
    }

    template<typename T>
    void UserType<T>::add_parameter(const std::string& name, Type upper_bound, Type lower_bound)
    {
        jl_gc_pause;
        static jl_function_t* add_parameter = jl_find_function("jluna.usertype", "add_parameter!");
        jluna::safe_call(add_parameter, _template, jl_symbol(name.c_str()), upper_bound, lower_bound);
        jl_gc_unpause;
    }

    template<typename T>
    Type UserType<T>::implement(Module module)
    {
        jl_gc_pause;
        static jl_function_t* implement = jl_find_function("jluna.usertype", "implement");
        auto out = Type((jl_datatype_t*) jluna::safe_call(implement, _template, module));
        jl_gc_unpause;

        return out;
    }
}