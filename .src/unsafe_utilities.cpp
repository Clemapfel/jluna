// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#include <include/unsafe_utilities.hpp>

namespace jluna
{
    unsafe::Symbol* operator""_sym(const char* str, uint64_t)
    {
        return jl_symbol(str);
    }

    unsafe::Value* operator""_eval(const char* str, uint64_t)
    {
        return jl_eval_string(str);
    }
}

namespace jluna::unsafe
{
    unsafe::Function* get_function(unsafe::Module* module, unsafe::Symbol* name)
    {
        return jl_get_global(module, name);
    }

    unsafe::Function* get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name)
    {
        return unsafe::get_function((unsafe::Module*) jl_get_global(jl_main_module, module_name), function_name);
    }

    unsafe::Value* eval(unsafe::Expression* expr, unsafe::Module* module)
    {
        gc_pause;
        static unsafe::Function* base_eval = get_function(jl_base_module, "eval"_sym);
        auto* res = call(base_eval, module, expr);
        gc_unpause;
        return res;
    }

    unsafe::Value* get_value(unsafe::Module* module, unsafe::Symbol* name)
    {
        return jl_get_global(module, name);
    }

    unsafe::Value* get_value(unsafe::Symbol* module_name, unsafe::Symbol* name)
    {
        return unsafe::get_function((unsafe::Module*) jl_get_global(jl_main_module, module_name), name);
    }

    void set_value(unsafe::Module* module, unsafe::Symbol* name, unsafe::Value* value)
    {
        jl_set_global(module, name, value);
    }

    void set_value(unsafe::Symbol* module_name, unsafe::Symbol* name, unsafe::Value* value)
    {
        unsafe::set_value((unsafe::Module*) jl_get_global(jl_main_module, module_name), name, value);
    }

    unsafe::Value* get_field(unsafe::Value* x, unsafe::Symbol* field)
    {
        gc_pause;
        static unsafe::Function* getfield = get_function(jl_base_module, "getfield"_sym);
        auto* res = call(getfield, x, field);
        gc_unpause;
        return res;
    }

    void set_field(unsafe::Value* x, unsafe::Symbol* field, unsafe::Value* new_value)
    {
        gc_pause;
        static unsafe::Function* setfield = get_function(jl_base_module, "setfield!"_sym);
        call(setfield, x, field, new_value);
        gc_unpause;
    }

    void gc_release(uint64_t id)
    {
        gc_pause;
        detail::gc_init();
           
        static unsafe::Value* delete_from_heap = get_value(jl_main_module, "__jluna_delete_from_heap"_sym);
        call(delete_from_heap, jl_box_uint64(id));
        gc_unpause;
    }

    void gc_enable()
    {
        jl_gc_enable(true);
    }

    void gc_disable()
    {
        jl_gc_enable(false);
    }

    bool gc_is_enabled()
    {
        return jl_gc_is_enabled();
    }

    uint64_t get_array_size(unsafe::Array* array)
    {
        return array->length;
    }

    uint64_t get_array_size(unsafe::Array* array, uint64_t dimension_index)
    {
        return jl_array_dim(array, dimension_index);
    }

    unsafe::Value* get_array_data(unsafe::Array* array)
    {
        return reinterpret_cast<unsafe::Value*>(array->data);
    }

    unsafe::Array* new_array(unsafe::Value* value_type, uint64_t one_d)
    {
        return jl_alloc_array_1d(jl_apply_array_type(value_type, 1), one_d);
    }

    unsafe::Array* new_array(unsafe::Value* value_type, uint64_t one_d, uint64_t two_d)
    {
        return jl_alloc_array_2d(jl_apply_array_type(value_type, 2), one_d, two_d);
    }

    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, uint64_t one_d)
    {
        return jl_ptr_to_array_1d(jl_apply_array_type(value_type, 1), data, one_d, 0);
    }

    void sizehint(unsafe::Array* arr, uint64_t n_elements)
    {
        jl_array_sizehint(arr, n_elements);
    }

    void override_array(unsafe::Array* overridden, const unsafe::Array* constant)
    {
        //memcpy(overridden->data, constant->data, constant->length);
        overridden->data = constant->data;
        jl_gc_wb(overridden, constant->data);
        overridden->length = constant->length;
        overridden->nrows = constant->nrows;
        overridden->ncols = constant->ncols;
        overridden->flags = constant->flags;
        overridden->offset = constant->offset;
        overridden->elsize = constant->elsize;
        overridden->maxsize = constant->maxsize;
    }

    void swap_array_data(unsafe::Array* a, unsafe::Array* b)
    {
        gc_pause;
        auto temp_data = a->data;
        auto temp_length = a->length;
        auto temp_nrows = a->nrows;
        auto temp_ncols = a->ncols;
        auto temp_flags = a->flags;
        auto temp_offset = a->offset;
        auto temp_elsize = a->elsize;
        auto temp_maxsize = a->maxsize;

        a->data = b->data;
        a->length = b->length;
        a->nrows = b->nrows;
        a->ncols = b->ncols;
        a->flags = b->flags;
        a->offset = b->offset;
        a->elsize = b->elsize;
        a->maxsize = b->maxsize;

        b->data = temp_data;
        b->length = temp_length;
        b->nrows = temp_nrows;
        b->ncols = temp_ncols;
        b->flags = temp_flags;
        b->offset = temp_offset;
        b->elsize = temp_elsize;
        b->maxsize = temp_maxsize;
        gc_unpause;
    }

    void resize_array(unsafe::Array* array, uint64_t one_d)
    {
        static unsafe::Function* array_value_t = unsafe::get_function("jluna"_sym, "get_value_type_of_array"_sym);

        if (jl_array_ndims(array) != 1)
        {
            gc_pause;
            std::array<unsafe::Value*, 1> types;
            for (uint64_t i = 0; i < types.size(); ++i)
                types.at(i) = (unsafe::Value*) jl_int64_type;

            auto* tuple_type = jl_apply_tuple_type_v(types.data(), types.size());

            #if JULIA_VERSION_MAJOR >= 2 or JULIA_VERSION_MINOR >= 10
                auto* tuple = jl_new_struct((jl_datatype_t*) tuple_type, jl_box_int64(static_cast<Int64>(one_d)));
            #else
                auto* tuple = jl_new_struct(tuple_type, jl_box_int64(static_cast<Int64>(one_d)));
            #endif

            auto* res = jl_reshape_array(jl_apply_array_type(unsafe::call(array_value_t, array), 1), array, tuple);
            override_array(array, res);
            gc_unpause;
            return;
        }

        array->ncols = 1;
        array->nrows = one_d;
        array->length = one_d;
        array->flags.isaligned = 0;
    }

    void resize_array(unsafe::Array* array, uint64_t one_d, uint64_t two_d)
    {
        static unsafe::Function* array_value_t = unsafe::get_function("jluna"_sym, "get_value_type_of_array"_sym);

        if (jl_array_ndims(array) != 2)
        {
            gc_pause;
            std::array<unsafe::Value*, 2> types;
            for (uint64_t i = 0; i < types.size(); ++i)
                types.at(i) = (unsafe::Value*) jl_int64_type;

            auto* tuple_type = jl_apply_tuple_type_v(types.data(), types.size());

            #if JULIA_VERSION_MAJOR >= 2 or JULIA_VERSION_MINOR >= 10
                auto* tuple = jl_new_struct((jl_datatype_t*) tuple_type, jl_box_int64(static_cast<Int64>(one_d)), jl_box_int64(static_cast<Int64>(two_d)));
            #else
                auto* tuple = jl_new_struct(tuple_type, jl_box_int64(static_cast<Int64>(one_d)), jl_box_int64(static_cast<Int64>(two_d)));
            #endif

            auto* res = jl_reshape_array(jl_apply_array_type(unsafe::call(array_value_t, array), 2), array, tuple);
            override_array(array, res);
            gc_unpause;
            return;
        }

        array->ncols += array->ncols - one_d;
        array->nrows += array->nrows - two_d;
        array->length = one_d * two_d;
        array->flags.isaligned = 0;
    }
}

