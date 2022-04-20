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
    template<is_boxable Value_t, size_t Rank>
    class Array : public Proxy
    {
        public:
            friend class ConstIterator;
            class Iterator;

            /// @brief value type
            using value_type = Value_t;

            /// @brief dimensionality, equivalent to julia-side Array{Value_t, Rank}
            static constexpr size_t rank = Rank;

            /// @brief ctor
            /// @param value
            /// @param owner
            /// @param symbol
            Array(Proxy*);

            /// @brief ctor from proxy
            /// @param proxy
            Array(unsafe::Value* value, jl_sym_t* = nullptr);

            /// @brief ctor as thin wrapper data, does not invoke copy
            /// @warning user is responsible for data being properly formatted and for it staying in scope
            /// @param data
            /// @param size_per_dimension
            template<typename... Dims>
            Array(Value_t*, Dims... size_per_dimension);

            /// @brief linear indexing, no bounds checking
            /// @param index, 0-based
            /// @returns assignable iterator to element
            /// @note this function intentionally shadows Proxy::operator[](size_t) -> Proxy
            auto operator[](size_t);

            /// @brief julia-style array comprehension indexing
            /// @param generator_expression
            /// @returns new array result of Julia-side getindex(this, range)
            jluna::Vector<Value_t> operator[](const GeneratorExpression&) const;

            /// @brief julia-style list indexing
            /// @param range: iterable range with indices
            /// @returns new array result of Julia-side getindex(this, range)
            jluna::Vector<Value_t> operator[](const std::vector<size_t>& range) const;

            /// @brief julia-style list indexing
            /// @param initializer list with indices
            /// @returns new array result of Julia-side getindex(this, range)
            template<is_boxable T>
            jluna::Vector<Value_t> operator[](std::initializer_list<T>&&) const;

            /// @brief linear indexing, no bounds checking
            /// @tparam return type
            /// @param index, 0-based
            /// @returns unboxed value
            template<is_unboxable T = Value_t>
            T operator[](size_t) const;

            /// @brief multi-dimensional indexing
            /// @tparam integral type
            /// @param n integrals, where n is the rank of the array
            /// @returns assignable iterator to value
            template<typename... Args, std::enable_if_t<sizeof...(Args) == Rank and (std::is_integral_v<Args> and ...), bool> = true>
            auto at(Args... in);

            /// @brief multi-dimensional indexing
            /// @tparam integral type
            /// @param n integrals, where n is the rank of the array
            /// @returns unboxed value
            template<is_unboxable T = Value_t, typename... Args, std::enable_if_t<sizeof...(Args) == Rank and (std::is_integral_v<Args> and ...), bool> = true>
            T at(Args... in) const;

            /// @brief manually assign a value using a linear index
            /// @param index: 0-based
            /// @param value
            template<is_boxable T = Value_t>
            void set(size_t i, T);

            /// @brief get number of elements, equal to Base.length
            /// @returns length
            size_t get_n_elements() const;

            /// @brief get size in specific dimension
            /// @param dimension_index: 0-based
            /// @returns result of Base.size(array, dimension_index)
            size_t size(size_t dimension_index) const;

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
            /// @returns true if 0 element, false otherwise
            bool empty() const;

            /// @brief call Base.sizehint!, allocates array to be of size
            /// @param target_size
            void reserve(size_t);

            /// @brief cast to unsafe::Value*
            using Proxy::operator unsafe::Value*;

            /// @brief cast to unsafe::Array*
            operator unsafe::Array*() const;

        protected:
            using Proxy::_content;

        private:
            void throw_if_index_out_of_range(int index, size_t dimension) const;
            size_t get_dimension(int) const;

        public:
            class ConstIterator
            {
                public:
                    /// @brief ctor
                    /// @param index
                    /// @param pointer to array
                    ConstIterator(size_t i, Array<Value_t, Rank>*);

                    /// @brief increment
                    auto& operator++();

                    /// @brief post-fix increment
                    auto& operator++(int);

                    /// @brief post-fix decrement
                    auto& operator--();

                    /// @brief post-fix decrement
                    auto& operator--(int);

                    /// @brief equality operator
                    /// @param other
                    /// @returns bool
                    bool operator==(const ConstIterator&) const;

                    /// @brief inequality operator
                    /// @param other
                    /// @returns bool
                    bool operator!=(const ConstIterator&) const;

                    /// @brief forward to self
                    auto operator*() const;

                    /// @brief forward to assignable
                    auto operator*();

                    /// @brief decay into unboxed value
                    /// @tparam value-type, not necessarily the same as declared in the array type
                    template<is_unboxable T = Value_t, std::enable_if_t<not is<Proxy, T>, bool> = true>
                    operator T() const;

                    /// @brief decay into proxy
                    operator Proxy();

                protected:
                    Array<Value_t, Rank>* _owner;
                    size_t _index;
            };

            struct Iterator : public ConstIterator
            {
                /// @brief ctor
                /// @param index
                /// @param pointer to array
                Iterator(size_t i, Array<Value_t, Rank>*);

                using ConstIterator::operator*;

                /// @brief assign value, also assign value of proxy, regardless of whether it is mutating
                /// @param value
                /// @returns reference to self
                template<is_boxable T = Value_t>
                auto& operator=(T value);

                /// @brief decay into unboxed value
                /// @tparam value-type, not necessarily the same as declared in the array type
                template<is_unboxable T = Value_t, std::enable_if_t<not std::is_same_v<T, Proxy>, bool> = true>
                operator T() const;

                protected:
                    using ConstIterator::_owner;
                    using ConstIterator::_index;
            };
    };

    /// typedefs of vectors that can attach to anything
    using Array1d = Array<unsafe::Value*, 1>;
    using Array2d = Array<unsafe::Value*, 2>;
    using Array3d = Array<unsafe::Value*, 3>;

    /// @brief vector typedef
    template<is_boxable Value_t>
    class Vector : public Array<Value_t, 1>
    {
        public:
            /// @param default ctor
            Vector();

            /// @brief ctor as unnamed proxy from vector
            /// @param vector
            Vector(const std::vector<Value_t>& vec);

            /// @brief ctor from generator expression. Only available for 1d arrays
            /// @param generator_expression
            Vector(const GeneratorExpression&);

            /// @brief ctor as thin wrapper around data
            /// @warning the user is responsible for data being correctly formatted
            /// @param data
            /// @param size
            Vector(Value_t* data, size_t size);

            /// @brief ctor from proxy
            /// @param proxy
            Vector(Proxy*);

            /// @brief ctor
            /// @param value
            /// @param symbol
            Vector(unsafe::Value* value, jl_sym_t* = nullptr);

            /// @brief insert
            /// @param linear index, 0-based
            /// @param value
            void insert(size_t pos, Value_t value);

            /// @brief erase
            /// @param linear index, 0-based
            void erase(size_t pos);

            /// @brief add to front
            /// @tparam type of value, not necessarily the same as the declared array type
            /// @param value
            template<is_boxable T = Value_t>
            void push_front(T value);

            /// @brief add to back
            /// @tparam type of value, not necessarily the same as the declared array type
            /// @param value
            template<is_boxable T = Value_t>
            void push_back(T value);

            /// @brief: cast to unsafe::Value*
            using Array<Value_t, 1>::operator unsafe::Value*;

        protected:
            using Array<Value_t, 1>::_content;
    };
}

#include <.src/array.inl>
#include <.src/array_iterator.inl>