// 
// Copyright 2022 Clemens Cords
// Created on 25.02.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>

namespace jluna
{
    template<typename T>
    UsertypeNotFullyInitializedException<T>::UsertypeNotFullyInitializedException()
    {
        std::stringstream str;
        str << "\n[C++][EXCEPTION] Usertype interface for this type T has not yet been fully implemented, make sure the following are true:" << std::endl;
        str << "\t1) T is default-constructible and a an implementation is available at compile time" << std::endl;
        str << "\t2) Usertype<T>::enable(\"<name of usertype here>\") was used to instance the usertype interface" << std::endl;
        str << "\t3) Usertype<T>::implement() was called, after which the interface cannot be extended" << std::endl;
        str << "\nIf all of the above are true, T is now (un)boxable and to_julia_type<T>::type_name is defined." << std::endl;
        str << "(for more information, visit: https://github.com/Clemapfel/jluna/blob/master/docs/manual.md#usertypes)" << std::endl;
        _msg = str.str();
    }

    template<typename T>
    const char * UsertypeNotFullyInitializedException<T>::what() const noexcept
    {
        return _msg.c_str();
    }

    template<typename T>
    UsertypeAlreadyImplementedException<T>::UsertypeAlreadyImplementedException()
    {
        std::stringstream str;
        str << "[C++][EXCEPTION] Usertype<T>::implement() was already called once for this type. It cannot be extended afterwards." << std::endl;
        _msg = str.str();
    }

    template<typename T>
    const char * UsertypeAlreadyImplementedException<T>::what() const noexcept
    {
        return _msg.c_str();
    }

    template<typename T>
    void Usertype<T>::enable(const std::string& name)
    {
        if (_implemented)
            throw UsertypeAlreadyImplementedException<T>();

        jl_gc_pause;
        static jl_function_t* new_usertype = jl_find_function("jluna.usertype", "new_usertype");
        _template = Proxy(jluna::safe_call(new_usertype, jl_symbol(name.c_str())));
        detail::to_julia_type_aux<Usertype<T>>::type_name = name;
        jl_gc_unpause;
    }

    template<typename T>
    std::string Usertype<T>::get_name()
    {
        return _template["_name"].operator std::string();
    }

    template<typename T>
    void Usertype<T>::set_mutable(bool b)
    {
        if (_implemented)
            throw UsertypeAlreadyImplementedException<T>();

        _template["_is_mutable"] = b;
    }

    template<typename T>
    bool Usertype<T>::is_mutable()
    {
        return _template["_is_mutable"];
    }

    template<typename T>
    void Usertype<T>::add_field(const std::string& name, const Type& type, std::function<Any*(T&)> box_get, std::function<void(T&, Any*)> unbox_set)
    {
        if (_implemented)
            throw UsertypeAlreadyImplementedException<T>();

        jl_gc_pause;
        static jl_function_t* add_field = jl_find_function("jluna.usertype", "add_field!");
        auto res = (*(_mapping.insert({name, {type, box_get, unbox_set}})).first);
        jluna::safe_call(add_field, _template, jl_symbol(name.c_str()), std::get<0>(res.second).operator jl_datatype_t*());
        jl_gc_unpause;
    }

    template<typename T>
    Any* Usertype<T>::box(T& in)
    {
        if (not is_implemented())
            throw UsertypeNotFullyInitializedException<T>();

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
    T Usertype<T>::unbox(Any* in)
    {
        if (not is_implemented())
            throw UsertypeNotFullyInitializedException<T>();

        jl_gc_pause;
        static jl_function_t* getfield = jl_get_function(jl_base_module, "getfield");

        auto out = T();

        for (auto pair : _mapping)
            std::get<2>(pair.second)(out, jluna::safe_call(getfield, in, jl_symbol(pair.first.c_str())));

        jl_gc_unpause;
        return out;
    }

    template<typename T>
    Type Usertype<T>::implement(Module module)
    {
        if (_implemented)
            throw UsertypeAlreadyImplementedException<T>();

        jl_gc_pause;
        static jl_function_t* implement = jl_find_function("jluna.usertype", "implement");
        _implemented_type = jluna::safe_call(implement, _template, module);
        _implemented = true;
        jl_gc_unpause;

        return Type((jl_datatype_t*) _implemented_type);
    }

    template<typename T>
    bool Usertype<T>::is_implemented()
    {
        return _implemented;
    }

    template<IsUsertype T>
    T unbox(Any* in)
    {
        if (not Usertype<T>::is_implemented())
            throw UsertypeNotFullyInitializedException<T>();

        return Usertype<T>::unbox(in);
    }

    template<IsUsertype T>
    Any* box(T in)
    {
        if (not Usertype<T>::is_implemented())
            throw UsertypeNotFullyInitializedException<T>();

        return Usertype<T>::box(in);
    }

    namespace detail
    {
        template<typename T>
        struct to_julia_type_aux<Usertype<T>>
        {
            // usertype name is only available, after Usertype<T>::Usertype(T) was called at least once
            static inline std::string type_name = "<USERTYPE_NAME_UNINITIALIZED>";
        };
    }
}