// 
// Copyright 2022 Clemens Cords
// Created on 16.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/proxy.hpp>

namespace jluna
{
    /// @brief treat the string as a generator expression and return iterable, lazy-eval range
    class GeneratorExpression;
    GeneratorExpression operator""_gen(const char*, size_t);

    /// @brief wrapper for Base.Generator, only create by _gen
    /// @note expression is evaluated lazily
    class GeneratorExpression
    {
        friend GeneratorExpression operator""_gen(const char*, size_t);

        public:
            /// @brief dtor
            ~GeneratorExpression();

            /// @brief iterator class, can only iterate in one direction
            class ForwardIterator;

            /// @brief get iterator to front of range
            /// @returns iterator
            ForwardIterator begin() const;

            /// @brief get iterator to past-the-end element
            /// @returns iterator
            ForwardIterator end() const;

            /// @brief get length of iterable component
            size_t size() const;

        protected:
            /// @brief ctor
            /// @param pointer to Base.Generator
            GeneratorExpression(unsafe::Value*);

        private:
            Int64 _length;

            unsafe::Value* get() const;
            size_t _value_key;
            unsafe::Value* _value_ref;

            static inline jl_function_t* _iterate = nullptr;
    };

    /// @brief Iterator
    class GeneratorExpression::ForwardIterator
    {
        public:
            /// @brief Ctor
            /// @param pointer
            /// @param state: state in Int64, for begin() set State 0, for end() set State GeneratorExpression.lenght()
            ForwardIterator(const GeneratorExpression*, Int64);

            /// @brief dereference to julia object
            unsafe::Value* operator*();

            /// @brief cast to C++ type
            template<Unboxable T>
            operator T()
            {
                return unbox<T>(this->operator*());
            }

            /// @brief prefix advance by 1 state
            void operator++();

            /// @brief postfix advance by 1 state
            void operator++(int);

            /// @brief comparison
            /// @param other
            /// @returns true if iterators point to the same expression and have the same state
            bool operator==(const GeneratorExpression::ForwardIterator& other) const;

            /// @brief comparison
            /// @param other
            /// @returns false if iterators point to the same expression and have the same state
            bool operator!=(const GeneratorExpression::ForwardIterator& other) const;

        private:
            const GeneratorExpression* _owner;
            Int64 _state;
            bool _is_end = false;
    };
}