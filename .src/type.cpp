// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#include <include/type.hpp>
#include <include/symbol.hpp>

namespace jluna
{
    Type::Type() = default;

    Type::Type(jl_datatype_t* value)
        : Proxy((unsafe::Value*) value, (value->name == NULL ? jl_symbol("Union{}") : value->name->name))
    {}

    Type::Type(Proxy* owner)
        : Proxy(*owner)
    {
        gc_pause;
        detail::assert_type((unsafe::DataType*) jl_typeof(owner->operator unsafe::Value*()), (unsafe::DataType*) jl_type_type);
        gc_unpause;
    }

    Type Type::unroll() const
    {
        gc_pause;
        static jl_function_t* unroll = unsafe::get_function("jluna"_sym, "unroll_type"_sym);
        auto* out = jluna::safe_call(unroll, get());
        gc_unpause;
        return Type((jl_datatype_t*) out);
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
        return Type(get()->super);
    }

    Symbol Type::get_symbol() const
    {
        return get()->name->name;
    }

    size_t Type::get_n_fields() const
    {
        gc_pause;
        static jl_function_t* get_n_fields = unsafe::get_function("jluna"_sym, "get_n_fields"_sym);
        auto* out = jluna::safe_call(get_n_fields, get());
        gc_unpause;
        return unbox<size_t>(out);
    }

    std::vector<std::pair<Symbol, Type>> Type::get_fields() const
    {
        gc_pause;
        static jl_function_t* get_fields = unsafe::get_function("jluna"_sym, "get_fields"_sym);
        auto* out = jluna::safe_call(get_fields, get());
        gc_unpause;
        return unbox<std::vector<std::pair<Symbol, Type>>>(out);
    }

    std::vector<std::pair<Symbol, Type>> Type::get_parameters() const
    {
        gc_pause;
        static jl_function_t* get_parameters = unsafe::get_function("jluna"_sym, "get_parameters"_sym);
        auto* out = jluna::safe_call(get_parameters, get());
        gc_unpause;
        return unbox<std::vector<std::pair<Symbol, Type>>>(out);
    }

    size_t Type::get_n_parameters() const
    {
        gc_pause;
        static jl_function_t* get_n_fields = unsafe::get_function("jluna"_sym, "get_n_parameters"_sym);
        auto* out = jluna::safe_call(get_n_fields, get());
        gc_unpause;
        return unbox<size_t>(out);
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
        gc_pause;
        static auto* is_identical = unsafe::get_function(jl_base_module, "==="_sym);
        auto* out = unsafe::call(is_identical, (unsafe::Value*) get(), (unsafe::Value*) other.get());
        gc_unpause;
        return out;
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
        gc_pause;
        static jl_function_t* is_name_typename = unsafe::get_function("jluna"_sym, "is_name_typename"_sym);
        auto out = unbox<bool>(jluna::safe_call(is_name_typename, get(), other.get()));
        gc_unpause;
        return out;
    }

    bool Type::typename_is(const std::string& symbol)
    {
        gc_pause;
        static jl_function_t* is_name_typename = unsafe::get_function("jluna"_sym, "is_name_typename"_sym);
        auto out = unbox<bool>(jluna::safe_call(is_name_typename, get(), jl_eval_string(("Main.eval(Symbol(\"" + symbol + "\"))").c_str())));
        gc_unpause;
        return out;
    }
}

