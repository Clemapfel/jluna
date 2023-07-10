// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#include <include/symbol.hpp>

namespace jluna
{
    Symbol::Symbol()
        : Symbol("")
    {}

    Symbol::Symbol(const std::string& str)
        : Proxy((unsafe::Value*) jl_symbol(str.data()))
    {}

    Symbol::Symbol(jl_sym_t* value, jl_sym_t* symbol)
        : Proxy((unsafe::Value*) value, symbol)
    {
        detail::assert_type((unsafe::DataType*) jl_typeof((unsafe::Value*) value), jl_symbol_type);
    }

    Symbol::Symbol(Proxy* owner)
        : Proxy(*owner)
    {
        detail::assert_type((unsafe::DataType*) jl_typeof((unsafe::Value*) owner->operator jl_value_t *()), jl_symbol_type);
    }

    Symbol::operator jl_sym_t*() const
    {
        return (jl_sym_t*) Proxy::operator const unsafe::Value*();
    }

    uint64_t Symbol::hash() const
    {
        return ((jl_sym_t*) Proxy::operator const unsafe::Value*())->hash;
    }

    bool Symbol::operator==(const Symbol& other) const
    {
        return this->hash() == other.hash();
    }

    bool Symbol::operator!=(const Symbol& other) const
    {
        return not (*this == other);
    }

    bool Symbol::operator<(const Symbol& other) const
    {
        return this->hash() < other.hash();
    }

    bool Symbol::operator<=(const Symbol& other) const
    {
        return (*this == other) or (*this < other);
    }

    bool Symbol::operator>(const Symbol& other) const
    {
        return not (*this < other);
    }

    bool Symbol::operator>=(const Symbol& other) const
    {
        return (*this == other) or (*this > other);
    }
}

