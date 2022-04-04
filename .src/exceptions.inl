// 
// Copyright 2022 Clemens Cords
// Created on 05.02.22 by clem (mail@clemens-cords.com)
//

#include <include/unsafe_utilities.hpp>

namespace jluna
{
    template<IsJuliaValuePointer... Args_t>
    unsafe::Value* safe_call(unsafe::Function* function, Args_t... in)
    {
        throw_if_uninitialized();

        static auto* jl_safe_call = unsafe::get_function("jluna"_sym, "safe_call"_sym);

        static std::array<unsafe::Value*, sizeof...(Args_t) + 1> args;
        static auto set = [&](size_t i, unsafe::Value* x) {args[i] = x;};

        args[0] = (unsafe::Value*) function;

        size_t i = 1;
        (set(i++, (unsafe::Value*) in), ...);

        auto* tuple_res = jl_call(jl_safe_call, args.data(), args.size());

        if (jl_unbox_bool(jl_get_nth_field(tuple_res, 1)))
            throw JuliaException(jl_get_nth_field(tuple_res, 2), jl_string_ptr(jl_get_nth_field(tuple_res, 3)));

        return jl_get_nth_field(tuple_res, 0);
    }
}