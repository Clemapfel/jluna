// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>
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

            /// @brief ctor from proxy
            /// @param proxy
            Symbol(Proxy*);

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
}

#include <.src/symbol.inl>