// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>
#include <include/proxy.hpp>

namespace jluna
{
    /// @brief wrapper for `jl_sym_t*`
    class Symbol : public Proxy
    {
        public:
            /// @brief default ctor
            Symbol();

            /// @brief allocate symbol Julia-side, implicit
            /// @param string: value of symbol, constructed via `Base.Symbol(string)`
            Symbol(const std::string&);

            /// @brief construct as unnamed proxy, implicit
            /// @param value: Julia-side symbol
            /// @param name: name of proxy, or nullptr for unnamed proxy
            Symbol(jl_sym_t* value, jl_sym_t* symbol = nullptr);

            /// @brief construct as child of proxy, implicit
            /// @param proxy: proxy
            Symbol(Proxy*);

            /// @brief decay to C-type, implicit
            operator jl_sym_t*() const;

            /// @brief decay to unsafe pointer
            using Proxy::operator unsafe::Value*;

            /// @brief hash, constant runtime
            /// @returns hash
            uint64_t hash() const;

            /// @brief equality operator, uses hash
            /// @param other: other symbol
            /// @returns true if hashes equal, false otherwise
            bool operator==(const Symbol& other) const;

            /// @brief inequality operator, uses hash
            /// @param other: other symbol
            /// @returns false if hashes equal, true otherwise
            bool operator!=(const Symbol& other) const;

            /// @brief comparison operator, uses hash
            /// @param other: other symbol
            /// @returns true if this.hash < other.hash
            bool operator<(const Symbol& other) const;

            /// @brief comparison operator, uses hash
            /// @param other: other symbol
            /// @returns true if this.hash <= other.hash
            bool operator<=(const Symbol& other) const;

            /// @brief comparison operator, uses hash
            /// @param other: other symbol
            /// @returns true if this.hash >= other.hash
            bool operator>=(const Symbol& other) const;

            /// @brief comparison operator, uses hash
            /// @param other: other symbol
            /// @returns true if this.hash > other.hash
            bool operator>(const Symbol& other) const;
    };
}

#include <.src/symbol.inl>