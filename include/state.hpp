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

namespace jluna
{
    // forward declarations
    class Proxy;
    class Type;
    class Symbol;
    template<Boxable, size_t>
    class Array;
}

namespace jluna::State
{
    /// @brief manually set the C-adapter path
    void set_c_adapter_path(const std::string& path);

    /// @brief initialize environment
    void initialize();

    /// @brief initialize environment from image
    /// @param absolute path to julia image
    void initialize(const std::string&);

    /// @brief execute line of code, evaluated in Main
    /// @param command
    /// @returns proxy to result, if any
    /// @exceptions if an error occurs julia-side, it will be ignored and the result of the call will be undefined
    Proxy eval(const std::string&) noexcept;

    /// @brief execute line of code with exception handling
    /// @param command
    /// @returns proxy to result, if any
    /// @exceptions if an error occurs julia-side, a JuliaException will be thrown
    Proxy safe_eval(const std::string&);

    /// @brief execute file
    /// @param path to file
    /// @returns proxy to result, if any
    Proxy eval_file(const std::string& path) noexcept;

    /// @brief execute file
    /// @param path to file
    /// @returns proxy to result, if any
    Proxy safe_eval_file(const std::string& path) noexcept;

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
    [[nodiscard]] Proxy new_named_undef(const std::string& name);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: bool value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_bool(const std::string& name, bool value = false);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint32 unicode code, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_char(const std::string& name, uint32_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint8 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_uint8(const std::string& name, uint8_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint16 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_uint16(const std::string& name, uint16_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint32 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_uint32(const std::string& name, uint32_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint64 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_uint64(const std::string& name, uint64_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int8 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_int8(const std::string& name, int8_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int16 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_int16(const std::string& name, int16_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int32 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_int32(const std::string& name, int32_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int64 value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_int64(const std::string& name, int64_t value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: float value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_float32(const std::string& name, float value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: double value, default 0
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_float64(const std::string& name, double value = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: string
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_string(const std::string& name, const std::string& value = "");

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: string
    /// @returns *named* proxy to newly created value
    [[nodiscard]] Proxy new_named_symbol(const std::string& name, const std::string& value = "");

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam value type T of Core.Complex{T}
    /// @param variable_name: exact name of variable
    /// @param real
    /// @param imaginary
    /// @returns *named* proxy to newly created value
    template<IsPrimitive T>
    [[nodiscard]] Proxy new_named_complex(const std::string& name, T real = 0, T imag = 0);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam value type T of Core.Vector{T}
    /// @param variable_name: exact name of variable
    /// @param vector
    /// @returns *named* proxy to newly created value
    template<Boxable T>
    [[nodiscard]] Proxy new_named_vector(const std::string& name, const std::vector<T>& = {});

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam Key_t: key type in Base.IdDict{Key_t, Value_t}
    /// @tparam Value_t: value type in Base.IdDict{Key_t, Value_t}
    /// @param variable_name: exact name of variable
    /// @param map
    /// @returns *named* proxy to newly created value
    template<Boxable Key_t, Boxable Value_t>
    [[nodiscard]] Proxy new_named_dict(const std::string& name, const std::map<Key_t, Value_t>& = {});

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam Key_t: key type in Base.Dict{Key_t, Value_t}
    /// @tparam Value_t: value type in Base.Dict{Key_t, Value_t}
    /// @param variable_name: exact name of variable
    /// @param unordered_map
    /// @returns *named* proxy to newly created value
    template<Boxable Key_t, Boxable Value_t>
    [[nodiscard]] Proxy new_named_dict(const std::string& name, const std::unordered_map<Key_t, Value_t>& = {});

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam value type of Base.Set{T}
    /// @param variable_name: exact name of variable
    /// @param set
    /// @returns *named* proxy to newly created value
    template<Boxable T>
    [[nodiscard]] Proxy new_named_set(const std::string& name, const std::set<T>& value = {});

    /// @brief creates new variable in main, then returns named proxy to it
    /// @tparam T1: first value type
    /// @tparam T2: second value type
    /// @param variable_name: exact name of variable
    /// @param first
    /// @param second
    /// @returns *named* proxy to newly created value
    template<Boxable T1, Boxable T2>
    [[nodiscard]] Proxy new_named_pair(const std::string& name, T1 first, T2 second);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param Ts...: value types
    /// @param variable_name: exact name of variable
    /// @param values
    /// @returns *named* proxy to newly created value
    template<Boxable... Ts>
    [[nodiscard]] Proxy new_named_tuple(const std::string& name, Ts...);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param T: value types
    /// @param variable_name: exact name of variable
    /// @param dims: length in each dimension
    /// @returns *named* proxy to newly created array, filled with undef
    template<Boxable T, size_t N, Is<size_t>... Dims>
    [[nodiscard]] Array<T, N> new_named_array(const std::string& name, Dims... dims);

    /// @brief creates new variable in main, then returns named proxy to it
    /// @param variable_name: exact name of variable
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_undef();

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: bool value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_bool(bool value = false);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint32 unicode code, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_char(uint32_t value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint8 value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_uint8(uint8_t value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint16 value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_uint16(uint16_t value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint32 value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_uint32(uint32_t value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: uint64 value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_uint64(uint64_t value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int8 value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_int8(int8_t value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int16 value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_int16(int16_t value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int32 value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_int32(int32_t value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: int64 value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_int64(int64_t value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: float value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_float32(float value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: double value, default 0
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_float64(double value = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: string
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_string(const std::string& value = "");

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param variable_name: exact name of variable
    /// @param value: string
    /// @returns *unnamed* proxy to newly created value
    [[nodiscard]] Proxy new_unnamed_symbol(const std::string& value = "");

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @tparam value type T of Core.Complex{T}
    /// @param variable_name: exact name of variable
    /// @param real
    /// @param imaginary
    /// @returns *unnamed* proxy to newly created value
    template<IsPrimitive T>
    [[nodiscard]] Proxy new_unnamed_complex(T real = 0, T imag = 0);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @tparam value type T of Core.Vector{T}
    /// @param variable_name: exact name of variable
    /// @param vector
    /// @returns *unnamed* proxy to newly created value
    template<Boxable T>
    [[nodiscard]] Proxy new_unnamed_vector(const std::vector<T>& = {});

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @tparam Key_t: key type in Base.IdDict{Key_t, Value_t}
    /// @tparam Value_t: value type in Base.IdDict{Key_t, Value_t}
    /// @param variable_name: exact name of variable
    /// @param map
    /// @returns *unnamed* proxy to newly created value
    template<Boxable Key_t, Boxable Value_t>
    [[nodiscard]] Proxy new_unnamed_iddict(const std::map<Key_t, Value_t>& = {});

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @tparam Key_t: key type in Base.Dict{Key_t, Value_t}
    /// @tparam Value_t: value type in Base.Dict{Key_t, Value_t}
    /// @param variable_name: exact name of variable
    /// @param unordered_map
    /// @returns *unnamed* proxy to newly created value
    template<Boxable Key_t, Boxable Value_t>
    [[nodiscard]] Proxy new_unnamed_dict(const std::unordered_map<Key_t, Value_t>& = {});

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @tparam value type of Base.Set{T}
    /// @param variable_name: exact name of variable
    /// @param set
    /// @returns *unnamed* proxy to newly created value
    template<Boxable T>
    [[nodiscard]] Proxy new_unnamed_set(const std::set<T>& value = {});

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @tparam T1: first value type
    /// @tparam T2: second value type
    /// @param variable_name: exact name of variable
    /// @param first
    /// @param second
    /// @returns *unnamed* proxy to newly created value
    template<Boxable T1, Boxable T2>
    [[nodiscard]] Proxy new_unnamed_pair(T1 first, T2 second);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param Ts...: value types
    /// @param variable_name: exact name of variable
    /// @param values
    /// @returns *unnamed* proxy to newly created value
    template<Boxable... Ts>
    [[nodiscard]] Proxy new_unnamed_tuple(Ts...);

    /// @brief creates new variable in main, then returns unnamed proxy to it
    /// @param T: value types
    /// @param variable_name: exact name of variable
    /// @param dims: length in each dimension
    /// @returns *unnamed* proxy to newly created array, filled with undef
    template<Boxable T, size_t N, Is<size_t>... Dims>
    [[nodiscard]] Array<T, N> new_unnamed_array(Dims... dims);

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

        void initialize_modules();
        void initialize_types();

        template<Is<Any*>... Ts>
        Proxy create_or_assign(const std::string& symbol, Ts... args);
    }

    [[deprecated("use State::eval instead")]] Proxy script(const std::string&) noexcept;
    [[deprecated("use State::safe_eval instead")]] Proxy safe_script(const std::string&);
}

#include <.src/state.inl>