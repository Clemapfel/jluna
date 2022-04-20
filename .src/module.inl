// 
// Copyright 2022 Clemens Cords
// Created on 09.02.22 by clem (mail@clemens-cords.com)
//

#include <include/safe_utilities.hpp>

namespace jluna
{
    template<is_boxable T>
    Proxy Module::assign(const std::string& variable_name, T new_value)
    {
        static jl_function_t* assign_in_module = unsafe::get_function("jluna"_sym, "assign_in_module"_sym);

        initialize_lock();
        _lock->lock();
        gc_pause;
        jluna::safe_call(assign_in_module, value(), jl_symbol(variable_name.c_str()), box(new_value));
        auto out = this->operator[](variable_name);
        gc_unpause;
        _lock->unlock();
        return out;
    }

    template<is_boxable T>
    Proxy Module::create_or_assign(const std::string& variable_name, T new_value)
    {
        static jl_function_t* assign_in_module = unsafe::get_function("jluna"_sym, "create_or_assign_in_module"_sym);

        initialize_lock();
        _lock->lock();
        gc_pause;
        jluna::safe_call(assign_in_module, this->value(), jl_symbol(variable_name.c_str()), box<T>(new_value));
        auto out = this->operator[](variable_name);
        gc_unpause;
        _lock->unlock();
        return out;
    }
    
    inline Proxy Module::new_undef(const std::string& name)
    {
        static auto* undef_t = (unsafe::DataType*) jl_eval_string("return UndefInitializer");

        initialize_lock();
        _lock->lock();
        auto out = create_or_assign(name, jl_new_struct(undef_t));
        _lock->unlock();
        return out;
    }

    inline Proxy Module::new_bool(const std::string& name, bool value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_char(const std::string& name, char value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_uint8(const std::string& name, uint8_t value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_uint16(const std::string& name, uint16_t value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_uint32(const std::string& name, uint32_t value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_uint64(const std::string& name, uint64_t value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_int8(const std::string& name, int8_t value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_int16(const std::string& name, int16_t value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_int32(const std::string& name, int32_t value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_int64(const std::string& name, int64_t value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_float32(const std::string& name, float value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_float64(const std::string& name, double value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_string(const std::string& name, const std::string& value)
    {
        return create_or_assign(name, value);
    }

    inline Proxy Module::new_symbol(const std::string& name, const std::string& value)
    {
        return create_or_assign(name, (unsafe::Value*) jl_symbol(value.c_str()));
    }

    template<is_primitive T>
    Proxy Module::new_complex(const std::string& name, T real, T imag)
    {
        return create_or_assign(name, box<std::complex<T>>(std::complex<T>(real, imag)));
    }

    template<is_boxable T>
    Proxy Module::new_vector(const std::string& name, const std::vector<T>& value)
    {
        return create_or_assign(name, box<std::vector<T>>(value));
    }

    template<is_boxable Key_t, is_boxable Value_t>
    Proxy Module::new_dict(const std::string& name, const std::map<Key_t, Value_t>& value)
    {
        return create_or_assign(name, box<std::map<Key_t, Value_t>>(value));
    }

    template<is_boxable Key_t, is_boxable Value_t>
    Proxy Module::new_dict(const std::string& name, const std::unordered_map<Key_t, Value_t>& value)
    {
        return create_or_assign(name, box<std::unordered_map<Key_t, Value_t>>(value));
    }

    template<is_boxable T>
    Proxy Module::new_set(const std::string& name, const std::set<T>& value)
    {
        return create_or_assign(name, box<std::set<T>>(value));
    }

    template<is_boxable T1, is_boxable T2>
    Proxy Module::new_pair(const std::string& name, T1 first, T2 second)
    {
        return create_or_assign(name, box<std::pair<T1, T2>>(std::pair<T1, T2>(first, second)));
    }

    template<is_boxable... Ts>
    Proxy Module::new_tuple(const std::string& name, Ts... args)
    {
        return create_or_assign(name, box<std::tuple<Ts...>>(std::make_tuple(args...)));
    }

    template<is_boxable T, size_t N, is<size_t>... Dims>
    Array<T, N> Module::new_array(const std::string& name, Dims... dims)
    {
        static_assert(sizeof...(Dims) == N, "wrong number of dimension initializers");

        std::stringstream str;
        str << name << " = " << as_julia_type<Array<T, N>>::type_name << "(undef,";

        auto add = [&](size_t dim, size_t i){
            str << dim << (i != sizeof...(Dims) ? ", " : ")");
        };

        {
            size_t i = 0;
            (add(dims, ++i), ...);
        }

        initialize_lock();
        _lock->lock();
        jluna::safe_eval(str.str());
        _lock->unlock();
        return Main[name];
    }

    template<is_unboxable T>
    T Module::get(const std::string& variable_name)
    {
        static auto* eval = unsafe::get_function(jl_base_module, "eval"_sym);
        return unbox<T>(jluna::safe_call(eval, jl_new_struct(jl_expr_type, "return"_sym, jl_symbol(variable_name.c_str()))));
    }

    inline Proxy Module::get(const std::string& variable_name)
    {
        static auto* eval = unsafe::get_function(jl_base_module, "eval"_sym);
        return Proxy(jluna::safe_call(eval, jl_new_struct(jl_expr_type, "return"_sym, jl_symbol(variable_name.c_str()))));
    }
}