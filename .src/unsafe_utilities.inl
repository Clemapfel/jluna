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

    namespace detail
    {
        inline nullptr_t gc_init()
        {
            jl_eval_string(R"(
                if ! (isdefined(Main, :__jluna_heap) && isdefined(Main, :__jluna_heap_index))
                    Main.eval(:(__jluna_heap = Dict{UInt64, Base.RefValue{Any}}()));
                    Main.eval(:(__jluna_heap_index = Ref(UInt64(0))));
                end
            )");

            return nullptr;
        }
    }

    template<IsReinterpretableTo<unsafe::Value*> T>
    size_t gc_preserve(T value)
    {
        static auto _ = detail::gc_init();
        static unsafe::Value* heap = get_value(jl_main_module, "__jluna_heap"_sym);
        static unsafe::Value* heap_index = get_value(jl_main_module, "__jluna_heap_index"_sym);

        static unsafe::Function* ref = get_function(jl_base_module, "Ref"_sym);
        static unsafe::Function* pointer_from_objref = get_function(jl_base_module, "unsafe_pointer_to_objref"_sym);
        static unsafe::Function* setindex = get_function(jl_base_module, "setindex!"_sym);

        call(setindex, heap, call(ref, value), jl_get_nth_field(heap_index, 0));

        size_t new_index = jl_unbox_uint64(heap_index) + 1;
        jl_set_nth_field(heap_index, 0, jl_box_uint64(new_index));
        return new_index;
    }

    template<Is<size_t>... Dims, std::enable_if_t<(sizeof...(Dims) > 3), bool>>
    unsafe::Array* new_array(unsafe::Value* value_type, Dims... size_per_dimension)
    {
        static unsafe::Function* new_array = get_function("jluna"_sym, "new_array"_sym);
        return (unsafe::Array*) call(new_array, value_type, jl_box_uint64(size_per_dimension)...);
    }

    template<Is<size_t>... Dims, std::enable_if_t<(sizeof...(Dims) > 3), bool>>
    void resize_array(unsafe::Array* array, Dims... dims)
    {
        static unsafe::Function* reshape = get_function(jl_base_module, "reshape"_sym);
        std::array<size_t, sizeof...(Dims)> dims_array = {dims...};
        auto* res = jl_reshape_array(jl_array_value_t(array), array, dims_array);
        override_array(array, res);
    }


    /*
    template<Is<size_t>... Dims>
    void reshape_array(unsafe::Array* array, Dims... dims)
    {
        if (sizeof...(dims) == 1 and array->flags.ndims == 1)
        {
            size_t new_size = std::get<0>(dims...);
            size_t current_size = jl_array_len(array);

            if (new_size > current_size)
                jl_array_grow_end(array, new_size - current_size);
            else
                jl_array_del_end(array, current_size - new_size);
        }
        else if (sizeof...(dims) == 2 and array->flags.ndims == 2)
        {
            size_t current_x = jl_array_dim(array, 0);
            size_t new_x = std::get<0>(dims...);

            for (size_t i = 0; i < std::max(array->ncols, new_x); ++i)
                if (new_x > current_x)
                jl_array_grow_end(array, new_x - current_x);
            else
                jl_array_del_end(array, current_x - new_x);

            size_t current_y = jl_array_dim(array, 0);
            size_t new_y = std::get<0>(dims...);

            for (size_t i = 0; i < std::max(array->ncols, new_y); ++i)
                if (new_y > current_y)
                jl_array_grow_end(array, new_y - current_y);
            else
                jl_array_del_end(array, current_y - new_y);
        }
        else
        {
            static unsafe::Function* reshape = get_function(jl_base_module, "reshape"_sym);

        }
    }
     */

    template<Is<size_t>... Index>
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

    template<Is<size_t>... Index>
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