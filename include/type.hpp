// 
// Copyright 2022 Clemens Cords
// Created on 07.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>
#include <include/proxy.hpp>

namespace jluna
{
    class Type : public Proxy
    {
        public:
            /// @brief ctor
            /// @param value
            Type(jl_datatype_t* value);

            /// @brief get direct super type
            Type get_super() const;



    };
}