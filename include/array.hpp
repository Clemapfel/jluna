//
// Copyright 2022 Clemens Cords
// Created on 11.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/concepts.hpp>
#include <include/proxy.hpp>
#include <include/generator_expression.hpp>

namespace jluna
{
    template<is_boxable T>
    class Vector;

    /// @brief wrapper for julia-side Array{Value_t, Rank}
    /// @tparam Value_t: boxable value ype
    /// @tparam Rank: rank of the array
    template<is_boxable Value_t, uint64_t Rank>
    class Array : public Proxy
    {
        public:
            friend class ConstIterator;
            class Iterator;

            /// @brief value type
            using value_type = Value_t;

            /// @brief dimensionality, equivalent to julia-side Array{Value_t, Rank}
            static constexpr uint64_t rank = Rank;

            /// @brief construct as child of already existing proxy, implicit
            /// @param proxy: pointer to already created proxy
            Array(Proxy*);

            /// @brief constructor
            /// @param value: pointer to Julia-side allocated array
            /// @param symbol: name of Julia-side variable, or nullptr for anonymous variable
            Array(unsafe::Value* value, jl_sym_t* symbol = nullptr);

            /// @brief constructor, initialize values as undef, implicit
            /// @param size: target size of the new array
            Array(uint64_t);

            /// @brief construct as thin wrapper, does not invoke copy
            /// @warning user is responsible for data being properly formatted and staying in scope
            /// @param data: raw pointer to Julia-side data
            /// @param size_per_dimension: size along all dimensions, where Rank is the number of dimensions
            template<typename... Dims>
            Array(Value_t*, Dims... size_per_dimension);

            /// @brief julia-style array comprehension indexing
            /// @param generator_expression: generator expression used for the index list
            /// @returns new array, result of Julia-side getindex(this, range)
            jluna::Vector<Value_t> at(const GeneratorExpression&) const;

            /// @brief julia-style list indexing
            /// @param range: iterable range with indices
            /// @returns new array, result of Julia-side getindex(this, range)
            jluna::Vector<Value_t> at(const std::vector<uint64_t>& range) const;

            /// @brief julia-style list indexing
            /// @param list: initializer list with indices
            /// @returns new array, result of Julia-side getindex(this, range)
            template<is_boxable T>
            jluna::Vector<Value_t> at(std::initializer_list<T>&&) const;

            /// @brief multi-dimensional indexing
            /// @param n: Rank-many integers
            /// @returns non-const (assignable) iterator to value
            template<typename... Args, std::enable_if_t<Rank >= 2 and sizeof...(Args) == Rank and (std::is_integral_v<Args> and ...), bool> = true>
            auto at(Args... in);

            /// @brief multi-dimensional indexing
            /// @param n: Rank-many integers
            /// @returns unboxed value
            template<is_unboxable T = Value_t, typename... Args, std::enable_if_t<Rank >= 2 and sizeof...(Args) == Rank and (std::is_integral_v<Args> and ...), bool> = true>
            T at(Args... in) const;

            /// @brief linear indexing
            /// @param n: singular index
            /// @returns non-const (assignable) iterator to value
            template<typename... Args, std::enable_if_t<sizeof...(Args) == 1 and (std::is_integral_v<Args> and ...), bool> = true>
            auto at(Args... in);

            /// @brief linear indexing
            /// @param n: singular index
            /// @returns unboxed value
            template<is_unboxable T = Value_t, typename... Args, std::enable_if_t<sizeof...(Args) == 1 and (std::is_integral_v<Args> and ...), bool> = true>
            T at(Args... in) const;

            /// @brief manually assign a value using a linear index
            /// @param index: linear index, 0-based
            /// @param value: new value
            template<is_boxable T = Value_t>
            void set(uint64_t i, T);

            /// @brief get number of elements
            /// @returns Base.length
            uint64_t get_n_elements() const;

            /// @brief get size in specific dimension
            /// @param dimension_index: 0-based
            /// @returns result of Base.size(array, dimension_index)
            uint64_t size(uint64_t dimension_index) const;

            /// @brief get iterator to 0-indexed element
            /// @returns assignable iterator
            auto begin();

            /// @brief get iterator to 0-indexed element
            /// @returns const iterator
            auto begin() const;

            /// @brief get iterator to past-the-end element
            /// @returns assignable iterator
            auto end();

            /// @brief get iterator to past-the-end element
            /// @returns const iterator
            auto end() const;

            /// @brief get first element, equivalent to operator[](0)
            /// @returns assignable iterator
            auto front();

            /// @brief get first element, equivalent to operator[](0)
            /// @returns unboxed value
            template<is_unboxable T = Value_t>
            T front() const;

            /// @brief get last valid element
            /// @returns assignable iterator
            auto back();

            /// @brief get last valid element
            /// @returns unboxed value
            template<is_unboxable T = Value_t>
            T back() const;

            /// @brief is empty
            /// @returns true if size is 0 along all dimensions, false otherwise
            bool empty() const;

            /// @brief call Base.sizehint!, allocates array to be of specified size. Useful for optimization
            /// @param size: target size
            void reserve(uint64_t);

            /// @brief cast to unsafe::Value*, implicit
            using Proxy::operator unsafe::Value*;

            /// @brief cast to unsafe::Array*, implicit
            operator unsafe::Array*() const;

            /// @brief expose raw data as void*
            /// @returns void pointer
            void* data();

            /// @brief linear indexing, no bounds checking
            /// @param index: 0-based
            /// @returns assignable iterator to element
            /// @note this function intentionally shadows Proxy::operator[](uint64_t) -> Proxy
            [[deprecated("Use Array:at instead")]] auto operator[](uint64_t);

            /// @brief linear indexing, no bounds checking
            /// @tparam T: return type, usually Value_t
            /// @param index: index, 0-based
            /// @returns unboxed value at given index. May throw jluna::JuliaException if index out of range
            template<is_unboxable T = Value_t>
            [[deprecated("Use Array:at instead")]] T operator[](uint64_t) const;

            /// @brief julia-style array comprehension indexing
            /// @param generator_expression: generator expression used for the index list
            /// @returns new array, result of Julia-side getindex(this, range)
            [[deprecated("Use Array:at instead")]] jluna::Vector<Value_t> operator[](const GeneratorExpression&) const;

            /// @brief julia-style list indexing
            /// @param range: iterable range with indices
            /// @returns new array, result of Julia-side getindex(this, range)
            [[deprecated("Use Array:at instead")]] jluna::Vector<Value_t> operator[](const std::vector<uint64_t>& range) const;

            /// @brief julia-style list indexing
            /// @param list: initializer list with indices
            /// @returns new array, result of Julia-side getindex(this, range)
            template<is_boxable T>
            [[deprecated("Use Array:at instead")]] jluna::Vector<Value_t> operator[](std::initializer_list<T>&&) const;

        protected:
            using Proxy::_content;

            auto linear_index(uint64_t i);

            template<is_unboxable T = Value_t>
            T linear_index(uint64_t) const;

        private:
            void throw_if_index_out_of_range(int index, uint64_t dimension) const;
            uint64_t get_dimension(uint64_t) const;

        public:
            /// @brief non-assignable iterator
            class ConstIterator
            {
                public:
                    /// @brief ctor
                    /// @param index
                    /// @param pointer to array
                    ConstIterator(uint64_t i, Array<Value_t, Rank>*);

                    /// @brief increment
                    /// @returns reference to self
                    auto& operator++();

                    /// @brief post-fix increment
                    /// @returns reference to self
                    auto& operator++(int);

                    /// @brief post-fix decrement
                    /// @returns reference to self
                    auto& operator--();

                    /// @brief post-fix decrement
                    /// @returns reference to self
                    auto& operator--(int);

                    /// @brief equality operator
                    /// @param other: other iterator
                    /// @returns true if both iterators iterat the same array and have the same linear index
                    bool operator==(const ConstIterator&) const;

                    /// @brief inequality operator
                    /// @param other: other iterator
                    /// @returns not (this == other)
                    bool operator!=(const ConstIterator&) const;

                    /// @brief forward to self
                    /// @returns self
                    auto operator*() const;

                    /// @brief forward to assignable
                    /// @returns assignable
                    auto operator*();

                    /// @brief decay into unboxed value, implicit
                    /// @tparam T: value type, not necessarily the same as declared in the array type
                    template<is_unboxable T = Value_t, std::enable_if_t<not is<Proxy, T>, bool> = true>
                    operator T() const;

                    /// @brief decay into proxy, implicit
                    operator Proxy();

                protected:
                    Array<Value_t, Rank>* _owner;
                    uint64_t _index;
            };

            /// @brief assignable iterator
            struct Iterator : public ConstIterator
            {
                /// @brief ctor
                /// @param index
                /// @param pointer to array
                Iterator(uint64_t i, Array<Value_t, Rank>*);

                using ConstIterator::operator*;

                /// @brief assign array being iterated at given position, also assigns Julia-side proxy, even if host proxy is not mutating
                /// @param value: new value
                /// @returns reference to self
                template<is_boxable T = Value_t>
                auto& operator=(T value);

                /// @brief decay into unboxed value
                /// @tparam value-type, not necessarily the same as declared in the array type, implicit
                template<is_unboxable T = Value_t, std::enable_if_t<not std::is_same_v<T, Proxy>, bool> = true>
                operator T() const;

                protected:
                    using ConstIterator::_owner;
                    using ConstIterator::_index;
            };
    };

    /// @brief vector, alias for array of rank 1
    template<is_boxable Value_t>
    class Vector : public Array<Value_t, 1>
    {
        public:
            /// @param default ctor
            Vector();

            /// @brief construct as unnamed proxy from vector
            /// @param vector: C++-side vector
            Vector(const std::vector<Value_t>& vec);

            /// @brief construct from generator expression
            /// @param expression: generator expression, will be fully serialized on constructor call
            Vector(const GeneratorExpression&);

            /// @brief constrct as thin wrapper around data
            /// @warning the user is responsible for data being correctly formatted and staying in scope
            /// @param data: pointer to Julia-side data
            /// @param size: size along the first dimension
            Vector(Value_t* data, uint64_t size);

            /// @brief construct as child of already existing proxy, implicit
            /// @param proxy: proxy
            Vector(Proxy*);

            /// @brief construct from value
            /// @param value: Julia-side value
            /// @param symbol: name of variable, or nullptr if anonymous variable
            Vector(unsafe::Value* value, jl_sym_t* symbol = nullptr);

            /// @brief insert a value
            /// @param pos: linear index, 0-based
            /// @param value: new value
            void insert(uint64_t pos, Value_t value);

            /// @brief erase at specified position
            /// @param pos: linear index, 0-based
            void erase(uint64_t pos);

            /// @brief add to front
            /// @tparam T: type of value, not necessarily the same as the declared array type
            /// @param value: new value
            template<is_boxable T = Value_t>
            void push_front(T value);

            /// @brief add to back
            /// @tparam T: type of value, not necessarily the same as the declared array type
            /// @param value: new value
            template<is_boxable T = Value_t>
            void push_back(T value);

            /// @brief: cast to unsafe::Value*
            using Array<Value_t, 1>::operator unsafe::Value*;

        protected:
            using Array<Value_t, 1>::_content;
    };

    ///@brief typedefs for Array{Any, 1}
    using ArrayAny1d = Array<unsafe::Value*, 1>;

    ///@brief typedefs for Array{Any, 2}
    using ArrayAny2d = Array<unsafe::Value*, 2>;

    ///@brief typedefs for Array{Any, 3}
    using ArrayAny3d = Array<unsafe::Value*, 3>;

    ///@brief typedefs for Array{Any, N}
    using VectorAny = Vector<unsafe::Value*>;
}

#include <.src/array.inl>
#include <.src/array_iterator.inl>