// 
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>
#include <include/unsafe_utilities.hpp>

namespace jluna::detail
{
    static inline size_t _num_threads = 1;

    void initialize_modules();
    void initialize_types();
    void on_exit();

    constexpr char _id_marker = '#';

    size_t create_reference(unsafe::Value* in);
    unsafe::Value* get_reference(size_t key);
    void free_reference(size_t key);
}

namespace jluna
{
    template<is_julia_value_pointer... Args_t>
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

        auto* res = jl_get_nth_field(tuple_res, 0);
        return res;
    }

    template<is_julia_value_pointer... Ts>
    void println(Ts... in)
    {
        static auto* jl_println = unsafe::get_function(jl_base_module, "println"_sym);
        safe_call(jl_println, in...);
    }

    inline unsafe::Value* as_julia_pointer(unsafe::Value* in)
    {
        static auto* forward_as_pointer = unsafe::get_function("jluna"_sym, "forward_as_pointer"_sym);
        return safe_call(forward_as_pointer, jl_typeof(in), jl_box_voidpointer((void*) in));
    }
}