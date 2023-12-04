// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#include <include/type.hpp>
#include <include/symbol.hpp>

namespace jluna
{
    namespace detail
    {
        inline bool is_union_empty(jl_datatype_t* type)
        {
            static auto* propertynames = jl_get_function(jl_main_module, "propertynames");
            static auto* sizeof_f = jl_get_function(jl_main_module, "sizeof");
            return jl_unbox_int64(jl_call1(sizeof_f, jl_call1(propertynames, (jl_value_t*) type))) == 0;
        }
    }

    Type::Type() = default;

    Type::Type(jl_datatype_t* value)
        : Proxy((unsafe::Value*) value, (detail::is_union_empty(value) ? jl_symbol("Union{}") : value->name->name))
    {}

    Type::Type(Proxy* owner)
        : Proxy(*owner)
    {
        detail::assert_type((unsafe::DataType*) jl_typeof(owner->operator unsafe::Value*()), (unsafe::DataType*) jl_type_type);
    }

    Type Type::unroll() const
    {
        static jl_function_t* unroll = unsafe::get_function("jluna"_sym, "unroll_type"_sym);
        return {(jl_datatype_t*) jluna::safe_call(unroll, get())};
    }

    Type::operator jl_datatype_t*()
    {
        return get();
    }

    jl_datatype_t* Type::get() const
    {
        return (jl_datatype_t*) Proxy::operator const unsafe::Value*();
    }

    Type Type::get_super_type() const
    {
        return {get()->super};
    }

    Symbol Type::get_symbol() const
    {
        return get()->name->name;
    }

    uint64_t Type::get_n_fields() const
    {
        static jl_function_t* get_n_fields = unsafe::get_function("jluna"_sym, "get_n_fields"_sym);
        return unbox<uint64_t>(jluna::safe_call(get_n_fields, get()));
    }

    std::vector<std::pair<Symbol, Type>> Type::get_fields() const
    {
        static jl_function_t* get_fields = unsafe::get_function("jluna"_sym, "get_fields"_sym);
        return unbox<std::vector<std::pair<Symbol, Type>>>(jluna::safe_call(get_fields, get()));
    }

    std::vector<std::pair<Symbol, Type>> Type::get_parameters() const
    {
        static jl_function_t* get_parameters = unsafe::get_function("jluna"_sym, "get_parameters"_sym);
        return unbox<std::vector<std::pair<Symbol, Type>>>(jluna::safe_call(get_parameters, get()));
    }

    uint64_t Type::get_n_parameters() const
    {
        static jl_function_t* get_n_fields = unsafe::get_function("jluna"_sym, "get_n_parameters"_sym);
        return unbox<uint64_t>(jluna::safe_call(get_n_fields, get()));
    }

    unsafe::Value* Type::get_singleton_instance() const
    {
        return get()->instance;
    }

    bool Type::is_subtype_of(const Type& other) const
    {
        return jl_subtype((unsafe::Value*) get(), (unsafe::Value*) other.get());
    }

    bool Type::operator<(const Type& other) const
    {
        return this->is_subtype_of(other);
    }

    bool Type::is_supertype_of(const Type& other) const
    {
        return other.is_subtype_of(*this);
    }

    bool Type::operator>(const Type& other) const
    {
        return this->is_supertype_of(other);
    }

    bool Type::is_same_as(const Type& other) const
    {
        static auto* is_identical = unsafe::get_function(jl_base_module, "==="_sym);
        return unsafe::call(is_identical, (unsafe::Value*) get(), (unsafe::Value*) other.get());
    }

    bool Type::operator==(const Type& other) const
    {
        return this->is_same_as(other);
    }

    bool Type::operator!=(const Type& other) const
    {
        return not this->is_same_as(other);
    }

    bool Type::is_primitive() const
    {
        return jl_is_primitivetype(get());
    }

    bool Type::is_struct_type() const
    {
        return jl_is_structtype(get());
    }

    bool Type::is_isbits() const
    {
        return jl_isbits(get());
    }

    bool Type::is_singleton() const
    {
        return get()->instance != nullptr;
    }

    bool Type::is_abstract_type() const
    {
        return jl_is_abstracttype(get());
    }

    bool Type::is_abstract_ref_type() const
    {
        return jl_is_abstract_ref_type((unsafe::Value*) get());
    }

    bool Type::is_declared_mutable() const
    {
        return jl_is_mutable_datatype(get());
    }

    bool Type::typename_is(const Type& other)
    {
        static jl_function_t* is_name_typename = unsafe::get_function("jluna"_sym, "is_name_typename"_sym);
        return unbox<bool>(jluna::safe_call(is_name_typename, get(), other.get()));
    }

    bool Type::typename_is(const std::string& symbol)
    {
        static jl_function_t* is_name_typename = unsafe::get_function("jluna"_sym, "is_name_typename"_sym);
        return unbox<bool>(jluna::safe_call(is_name_typename, get(), jl_eval_string(("Main.eval(Symbol(\"" + symbol + "\"))").c_str())));
    }
}

