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
    /// @brief holds ownership of julia-side value, if attached to a symbol, can be made mutable so it also modifies a julia-side variable
    class Proxy
    {
        protected: class ProxyValue;

        public:
            Proxy() = default;

            /// @brief construct with no owner, reserved for global temporaries and main
            /// @param value
            /// @param symbol
            Proxy(Any* value, Symbol* symbol = nullptr);

            /// @brief construct with no owner, reserved for global temporaries and main
            /// @param value
            /// @param owner: shared pointer to owner, get's incremented
            /// @param symbol
            Proxy(Any* value, std::shared_ptr<ProxyValue>& owner, Symbol* symbol);

            /// @brief dtor
            ~Proxy() = default;

            /// @brief access field
            /// @param field_name
            /// @returns field as proxy
            virtual Proxy operator[](const std::string& field);

            /// @brief access field
            /// @param field_name
            /// @returns unboxed value
            template<Unboxable T>
            T operator[](const std::string& field);

            /// @brief access via linear index, if array type returns getindex! result
            /// @param index
            /// @returns field as proxy
            Proxy operator[](size_t);

            /// @brief access via linear index, if array type returns getindex! result
            /// @param index
            /// @returns field as proxy
            template<Unboxable T>
            T operator[](size_t);

            /// @brief cast to Any
            operator Any*();

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
            std::string get_name() const;

            /// @brief if proxy is a value, get fieldnames of typeof(value), if proxy is a type, get fieldnames of itself
            /// @returns vector of strings
            std::vector<std::string> get_field_names() const;

            /// @brief get type
            /// @returns proxy to singleton type
            Proxy get_type() const;

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
            /// @returns decltype(this)
            Proxy value() const;

            /// @brief update value if proxy symbol was reassigned outside of operator=
            void update();

        protected:
            class ProxyValue
            {
                friend class Proxy;

                public:
                    ProxyValue(Any*, Symbol*);
                    ProxyValue(Any*, std::shared_ptr<ProxyValue>& owner, Symbol*);
                    ~ProxyValue();

                    Any* get_field(Symbol*);

                    std::shared_ptr<ProxyValue> _owner;

                    Any* value();
                    Any* symbol();

                    size_t value_key();
                    size_t symbol_key();

                    const Any* value() const;
                    const Any* symbol() const;

                    const bool _is_mutating = true;

                private:
                    size_t _symbol_key;
                    size_t _value_key;

                    Any* _symbol_ref;
                    Any* _value_ref;
            };

            std::shared_ptr<ProxyValue> _content;
            std::deque<Symbol*> assemble_name() const;
    };
}

#include ".src/proxy.inl"