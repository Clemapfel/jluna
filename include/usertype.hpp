// 
// Copyright 2022 Clemens Cords
// Created on 22.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>

namespace jluna
{
    auto* a = (jl_datatype_t*) jl_array_type;


    class Usertype
    {
        public:


        private:
            Any* _expression;
    };
}