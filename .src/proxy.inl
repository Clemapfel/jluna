// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <include/unsafe_utilities.hpp>

namespace jluna
{
    /// @brief unbox to proxy
    template<is<Proxy> T>
    inline T unbox(unsafe::Value* value)
    {
        return Proxy(value, nullptr);
    }

    /// @brief box jluna::Proxy to Base.Any
    template<is<Proxy> T>
    inline unsafe::Value* box(T value)
    {
        return value.operator unsafe::Value*();
    }

    /// @brief type deduction
    template<>
    struct detail::as_julia_type_aux<Proxy>
    {
        static inline const std::string type_name = "Any";
    };

    template<is_unboxable T>
    T Proxy::operator[](size_t i)
    {
        static jl_function_t* getindex = jl_get_function(jl_base_module, "getindex");
        return unbox<T>(jluna::safe_call(getindex, _content->value(), box(i + 1)));
    }

    template<is_unboxable T, std::enable_if_t<not std::is_same_v<T, std::string>, bool>>
    Proxy::operator T() const
    {
        return unbox<T>(_content->value());
    }

    template<typename T, std::enable_if_t<std::is_base_of_v<Proxy, T>, bool>>
    Proxy::operator T()
    {
        return as<T>();
    }

    template<typename T, std::enable_if_t<std::is_base_of_v<Proxy, T>, bool>>
    T Proxy::as()
    {
        return T(this);
    }

    template<is_boxable T>
    Proxy & Proxy::operator=(T value)
    {
        gc_pause;
        return this->operator=(box(value));
        gc_unpause;
    }

    template<is_unboxable T>
    T Proxy::operator[](const std::string& field)
    {
        return operator[](field.c_str());
    }

    template<typename T, std::enable_if_t<std::is_same_v<T, char>, Bool>>
    Proxy Proxy::operator[](const T* field)
    {
        jl_sym_t* symbol = jl_symbol(field);
        return Proxy(
            _content.get()->get_field(symbol),
            _content,
            (unsafe::Value*) symbol
        );
    }

    template<is_unboxable T, typename U, std::enable_if_t<std::is_same_v<U, char>, Bool>>
    T Proxy::operator[](const U* field)
    {
        return unbox<T>(_content.get()->get_field(jl_symbol(field)));
    }

    template<is_boxable... Args_t>
    Proxy Proxy::safe_call(Args_t&&... args)
    {
        static jl_module_t* jluna_module = (jl_module_t*) jl_eval_string("return jluna");
        static jl_function_t* invoke = jl_get_function(jluna_module, "invoke");

        return Proxy(jluna::safe_call(invoke, _content->value(), box(args)...), nullptr);
    }

    template<is_boxable... Args_t>
    Proxy Proxy::operator()(Args_t&&... args)
    {
        return this->safe_call(args...);
    }
}