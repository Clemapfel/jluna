// 
// Copyright 2022 Clemens Cords
// Created on 05.02.22 by clem (mail@clemens-cords.com)
//

namespace jluna::detail
{
    template<Is<Any*>... Ts>
    inline Proxy create_or_assign(const std::string& symbol, Ts... args)
    {
        static jl_function_t* create_or_assign = jl_find_function("jluna", "create_or_assign");
        safe_call(create_or_assign, (jl_value_t*) jl_symbol(symbol.c_str()), args...);
        return Main[symbol];
    }
}