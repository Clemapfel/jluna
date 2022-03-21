// 
// Copyright 2022 Clemens Cords
// Created on 22.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>

#include <include/type.hpp>
#include <include/proxy.hpp>

namespace jluna
{
    /// @brief declare T to be a usertype at compile time, uses C++-side name as Julia-side typename
    /// @param T: type
    #define set_usertype_enabled(T) template<> struct jluna::usertype_enabled<T> {static_assert(IsDefaultConstructible<T>, "types managed by Usertype<T> need to be default constructable"); constexpr static inline const char* name = #T; constexpr static inline bool value = true;};

    /// @brief customizable wrapper for non-julia type T
    /// @note for information on how to use this class, visit https://github.com/Clemapfel/jluna/blob/master/docs/manual.md#usertypes
    template<typename T>
    class Usertype
    {
        template<typename U>
        static inline std::function<void(T&, U)> noop_set = [](T&, U) {return;};

        public:
            /// @brief original type
            using original_type = T;

            /// @brief ctor delete, static-only interface
            Usertype() = delete;

            /// @brief was enabled at compile time using set_usertype_enabled
            /// @returns bool
            static bool is_enabled();

            /// @brief get julia-side name
            /// @returns name
            static std::string get_name();

            /// @brief add field
            /// @param name: julia-side name of field
            /// @param box_get: lambda with signature (T&) -> unsafe::Value*
            /// @param unbox_set: lambda with signature (T&, unsafe::Value*) -> void
            template<typename Field_t>
            static void add_property(
                const std::string& name,
                std::function<Field_t(T&)> box_get,
                std::function<void(T&, Field_t)> unbox_set = noop_set<Field_t>
            );

            /// @brief create the type, setup through the interface, julia-side
            /// @param module: module in which the type is evaluated
            static void implement(Module module = Main);

            /// @brief has implement() been called at least once
            /// @returns bool
            static bool is_implemented();

            /// @brief box interface
            /// @param T&: instance
            /// @returns boxed value
            /// @note this function will call implement() if it has not been called before, incurring a tremendous overhead on first execution, once
            static unsafe::Value* box(T&);

            /// @brief box interface
            /// @param unsafe::Value*
            /// @returns unboxed value
            /// @note this function will call implement() if it has not been called before, incurring a tremendous overhead on first execution, once
            static T unbox(unsafe::Value*);

        private:
            static void initialize();
            static inline bool _implemented = false;

            static inline std::unique_ptr<Type> _type = std::unique_ptr<Type>(nullptr);
            static inline std::unique_ptr<Symbol> _name = std::unique_ptr<Symbol>(nullptr);

            static inline std::vector<Symbol> _fieldnames_in_order = {};
            static inline std::map<Symbol, std::tuple<
                std::function<unsafe::Value*(T&)>,        // getter
                std::function<void(T&, unsafe::Value*)>,   // setter
                Type
            >> _mapping = {};
    };
}

#include <.src/usertype.inl>