// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>
#include <include/proxy.hpp>

namespace jluna
{
    /// @brief wrapper for jl_sym_t*
    class Symbol : public Proxy
    {
        public:
            /// @brief default ctor
            Symbol();

            /// @param allocate symbol julia side
            Symbol(const std::string&);

            /// @brief construct as unnamed proxy
            /// @param value
            /// @param name
            Symbol(jl_sym_t* value, jl_sym_t* symbol = nullptr);

            /// @brief ctor as owned
            /// @param value
            /// @param owner: internal proxy value owner
            /// @param name: symbol
            Symbol(jl_value_t* value, std::shared_ptr<ProxyValue>& owner, jl_sym_t* symbol);

            /// @brief decay to C-type
            operator jl_sym_t*() const;

            /// @brief hash, constant runtime
            /// @returns hash
            size_t hash() const;

            /// @brief equality operator, uses hash
            /// @param other
            /// @returns true if hashes equal, false otherwise
            bool operator==(const Symbol& other) const;

            /// @brief inequality operator, uses hash
            /// @param other
            /// @returns false if hashes equal, true otherwise
            bool operator!=(const Symbol& other) const;

            /// @brief comparison operator, uses hash
            /// @param other
            /// @returns true if this.hash < other.hash
            bool operator<(const Symbol& other) const;

            /// @brief comparison operator, uses hash
            /// @param other
            /// @returns true if this.hash <= other.hash
            bool operator<=(const Symbol& other) const;

            /// @brief comparison operator, uses hash
            /// @param other
            /// @returns true if this.hash >= other.hash
            bool operator>=(const Symbol& other) const;

            /// @brief comparison operator, uses hash
            /// @param other
            /// @returns true if this.hash > other.hash
            bool operator>(const Symbol& other) const;
    };

    /// @brief unbox symbol to symbol
    template<Is<Symbol> T>
    inline T unbox(Any* value)
    {
        jl_assert_type(value, "Symbol");
        return Symbol((jl_sym_t*) value);
    }

    /// @brief box jluna::Module to Base.Module
    template<Is<Symbol> T>
    inline Any* box(T value)
    {
        return value.operator Any*();
    }

    template<Is<Symbol> T>
    inline Any* box(const std::string& value)
    {
        return (Any*) jl_symbol(value.c_str());
    }

    /// @brief type deduction
    template<>
    struct detail::to_julia_type_aux<Symbol>
    {
        static inline const std::string type_name = "Symbol";
    };
}