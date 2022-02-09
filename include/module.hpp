// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>
#include <include/proxy.hpp>
#include <include/symbol.hpp>

namespace jluna
{
    // wraps jl_module_t*
    class Module : public Proxy
    {
        public:
            Module() = default;

            /// @brief ctor
            /// @param value
            /// @param name: symbol
            Module(jl_module_t*);

            /// @brief ctor as owned
            /// @param value
            /// @param owner: internal proxy value owner
            /// @param name: symbol
            Module(jl_value_t* value, std::shared_ptr<ProxyValue>& owner, jl_sym_t* name);

            /// @brief decay to C-type
            explicit operator jl_module_t*();

            /// @brief eval string in module scope without exception forwarding
            /// @param code
            /// @returns result of expression
            Proxy eval(const std::string&);

            /// @brief eval string in module scope with exception forwarding
            /// @param code
            /// @returns result of expression
            Proxy safe_eval(const std::string&);

            /// @brief assign variable with given name in module, if variable does not exist, throw UndefVarError
            /// @param name: variable name, should not contain "."
            /// @param value
            /// @returns jluna::Proxy to value after assignment
            template<Boxable T>
            Proxy assign(const std::string& variable_name, T value);

            /// @brief assign variable with given name in module, if variable does not exist, create it
            /// @param name: variable name, should not contain "."
            /// @param value
            /// @returns jluna::Proxy to value after assignment
            template<Boxable T>
            Proxy create_or_assign(const std::string& variable_name, T value);

            /// @brief wrap c-property name
            /// @returns name as symbol
            jl_sym_t* get_symbol() const;

            /// @brief wrap c-property parent
            /// @returns parent as module, nullptr if topmod
            Module get_parent_module() const;

            /// @brief wrap hidden c-property build_id
            /// @returns size_t
            size_t get_build_id() const;

            /// @brief wrap hidden c-property uuid
            /// @returns uuid where .first = hi; .second = lo;
            std::pair<size_t, size_t> get_uuid() const;

            /// @brief wrap hidden c-property istopmod
            /// @returns bool
            bool is_top_module() const;

            /// @brief wrap hidden c-property optlevel
            /// @returns level as int
            int8_t get_optimization_level() const;

            /// @brief wrap hidden c-property compile
            /// @returns status : -1, false, true
            int8_t get_compile_status() const;

            /// @brief wrap hidden c-property infer, is type inference enabled
            /// @returns status: -1, false, true
            int8_t get_type_inference_status() const;

            /// @brief wrap hidden c-property bindings
            /// @returns hashtable where keys are symbols and values are the memory symbols are bound to
            [[nodiscard]] std::map<Symbol, Any*> get_bindings() const;

            /// @brief wrap hidden c-property usings
            /// @returns list of modules declared
            [[nodiscard]] std::vector<Module> get_usings() const;

            /// @brief is variable defined
            /// @param name: exact variable name
            /// @returns Base.isdefined(this, :name)
            bool is_defined(const std::string& variable_name) const;

        private:
            jl_module_t* get() const;
    };
    /// @brief Proxy of singleton Main, initialized by State::initialize
    inline Module Main;

    /// @brief Proxy of singleton Main.Base, initialized by State::initialize
    inline Module Base;

    /// @brief Proxy of singleton Main.Base.Core, initialized by State::initialize
    inline Module Core;

    /// @brief unbox to module
    template<Is<Module> T>
    inline T unbox(Any* value)
    {
        jl_assert_type(value, "Module");
        return Module((jl_module_t*) value);
    }

    /// @brief box jluna::Module to Base.Module
    template<Is<Module> T>
    inline Any* box(T value)
    {
        return value.operator Any*();
    }

    /// @brief type deduction
    template<>
    struct detail::to_julia_type_aux<Module>
    {
        static inline const std::string type_name = "Module";
    };
}

#include ".src/module.inl"