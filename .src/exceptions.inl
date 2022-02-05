// 
// Copyright 2022 Clemens Cords
// Created on 05.02.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
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