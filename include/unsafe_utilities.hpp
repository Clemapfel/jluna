// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/typedefs.hpp>
#include <include/concepts.hpp>
#include <.src/gc_sentinel.hpp>

namespace jluna
{
    /// @brief string suffix operator to create a symbol from a string
    /// @returns symbol
    unsafe::Symbol* operator""_sym(const char*, uint64_t);

    /// @brief literal operator for prettier syntax
    /// @returns result of jl_eval_string
    unsafe::Value* operator""_eval(const char*, uint64_t);
}

namespace jluna::unsafe
{
    /// @brief preserve a julia heap-allocated object until unsafe::gc_release is called on it
    /// @param object: pointer
    /// @returns id, keep track of this as it is needed to free the object
    template<is_julia_value T>
    [[nodiscard]] uint64_t gc_preserve(T*);

    /// @brief preserve multiple values
    /// @param values: pointer
    /// @returns vector of ids, needed to free the objects
    template<is_julia_value_pointer... Ts, std::enable_if_t<(sizeof...(Ts) > 2), bool> = true>
    [[nodiscard]] std::vector<uint64_t> gc_preserve(Ts... values);

    /// @brief free a preserved object, the object may not be dealloaced immediately, simply marked for garbage collection
    /// @param id: id of object, result of gc_preserve
    void gc_release(uint64_t id);

    /// @brief free preserved objects
    /// @param ids: vector of ids
    void gc_release(std::vector<uint64_t>& ids);

    /// @brief set garbage collection to inactive
    void gc_disable();

    /// @brief set garbage collection to active
    void gc_enable();

    /// @brief is garbage collection enabled
    /// @returns true if active, false otherwise
    bool gc_is_enabled();

    /// @brief access function in module
    /// @param module: pointer to module object
    /// @param name: function name
    /// @returns function pointer, or nullptr if not found
    unsafe::Function* get_function(unsafe::Module* module, unsafe::Symbol* name);

    /// @brief access function by module name
    /// @param module_name: name of module, as opposed to the module itself
    /// @param function_name: name of function
    /// @returns function pointer, or nullptr if not found
    unsafe::Function* get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name);

    /// @brief call function with args
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param function: function to invoke
    /// @param args: argument, forwarded to function
    /// @returns results of function or `nothing`
    template<is_julia_value_pointer... Args_t>
    unsafe::Value* call(unsafe::Function* function, Args_t... args);

    /// @brief invoke a types constructor
    /// @tparam Args_t: argument types, must be castable to unsafe::Value*
    /// @param type: type whose constructor to call
    /// @param args: arguments, forwarded to constructor
    /// @returns resulting type instance
    template<is_julia_value_pointer... Args_t>
    unsafe::Value* call(unsafe::DataType* type, Args_t... args);

    /// @brief construct a Julia-side expression
    /// @param symbol: used as Expr.head
    /// @params arguments: [optional] used as Expr.args
    /// @returns pointer to resulting expression
    template<is_julia_value_pointer... Args_t>
    unsafe::Expression* Expr(unsafe::Symbol*, Args_t...);

    /// @brief eval expression in module scope
    /// @param expression: expression
    /// @param module: module to eval in
    unsafe::Value* eval(unsafe::Expression*, unsafe::Module* = jl_main_module);

    /// @brief get julia-side value by variable name
    /// @param module: module the variable is defined in
    /// @param name: variable name
    /// @returns pointer to value
    unsafe::Value* get_value(unsafe::Module* module, unsafe::Symbol* name);

    /// @brief get julia-side value by variable name
    /// @param module: name of the module the variable is defined in, as opposed to the module itself
    /// @param name: variable name
    /// @returns pointer to value
    unsafe::Value* get_value(unsafe::Symbol* module_name, unsafe::Symbol* variable_name);

    /// @brief set julia-side value, by variable name
    /// @param module: module the variable is defined in
    /// @param name: variable name
    /// @param value: new value
    /// @returns pointer to variable after assignment, or nullptr if no assignment took place
    void set_value(unsafe::Module* module, unsafe::Symbol* name, unsafe::Value* value);

    /// @brief set julia-side value by variable name
    /// @param module: name of the module the variable is defined in, as opposed to the module itself
    /// @param name: variable name
    /// @param value: new value
    /// @returns pointer to variable after assignment, or nullptr if no assignment took place
    void set_value(unsafe::Symbol* module_name, unsafe::Symbol* variable_name);

    /// @brief get value of a instances field
    /// @param value: instance
    /// @param field: name of the field
    /// @returns value of the field
    unsafe::Value* get_field(unsafe::Value* value, unsafe::Symbol* field);

    /// @brief set value of instances field
    /// @param value: instance
    /// @param field_name: name of the field
    /// @param new_value: new value assigned to be assigned
    void set_field(unsafe::Value* x, unsafe::Symbol* field, unsafe::Value* new_value);

    /// @brief allocate new array
    /// @param value_type: value type of the array
    /// @params size_per_dimension: size along each dimension
    /// @returns array after allocation
    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool> = true>
    unsafe::Array* new_array(unsafe::Value* value_type, Dims... size_per_dimension);

    /// @brief allocate 1d array
    /// @param value_type: value type of the array
    /// @params one_d: size along first dimension
    /// @returns array after allocation
    unsafe::Array* new_array(unsafe::Value* value_type, uint64_t one_d);

    /// @brief allocate 1d array
    /// @param value_type: value type of the array
    /// @params one_d: size along first dimension
    /// @params two_d: size along second dimension
    /// @returns array after allocation
    unsafe::Array* new_array(unsafe::Value* value_type, uint64_t one_d, uint64_t two_d);

    /// @brief create an array from already existing data, does not invoking a copy
    /// @param value_type: value type of the array
    /// @param data: pointer to the data, no verification is performed that the data is properly aligned or of the given value type
    /// @param size_per_dimension: size along each dimension
    /// @returns array object as thin wrapper
    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 1), bool> = true>
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, Dims... size_per_dimension);

    /// @brief allocate 1d array as thin wrapper
    /// @param value_type: value type of the array
    /// @param data: pointer to the data, no verification is performed that the data is properly aligned or of the given value type
    /// @params size: size along first dimension
    /// @returns array object as thin wrapper
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, uint64_t one_d);

    /// @brief pre-allocate array, may increase performance
    /// @param array: array to reserve
    /// @param n_elements: new number of elements
    void sizehint(unsafe::Array*, uint64_t n_elements);

    /// @brief reshape array along all dimensions
    /// @param array: array to reshape
    /// @param size_per_dimension: size along each dimension, number of dimension is denoted by the number of sizes
    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool> = true>
    void resize_array(unsafe::Array* array, Dims...);

    /// @brief reshape array to 1d
    /// @param array: array to reshape
    /// @param one_d: size along first dimension
    /// @note optimal performance is only achieved if the array is already 1d
    void resize_array(unsafe::Array* array, uint64_t one_d);

    /// @brief reshape array to 2d
    /// @param array: array to reshape
    /// @param one_d: size along first dimension
    /// @param two_d: size along second dimension
    /// @note optimal performance is only achieved if the array is already 2d
    void resize_array(unsafe::Array* array, uint64_t one_d, uint64_t two_d);

    /// @brief replace one arrays content with another, does not cause allocation
    /// @param overridden: array to be modified
    /// @param constant: unmodified array whose data will be written to the argument
    void override_array(unsafe::Array* overridden, const unsafe::Array* constant);

    /// @brief get number of elements in array
    /// @param array: array
    /// @returns number of elements
    uint64_t get_array_size(unsafe::Array*);

    /// @brief get array size along specified dimension
    /// @param dimension_index: index, 0-based
    /// @returns size along that dimension
    uint64_t get_array_size(unsafe::Array*, uint64_t dimension_index);

    /// @brief access element
    /// @param array: array to access
    /// @params index_per_dimension: index in each dimension
    /// @returns pointer to element, or nullptr if out of bounds
    template<typename... Index, std::enable_if_t<(sizeof...(Index) > 2), bool> = true>
    unsafe::Value* get_index(unsafe::Array*, Index... index_per_dimension);
    unsafe::Value* get_index(unsafe::Array*, uint64_t);
    unsafe::Value* get_index(unsafe::Array*, uint64_t, uint64_t);

    /// @brief modify element by index
    /// @param array: array to modify
    /// @param new_value: new value assigned to he element
    /// @params index_per_dimension: index in each dimension
    template<typename... Index, std::enable_if_t<(sizeof...(Index) > 2), bool> = true>
    void set_index(unsafe::Array*, unsafe::Value* value, Index... index_per_dimension);
    void set_index(unsafe::Array*, unsafe::Value* value, uint64_t);
    void set_index(unsafe::Array*, unsafe::Value* value, uint64_t, uint64_t);

    /// @brief access raw array data
    /// @param array: array
    /// @returns pointer to first element
    unsafe::Value* get_array_data(unsafe::Array*);

    /// @brief swap raw array data, resizes arrays if necessary
    /// @param before: initial array, will be modified
    /// @param after: array whos data will be written to the initial array
    void swap_array_data(unsafe::Array* before, unsafe::Array* after);

    /// @brief set array data, resizes array if necessary
    /// @param array: array to be modified
    /// @param new_data: pointer to new data
    /// @param new_size: number of elements in new_data
    /// @note if value types mismatch, the behavior is undefined
    template<typename T>
    void set_array_data(unsafe::Array* array, T* new_data, uint64_t new_size);

    /// @brief push element to front of 1d array
    /// @param array: array
    /// @param value: value
    void push_front(unsafe::Array*, unsafe::Value* value);

    /// @brief push element to back of 1d array
    /// @param array: array
    /// @param value: value
    void push_back(unsafe::Array*, unsafe::Value* value);

    /// @brief box without type checking
    /// @param value: C++-side value
    /// @returns julia-side value of equivalent type to C++ arg
    template<is_julia_value T>
    unsafe::Value* unsafe_box(T*);

    /// @brief unbox without type checking
    /// @param value: Julia-side value
    /// @returns unboxed value
    template<typename T>
    T unsafe_unbox(unsafe::Value*);
}

/// @brief pause GC, remembers current state
#define gc_pause bool __before__ = jl_gc_is_enabled(); jl_gc_enable(false);

/// @brief restore GC state
#define gc_unpause if (__before__) {jl_gc_enable(true); jl_gc_safepoint();}

#include <.src/unsafe_utilities.inl>
