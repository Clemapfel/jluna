// 
// Copyright 2022 Clemens Cords
// Created on 19.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>

#include <cstddef>
#include <functional>
#include <string>

extern "C"
{
    namespace jluna::detail
    {
        using lambda_0_arg = std::function<jl_value_t *()>;
        using lambda_1_arg = std::function<jl_value_t *(jl_value_t *)>;
        using lambda_2_arg = std::function<jl_value_t *(jl_value_t *, jl_value_t *)>;
        using lambda_3_arg = std::function<jl_value_t *(jl_value_t *, jl_value_t *, jl_value_t *)>;
        //using lambda_n_arg = std::function<jl_value_t*(const std::vector<jl_value_t*>&)>;
    }

    /// @brief construct an UnnamedFunctionProxy object
    /// @param function_ptr: allocated with `new`
    /// @param n_args: number of arguments: 0, 1, 2, 3 or -1
    /// @returns ptr to UnnamedFunctionProxy object
    jl_value_t* jluna_make(void* function_ptr, int n_args);

    jl_value_t* jluna_invoke_lambda_0(void* function_ptr);
    jl_value_t* jluna_invoke_lambda_1(void* function_ptr, jl_value_t*);
    jl_value_t* jluna_invoke_lambda_2(void* function_ptr, jl_value_t*,  jl_value_t*);
    jl_value_t* jluna_invoke_lambda_3(void* function_ptr, jl_value_t*,  jl_value_t*, jl_value_t*);

    /// @brief `delete` a function pointer held
    /// @param pointer to function
    /// @param n_args: 0, 1, 2, 3 or -1
    void jluna_free_lambda(void* function_ptr, int n_args);

    /// @brief get pointer to arbitrary object
    void* jluna_to_pointer(jl_value_t*);

    /// @brief invoke function ptr, used within threadpool
    /// @param function_pointer
    /// @returns result pointer
    uint64_t jluna_invoke_from_task(uint64_t function_ptr);

    /// @brief verify c_adapter is working, used for test
    /// @returns true
    bool jluna_verify();
}