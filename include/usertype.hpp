// 
// Copyright 2022 Clemens Cords
// Created on 22.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>

#include <include/type.hpp>
#include <include/proxy.hpp>

namespace jluna
{
    /// @brief customizable wrapper for non-julia type T
    /// @note for information on how to use this class, visit https://github.com/Clemapfel/jluna/blob/master/docs/manual.md#usertypes
    template<typename T>
    class UserType
    {
        static inline std::function<void(T&, Any*)> noop_set = [](T&, Any* ) {return;};

        public:
            /// @brief original type
            using original_type = T;

            /// @brief instance
            UserType(const std::string& name);

            /// @brief set julia-side name
            /// @param name
            static void set_name(const std::string& name);

            /// @brief get julia-side name
            /// @returns name
            static std::string get_name();

            /// @brief set mutability, no by default
            /// @param bool
            static void set_mutable(bool);

            /// @brief get mutability
            /// @returns bool
            static bool is_mutable();

            /// @brief add field
            static void add_field(
                const std::string& name,
                const std::string& type_name,
                std::function<Any*(T&)> box_get,
                std::function<void(T&, Any*)> unbox_set = noop_set
            );

            /// @brief add parameter
            /// @param name: e.g. T
            /// @param upper_bound: .ub of TypeVar, equivalent to T <: upper_bound
            /// @param lower_bound: .lb of TypeVar, equivalent to lower_bound <: T
            static void add_parameter(const std::string& name, Type upper_bound = Any_t, Type lower_bound = UnionEmpty_t);

            /// @brief push to state and eval, cannot be extended afterwards
            /// @param module: module the type will be set in
            /// @returns julia-side type
            static Type implement(Module module = Main);

            /// @brief is already implemented
            /// @brief true if implement was called, false otherwise
            static bool is_implemented();

            /// @brief box interface
            static Any* box(T);

            /// @brief unbox interface
            static T unbox(Any*);

        private:
            static inline Proxy _template = Proxy(jl_nothing);

            static inline bool _implemented = false;
            static inline Any* _implemented_type = nullptr;

            static inline std::map<std::string, std::tuple<
                std::string,                    // symbol of typename
                std::function<Any*(T&)>,        // getter
                std::function<void(T&, Any*)>   // setter
            >> _mapping = {};
    };

    /// @brief unbox using unboxing routine
    /// @param pointer
    /// @returns T
    template<typename T,
        typename U = typename T::original_type,
        typename std::enable_if_t<std::is_same_v<T, UserType<U>>, Bool> = true>
    T unbox(Any* in);

    /// @brief box using boxing routine
    /// @param
    /// @returns pointer to julia-side memory
    template<typename T,
        typename U = typename T::original_type,
        typename std::enable_if_t<std::is_same_v<T, UserType<U>>, Bool> = true>
    Any* box(T in);


}

#include ".src/usertype.inl"