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
            /// @brief ctor
            /// @param value
            /// @param name: symbol
            Module(jl_module_t*);

            /// @brief ctor as owned
            /// @param value
            /// @param owner: internal proxy value owner
            /// @param name: symbol
            Module(jl_module_t* value, std::shared_ptr<ProxyValue>& owner);

            /// @brief decay to C-type
            operator jl_module_t*();

            /// @brief wrap c-property name
            /// @returns name as symbol
            jl_sym_t* name() const;

            /// @brief wrap c-property parent
            /// @returns parent as module, nullptr if topmod
            Module parent() const;

            /// @brief wrap hidden c-property build_id
            /// @returns size_t
            size_t build_id() const;

            /// @brief wrap hidden c-property uuid
            /// @returns uuid where .first = hi; .second = lo;
            std::pair<size_t, size_t> get_uuid() const;

            /// @brief wrap hidden c-property istopmod
            /// @returns bool
            bool is_top_module() const;

            /// @brief wrap hidden c-property optlevel
            /// @returns level as int
            int8_t optimization_level() const;

            /// @brief wrap hidden c-property compile
            /// @returns status : -1, 0, 1
            int8_t compile_status() const;

            /// @brief wrap hidden c-property bindings
            /// @returns hashtable where keys are symbols and values are the memory symbols are bound to
            std::map<Symbol, Any*> bindings() const;

            /// @brief wrap hidden c-property usings
            /// @returns list of modules declared
            std::vector<Module> usings() const;

        private:
            jl_module_t* get() const;





        private:
            jl_module_t* any;
    };
}