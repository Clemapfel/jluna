// 
// Copyright 2022 Clemens Cords
// Created on 16.02.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    /// @brief unbox symbol to symbol
    template<Is<Symbol> T>
    inline T unbox(Any* value)
    {
        static Any* symbol_t = jl_eval_string("return Symbol");
        jl_assert_type(value, symbol_t);
        return Symbol((jl_sym_t*) value);
    }

    /// @brief box jluna::Module to Base.Module
    template<Is<Symbol> T>
    inline Any* box(T value)
    {
        return value.operator Any*();
    }

    template<Is<Symbol> T>
    inline Any* box(const std::string& value)
    {
        return (Any*) jl_symbol(value.c_str());
    }

    /// @brief type deduction
    template<>
    struct detail::to_julia_type_aux<Symbol>
    {
        static inline const std::string type_name = "Symbol";
    };
}