// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>
#include <include/julia_extension.hpp>

#include <sstream>

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

    JuliaUnitializedException::JuliaUnitializedException()
        : std::exception()
    {}

    const char * JuliaUnitializedException::what() const noexcept
    {
        return "[C++][ERROR] jluna and julia need to be initialized using jluna::initialize() before usage";
    }

    void throw_if_unitialized()
    {
        if (not jl_is_initialized())
            throw JuliaUnitializedException();
    }

    void forward_last_exception()
    {
        throw_if_unitialized();

        auto* jl_c_exception = jl_exception_occurred();
        if (jl_c_exception != nullptr)
            throw JuliaException(jl_c_exception, jl_to_string(jl_c_exception));

        auto jluna_exception_occurred = jl_unbox_bool(jl_eval_string("return jluna.exception_handler.has_exception_occurred()"));
        if (jluna_exception_occurred)
        {
            throw JuliaException(
                    jl_eval_string("return jluna.exception_handler.get_last_exception()"),
                    jl_to_string(jl_eval_string("return jluna.exception_handler.get_last_message()"))
            );
            return;
        }
    }
}

