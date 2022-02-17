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
    Proxy::ProxyValue::ProxyValue(Any* value, std::shared_ptr<ProxyValue>& owner, jl_sym_t* symbol)
        : _is_mutating(symbol != nullptr)
    {
        if (value == nullptr)
            return;

        jl_gc_pause;
        _owner = owner;
        _value_key = State::detail::create_reference(value);
        _value_ref = State::detail::get_reference(_value_key);

        if (symbol == nullptr)
        {
            std::stringstream str;
            str << State::detail::_id_marker << _value_key << "[]";
            symbol = jl_symbol(str.str().c_str());
        }

        _symbol_key = State::detail::create_reference((Any*) symbol);
        _symbol_ref = State::detail::get_reference(_symbol_key);
        jl_gc_unpause;
    }

    Proxy::ProxyValue::ProxyValue(Any* value, jl_sym_t* symbol)
        : _owner(nullptr), _is_mutating(symbol != nullptr)
    {
        if (value == nullptr)
            return;

        jl_gc_pause;
        _value_key = State::detail::create_reference(value);
        _value_ref = State::detail::get_reference(_value_key);

        if (symbol == nullptr)
        {
            std::stringstream str;
            str << State::detail::_id_marker << _value_key;
            symbol = jl_symbol(str.str().c_str());
        }

        _symbol_key = State::detail::create_reference((Any*) symbol);
        _symbol_ref = State::detail::get_reference(_symbol_key);
        jl_gc_unpause;
    }

    Proxy::ProxyValue::~ProxyValue()
    {
        State::detail::free_reference(_value_key);
        State::detail::free_reference(_symbol_key);
    }

    Any * Proxy::ProxyValue::value()
    {
        return jl_ref_value(_value_ref);
    }

    Any * Proxy::ProxyValue::symbol()
    {
        return jl_ref_value(_symbol_ref);
    }

    size_t Proxy::ProxyValue::value_key()
    {
        return _value_key;
    }

    size_t Proxy::ProxyValue::symbol_key()
    {
        return _symbol_key;
    }

    const Any * Proxy::ProxyValue::value() const
    {
        return jl_ref_value(_value_ref);
    }

    const Any * Proxy::ProxyValue::symbol() const
    {
        return jl_ref_value(_symbol_ref);
    }

    Any * Proxy::ProxyValue::get_field(jl_sym_t* symbol)
    {
        static jl_module_t* jluna_module = (jl_module_t*) jl_eval_string("return Main.jluna");
        static jl_module_t* exception_module = (jl_module_t*) jl_eval_string("return Main.jluna.exception_handler");
        static jl_function_t* safe_call = jl_get_function(exception_module, "safe_call");
        static jl_function_t* dot = jl_get_function(jluna_module, "dot");

        jl_gc_pause;
        Any* args[3] = {(Any*) dot, value(), (Any*) symbol};
        auto* res = jl_call(safe_call, args, 3);
        forward_last_exception();
        jl_gc_unpause;

        return res;
    }

    /// ####################################################################

    Proxy::Proxy()
        : Proxy(jl_nothing, nullptr)
    {}

    Proxy::Proxy(Any* value, std::shared_ptr<ProxyValue>& owner, jl_sym_t* symbol)
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
        return Proxy(_content.get()->get_field(symbol), _content, _content->symbol() == nullptr ? nullptr : symbol);
    }

    Proxy Proxy::operator[](size_t i)
    {
        static jl_function_t* getindex = jl_get_function(jl_base_module, "getindex");
        return Proxy(
                jluna::safe_call(getindex, _content->value(), box(i + 1)),
                _content,
                jl_symbol(std::to_string(i).c_str())
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

    std::deque<jl_sym_t*> Proxy::assemble_name() const
    {
        jl_gc_pause;
        const ProxyValue* ptr = _content.get();
        std::deque<jl_sym_t*> name;

        while (ptr != nullptr and ptr->symbol() != nullptr)
        {
            name.push_front((jl_sym_t*) ptr->symbol());
            ptr = ptr->_owner.get();
        }
        jl_gc_unpause;

        return name;
    }

    std::string Proxy::get_name() const
    {
        std::deque<jl_sym_t*> name = assemble_name();
        std::stringstream str;

        jl_gc_pause;
        for (size_t i = 0; i < name.size(); ++i)
        {
            std::string sname = jl_symbol_name(name.at(i));

            if (i != 0 and sname.at(0) != '[')
                str << ".";

            if (sname.at(0) == State::detail::_id_marker)
                if (sname.at(1) == '1' and sname.size() == 2)
                    str << "Main";
                else
                    str << "<unnamed proxy " << jl_symbol_name(name.at(i)) << ">";
            else
                str << jl_symbol_name(name.at(i));
        }
        jl_gc_unpause;

        return str.str();
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
        static jl_function_t* safe_call = jl_get_function((jl_module_t*) jl_eval_string("return Main.jluna.exception_handler"), "safe_call");
        static jl_function_t* set_reference = jl_get_function((jl_module_t*) jl_eval_string("return Main.jluna.memory_handler"), "set_reference");

        jl_gc_pause;

        _content->_value_ref = jl_call3(safe_call, (Any*) set_reference,
                                        jl_box_uint64(_content->value_key()), new_value);
        forward_last_exception();

        if (_content->_is_mutating)
        {
            static jl_function_t* assign = jl_find_function("Main.jluna.memory_handler", "assign");
            auto name_q = assemble_name();
            std::vector<Any*> params = {assign, new_value};
            for (auto* s : name_q)
                params.push_back((Any*) s);

            static jl_function_t* safe_call = jl_get_function((jl_module_t*) jl_eval_string("return jluna.exception_handler"), "safe_call");
            jl_call(safe_call, params.data(), params.size());
            forward_last_exception();
        }

        jl_gc_unpause;
        return *this;
    }

    Proxy Proxy::value() const
    {
        return Proxy(jl_deepcopy(_content->value()), nullptr);
    }

    void Proxy::update()
    {
        static jl_function_t* safe_call = jl_get_function((jl_module_t*) jl_eval_string("return Main.jluna.exception_handler"), "safe_call");
        static jl_function_t* assemble_eval = jl_get_function((jl_module_t*) jl_eval_string("return Main.jluna.memory_handler"), "evaluate");
        static jl_function_t* set_reference = jl_get_function((jl_module_t*) jl_eval_string("return Main.jluna.memory_handler"), "set_reference");

        jl_gc_pause;
        auto name = assemble_name();
        std::vector<Any*> args = {(Any*) assemble_eval};

        for (auto* n : name)
            args.push_back((Any*) n);

        Any* new_value = jl_call(safe_call, args.data(), args.size());
        forward_last_exception();

        _content->_value_ref = jl_call3(safe_call, (Any*) set_reference, jl_box_uint64(_content->value_key()), new_value);
        forward_last_exception();
        jl_gc_unpause;
    }

    bool Proxy::isa(const Type& type)
    {
        static jl_function_t* isa = jl_get_function(jl_base_module, "isa");
        return unbox<bool>(jluna::safe_call(isa, this->operator const _jl_value_t *(), type.operator const _jl_value_t *()));
    }
}