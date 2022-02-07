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
            /// @brief construct as unnamed proxy
            /// @param value
            /// @param name
            Symbol(jl_sym_t*, jl_sym_t* symbol = nullptr);

            /// @brief ctor as owned
            /// @param value
            /// @param owner: internal proxy value owner
            /// @param name: symbol
            Symbol(jl_sym_t* value, std::shared_ptr<ProxyValue>& owner, jl_sym_t* symbol);

            /// @brief decay to C-type
            operator jl_sym_t*();

            /// @brief hash, constant runtime
            /// @returns hash
            size_t hash() const;
    };
}