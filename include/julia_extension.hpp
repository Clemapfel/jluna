// 
// Copyright 2022 Clemens Cords
// Created on 30.01.22 by clem (mail@clemens-cords.com)
//

#include <julia/julia.h>

extern "C"
{
    /// @brief convert any to string julia-side
    /// @param value
    /// @returns c-string
    const char* jl_to_string(jl_value_t* value)
    {
        static jl_function_t* tostring = jl_get_function(jl_base_module, "string");
        return jl_string_data(jl_call1(tostring, value));
    }

}