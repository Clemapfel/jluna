// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>
#include <include/typedefs.hpp>

#include <string>
#include <exception>

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
            Any* _value;
            std::string _message;
    };

    /// @brief exception thrown when trying to use jluna or julia before initialization
    struct JuliaUnitializedException : public std::exception
    {
        /// @brief ctor
        JuliaUnitializedException();

        /// @brief get description
        /// @returns c-string
        virtual const char* what() const noexcept override final;
    };

    /// @brief throw exception, used frequently for safeguarding code
    void throw_if_unitialized();

    /// @brief if exception occurred, forward as JuliaException
    void forward_last_exception();
}