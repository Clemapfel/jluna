// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>
#include <include/typedefs.hpp>

#include <string>
#include <exception>
#include <vector>

namespace jluna
{
    /// @brief wrapper for julia exceptions
    class JuliaException : public std::exception
    {
        public:
            /// @brief default ctor
            JuliaException() = default;

            /// @brief ctor
            /// @param exception: value pointing to a julia-side instance of the exception
            /// @param stacktrace: string describing the exception and the stacktrace
            JuliaException(jl_value_t* exception, std::string stacktrace);

            /// @brief get description
            /// @returns c-string
            virtual const char* what() const noexcept override final;

            /// @brief decay to jl value
            /// @returns jl_value_t*
            operator Any*();

        protected:
            Any* _value = nullptr;
            std::string _message;
    };

    /// @brief exception thrown when trying to use jluna or julia before initialization
    struct JuliaUninitializedException : public std::exception
    {
        /// @brief ctor
        JuliaUninitializedException();

        /// @brief get description
        /// @returns c-string
        virtual const char* what() const noexcept override final;
    };

    /// @brief throw exception, used frequently for safeguarding code
    void throw_if_uninitialized();

    /// @brief if exception occurred, forward as JuliaException
    void forward_last_exception();

    /// @brief call function with args, with brief exception forwarding
    /// @tparam Args_t: argument types, must be castable to Any*
    /// @param function
    /// @param args
    /// @returns result
    template<typename... Args_t>
    Any* call(Function* function, Args_t... args)
    {
        throw_if_uninitialized();

        std::vector<Any*> params;
        (params.push_back((Any*) args), ...);

        auto* res = jl_call(function, params.data(), params.size());
        forward_last_exception();
        return res;
    }

    /// @brief call function with args, with verbose exception forwarding
    /// @tparam Args_t: argument types, must be castable to Any*
    /// @param function
    /// @param args
    /// @returns result
    template<typename... Args_t>
    Any* safe_call(Function* function, Args_t... args)
    {
        throw_if_uninitialized();

        std::vector<Any*> params;
        params.push_back(function);
        (params.push_back((Any*) args), ...);

        static Function* safe_call = jl_get_function((jl_module_t*) jl_eval_string("jluna.exception_handler"), "safe_call");
        auto* res = jl_call(safe_call, params.data(), params.size());
        forward_last_exception();
        return res;
    }
}