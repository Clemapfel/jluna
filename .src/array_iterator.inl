// 
// Copyright 2022 Clemens Cords
// Created on 12.01.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    template<Boxable V, size_t R>
    Array<V, R>::ConstIterator::ConstIterator(size_t i, Array<V, R>* array)
        : _index(i), _owner(array)
    {}

    template<Boxable V, size_t R>
    auto& Array<V, R>::ConstIterator::operator++()
    {
        if (_index < _owner->get_n_elements())
            ++_index;

        return *this;
    }

    template<Boxable V, size_t R>
    auto& Array<V, R>::ConstIterator::operator++(int i)
    {
        if (_index < _owner->get_n_elements())
            _index++;

        return *this;
    }

    template<Boxable V, size_t R>
    auto& Array<V, R>::ConstIterator::operator--()
    {
        if (_index > 0)
            --_index;

        return *this;
    }

    template<Boxable V, size_t R>
    auto& Array<V, R>::ConstIterator::operator--(int i)
    {
        if (_index > 0)
            _index--;

        return *this;
    }

    template<Boxable V, size_t R>
    bool Array<V, R>::ConstIterator::operator==(const typename Array<V, R>::ConstIterator& other) const
    {
        return (this->_owner->operator const jl_value_t *()) == (other._owner->operator const jl_value_t *()) and this->_index == other._index;
    }

    template<Boxable V, size_t R>
    bool Array<V, R>::ConstIterator::operator!=(const typename Array<V, R>::ConstIterator& other) const
    {
        return not (*this == other);
    }

    template<Boxable V, size_t R>
    auto Array<V, R>::ConstIterator::operator*()
    {
        return Iterator(_index, const_cast<Array<V, R>*>(_owner));
    }

    template<Boxable V, size_t R>
    auto Array<V, R>::ConstIterator::operator*() const
    {
        return *this;
    }

    template<Boxable V, size_t R>
    Array<V, R>::ConstIterator::operator Proxy()
    {
        auto gc = GCSentinel();

        auto res = Proxy(
            jl_arrayref((jl_array_t*) _owner->_content->value(), _index),
            _owner->_content,
            jl_box_uint64(_index+1)
        );

        return res;
    }

    template<Boxable V, size_t R>
    template<Unboxable T, std::enable_if_t<not Is<Proxy, T>, bool>>
    Array<V, R>::ConstIterator::operator T() const
    {
        static jl_function_t* getindex = jl_get_function(jl_base_module, "getindex");
        return unbox<T>(jluna::safe_call(getindex, _owner->operator jl_value_t *(), box(_index + 1)));
    }

    template<Boxable V, size_t R>
    Array<V, R>::Iterator::Iterator(size_t i, Array<V, R>* array)
        : ConstIterator(i, array)
    {}

    template<Boxable V, size_t R>
    template<Boxable T>
    auto& Array<V, R>::Iterator::operator=(T value)
    {
        if (_index >= _owner->get_n_elements())
            throw std::out_of_range("In: jluna::Array::ConstIterator::operator=(): trying to assign value to past-the-end iterator");

        static jl_function_t* setindex = jl_get_function(jl_base_module, "setindex!");

        auto gc = GCSentinel();
        jl_call3(setindex, _owner->operator jl_value_t *(), box(value), box(_index + 1));

        return *this;
    }

    template<Boxable V, size_t R>
    template<Unboxable T, std::enable_if_t<not std::is_same_v<T, Proxy>, bool>>
    Array<V, R>::Iterator::operator T() const
    {
        return ConstIterator::operator T();
    }
}