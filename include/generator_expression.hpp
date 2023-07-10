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
    GeneratorExpression operator""_gen(const char*, uint64_t);

    /// @brief wrapper for Base.Generator, only create by _gen
    /// @note expression is evaluated lazily
    class GeneratorExpression
    {
        friend GeneratorExpression operator""_gen(const char*, uint64_t);

        public:
            /// @brief dtor
            ~GeneratorExpression();

            /// @brief iterator class, can only iterate in one direction
            class ForwardIterator;

            /// @brief get iterator to front of range
            /// @returns iterator
            [[nodiscard]] ForwardIterator begin() const;

            /// @brief get iterator to past-the-end element
            /// @returns iterator
            [[nodiscard]] ForwardIterator end() const;

            /// @brief get length of iterable component
            uint64_t size() const;

            /// @brief get julia-side Base.generator object
            explicit operator unsafe::Value*() const;

        protected:
            /// @brief ctor
            /// @param pointer to Base.Generator
            GeneratorExpression(unsafe::Value*);

        private:
            Int64 _length;

            unsafe::Value* get() const;
            uint64_t _value_key;
            unsafe::Value* _value_ref;

            static inline jl_function_t* _iterate = nullptr;
    };

    /// @brief Iterator
    class GeneratorExpression::ForwardIterator
    {
        public:
            /// @brief Ctor
            /// @param pointer: pointer to generator expression
            /// @param state: state in Int64, for begin() set State 0, for end() set State GeneratorExpression.lenght()
            ForwardIterator(const GeneratorExpression*, Int64);

            /// @brief dereference to julia object
            unsafe::Value* operator*();

            /// @brief cast to C++ type
            /// @tparam resulting type
            /// @returns T
            template<is_unboxable T>
            operator T();

            /// @brief prefix advance by 1 state
            void operator++();

            /// @brief postfix advance by 1 state
            void operator++(int);

            /// @brief comparison
            /// @param other: other iterator
            /// @returns true if iterators point to the same expression and have the same state
            bool operator==(const GeneratorExpression::ForwardIterator& other) const;

            /// @brief comparison
            /// @param other: other iterator
            /// @returns false if iterators point to the same expression and have the same state
            bool operator!=(const GeneratorExpression::ForwardIterator& other) const;

        private:
            const GeneratorExpression* _owner;
            Int64 _state;
    };
}

#include <.src/generator_expression.inl>