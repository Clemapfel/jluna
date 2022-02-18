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
        : Proxy(box<Symbol>(str), nullptr)
    {}

    Symbol::Symbol(jl_sym_t* value, jl_sym_t* symbol)
        : Proxy((jl_value_t*) value, symbol)
    {
       jl_assert_type((Any*) value, "Symbol");
    }

    Symbol::Symbol(jl_value_t* value, std::shared_ptr<ProxyValue>& owner, Any* symbol)
        : Proxy(value, owner, symbol)
    {
        jl_assert_type(value, "Symbol");
    }

    Symbol::operator jl_sym_t*() const
    {
        return (jl_sym_t*) Proxy::operator const jl_value_t*();
    }

    size_t Symbol::hash() const
    {
        return ((jl_sym_t*) Proxy::operator const jl_value_t*())->hash;
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

