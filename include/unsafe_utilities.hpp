// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/typedefs.hpp>
#include <include/concepts.hpp>

namespace jluna
{
    /// @brief string suffix operator to create a symbol from a string
    unsafe::Symbol* operator""_sym(const char*, size_t);

    /// @brief literal operator for prettier syntax
    /// @returns result of jl_eval_string
    unsafe::Value* operator""_eval(const char*, size_t);
}

namespace jluna::unsafe
{
    /// @brief preserve a julia heap-allocated object until unsafe::gc_release is called
    /// @param object: pointer
    /// @returns id, keep track of this as it is needed to free the object
    template<is_julia_value T>
    [[nodiscard]] size_t gc_preserve(T* value);

    /// @brief preserve multiple values
    /// @param values: pointer
    /// @returns vector of ids, needed to free the objects
    template<is_julia_value_pointer... Ts, std::enable_if_t<(sizeof...(Ts) > 2), bool> = true>
    [[nodiscard]] std::vector<size_t> gc_preserve(Ts... values);

    /// @brief free a preserved object
    /// @param id: id of object, result of gc_preserve
    void gc_release(size_t id);

    /// @brief free preserved objects
    /// @param vector of ids
    void gc_release(std::vector<size_t>& ids);

    /// @brief set garbage collection to inactive
    void gc_disable();

    /// @brief set garbage collection to active
    void gc_enable();

    /// @brief is garbage collection enabled
    /// @returns true if active, false otherwise
    bool gc_is_enabled();

    /// @brief access function in module
    /// @param module
    /// @param name: exact function name
    /// @returns function pointer or nullptr if not found
    unsafe::Function* get_function(unsafe::Module* module, unsafe::Symbol* name);

    /// @brief access function by module name
    /// @param module_name
    /// @param function_name
    /// @returns function pointer or nullptr if not found
    unsafe::Function* get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name);

    /// @brief call function with args
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param function
    /// @param args
    /// @returns result
    template<is_julia_value_pointer... Args_t>
    unsafe::Value* call(unsafe::Function* function, Args_t... args);

    /// @brief call type ctor with args
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param type
    /// @param args
    /// @returns result
    template<is_julia_value_pointer... Args_t>
    unsafe::Value* call(unsafe::DataType* type, Args_t... args);

    /// @brief ctor julia-side expression
    /// @param symbol
    /// @params arguments (optional)
    /// @returns pointer to julia-side expression
    template<is_julia_value_pointer... Args_t>
    unsafe::Expression* Expr(unsafe::Symbol*, Args_t...);

    /// @brief eval expression in module scope
    unsafe::Value* eval(unsafe::Expression*, unsafe::Module* = jl_main_module);

    /// @brief get julia-side value by variable name
    /// @param module
    /// @param name: variable name
    /// @returns pointer to value
    unsafe::Value* get_value(unsafe::Module* module, unsafe::Symbol* name);

    /// @brief get julia-side value by variable name
    /// @param module_name
    /// @param variable_name
    /// @returns pointer to value
    unsafe::Value* get_value(unsafe::Symbol* module_name, unsafe::Symbol* variable_name);

    /// @brief set julia-side value by variable name
    /// @param module
    /// @param name: variable name
    /// @param value: new value
    /// @returns pointer variable after assignment, null if failed
    void set_value(unsafe::Module* module, unsafe::Symbol* name, unsafe::Value* value);

    /// @brief set julia-side value by variable name
    /// @param module_name
    /// @param variable_name
    /// @param value: new value
    /// @returns pointer variable after assignment, null if failed
    void set_value(unsafe::Symbol* module_name, unsafe::Symbol* variable_name);

    /// @brief get field of value by name
    /// @param value
    /// @param field
    /// @returns field value
    unsafe::Value* get_field(unsafe::Value* x, unsafe::Symbol* field);

    /// @brief get field of value by name
    /// @param value
    /// @param field_name
    /// @param new_value
    void set_field(unsafe::Value* x, unsafe::Symbol* field, unsafe::Value* new_value);

    /// @brief allocate new array
    /// @param value_type
    /// @params size_per_dimension
    /// @returns array
    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool> = true>
    unsafe::Array* new_array(unsafe::Value* value_type, Dims... size_per_dimension);

    /// @brief allocate 1d array
    /// @param value_type
    /// @params size
    /// @returns array
    unsafe::Array* new_array(unsafe::Value* value_type, size_t one_d);

    /// @brief allocate 2d array
    /// @param value_type
    /// @params x_dimension
    /// @params y_dimension
    /// @returns array
    unsafe::Array* new_array(unsafe::Value* value_type, size_t one_d, size_t two_d);

    /// @brief create an array from already existing data without invoking a copy
    /// @param value_type
    /// @param data: pointer to the data, no verification is performed that the data is properly aligned or of the given value type
    /// @param size_per_dimension
    /// @returns pointer to the array
    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 1), bool> = true>
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, Dims... size_per_dimension);

    /// @brief allocate 1d array
    /// @param value_type
    /// @param data
    /// @params size
    /// @returns array
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, size_t one_d);

    /// @brief reshape array along all dimensions
    /// @param array
    /// @param size_per_dimension: size along each dimension, number of dimension is denoted by the number of sizes
    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool> = true>
    void resize_array(unsafe::Array* array, Dims...);

    /// @brief reshape array to 1d
    /// @param array
    /// @param one_d: number of rows
    /// @note optimal performance is only achieved, is already 1d
    void resize_array(unsafe::Array* array, size_t one_d);

    /// @brief reshape array to 2d
    /// @param array
    /// @param one_d: number of rows
    /// @param two_d: number of cols
    /// @note optimal performance is only achieved, is already 2d
    void resize_array(unsafe::Array* array, size_t one_d, size_t two_d);

    /// @brief replace one arrays content with another, does not cause allocation
    /// @param overridden: array that is modified
    /// @param constant: unmodified array whos data will be written to the argument
    void override_array(unsafe::Array* overridden, const unsafe::Array* constant);

    /// @brief get number of elements in array
    /// @param array
    /// @returns number of elements
    size_t get_array_size(unsafe::Array*);

    /// @brief get array size along dimension
    /// @param dimension_index
    /// @returns size along that dimension
    size_t get_array_size(unsafe::Array*, size_t dimension_index);

    /// @brief access element, linear indexing
    /// @param array
    /// @params index_per_dimension
    /// @returns pointer to array, nullptr if inaccessible
    template<typename... Index, std::enable_if_t<(sizeof...(Index) > 2), bool> = true>
    unsafe::Value* get_index(unsafe::Array*, Index... index_per_dimension);
    unsafe::Value* get_index(unsafe::Array*, size_t);
    unsafe::Value* get_index(unsafe::Array*, size_t, size_t);

    /// @brief modify element, linear indexing
    /// @param array
    /// @param new_value
    /// @params index_per_dimension
    template<typename... Index, std::enable_if_t<(sizeof...(Index) > 2), bool> = true>
    void set_index(unsafe::Array*, unsafe::Value* value, Index... index_per_dimension);
    void set_index(unsafe::Array*, unsafe::Value* value, size_t);
    void set_index(unsafe::Array*, unsafe::Value* value, size_t, size_t);

    /// @brief access raw array data
    /// @param array
    /// @returns pointer to first element
    unsafe::Value* get_array_data(unsafe::Array*);

    /// @brief swap raw array data, resizes arrays if necessary
    /// @param before
    /// @param after
    void swap_array_data(unsafe::Array* before, unsafe::Array* after);

    /// @brief set array data, resizes array if necessary
    /// @param array
    /// @param new_data: pointer to new data
    /// @param new_size: size of new_data
    /// @note if value types mismatch, the behavior is undefined
    template<typename T>
    void set_array_data(unsafe::Array* array, T* new_data, size_t new_size);

    /// @brief box without type checking
    /// @param value
    /// @returns julia-side value of equivalent type to C++ arg
    template<is_julia_value T>
    unsafe::Value* unsafe_box(T*);

    /// @brief unbox without type checking
    /// @param julia_value_ptr
    /// @returns unbox value
    template<typename T>
    T unsafe_unbox(unsafe::Value*);
}

/// @brief pause the garbage collector. This prevents values from being deallocated until gc_unpause was called
#define gc_pause bool __before__ = jl_gc_is_enabled(); jl_gc_enable(false);

/// @brief unpause the garbage collector, values allocated while it was paused my be collector
#define gc_unpause if(__before__) {jl_gc_enable(true), jl_gc_collect(JL_GC_AUTO);}

#include <.src/unsafe_utilities.inl>
