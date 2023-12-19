// 
// Copyright 2022 Clemens Cords
// Created on 13.02.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    /// @brief unbox to type
    template<is<Type> T>
    inline T unbox(unsafe::Value* value)
    {
        static jl_datatype_t* type_t = (jl_datatype_t*) jl_eval_string("return Type");
        detail::assert_type((unsafe::DataType*) jl_typeof(value), type_t);
        return Type((jl_datatype_t*) value);
    }

    /// @brief box jluna::Type to Base.Type
    template<is<Type> T>
    inline unsafe::Value* box(T value)
    {
        return value.operator unsafe::Value*();
    }

    /// @brief type deduction
    template<>
    struct detail::as_julia_type_aux<Type>
    {
        static inline const std::string type_name = "Type";
    };

    namespace detail
    {
        inline void initialize_types()
        {
            auto unroll = [](const std::string& name) -> jl_datatype_t*
            {
                return (jl_datatype_t*) jl_eval_string(("return jluna.unroll_type(" + name + ")").c_str());
            };

            gc_pause;
            AbstractArray_t = Type(unroll("Core.AbstractArray"));
            AbstractChar_t = Type(unroll("Core.AbstractChar"));
            AbstractFloat_t = Type(unroll("Core.AbstractFloat"));
            AbstractString_t = Type(unroll("Core.AbstractString"));
            Any_t = Type(unroll("Core.Any"));
            Array_t = Type(unroll("Core.Array"));
            Bool_t = Type(unroll("Core.Bool"));
            Char_t = Type(unroll("Core.Char"));
            DataType_t = Type(unroll("Core.DataType"));
            DenseArray_t = Type(unroll("Core.DenseArray"));
            Exception_t = Type(unroll("Core.Exception"));
            Expr_t = Type(unroll("Core.Expr"));
            Float16_t = Type(unroll("Core.Float16"));
            Float32_t = Type(unroll("Core.Float32"));
            Float64_t = Type(unroll("Core.Float64"));
            Function_t = Type(unroll("Core.Function"));
            GlobalRef_t = Type(unroll("Core.GlobalRef"));
            IO_t = Type(unroll("Core.IO"));
            Int128_t = Type(unroll("Core.Int128"));
            Int16_t = Type(unroll("Core.Int16"));
            Int32_t = Type(unroll("Core.Int32"));
            Int64_t = Type(unroll("Core.Int64"));
            Int8_t = Type(unroll("Core.Int8"));
            Integer_t = Type(unroll("Core.Integer"));
            LineNumberNode_t = Type(unroll("Core.LineNumberNode"));
            Method_t = Type(unroll("Core.Method"));
            Module_t = Type(unroll("Core.Module"));
            Missing_t = Type(unroll("Base.Missing"));
            NTuple_t = Type(unroll("Core.NTuple"));
            NamedTuple_t = Type(unroll("Core.NamedTuple"));
            Nothing_t = Type(unroll("Core.Nothing"));
            Number_t = Type(unroll("Core.Number"));
            Pair_t = Type(unroll("Core.Pair"));
            Ptr_t = Type(unroll("Core.Ptr"));
            QuoteNode_t = Type(unroll("Core.QuoteNode"));
            Real_t = Type(unroll("Core.Real"));
            Ref_t = Type(unroll("Core.Ref"));
            Signed_t = Type(unroll("Core.Signed"));
            String_t = Type(unroll("Core.String"));
            Symbol_t = Type(unroll("Core.Symbol"));
            Task_t = Type(unroll("Core.Task"));
            Tuple_t = Type(unroll("Core.Tuple"));
            Type_t = Type(unroll("Core.Type"));
            TypeVar_t = Type(unroll("Core.TypeVar"));
            UInt128_t = Type(unroll("Core.UInt128"));
            UInt16_t = Type(unroll("Core.UInt16"));
            UInt32_t = Type(unroll("Core.UInt32"));
            UInt64_t = Type(unroll("Core.UInt64"));
            UInt8_t = Type(unroll("Core.UInt8"));
            UndefInitializer_t = Type(unroll("Core.UndefInitializer"));
            Union_t = Type(unroll("Core.Union"));
            UnionAll_t = Type(unroll("Core.UnionAll"));
            UnionEmpty_t = Type(unroll("Union{}"));
            Unsigned_t = Type(unroll("Core.Unsigned"));
            VecElement_t = Type(unroll("Core.VecElement"));
            WeakRef_t = Type(unroll("Core.WeakRef"));
            gc_unpause;
        }
    }
}