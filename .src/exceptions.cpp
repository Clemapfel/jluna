// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>
#include <include/julia_extension.hpp>

#include <sstream>
#include <vector>
#include <iostream>

namespace jluna
{
    JuliaException::JuliaException(jl_value_t* exception, std::string stacktrace)
        : _value(exception), _message("[JULIA][EXCEPTION] " + stacktrace)
    {}

    const char* JuliaException::what() const noexcept
    {
        return _message.c_str();
    }

    JuliaException::operator Any*()
    {
        return _value;
    }

    JuliaUninitializedException::JuliaUninitializedException()
        : std::exception()
    {}

    const char * JuliaUninitializedException::what() const noexcept
    {
        return "[C++][ERROR] jluna and julia need to be initialized using jluna::initialize() before usage";
    }

    void throw_if_uninitialized()
    {
        if (not jl_is_initialized())
            throw JuliaUninitializedException();
    }

    void forward_last_exception()
    {
        throw_if_uninitialized();

        auto* jl_c_exception = jl_exception_occurred();
        if (jl_c_exception != nullptr)
            throw JuliaException(jl_c_exception, jl_to_string(jl_c_exception));

        static jl_function_t* exception_occurred = jl_find_function("jluna.exception_handler", "has_exception_occurred");
        if (jl_unbox_bool(jl_call0(exception_occurred)))
        {
            throw JuliaException(
                    jl_eval_string("return jluna.exception_handler.get_last_exception()"),
                    jl_to_string(jl_eval_string("return jluna.exception_handler.get_last_message()"))
            );
        }
    }

    Any* safe_eval(const char* str)
    {
        jluna::throw_if_uninitialized();

        static jl_function_t* safe_call = jl_find_function("jluna.exception_handler", "safe_call");
        static jl_function_t* has_exception_occurred = jl_find_function("jluna.exception_handler", "has_exception_occurred");

        jl_gc_pause;
        auto* result = jl_call1(safe_call, jl_quote(str));
        if (jl_exception_occurred() or jl_unbox_bool(jl_call0(has_exception_occurred)))
        {
            std::cerr << "exception in jluna::State::safe_eval for expression:\n\"" << str << "\"\n" << std::endl;
            forward_last_exception();
        }
        jl_gc_unpause;
        return result;
    }

    Any* operator""_eval(const char* str, size_t _)
    {
        return safe_eval(str);
    }
}

