// 
// Copyright 2022 Clemens Cords
// Created on 12.01.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    /// @brief box array
    /// @param array
    /// @returns pointer to newly allocated julia-side value
    template<typename T,
        typename Value_t = typename T::value_type,
        uint64_t Rank = T::rank,
        std::enable_if_t<std::is_same_v<T, Array<Value_t, Rank>>, bool> = true>
    unsafe::Value* box(T value)
    {
        return value.operator unsafe::Value*();
    }

    /// @brief box vector
    /// @param vector
    /// @returns pointer to newly allocated julia-side value
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, Vector<Value_t>>, bool> = true>
    unsafe::Value* box(T value)
    {
        return value.operator unsafe::Value*();
    }

    /// @brief unbox to array
    template<typename T,
        typename Value_t = typename T::value_type,
        uint64_t Rank = T::rank,
        std::enable_if_t<std::is_same_v<T, Array<Value_t, Rank>>, bool> = true>
    T unbox(unsafe::Value* in)
    {
        return Array<Value_t, Rank>(in);
    }

    /// @brief unbox to vector
    template<typename T,
        typename Value_t = typename T::value_type,
        std::enable_if_t<std::is_same_v<T, Vector<Value_t>>, bool> = true>
    T unbox(unsafe::Value* in)
    {
        return Vector<Value_t>(in);
    }

    namespace detail
    {
        template<typename Value_t, uint64_t N>
        struct as_julia_type_aux<Array<Value_t, N>>
        {
            static inline const std::string type_name = "Array{" + as_julia_type_aux<Value_t>::type_name + ", " + std::to_string(N) + "}";
        };
    }

    template<is_boxable V, uint64_t R>
    Array<V, R>::Array(unsafe::Value* value, jl_sym_t* symbol)
        : Proxy(value, symbol)
    {
        detail::assert_type((unsafe::DataType*) jl_typeof(value), (unsafe::DataType*) jl_array_type);
        detail::assert_type((unsafe::DataType*) detail::array_value_type((unsafe::Array*) value), (unsafe::DataType*) as_julia_type<V>::type());
    }

    template<is_boxable V, uint64_t R>
    Array<V, R>::Array(Proxy* proxy)
        : Proxy(*proxy)
    {
        auto* value = proxy->operator unsafe::Value*();
        detail::assert_type((unsafe::DataType*) jl_typeof(value), (unsafe::DataType*) jl_array_type);
        detail::assert_type((unsafe::DataType*) detail::array_value_type((unsafe::Array*) value), (unsafe::DataType*) as_julia_type<V>::type());
    }

    template<is_boxable V, uint64_t R>
    template<typename... Dims>
    Array<V, R>::Array(V* data, Dims... size_per_dimension)
        : Proxy((unsafe::Value*) unsafe::new_array_from_data((unsafe::Value*) as_julia_type<V>::type(),(void*) data, size_per_dimension...))
    {}

    template<is_boxable T, uint64_t Rank>
    uint64_t Array<T, Rank>::get_dimension(uint64_t index) const
    {
        return jl_array_dim(this->operator unsafe::Array*(), index);
    }

    template<is_boxable T, uint64_t Rank>
    Array<T, Rank>::operator unsafe::Array*() const
    {
        return (unsafe::Array*) _content->value();
    }

    template<is_boxable T, uint64_t Rank>
    void Array<T, Rank>::throw_if_index_out_of_range(int index, uint64_t dimension) const
    {
        if (index < 0)
        {
            std::stringstream str;
            str << "negative index " << index << ", only indices >= 0 are permitted" << std::endl;
            throw std::out_of_range(str.str().c_str());
        }

        uint64_t dim = get_dimension(dimension);

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

    template<is_boxable V, uint64_t R>
    auto Array<V, R>::linear_index(uint64_t i)
    {
        if (i >= get_n_elements())
        {
            std::stringstream str;
            str << "0-based index " << i << " out of range for array of length " << get_n_elements() << std::endl;
            throw std::out_of_range(str.str().c_str());
        }

        return Iterator(i, this);
    }

    template<is_boxable V, uint64_t R>
    auto Array<V, R>::operator[](uint64_t i)
    {
       return linear_index(i);
    }

    template<is_boxable V, uint64_t R>
    Vector<V> Array<V, R>::at(const std::vector<uint64_t>& range) const
    {
        gc_pause;
        auto* out = unsafe::new_array((unsafe::Value*) as_julia_type<V>::type(), range.size());
        auto* me = operator jl_array_t*();

        for (uint64_t i = 0; i < range.size(); ++i)
            jl_arrayset(out, jl_arrayref(me, range.at(i)), i);

        gc_unpause;
        return Vector<V>((unsafe::Value*) out);
    }

    template<is_boxable V, uint64_t R>
    Vector<V> Array<V, R>::at(const GeneratorExpression& gen) const
    {
        gc_pause;
        auto res = Vector<V>();
        res.reserve(gen.size());

        for (auto it : gen)
            res.push_back(this->at(unbox<uint64_t>(it)));

        gc_unpause;
        return res;
    }

    template<is_boxable V, uint64_t R>
    template<is_boxable T>
    Vector<V> Array<V, R>::at(std::initializer_list<T>&& list) const
    {
        std::vector<uint64_t> index;
        index.reserve(list.size());
        for (auto& e : list)
            index.push_back(e);

        return at(index);
    }


    template<is_boxable V, uint64_t R>
    Vector<V> Array<V, R>::operator[](const std::vector<uint64_t>& range) const
    {
        return this->at(range);
    }

    template<is_boxable V, uint64_t R>
    Vector<V> Array<V, R>::operator[](const GeneratorExpression& gen) const
    {
        return this->at(gen);
    }

    template<is_boxable V, uint64_t R>
    template<is_boxable T>
    Vector<V> Array<V, R>::operator[](std::initializer_list<T>&& list) const
    {
        return this->at(std::forward<std::initializer_list<T>&&>(list));
    }

    template<is_boxable V, uint64_t R>
    template<is_unboxable T>
    T Array<V, R>::linear_index(uint64_t i) const
    {
        if (i >= get_n_elements())
        {
            std::stringstream str;
            str << "0-based index " << i << " out of range for array of length " << get_n_elements() << std::endl;
            throw std::out_of_range(str.str().c_str());
        }

        return unbox<T>(jl_arrayref((jl_array_t*) _content->value(), i));
    }

    template<is_boxable V, uint64_t R>
    template<is_unboxable T>
    T Array<V, R>::operator[](uint64_t i) const
    {
        return linear_index(i);
    }

    template<is_boxable V, uint64_t R>
    template<is_unboxable T, typename... Args, std::enable_if_t<R >= 2 and sizeof...(Args) == R and (std::is_integral_v<Args> and ...), bool>>
    T Array<V, R>::at(Args... in) const
    {
        {
            uint64_t i = 0;
            (throw_if_index_out_of_range(in, i++), ...);
        }

        std::array<uint64_t, R> indices = {uint64_t(in)...};
        uint64_t index = 0;
        uint64_t mul = 1;

        for (uint64_t i = 0; i < R; ++i)
        {
            index += (indices.at(i)) * mul;
            uint64_t dim = get_dimension(i);
            mul *= dim;
        }

        return operator[]<T>(index);
    }

    #ifdef _MSC_VER
        // silence false positive conversion warning on MSVC
        #pragma warning(push)
        #pragma warning(disable:4267)
    #endif

    template<is_boxable V, uint64_t R>
    template<typename... Args, std::enable_if_t<R >= 2 and sizeof...(Args) == R and (std::is_integral_v<Args> and ...), bool>>
    auto Array<V, R>::at(Args... in)
    {
        {
            uint64_t i = 0;
            (throw_if_index_out_of_range(in, i++), ...);
        }

        std::array<uint64_t, R> indices = {uint64_t(in)...};
        uint64_t index = 0;
        uint64_t mul = 1;

        for (uint64_t i = 0; i < R; ++i)
        {
            index += (indices.at(i)) * mul;
            uint64_t dim = get_dimension(i);
            mul *= dim;
        }

        return linear_index(index);
    }

    template<is_boxable V, uint64_t R>
    template<typename... Args, std::enable_if_t<sizeof...(Args) == 1 and (std::is_integral_v<Args> and ...), bool>>
    auto Array<V, R>::at(Args... in)
    {
        auto index = (in, ...);
        return linear_index(index);
    }

    /// @brief linear indexing
    /// @param n: singular index
    /// @returns unboxed value
    template<is_boxable V, uint64_t R>
    template<is_unboxable T, typename... Args, std::enable_if_t<sizeof...(Args) == 1 and (std::is_integral_v<Args> and ...), bool>>
    T Array<V, R>::at(Args... in) const
    {
        auto index = (in, ...);
        return linear_index<T>(index);
    }

    #ifdef _MSC_VER
        #pragma warning(pop)
    #endif

    template<is_boxable V, uint64_t R>
    template<is_boxable T>
    void Array<V, R>::set(uint64_t i, T value)
    {
        jl_arrayset((jl_array_t*) _content->value(), box<T>(value), i);
    }

    template<is_boxable V, uint64_t R>
    auto Array<V, R>::front()
    {
        return linear_index(0);
    }

    template<is_boxable V, uint64_t R>
    auto Array<V, R>::begin()
    {
        return Iterator(0, this);
    }

    template<is_boxable V, uint64_t R>
    auto Array<V, R>::begin() const
    {
        return ConstIterator(0, const_cast<Array<V, R>*>(this));
    }

    template<is_boxable V, uint64_t R>
    auto Array<V, R>::end()
    {
        return Iterator(get_n_elements(), this);
    }

    template<is_boxable V, uint64_t R>
    auto Array<V, R>::end() const
    {
        return ConstIterator(get_n_elements(), const_cast<Array<V, R>*>(this));
    }

    template<is_boxable V, uint64_t R>
    template<is_unboxable T>
    T Array<V, R>::front() const
    {
        return static_cast<T>(linear_index(0));
    }

    template<is_boxable V, uint64_t R>
    auto Array<V, R>::back()
    {
        return linear_index(get_n_elements() - 1);
    }

    template<is_boxable V, uint64_t R>
    template<is_unboxable T>
    T Array<V, R>::back() const
    {
        return linear_index<T>(get_n_elements() - 1);
    }

    template<is_boxable V, uint64_t R>
    uint64_t Array<V, R>::get_n_elements() const
    {
        return reinterpret_cast<const jl_array_t*>(this->operator const unsafe::Value*())->length;
    }

    template<is_boxable V, uint64_t R>
    uint64_t Array<V, R>::size(uint64_t dimension_index) const
    {
        auto* as = reinterpret_cast<const jl_array_t*>(this->operator const unsafe::Value*());
        return jl_array_dim(as, dimension_index);
    }

    template<is_boxable V, uint64_t R>
    bool Array<V, R>::empty() const
    {
        return reinterpret_cast<const jl_array_t*>(this->operator const unsafe::Value*())->length == 0;
    }

    template<is_boxable V, uint64_t R>
    void Array<V, R>::reserve(uint64_t n)
    {
        jl_array_sizehint(reinterpret_cast<jl_array_t*>(this->operator unsafe::Value*()), n);
    }

    template<is_boxable V, uint64_t R>
    void* Array<V, R>::data()
    {
        return (operator unsafe::Array*())->data;
    }

    // ###

    template<is_boxable V>
    Vector<V>::Vector()
        : Array<V, 1>((unsafe::Value*) unsafe::new_array((unsafe::Value*) as_julia_type<V>::type(), 0), nullptr)
    {}

    template<is_boxable V>
    Vector<V>::Vector(const std::vector<V>& vec)
        : Array<V, 1>((unsafe::Value*) unsafe::new_array((unsafe::Value*) as_julia_type<V>::type(), vec.size()), nullptr)
    {
        for (uint64_t i = 0; i < vec.size(); ++i)
            jl_arrayset(reinterpret_cast<jl_array_t*>(this->operator unsafe::Value*()), box(vec.at(i)), i);
    }

    template<is_boxable V>
    Vector<V>::Vector(V* data, uint64_t size)
        : Array<V, 1>(data, size)
    {}

    template<is_boxable V>
    Vector<V>::Vector(Proxy* owner)
        : Array<V, 1>(owner)
    {}

    template<is_boxable V>
    Vector<V>::Vector(unsafe::Value* value, jl_sym_t* symbol)
        : Array<V, 1>(value, symbol)
    {}

    template<is_boxable V>
    Vector<V>::Vector(const GeneratorExpression& gen)
        : Array<V, 1>([&]() -> unsafe::Value* {
            static unsafe::Function* collect = unsafe::get_function(jl_base_module, "collect"_sym);
            return unsafe::call(collect, gen.operator jl_value_t *());
        }())
    {}

    template<is_boxable V>
    void Vector<V>::insert(uint64_t pos, V value)
    {
        static unsafe::Value* insert = jl_get_function(jl_base_module, "insert!");

        gc_pause;
        jl_call3(insert, _content->value(), jl_box_uint64(pos + 1), box(value));
        forward_last_exception();
        gc_unpause;
    }

    template<is_boxable V>
    void Vector<V>::erase(uint64_t pos)
    {
        static unsafe::Value* deleteat = jl_get_function(jl_base_module, "deleteat!");

        gc_pause;
        jl_call2(deleteat, _content->value(), jl_box_uint64(pos + 1));
        forward_last_exception();
        gc_unpause;
    }

    template<is_boxable V>
    template<is_boxable T>
    void Vector<V>::push_front(T value)
    {
        gc_pause;
        auto* array = (jl_array_t*) _content->value();
        jl_array_grow_beg(array, 1);
        jl_arrayset((unsafe::Array*) _content->value(), box<V>(value), 0);
        gc_unpause;
    }

    template<is_boxable V>
    template<is_boxable T>
    void Vector<V>::push_back(T value)
    {
        gc_pause;
        auto* array = (jl_array_t*) _content->value();
        jl_array_grow_end(array, 1);
        jl_arrayset((unsafe::Array*) _content->value(), box<V>(value), jl_array_len(array)-1);
        gc_unpause;
    }
}