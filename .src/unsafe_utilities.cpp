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

    unsafe::Value* operator""_eval(const char* str, size_t)
    {
        return jl_eval_string(str);
    }
}

namespace jluna::unsafe
{
    unsafe::Function* get_function(unsafe::Module* module, unsafe::Symbol* name)
    {
        gc_pause;
        static jl_function_t* eval = jl_get_function(jl_base_module, "eval");
        auto* res = unsafe::call(eval, (unsafe::Value*) module, name);
        gc_unpause;
        return res;
    }

    unsafe::Function* get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name)
    {
        gc_pause;
        static jl_function_t* eval = jl_get_function(jl_base_module, "eval");
        auto* res = unsafe::call(eval, unsafe::call(eval, jl_main_module, (unsafe::Value*) module_name), function_name);
        gc_unpause;
        return res;
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
        gc_pause;
        static auto* eval = unsafe::get_function(jl_base_module, "eval"_sym);
        auto* res = call(eval, (unsafe::Value*) module, name);
        gc_unpause;
        return res;
    }

    unsafe::Value* get_value(unsafe::Symbol* module, unsafe::Symbol* name)
    {
        gc_pause;
        static unsafe::Function* eval = get_function(jl_base_module, "eval"_sym);
        auto* res = call(eval, call(eval, jl_main_module, module), name);
        gc_unpause;
        return res;
    }

    unsafe::Value* set_value(unsafe::Module* module, unsafe::Symbol* name, unsafe::Value* value)
    {
        gc_pause;
        static unsafe::Function* eval = get_function(jl_base_module, "eval"_sym);
        auto* res = call(eval, module, Expr("="_sym, name, value));
        gc_unpause;
        return res;
    }

    unsafe::Value* set_value(unsafe::Symbol* module, unsafe::Symbol* name, unsafe::Value* value)
    {
        gc_pause;
        static unsafe::Function* eval = get_function(jl_base_module, "eval"_sym);
        auto* res = call(eval, call(eval, jl_main_module, module), Expr("="_sym, name, value));
        gc_unpause;
        return res;
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

    void gc_release(size_t id)
    {
        gc_pause;
        static auto _ = detail::gc_init();
        static unsafe::Value* heap = get_value(jl_main_module, "__jluna_heap"_sym);
        static unsafe::Value* heap_index = get_value(jl_main_module, "__jluna_heap_index"_sym);

        static unsafe::Function* delete_key = get_function(jl_base_module, "delete!"_sym);
        call(delete_key, heap, jl_box_uint64(id));
        gc_unpause;
    }

    void gc_enable()
    {
        jl_gc_enable(false);
    }

    void gc_disable()
    {
        jl_gc_enable(true);
    }

    bool gc_is_enabled()
    {
        return jl_gc_is_enabled();
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
        std::array<unsafe::Value*, 2> dims = {jl_box_uint64(one_d), jl_box_uint64(two_d)};
        return jl_ptr_to_array(jl_apply_array_type(value_type, 2), data, (unsafe::Value*) dims.data(), 0);
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

    void resize_array(unsafe::Array* array, size_t one_d)
    {
        static unsafe::Function* array_value_t = unsafe::get_function("jluna"_sym, "get_value_type_of_array"_sym);

        if (jl_array_ndims(array) != 1)
        {
            gc_pause;
            static auto* tuple_type = [&](){
                std::array<unsafe::Value*, 1> types;
                for (size_t i = 0; i < types.size(); ++i)
                    types.at(i) = (unsafe::Value*) jl_uint64_type;

                return jl_apply_tuple_type_v(types.data(), types.size());
            }();
            auto* tuple = jl_new_struct(tuple_type, jl_box_uint64(one_d));
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

    void resize_array(unsafe::Array* array, size_t one_d, size_t two_d)
    {
        static unsafe::Function* array_value_t = unsafe::get_function("jluna"_sym, "get_value_type_of_array"_sym);

        if (jl_array_ndims(array) != 2)
        {
            gc_pause;
            static auto* tuple_type = [&](){
                std::array<unsafe::Value*, 2> types;
                for (size_t i = 0; i < types.size(); ++i)
                    types.at(i) = (unsafe::Value*) jl_uint64_type;

                return jl_apply_tuple_type_v(types.data(), types.size());
            }();
            auto* tuple = jl_new_struct(tuple_type, jl_box_uint64(one_d));
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

