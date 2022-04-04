// 
// Copyright 2022 Clemens Cords
// Created on 11.01.22 by clem (mail@clemens-cords.com)
//

#include <sstream>
#include <deque>
#include <sstream>
#include <iostream>

#include <include/proxy.hpp>
#include <include/type.hpp>

namespace jluna
{
    // no owner
    Proxy::ProxyValue::ProxyValue(unsafe::Value* value, jl_sym_t* id)
        : _is_mutating(id != nullptr)
    {
        if (value == nullptr)
            return;

        static jl_function_t* make_unnamed_proxy_id = unsafe::get_function("jluna.memory_handler"_sym, "make_unnamed_proxy_id"_sym);
        static jl_function_t* make_named_proxy_id = unsafe::get_function("jluna.memory_handler"_sym, "make_named_proxy_id"_sym);

        auto gc = GCSentinel();

        _value_key = detail::create_reference(value);
        _value_ref = detail::get_reference(_value_key);

        if (id == nullptr)
            _id_key = detail::create_reference(jl_call1(make_unnamed_proxy_id, jl_box_uint64(_value_key)));
        else
            _id_key = detail::create_reference(jl_call2(make_named_proxy_id, (unsafe::Value*) id, jl_nothing));

        _id_ref = detail::get_reference(_id_key);
    }

    // with owner
    Proxy::ProxyValue::ProxyValue(unsafe::Value* value, std::shared_ptr<ProxyValue>& owner, unsafe::Value* id)
    {
        auto gc = GCSentinel();

        static jl_function_t* make_unnamed_proxy_id = unsafe::get_function("jluna.memory_handler"_sym, "make_unnamed_proxy_id"_sym);
        static jl_function_t* make_named_proxy_id = unsafe::get_function("jluna.memory_handler"_sym, "make_named_proxy_id"_sym);

        _owner = owner;

        _value_key = detail::create_reference(value);
        _value_ref = detail::get_reference(_value_key);

        _id_key = detail::create_reference(jl_call2(make_named_proxy_id, id, owner->id()));
        _id_ref = detail::get_reference(_id_key);

    }

    Proxy::ProxyValue::~ProxyValue()
    {
        detail::free_reference(_value_key);
        detail::free_reference(_id_key);
    }

    unsafe::Value* Proxy::ProxyValue::value() const
    {
        return jl_get_nth_field(_value_ref, 0);
    }

    unsafe::Value* Proxy::ProxyValue::id() const
    {
        if (value() == (unsafe::Value*) jl_main_module)
            return jl_nothing;

        return jl_get_nth_field(_id_ref, 0);
    }

    unsafe::Value* Proxy::ProxyValue::get_field(jl_sym_t* symbol)
    {
        static jl_function_t* dot = unsafe::get_function("jluna"_sym, "dot"_sym);

        auto gc = GCSentinel();
        auto* res = jluna::safe_call(dot, value(), (unsafe::Value*) symbol);

        return res;
    }

    /// ####################################################################

    Proxy::Proxy()
        : Proxy(jl_nothing, nullptr)
    {}

    Proxy::Proxy(unsafe::Value* value, std::shared_ptr<ProxyValue>& owner, unsafe::Value* symbol)
        : _content(new ProxyValue(value, owner, symbol))
    {}

    Proxy::Proxy(unsafe::Value* value, jl_sym_t* symbol)
        : _content(new ProxyValue(value, symbol))
    {}

    Proxy::~Proxy()
    {
        _content.reset();
    }

    Proxy Proxy::operator[](const std::string& field)
    {
        return operator[](field.c_str());
    }

    Proxy Proxy::operator[](size_t i)
    {
        static jl_function_t* getindex = jl_get_function(jl_base_module, "getindex");
        return Proxy(
                jluna::safe_call(getindex, _content->value(), box(i + 1)),
                _content,
                jl_box_uint64(i+1)
        );
    }

    Proxy::operator unsafe::Value*()
    {
        auto gc = GCSentinel();
        auto* res = _content->value();
        if (res == nullptr)
            return jl_nothing;
        else
            return res;
    }

    Proxy::operator const unsafe::Value*() const
    {
        auto gc = GCSentinel();
        auto* res = _content->value();
        if (res == nullptr)
            return jl_nothing;
        else
            return res;
    }

    Proxy::operator std::string() const
    {
        static jl_function_t* to_string = jl_get_function(jl_base_module, "string");
        return std::string(jl_string_data(jl_call1(to_string, _content->value())));
    }

    std::string Proxy::get_name() const
    {
        static jl_function_t* get_name = unsafe::get_function("jluna.memory_handler"_sym, "get_name"_sym);
        return unbox<std::string>(jluna::safe_call(get_name, _content->id()));
    }

    std::vector<std::string> Proxy::get_field_names() const
    {
        auto gc = GCSentinel();
        auto* svec = jl_field_names((jl_datatype_t*) (jl_isa(_content->value(), (unsafe::Value*) jl_datatype_type) ? _content->value() : jl_typeof(_content->value())));
        std::vector<std::string> out;
        for (size_t i = 0; i < jl_svec_len(svec); ++i)
            out.push_back(std::string(jl_symbol_name((jl_sym_t*) jl_svecref(svec, i))));

        return out;
    }

    Type Proxy::get_type() const
    {
        return Type((jl_datatype_t*) jl_typeof(_content->value()));
    }

    bool Proxy::is_mutating() const
    {
        return _content->_is_mutating;
    }

    Proxy & Proxy::operator=(unsafe::Value* new_value)
    {
        static jl_function_t* assign = unsafe::get_function("jluna.memory_handler"_sym, "assign"_sym);
        static jl_function_t* set_reference = unsafe::get_function("jluna.memory_handler"_sym, "set_reference"_sym);

        auto gc = GCSentinel();

        _content->_value_ref = jluna::safe_call(set_reference, jl_box_uint64(_content->_value_key), new_value);

        if (_content->_is_mutating)
            jluna::safe_call(assign, new_value, _content->id());

        return *this;
    }

    Proxy Proxy::as_unnamed() const
    {
        static auto* deepcopy = unsafe::get_function(jl_base_module, "deepcopy"_sym);
        return Proxy(unsafe::call(deepcopy, _content->value()), nullptr);
    }

    void Proxy::update()
    {
        static jl_function_t* evaluate = unsafe::get_function("jluna.memory_handler"_sym, "evaluate"_sym);
        static jl_function_t* set_reference = unsafe::get_function("jluna.memory_handler"_sym, "set_reference"_sym);

        auto gc = GCSentinel();
        auto* new_value = jluna::safe_call(evaluate, _content->id());
        _content->_value_ref = jluna::safe_call(set_reference, jl_box_uint64(_content->_value_key), new_value);
    }

    bool Proxy::isa(const Type& type)
    {
        return jl_isa(operator jl_value_t*(), type);
    }
}