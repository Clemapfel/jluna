// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>
#include <include/typedefs.hpp>
#include <include/proxy.hpp>
#include <include/symbol.hpp>

namespace jluna
{
    /// @brief forward declaration
    class Type : public Proxy
    {
        public:
            /// @brief default ctor, construct as Nothing
            Type();

            /// @brief construct from type, implicit
            /// @param value: Julia-side type
            Type(jl_datatype_t* value);

            /// @brief construct as child of proxy
            /// @param proxy
            Type(Proxy*);

            /// @brief decay to C-type, implicit
            operator jl_datatype_t*();

            /// @brief unroll type as much as possible
            /// @returns unrolled type
            [[nodiscard]] Type unroll() const;

            /// @brief get direct super type
            /// @returns type
            Type get_super_type() const;

            /// @brief get name
            /// @returns Julia-side name as symbol
            Symbol get_symbol() const;

            /// @brief get number of parameters
            /// @returns uint64_t
            uint64_t get_n_parameters() const;

            /// @brief get parameter names
            /// @returns vector
            std::vector<std::pair<Symbol, Type>> get_parameters() const;

            /// @brief get number of fields
            /// @returns uint64_t
            uint64_t get_n_fields() const;

            /// @brief get field names
            /// @returns vector
            std::vector<std::pair<Symbol, Type>> get_fields() const;

            /// @brief if type is singleton, get instance of that singleton
            /// @returns instance ptr if singleton-type, nullptr otherwise
            unsafe::Value* get_singleton_instance() const;

            /// @brief this <: other
            /// @param other: other type
            /// @returns true if subtype, false otherwise
            bool is_subtype_of(const Type&) const;

            /// @brief this <: other
            /// @param other: other type
            /// @returns true if subtype, false otherwise
            bool operator<(const Type&) const;

            /// @brief other <: this
            /// @param other: other type
            /// @returns true if super, false otherwise
            bool is_supertype_of(const Type&) const;

            /// @brief this <: other
            /// @param other: other type
            /// @returns true if subtype, false otherwise
            bool operator>(const Type&) const;

            /// @brief this === other
            /// @param other: other type
            /// @returns true if both are the same type, false otherwise
            bool is_same_as(const Type&) const;

            /// @brief this === other
            /// @param other: other type
            /// @returns true if both are the same type, false otherwise
            bool operator==(const Type&) const;

            /// @brief not(this === other)
            /// @param other: other type
            /// @returns true if both are the same type, false otherwise
            bool operator!=(const Type&) const;

            /// @brief is primitive type
            /// @returns bool
            bool is_primitive() const;

            /// @brief is struct type
            /// @returns bool
            bool is_struct_type() const;

            /// @brief is mutable
            /// @returns bool
            bool is_declared_mutable() const;

            /// @brief is bits type
            /// @returns bool
            bool is_isbits() const;

            /// @brief is singleton
            /// @returns bool
            bool is_singleton() const;

            /// @brief is abstract
            /// @returns bool
            bool is_abstract_type() const;

            /// @brief is abstract ref
            /// @returns bool
            bool is_abstract_ref_type() const;

            /// @brief check if .name property of unrolled type is equal to typename
            /// @param symbol: name of type, evaluated to Main.eval(symbol)
            /// @returns bool
            bool typename_is(const std::string& symbol);

            /// @brief check if .name property of unrolled type is equal to typename
            /// @param type
            /// @returns bool
            bool typename_is(const Type& other);

        private:
            jl_datatype_t* get() const;
    };

    /// @brief pre-initialized version of Julia-side type AbstractArray
    inline Type AbstractArray_t;

    /// @brief pre-initialized version of Julia-side type AbstractChar
    inline Type AbstractChar_t;

    /// @brief pre-initialized version of Julia-side type AbstractFloat
    inline Type AbstractFloat_t;

    /// @brief pre-initialized version of Julia-side type AbstractString
    inline Type AbstractString_t;

    /// @brief pre-initialized version of Julia-side type Any
    inline Type Any_t;

    /// @brief pre-initialized version of Julia-side type Array
    inline Type Array_t;

    /// @brief pre-initialized version of Julia-side type Bool
    inline Type Bool_t;

    /// @brief pre-initialized version of Julia-side type Char
    inline Type Char_t;

    /// @brief pre-initialized version of Julia-side type DataType
    inline Type DataType_t;

    /// @brief pre-initialized version of Julia-side type DenseArray
    inline Type DenseArray_t;

    /// @brief pre-initialized version of Julia-side type Exception
    inline Type Exception_t;

    /// @brief pre-initialized version of Julia-side type Expr
    inline Type Expr_t;

    /// @brief pre-initialized version of Julia-side type Float16
    inline Type Float16_t;

    /// @brief pre-initialized version of Julia-side type Float32
    inline Type Float32_t;

    /// @brief pre-initialized version of Julia-side type Float64
    inline Type Float64_t;

    /// @brief pre-initialized version of Julia-side type Function
    inline Type Function_t;

    /// @brief pre-initialized version of Julia-side type GlobalRef
    inline Type GlobalRef_t;

    /// @brief pre-initialized version of Julia-side type IO
    inline Type IO_t;

    /// @brief pre-initialized version of Julia-side type Int128
    inline Type Int128_t;

    /// @brief pre-initialized version of Julia-side type Int16
    inline Type Int16_t;

    /// @brief pre-initialized version of Julia-side type Int32
    inline Type Int32_t;

    /// @brief pre-initialized version of Julia-side type Int64
    inline Type Int64_t;

    /// @brief pre-initialized version of Julia-side type Int8
    inline Type Int8_t;

    /// @brief pre-initialized version of Julia-side type Integer
    inline Type Integer_t;

    /// @brief pre-initialized version of Julia-side type LineNumber
    inline Type LineNumberNode_t;

    /// @brief pre-initialized version of Julia-side type Method
    inline Type Method_t;

    /// @brief pre-initialized version of Julia-side type Missing
    inline Type Missing_t;

    /// @brief pre-initialized version of Julia-side type Module
    inline Type Module_t;

    /// @brief pre-initialized version of Julia-side type NTuple
    inline Type NTuple_t;

    /// @brief pre-initialized version of Julia-side type NamedTuple
    inline Type NamedTuple_t;

    /// @brief pre-initialized version of Julia-side type Nothing
    inline Type Nothing_t;

    /// @brief pre-initialized version of Julia-side type Number
    inline Type Number_t;

    /// @brief pre-initialized version of Julia-side type Pair
    inline Type Pair_t;

    /// @brief pre-initialized version of Julia-side type Ptr
    inline Type Ptr_t;

    /// @brief pre-initialized version of Julia-side type QuoteNode
    inline Type QuoteNode_t;

    /// @brief pre-initialized version of Julia-side type Real
    inline Type Real_t;

    /// @brief pre-initialized version of Julia-side type Ref
    inline Type Ref_t;

    /// @brief pre-initialized version of Julia-side type Signed
    inline Type Signed_t;

    /// @brief pre-initialized version of Julia-side type String
    inline Type String_t;

    /// @brief pre-initialized version of Julia-side type Symbol
    inline Type Symbol_t;

    /// @brief pre-initialized version of Julia-side type Task
    inline Type Task_t;

    /// @brief pre-initialized version of Julia-side type Tuple
    inline Type Tuple_t;

    /// @brief pre-initialized version of Julia-side type Type
    inline Type Type_t;

    /// @brief pre-initialized version of Julia-side type TypeVar
    inline Type TypeVar_t;

    /// @brief pre-initialized version of Julia-side type UInt128
    inline Type UInt128_t;

    /// @brief pre-initialized version of Julia-side type UInt16
    inline Type UInt16_t;

    /// @brief pre-initialized version of Julia-side type UInt32
    inline Type UInt32_t;

    /// @brief pre-initialized version of Julia-side type UInt64
    inline Type UInt64_t;

    /// @brief pre-initialized version of Julia-side type UInt8
    inline Type UInt8_t;

    /// @brief pre-initialized version of Julia-side type UndefInitializer
    inline Type UndefInitializer_t;

    /// @brief pre-initialized version of Julia-side type Union
    inline Type Union_t;

    /// @brief pre-initialized version of Julia-side type UnionAll
    inline Type UnionAll_t;

    /// @brief pre-initialized version of Julia-side type UnionEmpty
    inline Type UnionEmpty_t;

    /// @brief pre-initialized version of Julia-side type Unsigned
    inline Type Unsigned_t;

    /// @brief pre-initialized version of Julia-side type VecElement
    inline Type VecElement_t;

    /// @brief pre-initialized version of Julia-side type WeakRef
    inline Type WeakRef_t;
}

#include <.src/type.inl>