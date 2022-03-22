// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#include <include/unsafe_utilities.hpp>

namespace jluna::unsafe
{
    unsafe::Function* get_function(unsafe::Module* module, unsafe::Symbol* name)
    {
        static jl_function_t* eval = jl_get_function(jl_base_module, "eval");
        return unsafe::call(eval, module, name);
    }

    unsafe::Function* get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name)
    {
        static jl_function_t* eval = jl_get_function(jl_base_module, "eval");
        return unsafe::call(eval, unsafe::call(eval, jl_main_module, module_name), function_name);
    }

    unsafe::Symbol* operator""_sym(const char* str, size_t)
    {
        return jl_symbol(str);
    }

    unsafe::Value* eval(unsafe::Expression* expr, unsafe::Module* module)
    {
        static unsafe::Function* base_eval = get_function(jl_base_module, "eval"_sym);
        return call(base_eval, module, expr);
    }

    unsafe::Value* get_value(unsafe::Module* module, unsafe::Symbol* name)
    {
        static unsafe::Function* eval = get_function(jl_base_module, "eval"_sym);
        return call(eval, module, name);
    }

    unsafe::Value* get_value(unsafe::Symbol* module, unsafe::Symbol* name)
    {
        static unsafe::Function* eval = get_function(jl_base_module, "eval"_sym);
        return call(eval, call(eval, jl_main_module, module), name);
    }

    unsafe::Value* get_field(unsafe::Value* x, unsafe::Symbol* field)
    {
        static unsafe::Function* getfield = get_function(jl_base_module, "getfield"_sym);
        return call(getfield, x, field);
    }

    void set_field(unsafe::Value* x, unsafe::Symbol* field, unsafe::Value* new_value)
    {
        static unsafe::Function* setfield = get_function(jl_base_module, "setfield!"_sym);
        call(setfield, x, field, new_value);
    }

    void gc_release(size_t id)
    {
        static auto _ = detail::gc_init();
        static unsafe::Value* heap = get_value(jl_main_module, "__jluna_heap"_sym);
        static unsafe::Value* heap_index = get_value(jl_main_module, "__jluna_heap_index"_sym);

        static unsafe::Function* delete_key = get_function(jl_base_module, "delete!"_sym);
        call(delete_key, heap, jl_box_uint64(id));
    }

    unsafe::Array* new_array(unsafe::Value* value_type, size_t size)
    {
        static unsafe::Function* new_array = get_function("jluna"_sym, "new_array"_sym);
        return (unsafe::Array*) call(new_array, value_type, jl_box_uint64(size));
    }

    size_t get_array_size(unsafe::Array* array)
    {
        return array->length;
    }

    size_t get_array_size(unsafe::Array* array, size_t dimension_index)
    {
        return jl_array_dim(array, dimension_index);
    }

    unsafe::Value* get_array_data(unsafe::Array* array)
    {
        return reinterpret_cast<unsafe::Value*>(array->data);
    }

    void set_array_data(unsafe::Array* array, unsafe::Value* new_data, size_t new_size)
    {
        /*
        size_t current_size = jl_array_len(array);

        if (current_size > new_size)
            jl_array_del_end(current, current_size - new_size);
        else
            jl_array_sizehint(current, new_size);

        auto* before = current->data;
        current->data = new_data;
         */
    }





        /*
        size_t current_size = jl_array_len(array);
        size_t new_size = 1;
        for (size_t i : {size_per_dimension...})
            new_size *= i;

        jl_array_sizehint(array, new_size);

        if (current_size < new_size)
            jl_array_grow_end(array, new_size - current_size);
        else
            jl_array_del_end(array, current_size - new_size);

        array->flags.how = 2;
        array->flags.ndims = sizeof...(Dims);
    }
         */




}

