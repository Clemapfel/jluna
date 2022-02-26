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
    template<typename>
    struct UserTypeAlreadyImplementedException;

    template<typename>
    struct UserTypeNotFullyInitializedException;

    /// @brief customizable wrapper for non-julia type T
    /// @note for information on how to use this class, visit https://github.com/Clemapfel/jluna/blob/master/docs/manual.md#usertypes
    template<typename T>
    class UserType
    {
        public:
            /// @brief original type
            using original_type = T;

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
            /// @param name of field
            /// @param type of field
            /// @param initial value
            template<Boxable Value_t>
            static void add_field(const std::string& name, Value_t initial_value);

            /// @brief add parameter
            /// @param name: e.g. T
            /// @param upper_bound: .ub of TypeVar, equivalent to T <: upper_bound
            /// @param lower_bound: .lb of TypeVar, equivalent to lower_bound <: T
            static void add_parameter(const std::string& name, Type upper_bound = Any_t, Type lower_bound = UnionEmpty_t);

            /// @brief boxing routine called during box<UserType<T>>
            /// @param: lambda
            /// @note c.f. https://github.com/Clemapfel/jluna/blob/master/docs/manual.md#usertypes
            static void set_boxing_routine(std::function<Any*(T)> lambda);

            /// @brief boxing routine called during unbox<UserType<T>>
            /// @param: lambda
            /// @note c.f. https://github.com/Clemapfel/jluna/blob/master/docs/manual.md#usertypes
            static void set_unboxing_routine(std::function<T(Any*)> lambda);

            /// @brief push to state and eval, cannot be extended afterwards
            /// @param module: module the type will be set in
            /// @returns julia-side type
            static Type implement(Module module = Main);

            /// @brief is already implemented
            /// @brief true if implement was called, false otherwise
            static bool is_implemented();

            /// @brief is fully initialized
            /// @returns true if boxing, unboxing and name was set, false otherwise
            static bool is_initialized();

            /// @brief no ctor
            UserType() = delete;

            /// @brief instance this type using an original
            /// @param cpp_side_type
            /// @returns proxy to new instance
            static Proxy create_instance(T);

        private:
            static void pre_initialize();

            static inline bool _pre_initialized = false;
            static inline Proxy _template = Proxy(jl_nothing);

            static std::function<Any*(T)> _boxing_routine;
            static inline bool _boxing_routine_set = false;

            static std::function<T(Any*)> _unboxing_routine;
            static inline bool _unboxing_routine_set = false;

            bool _name_set = false;
            bool _implemented = false;

    };

    /// @brief unbox using unboxing routine
    /// @param pointer
    /// @returns T
    template<typename T,
        typename U = typename T::original_type,
        typename std::enable_if_t<std::is_same_v<T, UserType<U>>, Bool> = true>
    inline T unbox(Any* in)
    {
        if(not T::is_implemented() or not T::is_initialized())
            throw UserTypeNotFullyInitializedException<T>();

        return T::unboxing_routine(in);
    }

    /// @brief box using boxing routine
    /// @param
    /// @returns pointer to julia-side memory
    template<typename T,
        typename U = typename T::original_type,
        typename std::enable_if_t<std::is_same_v<T, UserType<U>>, Bool> = true>
    inline Any* box(T in)
    {
        if(not T::is_implemented() or not T::is_initialized())
            throw UserTypeNotFullyInitializedException<T>();

        return T::boxing_routine(in);
    }

    template<typename T>
    struct UserTypeNotFullyInitializedException : public std::exception
    {
        /// @brief ctor
        /// @param name
        UserTypeNotFullyInitializedException();

        /// @brief what
        /// @returns message
        virtual const char* what() const noexcept override final;
    };
}

#include ".src/usertype.inl"