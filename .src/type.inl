// 
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    /// @brief unbox to type
    template<Is<Type> T>
    inline T unbox(unsafe::Value* value)
    {
        static jl_datatype_t* type_t = (jl_datatype_t*) jl_eval_string("return Type");
        detail::assert_type((unsafe::DataType*) jl_typeof(value), type_t);
        return Type((jl_datatype_t*) value);
    }

    /// @brief box jluna::Type to Base.Type
    template<Is<Type> T>
    inline unsafe::Value* box(T value)
    {
        return value.operator unsafe::Value*();
    }

    /// @brief type deduction
    template<>
    struct detail::to_julia_type_aux<Type>
    {
        static inline const std::string type_name = "Type";
    };
}