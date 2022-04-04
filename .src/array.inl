// 
// Copyright 2022 Clemens Cords
// Created on 12.01.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    namespace detail
    {
        template<typename Value_t, size_t N>
        struct to_julia_type_aux<Array<Value_t, N>>
        {
            static inline const std::string type_name = "Array{" + to_julia_type_aux<Value_t>::type_name + ", " + std::to_string(N) + "}";
        };
    }

    template<Boxable V, size_t R>
    Array<V, R>::Array(unsafe::Value* value, jl_sym_t* symbol)
        : Proxy(value, symbol)
    {
        detail::assert_type((unsafe::DataType*) jl_typeof(value), (unsafe::DataType*) jl_array_type);
        detail::assert_type((unsafe::DataType*) detail::array_value_type((unsafe::Array*) value), (unsafe::DataType*) to_julia_type<V>::type());
    }

    template<Boxable V, size_t R>
    Array<V, R>::Array(Proxy* proxy)
        : Proxy(*proxy)
    {
        auto* value = proxy->operator unsafe::Value*();
        detail::assert_type((unsafe::DataType*) jl_typeof(value), (unsafe::DataType*) jl_array_type);
        detail::assert_type((unsafe::DataType*) detail::array_value_type((unsafe::Array*) value), (unsafe::DataType*) to_julia_type<V>::type());
    }

    template<Boxable V, size_t R>
    template<typename... Dims>
    Array<V, R>::Array(V* data, Dims... size_per_dimension)
        : Proxy((unsafe::Value*) unsafe::new_array_from_data((unsafe::Value*) to_julia_type<V>::type(),(void*) data, size_per_dimension...))
    {}

    template<Boxable T, size_t Rank>
    size_t Array<T, Rank>::get_dimension(int index) const
    {
        return jl_array_dim(this->operator unsafe::Array*(), index);
    }

    template<Boxable T, size_t Rank>
    Array<T, Rank>::operator unsafe::Array*() const
    {
        return (unsafe::Array*) _content->value();
    }

    template<Boxable T, size_t Rank>
    void Array<T, Rank>::throw_if_index_out_of_range(int index, size_t dimension) const
    {
        if (index < 0)
        {
            std::stringstream str;
            str << "negative index " << index << ", only indices >= 0 are permitted" << std::endl;
            throw std::out_of_range(str.str().c_str());
        }

        size_t dim = get_dimension(dimension);

        if (index >= dim)
        {
            std::string dim_id;

            if (dimension == 0)
                dim_id = "1st dimension";
            else if (dimension == 1)
                dim_id = "2nd dimension";
            else if (dimension == 3)
                dim_id = "3rd dimension";
            else if (dimension < 11)
                dim_id = std::to_string(dimension) + "th dimension";
            else
                dim_id = "dimension " + std::to_string(dimension);

            std::stringstream str;
            str << "0-based index " << index << " out of range for array of length " << dim << " along " << dim_id << std::endl;
            throw std::out_of_range(str.str().c_str());
        }
    }

    template<Boxable V, size_t R>
    auto Array<V, R>::operator[](size_t i)
    {
        if (i >= get_n_elements())
        {
            std::stringstream str;
            str << "0-based index " << i << " out of range for array of length " << get_n_elements() << std::endl;
            throw std::out_of_range(str.str().c_str());
        }

        return Iterator(i, this);
    }

    template<Boxable V, size_t R>
    Vector<V> Array<V, R>::operator[](const std::vector<size_t>& range) const
    {
        gc_pause;
        auto* out = unsafe::new_array((unsafe::Value*) to_julia_type<V>::type(), range.size());
        auto* me = operator jl_array_t*();

        for (size_t i = 0; i < range.size(); ++i)
            jl_arrayset(out, jl_arrayref(me, range.at(i)), i);

        gc_unpause;
        return Vector<V>((unsafe::Value*) out);
    }

    template<Boxable V, size_t R>
    Vector<V> Array<V, R>::operator[](const GeneratorExpression& gen) const
    {
        gc_pause;
        auto* out = unsafe::new_array((unsafe::Value*) to_julia_type<V>::type(), gen.size());
        auto* me = operator jl_array_t*();

        {
            size_t i = 0;
            for (auto index : gen)
                jl_arrayset(out, jl_arrayref(me, unbox<size_t>(index)), i++);
        }

        auto res = Vector<V>((unsafe::Value*) out);
        gc_unpause;
        return res;
    }

    template<Boxable V, size_t R>
    template<Boxable T>
    Vector<V> Array<V, R>::operator[](std::initializer_list<T>&& list) const
    {
        std::vector<size_t> index;
        index.reserve(list.size());
        for (auto& e : list)
            index.push_back(e);

        return operator[](index);
    }

    template<Boxable V, size_t R>
    template<Unboxable T>
    T Array<V, R>::operator[](size_t i) const
    {
        if (i >= get_n_elements())
        {
            std::stringstream str;
            str << "0-based index " << i << " out of range for array of length " << get_n_elements() << std::endl;
            throw std::out_of_range(str.str().c_str());
        }

        return unbox<T>(jl_arrayref((jl_array_t*) _content->value(), i));
    }

    template<Boxable V, size_t R>
    template<Unboxable T, typename... Args, std::enable_if_t<sizeof...(Args) == R and (std::is_integral_v<Args> and ...), bool>>
    T Array<V, R>::at(Args... in) const
    {
        {
            size_t i = 0;
            (throw_if_index_out_of_range(in, i++), ...);
        }

        std::array<size_t, R> indices = {size_t(in)...};
        size_t index = 0;
        size_t mul = 1;

        for (size_t i = 0; i < R; ++i)
        {
            index += (indices.at(i)) * mul;
            size_t dim = get_dimension(i);
            mul *= dim;
        }

        return operator[]<T>(index);
    }

    template<Boxable V, size_t R>
    template<typename... Args, std::enable_if_t<sizeof...(Args) == R and (std::is_integral_v<Args> and ...), bool>>
    auto Array<V, R>::at(Args... in)
    {
        {
            size_t i = 0;
            (throw_if_index_out_of_range(in, i++), ...);
        }

        std::array<size_t, R> indices = {size_t(in)...};
        size_t index = 0;
        size_t mul = 1;

        for (size_t i = 0; i < R; ++i)
        {
            index += (indices.at(i)) * mul;
            size_t dim = get_dimension(i);
            mul *= dim;
        }

        return operator[](index);
    }

    template<Boxable V, size_t R>
    template<Boxable T>
    void Array<V, R>::set(size_t i, T value)
    {
        jl_arrayset((jl_array_t*) _content->value(), box<T>(value), i);
    }

    template<Boxable V, size_t R>
    auto Array<V, R>::front()
    {
        return operator[](0);
    }

    template<Boxable V, size_t R>
    auto Array<V, R>::begin()
    {
        return Iterator(0, this);
    }

    template<Boxable V, size_t R>
    auto Array<V, R>::begin() const
    {
        return ConstIterator(0, const_cast<Array<V, R>*>(this));
    }

    template<Boxable V, size_t R>
    auto Array<V, R>::end()
    {
        return Iterator(get_n_elements(), this);
    }

    template<Boxable V, size_t R>
    auto Array<V, R>::end() const
    {
        return ConstIterator(get_n_elements(), const_cast<Array<V, R>*>(this));
    }

    template<Boxable V, size_t R>
    template<Unboxable T>
    T Array<V, R>::front() const
    {
        return operator[](0);
    }

    template<Boxable V, size_t R>
    auto Array<V, R>::back()
    {
        return operator[](get_n_elements() - 1);
    }

    template<Boxable V, size_t R>
    template<Unboxable T>
    T Array<V, R>::back() const
    {
        return operator[]<T>(get_n_elements() - 1);
    }

    template<Boxable V, size_t R>
    size_t Array<V, R>::get_n_elements() const
    {
        return reinterpret_cast<const jl_array_t*>(this->operator const unsafe::Value*())->length;
    }

    template<Boxable V, size_t R>
    bool Array<V, R>::empty() const
    {
        return reinterpret_cast<const jl_array_t*>(this->operator const unsafe::Value*())->length == 0;
    }

    // ###

    template<Boxable V>
    Vector<V>::Vector()
        : Array<V, 1>((unsafe::Value*) unsafe::new_array((unsafe::Value*) to_julia_type<V>::type(), 0), nullptr)
    {}

    template<Boxable V>
    Vector<V>::Vector(const std::vector<V>& vec)
        : Array<V, 1>((unsafe::Value*) unsafe::new_array((unsafe::Value*) to_julia_type<V>::type(), vec.size()), nullptr)
    {
        for (size_t i = 0; i < vec.size(); ++i)
            jl_arrayset(reinterpret_cast<jl_array_t*>(this->operator unsafe::Value*()), box(vec.at(i)), i);
    }

    template<Boxable V>
    Vector<V>::Vector(V* data, size_t size)
        : Array<V, 1>(data, size)
    {}

    template<Boxable V>
    Vector<V>::Vector(Proxy* owner)
        : Array<V, 1>(owner)
    {}

    template<Boxable V>
    Vector<V>::Vector(unsafe::Value* value, jl_sym_t* symbol)
        : Array<V, 1>(value, symbol)
    {}

    template<Boxable V>
    Vector<V>::Vector(const GeneratorExpression& gen)
        : Array<V, 1>([&]() -> unsafe::Value* {
            static unsafe::Function* collect = unsafe::get_function(jl_base_module, "collect"_sym);
            return unsafe::call(collect, gen.operator jl_value_t *());
        }())
    {}

    template<Boxable V>
    void Vector<V>::insert(size_t pos, V value)
    {
        static unsafe::Value* insert = jl_get_function(jl_base_module, "insert!");

        gc_pause;
        jl_call3(insert, _content->value(), jl_box_uint64(pos + 1), box(value));
        forward_last_exception();
        gc_unpause;
    }

    template<Boxable V>
    void Vector<V>::erase(size_t pos)
    {
        static unsafe::Value* deleteat = jl_get_function(jl_base_module, "deleteat!");

        gc_pause;
        jl_call2(deleteat, _content->value(), jl_box_uint64(pos + 1));
        forward_last_exception();
        gc_unpause;
    }

    template<Boxable V>
    template<Boxable T>
    void Vector<V>::push_front(T value)
    {
        gc_pause;
        auto* array = (jl_array_t*) _content->value();
        jl_array_grow_beg(array, 1);
        jl_arrayset((unsafe::Array*) _content->value(), box<V>(value), 0);
        gc_unpause;
    }

    template<Boxable V>
    template<Boxable T>
    void Vector<V>::push_back(T value)
    {
        gc_pause;
        auto* array = (jl_array_t*) _content->value();
        jl_array_grow_end(array, 1);
        jl_arrayset((unsafe::Array*) _content->value(), box<V>(value), jl_array_len(array)-1);
        gc_unpause;
    }
}