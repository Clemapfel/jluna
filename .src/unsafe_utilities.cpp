// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#include <include/unsafe_utilities.hpp>

namespace jluna
{
    unsafe::Symbol* operator""_sym(const char* str, size_t)
    {
        return jl_symbol(str);
    }
}

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

    unsafe::Array* new_array(unsafe::Value* value_type, size_t one_d)
    {
        return jl_alloc_array_1d(jl_apply_array_type(value_type, 1), one_d);
    }

    unsafe::Array* new_array(unsafe::Value* value_type, size_t one_d, size_t two_d)
    {
        return jl_alloc_array_2d(jl_apply_array_type(value_type, 2), one_d, two_d);
    }

    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, size_t one_d)
    {
        return jl_ptr_to_array_1d(jl_apply_array_type(value_type, 1), data, one_d, 0);
    }

    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, size_t one_d, size_t two_d)
    {
        std::array<jl_value_t*, 2> dims = {jl_box_uint64(one_d), jl_box_uint64(two_d)};
        return jl_ptr_to_array(jl_apply_array_type(value_type, 2), data, (jl_value_t*) dims.data(), 0);
    }

    void override_array(unsafe::Array* overridden, const unsafe::Array* constant)
    {
        //memcpy(overridden->data, constant->data, constant->length);
        overridden->data = constant->data;
        overridden->length = constant->length;
        overridden->nrows = constant->nrows;
        overridden->ncols = constant->ncols;
        overridden->flags = constant->flags;
        overridden->offset = constant->offset;
        overridden->elsize = constant->elsize;
        overridden->maxsize = constant->maxsize;
    }

    void resize_array(unsafe::Array* array, size_t one_d)
    {
        if (jl_array_ndims(array) != 1)
        {
            std::array<jl_value_t*, 1> dims = {jl_box_uint64(one_d)};
            override_array(array, jl_reshape_array(jl_array_value_t(array), array, (jl_value_t*) dims.data()));
            return;
        }

        array->ncols = 1;
        array->nrows = one_d;
        array->length = one_d;
        array->flags.isaligned = 0;
    }

    void resize_array(unsafe::Array* array, size_t one_d, size_t two_d)
    {
        if (jl_array_ndims(array) != 2)
        {
            std::array<jl_value_t*, 2> dims = {jl_box_uint64(one_d), jl_box_uint64(two_d)};
            override_array(array, jl_reshape_array(jl_array_value_t(array), array, (jl_value_t*) dims.data()));
            return;
        }

        array->ncols += array->ncols - one_d;
        array->nrows += array->nrows - two_d;
        array->length = one_d * two_d;
        array->flags.isaligned = 0;
    }
}

