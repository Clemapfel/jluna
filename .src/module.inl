// 
// Copyright 2022 Clemens Cords
// Created on 09.02.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    template<Boxable T>
    Proxy Module::assign(const std::string& variable_name, T value)
    {
        static jl_function_t* assign_in_module = jl_find_function("jluna", "assign_in_module");
        jluna::safe_call(assign_in_module, get(), jl_symbol(variable_name.c_str()), box<T>(value));
        return this->operator[](variable_name);
    }

    template<Boxable T>
    Proxy Module::create_or_assign(const std::string& variable_name, T value)
    {
        static jl_function_t* assign_in_module = jl_find_function("jluna", "create_or_assign_in_module");
        jluna::safe_call(assign_in_module, get(), jl_symbol(variable_name.c_str()), box<T>(value));
        return this->operator[](variable_name);
    }
}