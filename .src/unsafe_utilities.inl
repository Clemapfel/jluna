// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#include <include/julia_extension.hpp>

namespace jluna::unsafe
{
    template<typename... Args_t>
    unsafe::Value* call(Function* function, Args_t... args)
    {
        std::array<jl_value_t*, sizeof...(Args_t)> wrapped = {(unsafe::Value*) args...};
        return jl_call(function, wrapped.data(), wrapped.size());
    }

    template<typename... Args_t>
    unsafe::Expression* Expr(unsafe::Symbol* first, Args_t... other)
    {
        static unsafe::Function* expr = get_function(jl_base_module, "Expr"_sym);
        return (unsafe::Expression*) call(expr, first, ((unsafe::Value*) other)...);
    }

    namespace detail
    {
        inline nullptr_t gc_init()
        {
            static bool initialized = false;

            if (initialized)
                return nullptr;

            jl_eval_string(R"(
                __jluna_heap = Dict{UInt64, Base.RefValue{Any}}();
                __jluna_heap_index = Base.RefValue(UInt64(0));

                function __jluna_add_to_heap(ptr::UInt64)
                    global __jluna_heap_index[] += 1
                    __jluna_heap[__jluna_heap_index[]] = Ref{Any}(unsafe_pointer_to_objref(Ptr{Any}(ptr)))
                    return __jluna_heap_index[];
                end
            )");
            initialized = true;
            return nullptr;
        }
    }

    template<typename T>
    size_t gc_preserve(T in)
    {
        auto* value = (unsafe::Value*) in;
        jl_gc_pause;

        static auto _ = detail::gc_init();
        static unsafe::Function* jluna_add_to_heap = get_function(jl_main_module, "__jluna_add_to_heap"_sym);

        auto res = jl_unbox_uint64(call(jluna_add_to_heap, jl_box_uint64((UInt64) value)));
        jl_gc_unpause;
        return res;
    }

    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 3), bool>>
    unsafe::Array* new_array(unsafe::Value* value_type, Dims... size_per_dimension)
    {
        std::array<jl_value_t*, sizeof...(Dims)> dims = {jl_box_uint64(size_per_dimension)...};
        return jl_new_array(jl_apply_array_type(value_type, sizeof...(Dims)), dims.data());
    }

    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 3), bool>>
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, Dims... size_per_dimension)
    {
        std::array<jl_value_t*, sizeof...(Dims)> dims = {jl_box_uint64(size_per_dimension)...};
        return jl_ptr_to_array(jl_apply_array_type(value_type, 2), data, (jl_value_t*) dims.data(), 0);
    }

    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 3), bool>>
    void resize_array(unsafe::Array* array, Dims... dims)
    {
        static unsafe::Function* reshape = get_function(jl_base_module, "reshape"_sym);
        std::array<size_t, sizeof...(Dims)> dims_array = {dims...};
        auto* res = jl_reshape_array(jl_array_value_t(array), array, dims_array);
        override_array(array, res);
    }

    template<typename... Index>
    unsafe::Value* get_index(unsafe::Array* array, Index... index_per_dimension)
    {
        std::array<size_t, sizeof...(Index)> indices = {size_t(index_per_dimension)...};
        size_t index = 0;
        size_t mul = 1;

        for (size_t i = 0; i < array->flags.ndims; ++i)
        {
            index += (indices.at(i)) * mul;
            size_t dim = jl_array_dim(array, i);
            mul *= dim;
        }

        return jl_arrayref(array, index);
    }

    template<typename... Index>
    void set_index(unsafe::Array* array, unsafe::Value* new_value, Index... index_per_dimension)
    {
        std::array<size_t, sizeof...(Index)> indices = {size_t(index_per_dimension)...};
        size_t index = 0;
        size_t mul = 1;

        for (size_t i = 0; i < array->flags.ndims; ++i)
        {
            index += (indices.at(i)) * mul;
            size_t dim = jl_array_dim(array, i);
            mul *= dim;
        }

        jl_arrayset(array, new_value, index);
    }
}