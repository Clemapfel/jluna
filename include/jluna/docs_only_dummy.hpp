#error "This file is only for documentation purposes and should never be included."

// doxygen cannot handle template meta functions and SFINAE, this file contains stand-in dummies for these functions

namespace jluna::docs_only
{
    /// @brief convert C++-side value to Julia-side value
    /// @param value: C++-side value
    /// @returns pointer to julia-side value
    /// @note this function is a stand-in for the multitude of box functions in namespace `jluna::`. For a complete list of what value types can be boxed, please consult the manual.
    template<typename T>
    unsafe::Value* box(T value) {assert(false);}

    /// @brief convert Julia-side value to C++-side value
    /// @param value: pointer to julia-side memory of arbitrary type
    /// @returns C++-side object by value
    /// @note this function is a stand-in for the multitude of unbox functions in namespace `jluna::`. For a complete list of what value types can be unboxed, please consult the manual.
    template<typename T>
    T unbox(unsafe::Value* value) {assert(false);};
}

namespace jluna::unsafe::docs_only
{
    /// @brief preserve value
    /// @param values: pointer
    /// @returns vector of ids, needed to free the objects
    template<typename T>
    uint64_t gc_preserve(T* value) {assert(false);};

    /// @brief preserve values
    /// @param values: pointer
    /// @returns vector of ids, needed to free the objects
    template<typename... Ts>
    uint64_t gc_preserve(Ts... value) {assert(false);};

    /// @brief allocate new array
    /// @param value_type: value type of the array
    /// @params size_per_dimension: size along each dimension
    /// @returns array after allocation
    template<typename... Dims>
    unsafe::Array* new_array(unsafe::Value* value_type, Dims... size_per_dimension) {assert(false);};

    /// @brief allocate 1d array
    /// @param value_type: value type of the array
    /// @params one_d: size along first dimension
    /// @returns array after allocation
    unsafe::Array* new_array(unsafe::Value* value_type, uint64_t one_d) {assert(false);};

    /// @brief allocate 1d array
    /// @param value_type: value type of the array
    /// @params one_d: size along first dimension
    /// @params two_d: size along second dimension
    /// @returns array after allocation
    unsafe::Array* new_array(unsafe::Value* value_type, uint64_t one_d, uint64_t two_d) {assert(false);};

    /// @brief create an array from already existing data, does not invoking a copy
    /// @param value_type: value type of the array
    /// @param data: pointer to the data, no verification is performed that the data is properly aligned or of the given value type
    /// @param size_per_dimension: size along each dimension
    /// @returns array object as thin wrapper
    template<typename... Dims>
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, Dims... size_per_dimension) {assert(false);};

    /// @brief allocate 1d array as thin wrapper
    /// @param value_type: value type of the array
    /// @param data: pointer to the data, no verification is performed that the data is properly aligned or of the given value type
    /// @params size: size along first dimension
    /// @returns array object as thin wrapper
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, uint64_t one_d) {assert(false);};

    /// @brief reshape array along all dimensions
    /// @param array: array to reshape
    /// @param size_per_dimension: size along each dimension, number of dimension is denoted by the number of sizes
    template<typename... Dims>
    void resize_array(unsafe::Array* array, Dims...) {assert(false);};

    /// @brief reshape array to 1d
    /// @param array: array to reshape
    /// @param one_d: size along first dimension
    /// @note optimal performance is only achieved if the array is already 1d
    void resize_array(unsafe::Array* array, uint64_t one_d) {assert(false);};

    /// @brief reshape array to 2d
    /// @param array: array to reshape
    /// @param one_d: size along first dimension
    /// @param two_d: size along second dimension
    /// @note optimal performance is only achieved if the array is already 2d
    void resize_array(unsafe::Array* array, uint64_t one_d, uint64_t two_d) {assert(false);};

    /// @brief access element
    /// @param array: array to access
    /// @params index_per_dimension: index in each dimension
    /// @returns pointer to element, or nullptr if out of bounds
    template<typename... Index>
    unsafe::Value* get_index(unsafe::Array*, Index... index_per_dimension) {assert(false);};

    /// @brief access element, linear indexing
    /// @param array: array to access
    /// @params index: index along the first dimension
    /// @returns pointer to element, or nullptr if out of bounds
    unsafe::Value* get_index(unsafe::Array*, uint64_t) {assert(false);};

    /// @brief access element, linear indexing
    /// @param array: array to access
    /// @params i: index along the first dimension
    /// @params j: index along the second dimension
    /// @returns pointer to element, or nullptr if out of bounds
    unsafe::Value* get_index(unsafe::Array*, uint64_t, uint64_t) {assert(false);};

    /// @brief modify element by index
    /// @param array: array to modify
    /// @param new_value: new value assigned to the element
    /// @params index_per_dimension: index in each dimension
    template<typename... Index>
    void set_index(unsafe::Array*, unsafe::Value* value, Index... index_per_dimension) {assert(false);};

    /// @brief modify element by index
    /// @param array: array to modify
    /// @param value: new value assigned to the element
    /// @params index: index along the first dimension
    void set_index(unsafe::Array*, unsafe::Value* value, uint64_t) {assert(false);};

    /// @brief modify element by index
    /// @param array: array to modify
    /// @param value: new value assigned to the element
    /// @params i: index along the first dimension
    /// @params j: index along the second dimension
    void set_index(unsafe::Array*, unsafe::Value* value, uint64_t, uint64_t) {assert(false);};
}