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

    /// @concept: is tuple but not pair
    //template<typename T>
    //concept IsTuple = std::tuple_size<T>::value != 2;

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
        not Is<T, bool> and
        not Is<T, std::bool_constant<true>> and
        not Is<T, std::bool_constant<false>> and
        not Is<T, char> and
        not Is<T, uint8_t> and
        not Is<T, uint16_t> and
        not Is<T, uint32_t> and
        not Is<T, uint64_t> and
        not Is<T, int8_t> and
        not Is<T, int16_t> and
        not Is<T, int32_t> and
        not Is<T, int64_t> and
        not Is<T, float> and
        not Is<T, double> and
        not Is<T, std::string> and
        not Is<T, const char*>;

    /// @concept is std::complex
    template<typename T, typename U = typename T::value_type>
    concept IsComplex = std::is_same_v<T, std::complex<U>>;

    /// @concept is std::vector
    template<typename T, typename U = typename T::value_type>
    concept IsVector = std::is_same_v<T, std::vector<U>>;

    /// @concept is map
    template<typename T, typename Key_t = typename T::key_type, typename Value_t = typename T::mapped_type>
    concept IsMap =
        std::is_same_v<T, std::map<Key_t, Value_t>> or
        std::is_same_v<T, std::multimap<Key_t, Value_t>> or
        std::is_same_v<T, std::unordered_map<Key_t, Value_t>>;

    /// @concept is std::set
    template<typename T, typename U = typename T::value_type>
    concept IsSet = std::is_same_v<T, std::set<U>>;

    /// @concept is pair
    template<typename T, typename T1 = typename T::first_type, typename T2 = typename T::second_type>
    concept IsPair = std::is_same_v<T, std::pair<T1, T2>> and std::tuple_size_v<T> == 2;

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


    /// @concept not unboxable out-of-the-box
    template<typename T>
    concept IsUsertype =
        not IsJuliaValuePointer<T> and
        not IsAnyPtrCastable<T> and
        not IsPrimitive<T> and
        not IsComplex<T> and
        not IsVector<T> and
        not IsMap<T> and
        not IsSet<T> and
        not IsPair<T> and
        not IsTuple<T>;
}