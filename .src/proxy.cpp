// 
// Copyright 2022 Clemens Cords
// Created on 11.01.22 by clem (mail@clemens-cords.com)
//

#include <sstream>
#include <deque>
#include <sstream>
#include <iostream>

#include <include/state.hpp>
#include <include/proxy.hpp>
#include <include/type.hpp>

namespace jluna
{
    // no owner
    Proxy::ProxyValue::ProxyValue(Any* value, jl_sym_t* id)
        : _is_mutating(id != nullptr)
    {
        if (value == nullptr)
            return;

        static jl_function_t* make_unnamed_proxy_id = jl_find_function("jluna.memory_handler", "make_unnamed_proxy_id");
        static jl_function_t* make_named_proxy_id = jl_find_function("jluna.memory_handler", "make_named_proxy_id");

        jl_gc_pause;

        _value_key = State::detail::create_reference(value);
        _value_ref = State::detail::get_reference(_value_key);

        if (id == nullptr)
            _id_key = State::detail::create_reference(jl_call1(make_unnamed_proxy_id, jl_box_uint64(_value_key)));
        else
            _id_key = State::detail::create_reference(jl_call2(make_named_proxy_id, (Any*) id, jl_nothing));

        _id_ref = State::detail::get_reference(_id_key);

        jl_gc_unpause;
    }

    // with owner
    Proxy::ProxyValue::ProxyValue(Any* value, std::shared_ptr<ProxyValue>& owner, Any* id)
    {
        jl_gc_pause;

        static jl_function_t* make_unnamed_proxy_id = jl_find_function("jluna.memory_handler", "make_unnamed_proxy_id");
        static jl_function_t* make_named_proxy_id = jl_find_function("jluna.memory_handler", "make_named_proxy_id");

        _owner = owner;

        _value_key = State::detail::create_reference(value);
        _value_ref = State::detail::get_reference(_value_key);

        _id_key = State::detail::create_reference(jl_call2(make_named_proxy_id, id, owner->id()));
        _id_ref = State::detail::get_reference(_id_key);

        jl_gc_unpause;
    }

    Proxy::ProxyValue::~ProxyValue()
    {
        State::detail::free_reference(_value_key);
        State::detail::free_reference(_id_key);
    }

    Any * Proxy::ProxyValue::value() const
    {
        return jl_ref_value(_value_ref);
    }

    Any * Proxy::ProxyValue::id() const
    {
        return jl_ref_value(_id_ref);
    }

    Any * Proxy::ProxyValue::get_field(jl_sym_t* symbol)
    {
        static jl_function_t* dot = jl_find_function("jluna", "dot");

        jl_gc_pause;
        auto* res = jluna::safe_call(dot, value(), (Any*) symbol);
        jl_gc_unpause;

        return res;
    }

    /// ####################################################################

    Proxy::Proxy()
        : Proxy(jl_nothing, nullptr)
    {}

    Proxy::Proxy(Any* value, std::shared_ptr<ProxyValue>& owner, Any* symbol)
        : _content(new ProxyValue(value, owner, symbol))
    {}

    Proxy::Proxy(Any* value, jl_sym_t* symbol)
        : _content(new ProxyValue(value, symbol))
    {}

    Proxy::~Proxy()
    {
        _content.reset();
    }

    Proxy Proxy::operator[](const std::string& field)
    {
        jl_sym_t* symbol = jl_symbol(field.c_str());
        return Proxy(
            _content.get()->get_field(symbol),
            _content,
            (Any*) (_content->id() == nullptr ? nullptr : symbol)
        );
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

    Proxy::operator Any*()
    {
        auto* res = _content->value();
        if (res == nullptr)
            return jl_nothing;
        else
            return res;
    }

    Proxy::operator const Any*() const
    {
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
        static jl_function_t* get_name = jl_find_function("jluna.memory_handler", "get_name");
        return unbox<std::string>(jluna::safe_call(get_name, _content->id()));
    }

    std::vector<std::string> Proxy::get_field_names() const
    {
        jl_gc_pause;
        auto* svec = jl_field_names((jl_datatype_t*) (jl_isa(_content->value(), (Any*) jl_datatype_type) ? _content->value() : jl_typeof(_content->value())));
        std::vector<std::string> out;
        for (size_t i = 0; i < jl_svec_len(svec); ++i)
            out.push_back(std::string(jl_symbol_name((jl_sym_t*) jl_svecref(svec, i))));

        jl_gc_unpause;
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

    Proxy & Proxy::operator=(Any* new_value)
    {
        static jl_function_t* assign = jl_find_function("jluna.memory_handler", "assign");
        static jl_function_t* set_reference = jl_find_function("jluna.memory_handler", "set_reference");

        jl_gc_pause;

        _content->_value_ref = jluna::safe_call(set_reference, jl_box_uint64(_content->_value_key), new_value);

        if (_content->_is_mutating)
            jluna::safe_call(assign, new_value, _content->id());

        jl_gc_unpause;
        return *this;
    }

    Proxy Proxy::value() const
    {
        return Proxy(jl_deepcopy(_content->value()), nullptr);
    }

    void Proxy::update()
    {
        static jl_function_t* evaluate = jl_find_function("jluna.memory_handler", "evaluate");
        static jl_function_t* set_reference = jl_find_function("jluna.memory_handler", "set_reference");

        jl_gc_pause;
        auto* new_value = jluna::safe_call(evaluate, _content->id());
        _content->_value_ref = jluna::safe_call(set_reference, jl_box_uint64(_content->_value_key), new_value);
        jl_gc_unpause;
    }

    bool Proxy::isa(const Type& type)
    {
        static jl_function_t* isa = jl_get_function(jl_base_module, "isa");
        return unbox<bool>(jluna::safe_call(isa, this->operator const _jl_value_t *(), type.operator const _jl_value_t *()));
    }
}