// 
// Copyright 2022 Clemens Cords
// Created on 16.02.22 by clem (mail@clemens-cords.com)
//

#include <include/generator_expression.hpp>

namespace jluna
{
    GeneratorExpression operator""_gen(const char* in, size_t n)
    {
        std::stringstream str;
        auto gc = GCSentinel();

        if (in[0] != '(' or in[n-1] != ')')
            std::cerr << "[C++][WARNING] generator expressions constructed via the _gen operator should *begin and end with rounds brackets*. Example: \"(i for i in 1:10)\"_gen" << std::endl;

        auto* res = jl_eval_string(in);
        forward_last_exception();

        static jl_datatype_t* generator_type = (jl_datatype_t*) jl_eval_string("Base.Generator");
        if (not jl_isa(res, (unsafe::Value*) generator_type))
        {
            std::stringstream error_str;
            error_str << "[C++][Exception] When parsing generator expression from string \"" << in << "\": result is not a generator expression object" << std::endl;
            throw std::invalid_argument(error_str.str());
        }

        auto out = GeneratorExpression(res);
        return out;
    }

    GeneratorExpression::GeneratorExpression(unsafe::Value* val)
    {
        if (_iterate == nullptr)
            _iterate = jl_get_function(jl_base_module, "iterate");

        _value_key = detail::create_reference(val);
        _value_ref = detail::get_reference(_value_key);

        static jl_function_t* length = unsafe::get_function("jluna"_sym, "get_length_of_generator"_sym);

        auto gc = GCSentinel();
        _length = jl_unbox_int64(jl_call1(length, get()));
        forward_last_exception();
    }

    GeneratorExpression::~GeneratorExpression()
    {
        detail::free_reference(_value_key);
    }

    unsafe::Value* GeneratorExpression::get() const
    {
        return jl_get_nth_field(_value_ref, 0);
    }

    typename GeneratorExpression::ForwardIterator GeneratorExpression::begin() const
    {
        return ForwardIterator(this, 0);
    }

    typename GeneratorExpression::ForwardIterator GeneratorExpression::end() const
    {
        return ForwardIterator(this, _length);
    }

    size_t GeneratorExpression::size() const
    {
        return _length;
    }

    GeneratorExpression::operator unsafe::Value*() const
    {
        return get();
    }

    GeneratorExpression::ForwardIterator::ForwardIterator(const GeneratorExpression* owner, Int64 state)
        : _owner(owner), _state(state), _is_end(_state)
    {}

    unsafe::Value* GeneratorExpression::ForwardIterator::operator*()
    {
        auto gc = GCSentinel();
        return jl_get_nth_field(jl_call2(_owner->_iterate, _owner->get(), jl_box_int64(_state)), 0);
    }

    void GeneratorExpression::ForwardIterator::operator++()
    {
        auto previous = _state;

        auto gc = GCSentinel();
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

