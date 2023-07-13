// 
// Copyright 2022 Clemens Cords
// Created on 11.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>

#include <memory>
#include <deque>
#include <string>

#include <include/typedefs.hpp>
#include <include/box.hpp>
#include <include/unbox.hpp>

namespace jluna
{
    class Type;

    /// @brief holds ownership of julia-side value. mutating named proxies mutate the corresponding variable, c.f docs/manual.md
    class Proxy
    {
        protected: class ProxyValue;

        public:
            /// @brief default ctor
            Proxy();

            /// @brief construct with no owner, reserved for global temporaries and main
            /// @param value: value
            /// @param symbol: name, or nullptr for unnamed proxy
            Proxy(unsafe::Value* value, jl_sym_t* symbol = nullptr);

            /// @brief construct with owner
            /// @param value: value
            /// @param owner: shared pointer to owner
            /// @param symbol: name, or nullptr for unnamed proxy
            Proxy(unsafe::Value* value, std::shared_ptr<ProxyValue>& owner, unsafe::Value* name_or_index);

            /// @brief dtor
            ~Proxy();

            /// @brief access field
            /// @param field_name: name of the field
            /// @returns field as proxy
            Proxy operator[](const std::string& field);

            /// @brief access field
            /// @param field_name: name of the field as const char*
            /// @returns field as proxy
            template<typename T, std::enable_if_t<std::is_same_v<T, char>, Bool> = true>
            Proxy operator[](const T* field);

            /// @brief access field
            /// @param field_name: name of the field as string
            /// @returns unboxed value
            template<is_unboxable T>
            T operator[](const std::string& field);

            /// @brief access field
            /// @param field_name:  name of the field as const char*
            /// @returns unboxed value
            template<is_unboxable T, typename U, std::enable_if_t<std::is_same_v<U, char>, Bool> = true>
            T operator[](const U* field);

            /// @brief linear indexing, if array type, returns getindex result
            /// @param index: index, 0-based
            /// @returns field as proxy
            Proxy operator[](uint64_t);

            /// @brief linear indexing, if array type returns getindex result
            /// @param index: index, 0-based
            /// @returns field as proxy
            template<is_unboxable T>
            T operator[](uint64_t);

            /// @brief cast to Any
            explicit operator unsafe::Value*();

            /// @brief cast to const Any
            explicit operator const unsafe::Value*() const;

            /// @brief cast to string using Julia's Base.string
            virtual operator std::string() const;

            /// @brief implicitly convert to T via unboxing
            /// @returns value as T
            template<is_unboxable T, std::enable_if_t<not std::is_same_v<T, std::string>, bool> = true>
            operator T() const;

            /// @brief implicitly downcast to base
            /// @returns value as T
            template<typename T, std::enable_if_t<std::is_base_of_v<Proxy, T>, bool> = true>
            operator T();

            /// @brief equivalent alternative to explicit operator T
            /// @returns value as T
            template<typename T, std::enable_if_t<std::is_base_of_v<Proxy, T>, bool> = true>
            T as();

            /// @brief get variable name, if any
            /// @returns name as string
            std::string get_name() const;

            /// @brief if proxy is not a type value, get the fields of the proxies types. If proxy is already a type, get fieldnames of itself
            /// @returns vector of strings
            std::vector<std::string> get_field_names() const;

            /// @brief get type
            /// @returns proxy to singleton type
            Type get_type() const;

            /// @brief check if this <: type
            /// @param type: type
            /// @returns true if `*this isa type`, false otherwise
            bool isa(const Type& type);

            /// @brief call with any arguments
            /// @tparams Args_t: types of arguments, need to be boxable
            /// @returns proxy
            template<is_boxable... Args_t>
            Proxy safe_call(Args_t&&...);

            /// @brief call and return value, does not construct proxy
            /// @tparams T: return value
            /// @tparams Args_t: types of arguments
            /// @returns value
            template<typename T, is_boxable... Args_t, std::enable_if_t<not std::is_void_v<T> and not is<Proxy, T>, bool> = true>
            T safe_call(Args_t&&... args);

            /// @brief call and return value, does not construct proxy
            /// @tparams Args_t: types of arguments
            /// @returns nothing
            template<typename T, is_boxable... Args_t, std::enable_if_t<std::is_void_v<T> and not is<Proxy, T>, bool> = true>
            T safe_call(Args_t&&... args);

            /// @brief call with arguments and exception forwarding, if proxy is a callable function
            /// @tparams Args_t: types of arguments, need to be boxable
            /// @returns proxy
            template<is_boxable... Args_t>
            Proxy operator()(Args_t&&...);

            /// @brief check whether assigning to this proxy will modify values julia-side
            /// @returns true if set as mutating and neither an immutable type, singleton or const variable
            bool is_mutating() const;

            /// @brief assign value to proxy, this modifies the value julia-side
            /// @param value: Julia-side value
            /// @returns reference to self
            Proxy& operator=(unsafe::Value*);

            /// @brief assign value to proxy, this modifies the value julia-side
            /// @param T: value
            /// @returns reference to self
            template<is_boxable T>
            Proxy& operator=(T);

            /// @brief create a new unnamed proxy that holds the same value
            /// @returns new proxy by valuejluna: Manual & Tut
            [[nodiscard]] Proxy as_unnamed() const;

            /// @brief update value if proxy symbol was reassigned outside of operator=
            void update();

        protected:
            std::shared_ptr<ProxyValue> _content;
    };

    /// @brief internal proxy value, not intended to be interfaced with by end-users
    class Proxy::ProxyValue
    {
        friend class Proxy;

        public:
            /// @brief dtor
            ~ProxyValue();

            /// @brief get value
            /// @returns pointer to value
            unsafe::Value* value() const;

            /// @brief get id
            /// @returns pointer to jluna.memory_handler.ProxyID
            unsafe::Value* id() const;

        protected:
            /// @brief ctor without owner
            /// @param value: pointer to value
            /// @param id: jluna.memory_handler.ProxyID object
            ProxyValue(unsafe::Value* value, jl_sym_t* id);

            /// @brief ctor with owner and proper name
            /// @param value: pointer to value
            /// @param id: jluna.memory_handler.ProxyID object
            ProxyValue(unsafe::Value* value, std::shared_ptr<ProxyValue>& owner, unsafe::Value* symbol_or_index);

            /// @brief copy ctor
            /// @param other: other proxy
            ProxyValue(const ProxyValue&);

            /// @brief access field
            /// @param symbol: name of field
            /// @returns pointer to field data
            unsafe::Value* get_field(jl_sym_t*) const;

            /// @brief owner
            std::shared_ptr<ProxyValue> _owner;

            /// @brief points to julia-side variable
            const bool _is_mutating = true;

            uint64_t* _id_key = new uint64_t(0);
            uint64_t* _value_key = new uint64_t(0);

            mutable unsafe::Value* _id_ref;
            mutable unsafe::Value* _value_ref;
    };
}

#include <.src/proxy.inl>
