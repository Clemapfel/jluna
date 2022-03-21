// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

namespace jluna::unsafe
{
    template<IsReinterpretableTo<unsafe::Value*>... Args_t>
    unsafe::Value* call(Function* function, Args_t... args)
    {
        std::array<jl_value_t*, sizeof...(Args_t)> wrapped;

        static auto set = [&](size_t i, auto args)
        {
            wrapped.at(i) = reinterpret_cast<jl_value_t*>(args);
        };

        {
            size_t i = 0;
            (set(i++, args), ...);
        }

        return jl_call(function, wrapped.data(), wrapped.size());
    }

    template<IsReinterpretableTo<unsafe::Value*>... Args_t>
    unsafe::Expression* Expr(unsafe::Symbol* first, Args_t... other)
    {
        static unsafe::Function* expr = get_function(jl_base_module, "Expr"_sym);
        return (unsafe::Expression*) call(expr, first, other...);
    }
}