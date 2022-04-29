// 
// Copyright 2022 Clemens Cords
// Created on 16.02.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    /// @brief unbox symbol to symbol
    template<is<Symbol> T>
    inline T unbox(unsafe::Value* value)
    {
        detail::assert_type((unsafe::DataType*) jl_typeof(value), jl_symbol_type);
        return Symbol((jl_sym_t*) value);
    }

    /// @brief box jluna::Module to Base.Module
    template<is<Symbol> T>
    inline unsafe::Value* box(T value)
    {
        return value.operator unsafe::Value*();
    }

    template<is<Symbol> T>
    inline unsafe::Value* box(const std::string& value)
    {
        return (unsafe::Value*) jl_symbol(value.c_str());
    }

    /// @brief type deduction
    template<>
    struct detail::as_julia_type_aux<Symbol>
    {
        static inline const std::string type_name = "Symbol";
    };
}