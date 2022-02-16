// 
// Copyright 2022 Clemens Cords
// Created on 16.02.22 by clem (mail@clemens-cords.com)
//

#include <include/generator_expression.hpp>
#include <include/state.hpp>

namespace jluna
{
    GeneratorExpression::GeneratorExpression(Any* val)
    {
        if (_iterate == nullptr)
            _iterate = jl_get_function(jl_base_module, "iterate");

        _value_key = State::detail::create_reference(val);
        _value_ref = State::detail::get_reference(_value_key);
    }

    GeneratorExpression::~GeneratorExpression()
    {
        State::detail::free_reference(_value_key);
    }

    Any* GeneratorExpression::get()
    {
        return jl_ref_value(_value_ref);
    }

    Int64 GeneratorExpression::length()
    {
        static jl_function_t* length = jl_get_function(jl_base_module, "length");
        return jl_unbox_int64(jl_call1(length, get()));
    }

    typename GeneratorExpression::ForwardIterator GeneratorExpression::begin()
    {
        return ForwardIterator(this, 1);
    }

    typename GeneratorExpression::ForwardIterator GeneratorExpression::end()
    {
        return ForwardIterator(this, length());
    }

    GeneratorExpression::ForwardIterator::ForwardIterator(GeneratorExpression* owner, Int64 state)
        : _owner(owner), _state(state)
    {}

    Any * GeneratorExpression::ForwardIterator::operator*()
    {
        jl_get_nth_field(jl_call2(_owner->_iterate, _owner->get(), jl_box_int64(_state)), 1);
    }

    void GeneratorExpression::ForwardIterator::operator++()
    {
        _state = jl_unbox_int64(jl_get_nth_field(jl_call2(_owner->_iterate, _owner->get(), jl_box_int64(_state)), 1));
    }

    void GeneratorExpression::ForwardIterator::operator++(int)
    {
        _state = jl_unbox_int64(jl_get_nth_field(jl_call2(_owner->_iterate, _owner->get(), jl_box_int64(_state)), 1));
    }

    bool GeneratorExpression::ForwardIterator::operator==(const GeneratorExpression::ForwardIterator& other) const
    {
        return this->_owner == other._owner and this->_state == other._state;
    }

    bool GeneratorExpression::ForwardIterator::operator!=(const GeneratorExpression::ForwardIterator& other) const
    {
        return not (*this == other);
    }
}

