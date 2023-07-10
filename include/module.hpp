// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/julia_wrapper.hpp>
#include <include/proxy.hpp>
#include <include/array.hpp>
#include <include/symbol.hpp>
#include <include/mutex.hpp>

namespace jluna
{
    // wraps jl_module_t*
    class Module : public Proxy
    {
        public:
            Module() = default;

            /// @brief construct from Julia module, implicit
            /// @param value: pointer to Julia module
            /// @param name: symbol
            Module(jl_module_t*);

            /// @brief construct as child of proxy
            /// @param proxy: proxy
            Module(Proxy*);

            /// @brief destructor
            ~Module();

            /// @brief decay to C-type, implicit
            operator jl_module_t*();

            /// @brief eval string in module scope with exception forwarding
            /// @param code: julia code as string
            /// @returns result of expression as unnamed proxy
            Proxy safe_eval(const std::string&);

            /// @brief eval file in module scope, with exception forwarding
            /// @param path: absolute path to .jl file
            /// @returns result of executing the file
            Proxy safe_eval_file(const std::string&);

            /// @brief [thread-safe] assign variable with given name in module, if variable does not exist, throw UndefVarError
            /// @param name: variable name, should not contain "."
            /// @param value: new value
            /// @returns jluna::Proxy to value after assignment
            template<is_boxable T>
            void assign(const std::string& variable_name, T value);

            /// @brief [thread-safe] assign variable with given name in module, if variable does not exist, create it
            /// @param name: variable name, should not contain "."
            /// @param value: new value
            /// @returns jluna::Proxy to value after assignment
            template<is_boxable T>
            void create_or_assign(const std::string& variable_name, T value);

            /// @brief get variable value as named proxy
            /// @param variable_name: name of variable
            /// @returns proxy
            Proxy get(const std::string& variable_name);

            /// @brief get variable value
            /// @tparam value: type
            /// @param variable_name: name of variable
            /// @returns value
            template<is_unboxable T>
            T get(const std::string& variable_name);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @returns named proxy to newly created value
            [[nodiscard]] Proxy new_undef(const std::string& name);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: bool value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_bool(const std::string& name, bool value = false);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: uint32 unicode code, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_char(const std::string& name, char value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: uint8 value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_uint8(const std::string& name, uint8_t value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: uint16 value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_uint16(const std::string& name, uint16_t value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: uint32 value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_uint32(const std::string& name, uint32_t value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: uint64 value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_uint64(const std::string& name, uint64_t value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: int8 value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_int8(const std::string& name, int8_t value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: int16 value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_int16(const std::string& name, int16_t value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: int32 value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_int32(const std::string& name, int32_t value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: int64 value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_int64(const std::string& name, int64_t value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: float value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_float32(const std::string& name, float value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: double value, default 0
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_float64(const std::string& name, double value = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: string
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_string(const std::string& name, const std::string& value = "");

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param variable_name: exact name of variable
            /// @param value: string
            /// @returns *named* proxy to newly created value
            [[nodiscard]] Proxy new_symbol(const std::string& name, const std::string& value = "");

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @tparam value type T of Core.Complex{T}
            /// @param variable_name: exact name of variable
            /// @param real: real part of complex number
            /// @param imag: imaginary part of complex number
            /// @returns *named* proxy to newly created value
            template<is_primitive T>
            [[nodiscard]] Proxy new_complex(const std::string& name, T real = 0, T imag = 0);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @tparam value type T of Core.Vector{T}
            /// @param variable_name: exact name of variable
            /// @param vector: vector
            /// @returns *named* proxy to newly created value
            template<is_boxable T>
            [[nodiscard]] Proxy new_vector(const std::string& name, const std::vector<T>& = {});

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @tparam Key_t: key type in Base.IdDict{Key_t, Value_t}
            /// @tparam Value_t: value type in Base.IdDict{Key_t, Value_t}
            /// @param variable_name: exact name of variable
            /// @param map: map
            /// @returns *named* proxy to newly created value
            template<is_boxable Key_t, is_boxable Value_t>
            [[nodiscard]] Proxy new_dict(const std::string& name, const std::map<Key_t, Value_t>& = {});

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @tparam Key_t: key type in Base.Dict{Key_t, Value_t}
            /// @tparam Value_t: value type in Base.Dict{Key_t, Value_t}
            /// @param variable_name: exact name of variable
            /// @param unordered_map: unordered map
            /// @returns *named* proxy to newly created value
            template<is_boxable Key_t, is_boxable Value_t>
            [[nodiscard]] Proxy new_dict(const std::string& name, const std::unordered_map<Key_t, Value_t>& = {});

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @tparam value type of Base.Set{T}
            /// @param variable_name: exact name of variable
            /// @param set: set
            /// @returns *named* proxy to newly created value
            template<is_boxable T>
            [[nodiscard]] Proxy new_set(const std::string& name, const std::set<T>& value = {});

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @tparam T1: first value type
            /// @tparam T2: second value type
            /// @param variable_name: exact name of variable
            /// @param first: first
            /// @param second: second
            /// @returns *named* proxy to newly created value
            template<is_boxable T1, is_boxable T2>
            [[nodiscard]] Proxy new_pair(const std::string& name, T1 first, T2 second);

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param Ts...: value types
            /// @param variable_name: exact name of variable
            /// @param values: tuple values
            /// @returns *named* proxy to newly created value
            template<is_boxable... Ts>
            [[nodiscard]] Proxy new_tuple(const std::string& name, Ts...);

            /// @brief get name
            /// @returns name of module
            Symbol get_symbol() const;

            /// @brief [thread-safe] creates new variable in main, then returns named proxy to it
            /// @param T: value types
            /// @param variable_name: exact name of variable
            /// @param dims: length in each dimension
            /// @returns *named* proxy to newly created array, filled with undef
            template<is_boxable T, uint64_t N, is<uint64_t>... Dims>
            [[nodiscard]] Array<T, N> new_array(const std::string& name, Dims... dims);

            /// @brief wrap c-property parent
            /// @returns parent as module, nullptr if topmod
            Module get_parent_module() const;

            /// @brief wrap hidden c-property istopmod
            /// @returns bool
            bool is_top_module() const;

            /// @brief is variable defined
            /// @param name: exact variable name
            /// @returns Base.isdefined(this, :name)
            bool is_defined(const std::string& variable_name) const;

            /// @brief import a package
            /// @param package_name
            void import(const std::string& package_name);

            /// @brief import names
            /// @param package_name
            void add_using(const std::string& package_name);

            /// @brief include file in module scope
            /// @param package_name
            void include(const std::string& file_path);

            /// @brief get binding
            /// @returns vector of pairs where .first is the variable name, .second is the value
            // auto bindings() -> std::vector<std::pair<unsafe::Symbol*, unsafe::Value*>>;

        private:
            jl_module_t* value() const;

            void initialize_lock();
            Mutex* _lock = nullptr;
    };

    /// @brief Proxy of singleton Main, initialized by State::initialize
    inline Module Main;

    /// @brief Proxy of singleton Main.Base, initialized by State::initialize
    inline Module Base;

    /// @brief Proxy of singleton Main.Base.Core, initialized by State::initialize
    inline Module Core;
}

#include <.src/module.inl>