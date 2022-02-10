// 
// Copyright 2022 Clemens Cords
// Created on 09.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/proxy.hpp>
#include <include/type.hpp>

namespace jluna
{
    /// @brief object describing the signature of a method
    struct Signature
    {
        /// @brief return type
        const Type return_type;

        /// @brief types of argument tuple, may be empty
        const std::vector<Type> argument_types;

        /// @brief construct signature
        /// @param return_type
        /// @param argument_types
        template<Is<Type>... Types>
        inline Signature(Type return_type, Types... arguments)
            : return_type(return_type), argument_types({arguments...})
        {}
    };

    /// @brief object describing one method of a function
    class Method
    {
        public:
            /// @brief construct from function and method table index
            /// @param function: hosting function
            /// @param
            Method(jl_function_t* function, size_t method_table_index);
            TODO: deepcopy

            Signature get_signature() const;

        private:
            Signature _signature;
    };

    // this feature is not yet implemented
}