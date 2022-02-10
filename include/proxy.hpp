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
            Proxy() = default;

            /// @brief construct with no owner, reserved for global temporaries and main
            /// @param value
            /// @param symbol
            Proxy(Any* value, jl_sym_t* symbol = nullptr);

            /// @brief construct with owner
            /// @param value
            /// @param owner: shared pointer to owner
            /// @param symbol
            Proxy(Any* value, std::shared_ptr<ProxyValue>& owner, jl_sym_t* symbol);

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
            [[nodiscard]] Proxy value() const;

            /// @brief update value if proxy symbol was reassigned outside of operator=
            void update();

            /// @brief check if this <: type
            /// @param type
            /// @returns true if `*this isa type`, false otherwise
            bool isa(const Type& type);

        protected:
            class ProxyValue
            {
                friend class Proxy;

                public:
                    ProxyValue(Any*, jl_sym_t*);
                    ProxyValue(Any*, std::shared_ptr<ProxyValue>& owner, jl_sym_t*);
                    ~ProxyValue();

                    Any* get_field(jl_sym_t*);

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
            std::deque<jl_sym_t*> assemble_name() const;
    };

    /// @brief unbox to proxy
    template<Is<Proxy> T>
    inline T unbox(Any* value)
    {
        return Proxy(value, nullptr);
    }

    /// @brief box jluna::Proxy to Base.Any
    template<Is<Proxy> T>
    inline Any* box(T value)
    {
        return value.operator Any*();
    }

    /// @brief type deduction
    template<>
    struct detail::to_julia_type_aux<Proxy>
    {
        static inline const std::string type_name = "Any";
    };
}

#include ".src/proxy.inl"