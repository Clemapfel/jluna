// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>
#include <type_traits>
#include <tuple>
#include <utility>
#include <functional>
#include <iostream>

namespace jluna
{
    /// @concept: wrapper for std::is_same_v
    template<typename T, typename U>
    concept Is = std::is_same<T, U>::value or std::is_same_v<T, const U>;

    /// @concept: has default ctor
    template<typename T>
    concept IsDefaultConstructible = requires(T)
    {
        {T()};
    };

    /// @concept: is iterable
    template<typename T>
    concept Iterable = requires(T t)
    {
        {t.begin()};
        {t.end()};
        typename T::value_type;
    };

    class Proxy;

    /// @concept describes lambda with signature (Args_t...) -> T
    template<typename T, typename... Args_t>
    concept LambdaType = std::is_invocable<T, Args_t...>::value and not std::is_base_of<Proxy, T>::value;

    /// @concept: can be reinterpret-cast to jl_value_t*
    template<typename T>
    concept IsJuliaValuePointer =
        std::is_same_v<T, jl_value_t*> or
        std::is_same_v<T, jl_module_t*> or
        std::is_same_v<T, jl_function_t*> or
        std::is_same_v<T, jl_sym_t*>;

    template<typename T>
    concept IsAnyPtrCastable = requires(T t)
    {
        static_cast<jl_value_t*>(t);
    };

    /// @concept is primitive
    template<typename T>
    concept IsPrimitive =
        Is<T, bool> or
        Is<T, std::bool_constant<true>> or
        Is<T, std::bool_constant<false>> or
        Is<T, char> or
        Is<T, uint8_t> or
        Is<T, uint16_t> or
        Is<T, uint32_t> or
        Is<T, uint64_t> or
        Is<T, int8_t> or
        Is<T, int16_t> or
        Is<T, int32_t> or
        Is<T, int64_t> or
        Is<T, float> or
        Is<T, double> or
        Is<T, std::string> or
        Is<T, const char*>;

    /// @concept is std::complex
    template<typename T>
    concept IsComplex = requires(T t)
    {
        typename T::value_type;
        std::is_same_v<T, std::complex<typename T::value_type>>;
    };

    /// @concept is std::vector
    template<typename T>
    concept IsVector = requires (T t)
    {
        typename T::value_type;
        std::is_same_v<T, std::vector<typename T::value_type>>;
    };

    /// @concept is map
    template<typename T>
    concept IsMap = requires(T t)
    {
        typename T::key_type;
        typename T::mapped_type;
        std::is_same_v<T, std::map<typename T::key_type, typename T::mapped_Type>> or
        std::is_same_v<T, std::unordered_map<typename T::key_type, typename T::mapped_Type>> or
        std::is_same_v<T, std::multimap<typename T::key_type, typename T::mapped_Type>>;
    };

    /// @concept is std::set
    template<typename T>
    concept IsSet = requires(T t)
    {
        typename T::value_type;
        std::is_same_v<T, std::set<typename T::value_type>>;
    };

    /// @concept is pair
    template<typename T>
    concept IsPair = requires(T)
    {
        std::is_same_v<T, std::pair<typename T::first_type, typename T::second_type>>;
    };

    /// @concept is tuple
    namespace detail
    {
        template<typename T, size_t... Ns>
        constexpr bool is_tuple_aux(std::index_sequence<Ns...> _)
        {
            return std::is_same_v<T, std::tuple<std::tuple_element_t<Ns, T>...>>;
        }
    }
    template<typename T>
    concept IsTuple = detail::is_tuple_aux<T>(std::make_index_sequence<std::tuple_size<T>::value>());

    /// @concept is none of the above
    template<typename T>
    concept IsUsertype =
        not IsJuliaValuePointer<T>
        and not IsAnyPtrCastable<T>
        and not IsPrimitive<T>
        and not IsComplex<T>
        and not IsVector<T>
        and not IsSet<T>
        and not IsMap<T>
        and not IsPair<T>
        and not IsTuple<T>;
}