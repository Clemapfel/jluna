// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>
#include <include/typedefs.hpp>
#include <include/concepts.hpp>

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
            JuliaException(jl_value_t* exception, const std::string& stacktrace);

            /// @brief get description
            /// @returns c-string
            [[nodiscard]] const char* what() const noexcept final;

            /// @brief decay to jl value
            /// @returns jl_value_t*
            operator unsafe::Value*();

        protected:
            unsafe::Value* _value = nullptr;
            std::string _message;
    };

    /// @brief exception thrown when trying to use jluna or julia before initialization
    struct JuliaUninitializedException : public std::exception
    {
        /// @brief ctor
        JuliaUninitializedException();

        /// @brief get description
        /// @returns c-string
        [[nodiscard]] const char* what() const noexcept final;
    };

    /// @brief if exception occurred, forward as JuliaException
    void forward_last_exception();

    /// @brief throw if initialize was not yet called
    void throw_if_uninitialized();
}

