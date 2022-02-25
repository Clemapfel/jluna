// 
// Copyright 2022 Clemens Cords
// Created on 22.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>


#include <include/type.hpp>
#include <include/proxy.hpp>

namespace jluna
{
    /// @brief customizable wrapper for non-julia type T
    template<typename T>
    class UserType
    {
        public:
            /// @brief ctor
            /// @param julia_side_name: exact name of the resulting julia type
            UserType(const std::string& julia_side_name);

            /// @brief add field
            template<typename Value_t, std::enable_if_t<not std::is_same_v<Value_t, Any*>, Bool> = true>
            void add_field(const std::string& name, Type type, Value_t initial_value);
            void add_field(const std::string& name, Type type = Any_t, Any* initial_value = jl_undef_initializer());

            /// @brief add const field
            template<typename Value_t, std::enable_if_t<not std::is_same_v<Value_t, Any*>, Bool> = true>
            void add_const_field(const std::string& name, Type type, Value_t initial_value);
            void add_const_field(const std::string& name, Type type = Any_t, Any* initial_value = jl_undef_initializer());

            /// @brief add member function
            template<typename Lambda_t, std::enable_if_t<not std::is_same_v<Lambda_t, Any*>, Bool> = true>
            void add_function(const std::string& name, Lambda_t lambda);
            void add_function(const std::string& name, Any* function);

            /// @brief add parameter
            void add_parameter(const std::string& name, Type upper_bound = Any_t, Type lower_bound = UnionEmpty_t);

            /// @brief push to state and eval, cannot be extended afterwards
            Type implement(Module module = Main);

        private:
            Proxy _template;
            jl_datatype_t* _type = nullptr;
    };
}

#include ".src/usertype.inl"