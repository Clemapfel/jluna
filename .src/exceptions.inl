// 
// Copyright 2022 Clemens Cords
// Created on 05.02.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    template<typename... Args_t>
    unsafe::Value* safe_call(unsafe::Function* function, Args_t... args)
    {
        throw_if_uninitialized();

        auto before = jl_gc_is_enabled();
        jl_gc_enable(false);

        std::vector<unsafe::Value*> params;
        params.push_back(function);
        (params.push_back((unsafe::Value*) args), ...);

        static unsafe::Function* safe_call = jl_get_function((jl_module_t*) jl_eval_string("jluna.exception_handler"), "safe_call");
        auto* res = jl_call(safe_call, params.data(), params.size());
        forward_last_exception();

        jl_gc_enable(true);
        return res;
    }
}