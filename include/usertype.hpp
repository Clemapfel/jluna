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
    class Usertype
    {
        static inline std::function<void(T&, Any*)> noop_set = [](T&, Any* ) {return;};

        public:
            /// @brief original type
            using original_type = T;

            Usertype() = delete;

            /// @brief enable interface
            /// @param name
            static void enable(const std::string&);

            /// @brief is enabled
            /// @returns bool
            static bool is_enabled();

            /// @brief add field
            /// @param name: julia-side name of field
            /// @param type: type of symbol. User the other overload if the type is a typevar, such as "P" (where P is a parameter)
            /// @param box_get: lambda with signature (T&) -> Any*
            /// @param unbox_set: lambda with signature (T&, Any*)
            /// @note this function will throw if called after implement()
            template<typename Field_t>
            static void add_property(
                const std::string& name,
                std::function<Field_t(T&)> box_get,
                std::function<void(T&, Field_t)> unbox_set = noop_set
            );

            /// @brief box interface
            static Any* box(T&);

            /// @brief unbox interface
            static T unbox(Any*);

        private:
            static inline bool _enabled = false;

            static inline std::unique_ptr<Symbol> _name = std::unique_ptr<Symbol>(nullptr);
            static inline std::map<Symbol, std::tuple<
                std::function<Any*(T&)>,        // getter
                std::function<void(T&, Any*)>,   // setter
                Type
            >> _mapping = {};
    };

    /// @brief exception thrown when usertype is used before being implemented
    template<typename T>
    struct UsertypeNotEnabledException : public std::exception
    {
        public:
            /// @brief ctor
            UsertypeNotEnabledException();

            /// @brief what
            /// @returns message
            const char* what() const noexcept override final;

        private:
            std::string _msg;
    };
}

#include ".src/usertype.inl"