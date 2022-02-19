// 
// Copyright 2022 Clemens Cords
// Created on 11.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>

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
            /// @param value
            /// @param symbol
            Proxy(Any* value, jl_sym_t* symbol = nullptr);

            /// @brief construct with owner
            /// @param value
            /// @param owner: shared pointer to owner
            /// @param symbol
            Proxy(Any* value, std::shared_ptr<ProxyValue>& owner, Any* name_or_index);

            /// @brief dtor
            ~Proxy();

            /// @brief access field
            /// @param field_name
            /// @returns field as proxy
            Proxy operator[](const std::string& field);

            /// @brief access field
            /// @param field_name
            /// @returns unboxed value
            template<Unboxable T>
            T operator[](const std::string& field);

            /// @brief linear indexing, if array type, returns getindex result
            /// @param index
            /// @returns field as proxy
            Proxy operator[](size_t);

            /// @brief linear indexing, if array type returns getindex result
            /// @param index
            /// @returns field as proxy
            template<Unboxable T>
            T operator[](size_t);

            /// @brief cast to Any
            operator Any*();

            /// @brief cast to const Any
            operator const Any*() const;

            /// @brief cast to string using julias Base.string
            virtual operator std::string() const;

            /// @brief implicitly convert to T via unboxing
            /// @returns value as T
            template<Unboxable T, std::enable_if_t<not std::is_same_v<T, std::string>, bool> = true>
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

            /// @brief if proxy is a value, get fieldnames of typeof(value), if proxy is a type, get fieldnames of itself
            /// @returns vector of strings
            std::vector<std::string> get_field_names() const;

            /// @brief get type
            /// @returns proxy to singleton type
            Type get_type() const;

            /// @brief call with any arguments
            /// @tparams Args_t: types of arguments, need to be boxable
            template<Boxable... Args_t>
            Proxy call(Args_t&&...);

            /// @brief call with any arguments
            /// @tparams Args_t: types of arguments, need to be boxable
            template<Boxable... Args_t>
            Proxy safe_call(Args_t&&...);

            /// call with arguments and exception forwarding, if proxy is a callable function
            /// @tparams Args_t: types of arguments, need to be boxable
            template<Boxable... Args_t>
            Proxy operator()(Args_t&&...);

            /// @brief check whether assigning to this proxy will modify values julia-side
            /// @returns true if set as mutating and neither an immutable type, singleton or const variable
            bool is_mutating() const;

            /// @brief assign value to proxy, this modifies the value julia-side
            /// @param Any*
            /// @returns reference to self
            Proxy& operator=(Any*);

            /// @brief assign value to proxy, this modifies the value julia-side
            /// @param T: value
            /// @returns reference to self
            template<Boxable T>
            Proxy& operator=(T);

            /// @brief create a new unnamed proxy that holds the same value
            /// @returns new proxy by value
            [[nodiscard]] Proxy as_unnamed() const;

            /// @brief update value if proxy symbol was reassigned outside of operator=
            void update();

            /// @brief check if this <: type
            /// @param type
            /// @returns true if `*this isa type`, false otherwise
            bool isa(const Type& type);

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
            Any* value() const;

            /// @brief get id
            /// @returns pointer to jluna.memory_handler.ProxyID
            Any* id() const;

        protected:
            /// @brief ctor without owner
            /// @param value: pointer to value
            /// @param id: jluna.memory_handler.ProxyID object
            ProxyValue(Any* value, jl_sym_t* id);

            /// @brief ctor with owner and proper name
            /// @param value: pointer to value
            /// @param id: jluna.memory_handler.ProxyID object
            ProxyValue(Any* value, std::shared_ptr<ProxyValue>& owner, Any* symbol_or_index);

            ProxyValue(const ProxyValue&);

            /// @brief access field
            /// @param symbol: name of field
            /// @returns pointer to field data
            Any* get_field(jl_sym_t*);

            /// @brief owner
            std::shared_ptr<ProxyValue> _owner;

            /// @brief points to julia-side variable
            const bool _is_mutating = true;

            size_t _id_key;
            size_t _value_key;

            mutable Any* _id_ref;
            mutable Any* _value_ref;
    };
}

#include ".src/proxy.inl"