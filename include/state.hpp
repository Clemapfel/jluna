// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <string>
#include <complex>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>

#include <include/concepts.hpp>
#include <include/typedefs.hpp>
#include <include/box.hpp>

/// forward declarations
namespace jluna
{
    class Proxy;
    template<Boxable, size_t>
    class Array;
}

namespace jluna::State
{
    /// @brief initialize environment
    void initialize();

    /// @brief initialize environment
    /// @param absolute path to julia image
    void initialize(const std::string&);

    /// @brief execute line of code, evaluated in Main
    /// @param command
    /// @returns proxy to result, if any
    /// @exceptions if an error occurs julia-side it will be ignore and the result of the call will be undefined
    Proxy script(const std::string&) noexcept;

    /// @brief execute line of code with exception handling
    /// @param command
    /// @returns proxy to result, if any
    /// @exceptions if an error occurs julia-side a JuliaException will be thrown
    Proxy safe_script(const std::string&);

    /// @brief access a value, equivalent to unbox<T>(jl_eval_string("return " + name))
    /// @tparam T: type to be unboxed to
    /// @param full name of the value, e.g. Main.variable._field[0]
    /// @returns T
    /// @exceptions if an error occurs julia-side, a JuliaException will be thrown
    template<typename T>
    T safe_return(const std::string& full_name);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_undef(const std::string& name);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: bool value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_bool(const std::string& name, bool value = false);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint32 unicode code, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_char(const std::string& name, uint32_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint8 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_uint8(const std::string& name, uint8_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint16 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_uint16(const std::string& name, uint16_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint32 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_uint32(const std::string& name, uint32_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint64 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_uint64(const std::string& name, uint64_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int8 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_int8(const std::string& name, int8_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int16 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_int16(const std::string& name, int16_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int32 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_int32(const std::string& name, int32_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int64 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_int64(const std::string& name, int64_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: float value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_float32(const std::string& name, float value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: double value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_float64(const std::string& name, double value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: string
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_string(const std::string& name, const std::string& value = "");

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: string
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_symbol(const std::string& name, const std::string& value = "");

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam value type T of Core.Complex{T}
    /// @param variable_name: exact name of variable
    /// @param real
    /// @param imaginary
    /// @returns *named* proxy to newly created value
    template<IsNumerical T>
    [[nodiscard]] Proxy new_complex(const std::string& name, T real = 0, T imag = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam value type T of Core.Vector{T}
    /// @param variable_name: exact name of variable
    /// @param vector
    /// @returns *named* proxy to newly created value
    template<Boxable T>
    [[nodiscard]] Proxy new_vector(const std::string& name, const std::vector<T>& = {});

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam Key_t: key type in Base.IdDict{Key_t, Value_t}
    /// @tparam Value_t: value type in Base.IdDict{Key_t, Value_t}
    /// @param variable_name: exact name of variable
    /// @param map
    /// @returns *named* proxy to newly created value
    template<Boxable Key_t, Boxable Value_t>
    [[nodiscard]] Proxy new_iddict(const std::string& name, const std::map<Key_t, Value_t>& = {});

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam Key_t: key type in Base.Dict{Key_t, Value_t}
    /// @tparam Value_t: value type in Base.Dict{Key_t, Value_t}
    /// @param variable_name: exact name of variable
    /// @param unordered_map
    /// @returns *named* proxy to newly created value
    template<Boxable Key_t, Boxable Value_t>
    [[nodiscard]] Proxy new_dict(const std::string& name, const std::unordered_map<Key_t, Value_t>& = {});

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam value type of Base.Set{T}
    /// @param variable_name: exact name of variable
    /// @param set
    /// @returns *named* proxy to newly created value
    template<Boxable T>
    [[nodiscard]] Proxy new_set(const std::string& name, const std::set<T>& value = {});

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam T1: first value type
    /// @tparam T2: second value type
    /// @param variable_name: exact name of variable
    /// @param first
    /// @param second
    /// @returns *named* proxy to newly created value
    template<Boxable T1, Boxable T2>
    [[nodiscard]] Proxy new_pair(const std::string& name, T1 first, T2 second);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param Ts...: value types
    /// @param variable_name: exact name of variable
    /// @param values
    /// @returns *named* proxy to newly created value
    template<Boxable... Ts>
    [[nodiscard]] Proxy new_tuple(const std::string& name, Ts...);

    template<Boxable T, size_t N, Is<size_t>... Dims>
    [[nodiscard]] Array<T, N> new_array(const std::string& name, Dims... dims);

    /// @brief trigger the garbage collector
    void collect_garbage();

    /// @brief activate/deactivate garbage collector
    void set_garbage_collector_enabled(bool);

    namespace detail
    {
        constexpr char _id_marker = '#';

        size_t create_reference(Any* in);
        Any* get_reference(size_t key);
        void free_reference(size_t key);
    }
}

#include ".src/state.inl"