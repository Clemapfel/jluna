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
}

namespace jluna::unsafe
{
    /// @brief preserve a julia-heap allocated object until gc_free is called
    /// @param object, pointer
    /// @returns id, keep track of this as it is needed to free the object
    template<IsReinterpretableTo<unsafe::Value*> T>
    size_t gc_preserve(T value);

    /// @brief free a preserved object
    /// @param id: id of object, result of gc_preserve
    void gc_release(size_t id);

    /// @brief access function in module
    /// @param module
    /// @param name: function name
    /// @returns function pointer or nullptr if not found
    unsafe::Function* get_function(unsafe::Module* module, unsafe::Symbol* name);

    /// @brief access function by module name
    /// @param module_name
    /// @param function_name
    /// @returns function pointer or nullptr if not found
    unsafe::Function* get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name);

    /// @brief call function with args, with brief exception forwarding
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param function
    /// @param args
    /// @returns result
    template<typename... Args_t>
    unsafe::Value* call(unsafe::Function* function, Args_t... args);

    /// @brief ctor julia-side expression
    /// @param symbol
    /// @params arguments (optional)
    /// @returns pointer to julia-side expression
    template<typename... Args_t>
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
    unsafe::Value* set_value(unsafe::Module* module, unsafe::Symbol* name, unsafe::Value* value);

    /// @brief set julia-side value by variable name
    /// @param module_name
    /// @param variable_name
    /// @param value: new value
    /// @returns pointer variable after assignment, null if failed
    unsafe::Value* set_value(unsafe::Symbol* module_name, unsafe::Symbol* variable_name);

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
    template<Is<size_t>... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool> = true>
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
    template<Is<size_t>... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool> = true>
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, Dims... size_per_dimension);

    /// @brief allocate 1d array
    /// @param value_type
    /// @param data
    /// @params size
    /// @returns array
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, size_t one_d);

    /// @brief allocate 2d array
    /// @param value_type
    /// @param data
    /// @params x_dimension
    /// @params y_dimension
    /// @returns array
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, size_t one_d, size_t two_d);

    /// @brief reshape array along all dimensions
    /// @param array
    /// @param size_per_dimension: size along each dimension, number of dimension is denoted by the number of sizes
    template<Is<size_t>... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool> = true>
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
    size_t get_array_size(unsafe::Array*, size_t dimension_index = 0);

    /// @brief access element, linear indexing
    /// @param array
    /// @params index_per_dimension
    /// @returns pointer to array, nullptr if inaccessible
    template<Is<size_t>... Index>
    unsafe::Value* get_index(unsafe::Array*, Index... index_per_dimension);

    /// @brief modify element, linear indexing
    /// @param array
    /// @param new_value
    /// @params index_per_dimension
    template<Is<size_t>... Index>
    void set_index(unsafe::Array*, unsafe::Value* value, Index... index_per_dimension);

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
    void set_array_data(unsafe::Array* array, unsafe::Value* new_data, size_t new_size);
}

#include <.src/unsafe_utilities.inl>
