// 
// Copyright 2022 Clemens Cords
// Created on 16.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/proxy.hpp>

namespace jluna
{
    class GeneratorExpression;
    GeneratorExpression operator""_gen(const char*, size_t);

    class GeneratorExpression
    {
        friend GeneratorExpression operator""_gen(const char*, size_t);

        public:
            class ForwardIterator;

            auto begin();
            auto end();

        protected:
            GeneratorExpression(Any*);
            ~GeneratorExpression();

        private:
            Any* get();
            Int64 length();
            size_t _value_key;
            Any* _value_ref;

            static inline jl_function_t* _iterate = nullptr;
    };

    class GeneratorExpression::ForwardIterator
    {
        public:
            ForwardIterator(GeneratorExpression*, Int64);

            Any* operator*();

            void operator++();
            void operator++(int);

            bool operator==(const GeneratorExpression::ForwardIterator& other) const;
            bool operator!=(const GeneratorExpression::ForwardIterator& other) const;

        private:
            GeneratorExpression* _owner;
            Int64 _state;
    };
}