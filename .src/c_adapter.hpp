// 
// Copyright 2022 Clemens Cords
// Created on 19.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <stddef.h>

#ifdef __cplusplus

#include <include/julia_wrapper.hpp>
#include <functional>
#include <string>

extern "C"
{
    namespace jluna::c_adapter
    {
        using lambda_0_arg = std::function<unsafe::Value*()>;
        using lambda_1_arg = std::function<unsafe::Value*(unsafe::Value*)>;
        using lambda_2_arg = std::function<unsafe::Value*(unsafe::Value*, unsafe::Value*)>;
        using lambda_3_arg = std::function<unsafe::Value*(unsafe::Value*, unsafe::Value*, unsafe::Value*)>;
        //using lambda_n_arg = std::function<unsafe::Value*(const std::vector<unsafe::Value*>&)>;

        /// @brief construct an UnnamedFunctionProxy object
        /// @param function_ptr: allocated with `new`
        /// @param n_args: number of arguments: 0, 1, 2, 3 or -1
        /// @returns ptr to UnnamedFunctionProxy object
        unsafe::Value* make(void* function_ptr, int n_args);

        unsafe::Value* invoke_lambda_0(void* function_ptr);
        unsafe::Value* invoke_lambda_1(void* function_ptr, unsafe::Value*);
        unsafe::Value* invoke_lambda_2(void* function_ptr, unsafe::Value*,  unsafe::Value*);
        unsafe::Value* invoke_lambda_3(void* function_ptr, unsafe::Value*,  unsafe::Value*, unsafe::Value*);
        //unsafe::Value* invoke_lambda_n(void* function_ptr, unsafe::Array* vector);

        /// @brief `delete` a function pointer held
        /// @param pointer to function
        /// @param n_args: 0, 1, 2, 3 or -1
        void free_lambda(void* function_ptr, int n_args);
    }
}

#else // exposed to juila as pure C header:

void free_lambda(void*, int);
unsafe::Value* invoke_lambda_0(void*);
unsafe::Value* invoke_lambda_1(void*, unsafe::Value*);
unsafe::Value* invoke_lambda_2(void*, unsafe::Value*,  unsafe::Value*);
unsafe::Value* invoke_lambda_3(void*, unsafe::Value*,  unsafe::Value*, unsafe::Value*);
//unsafe::Value* invoke_lambda_n(void*, unsafe::Array*);

#endif