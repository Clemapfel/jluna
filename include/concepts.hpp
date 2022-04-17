// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>
#include <type_traits>
#include <tuple>
#include <utility>
#include <functional>
#include <iostream>

#include <include/typedefs.hpp>

namespace jluna
{

    template<class T, class U>
    struct is_same_or_const
    {
        static constexpr bool value = false;
    };

    template<class T>
    struct is_same_or_const<T, T>
    {
        static constexpr bool value = true;
    };

    template<class T>
    struct is_same_or_const<T, const T>
    {
        static constexpr bool value = true;
    };

    /// @concept: wrapper for std::is_same_v
    template<typename T, typename U>
    concept is = std::conditional_t<
        std::is_void_v<T>,
        std::is_void<U>,
        is_same_or_const<T, U>
    >::value;

    template<typename T, typename U>
    concept is_not = not is<T, U>;

    /// @concept: has default ctor
    template<typename T>
    concept is_default_constructible = requires(T)
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

    /// @brief introspect functions
    template<typename T>
    struct function_traits;

    template<typename Return_t, typename... Args_t>
    struct function_traits<Return_t(Args_t...)>
    {
        // as C literal function
        using as_c_function = Return_t(Args_t...);

        // as std::function
        using as_std_function = std::function<Return_t(Args_t...)>;

        // return type
        using return_t = typename as_std_function::result_type;

        // number of arguments
        static constexpr size_t n_args = sizeof...(Args_t);

        // all argument types as tuple type
        using argument_ts = std::tuple<Args_t...>;

        // type of i-th argument
        template<size_t i>
        using argument_type = std::tuple_element_t<i, argument_ts>;
    };

    /// @brief forward function or lambda as C-literal function
    template<typename T>
    struct forward_as_function;

    template<typename Return_t, typename... Args_t>
    struct forward_as_function<Return_t(Args_t...)>
    {
        using value = Return_t(Args_t...);
    };

    template<typename Return_t, typename... Args_t>
    struct forward_as_function<std::function<Return_t(Args_t...)>>
    {
        using value = Return_t(Args_t...);
    };

    template<typename T>
    using forward_as_function_v = typename forward_as_function<T>::value;

    /// @concept: check signature of argument function or lambda
    template<typename T, typename Return_t, typename... Args_t>
    concept is_function_with_signature =
    std::is_invocable_v<forward_as_function_v<T>, Args_t...> and
    std::conditional_t<
        std::is_void_v<Return_t>,
        std::is_void<typename function_traits<forward_as_function_v<T>>::return_t>,
        std::is_same<typename function_traits<forward_as_function_v<T>>::return_t, Return_t>
    >::value;

    /// @concept: function with n args
    template<typename T, size_t N>
    concept is_function_with_n_args = (function_traits<T>::n_args == N);

    /// @concept: can be reinterpret-cast to jl_value_t*
    template<typename T>
    concept is_julia_value_pointer =
        std::is_same_v<T, jl_value_t*> or
        std::is_same_v<T, jl_module_t*> or
        std::is_same_v<T, jl_array_t*> or
        std::is_same_v<T, jl_datatype_t*> or
        std::is_same_v<T, jl_function_t*> or
        std::is_same_v<T, jl_sym_t*> or
        std::is_same_v<T, jl_expr_t*> or
        std::is_same_v<T, jl_unionall_t*>;

    template<typename T>
    concept is_julia_value =
        std::is_same_v<T, jl_value_t> or
        std::is_same_v<T, jl_module_t> or
        std::is_same_v<T, jl_array_t> or
        std::is_same_v<T, jl_datatype_t> or
        std::is_same_v<T, jl_function_t> or
        std::is_same_v<T, jl_sym_t> or
        std::is_same_v<T, jl_expr_t> or
        std::is_same_v<T, jl_unionall_t>;


    /// @concept is primitive
    template<typename T>
    concept is_primitive =
        is<T, bool> or
        is<T, std::bool_constant<true>> or
        is<T, std::bool_constant<false>> or
        is<T, char> or
        is<T, uint8_t> or
        is<T, uint16_t> or
        is<T, uint32_t> or
        is<T, uint64_t> or
        is<T, int8_t> or
        is<T, int16_t> or
        is<T, int32_t> or
        is<T, int64_t> or
        is<T, float> or
        is<T, double> or
        is<T, std::string> or
        is<T, const char*>;

    /// @concept is std::complex
    template<typename T>
    concept is_complex = requires(T t)
    {
        typename T::value_type;
        std::is_same_v<T, std::complex<typename T::value_type>>;
    };

    /// @concept is std::vector
    template<typename T>
    concept is_vector = requires (T t)
    {
        typename T::value_type;
        std::is_same_v<T, std::vector<typename T::value_type>>;
    };

    /// @concept is map
    template<typename T>
    concept is_map = requires(T t)
    {
        typename T::key_type;
        typename T::mapped_type;
        std::is_same_v<T, std::map<typename T::key_type, typename T::mapped_Type>> or
        std::is_same_v<T, std::unordered_map<typename T::key_type, typename T::mapped_Type>> or
        std::is_same_v<T, std::multimap<typename T::key_type, typename T::mapped_Type>>;
    };

    /// @concept is std::set
    template<typename T>
    concept is_set = requires(T t)
    {
        typename T::value_type;
        std::is_same_v<T, std::set<typename T::value_type>>;
    };

    /// @concept is pair
    template<typename T>
    concept is_pair = requires(T)
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
    concept is_tuple = detail::is_tuple_aux<T>(std::make_index_sequence<std::tuple_size<T>::value>());

    /// @concept should be resolved as usertype
    template<typename T>
    struct usertype_enabled
    {
        constexpr static inline const char* name = "<NO_USERTYPE_NAME_SPECIFIED>";
        constexpr static inline bool value = false;
    };

    template<typename T>
    concept is_usertype = usertype_enabled<T>::value;
}