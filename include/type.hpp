// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>
#include <include/proxy.hpp>
#include <include/symbol.hpp>

namespace jluna
{
    class Type : public Proxy
    {
        public:

            struct FieldDescription
            {
                Type* owner_type;
                Symbol name;
                Type type;
                size_t index;
            };

            /// @brief ctor
            /// @param value
            Type(jl_datatype_t* value);

            /// @brief get direct super type
            Type super() const;

            /// @brief get name
            Symbol name() const;

            /// @brief get parameter names
            std::vector<Symbol> parameters() const;

            /// @brief get parameter types
            std::vector<Symbol> types() const;

            /// @brief get number of fields
            /// @returns size_t
            size_t get_n_fields() const;

            /// @brief get field names
            /// @returns vector
            std::vector<FieldDescription> get_fields() const;

            /// @brief if type is singleton, get instance of that singleton
            /// @returns instance ptr if singleton-type, nullptr otherwise
            Any* get_singleton_instance() const;

            /// @brief is current type a subtype of arg
            /// @param other
            /// @returns true if subtype, false otherwise
            bool is_subtype_of(Type&) const;

            /// @brief is current type a super of arg
            /// @param other
            /// @returns true if super, false otherwise
            bool is_supertype_of(Type&) const;

            /// @brief is current type the same as arg
            /// @param other
            /// @returns true if both are the same type, false otherwise
            bool is_same_as(Type&) const;

            /// @brief is primitive type
            /// @returns bool
            bool is_primitive() const;

            /// @brief is struct type
            /// @returns bool
            bool is_struct_type() const;

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

            /// @brief is array type
            /// @returns bool
            bool is_array_type() const;

            /// @brief is opaque closure type
            /// @returns bool
            bool is_opaque_closure_type() const;

            /// @brief is tuple type
            /// @returns bool
            bool is_tuple_type() const;

            /// @brief is tuple type
            /// @returns bool
            bool is_named_tuple_type() const;

            /// @brief is type type
            /// @returns bool
            bool is_type_type() const;

            /// @brief is type a union of types
            /// @returns bool
            bool is_union_type() const;

            /// @brief is type a union all type
            /// @returns bool
            bool is_unionall_type() const;

            /// @brief is mutable
            /// @returns bool
            bool is_mutable() const;
    };
}