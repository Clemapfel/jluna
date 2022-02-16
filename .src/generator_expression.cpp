// 
// Copyright 2022 Clemens Cords
// Created on 16.02.22 by clem (mail@clemens-cords.com)
//

#include <include/generator_expression.hpp>
#include <include/state.hpp>

namespace jluna
{
    GeneratorExpression operator""_gen(const char* in, size_t n)
    {
        std::stringstream str;
        str << "Base.Generator";

        if (in[0] != '(')
            str << "(";

        str << in;

        if (in[n-1] != ')')
            str << ")";

        return GeneratorExpression(jl_eval_string(str.str().c_str()));
    }

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
        return jl_unbox_int64(jl_call1(length, jl_get_nth_field(get(), 1)));
    }

    typename GeneratorExpression::ForwardIterator GeneratorExpression::begin()
    {
        return ForwardIterator(this, 0);
    }

    typename GeneratorExpression::ForwardIterator GeneratorExpression::end()
    {
        return ForwardIterator(this, length());
    }

    GeneratorExpression::ForwardIterator::ForwardIterator(GeneratorExpression* owner, Int64 state)
        : _owner(owner), _state(state), _is_end(_state)
    {}

    Any* GeneratorExpression::ForwardIterator::operator*()
    {
        return jl_get_nth_field(jl_call2(_owner->_iterate, _owner->get(), jl_box_int64(_state)), 0);
    }

    void GeneratorExpression::ForwardIterator::operator++()
    {
        auto previous = _state;

        auto* next = jl_call2(_owner->_iterate, _owner->get(), jl_box_int64(_state));

        if (jl_is_nothing(next))
            _state = previous;
        else
            _state = jl_unbox_int64(jl_get_nth_field(next, 1));
    }

    void GeneratorExpression::ForwardIterator::operator++(int)
    {
        this->operator++();
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

