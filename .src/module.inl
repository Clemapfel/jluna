// 
// Copyright 2022 Clemens Cords
// Created on 09.02.22 by clem (mail@clemens-cords.com)
//

#include <include/safe_utilities.hpp>

namespace jluna
{
    /// @brief unbox to module
    template<is<Module> T>
    inline T unbox(unsafe::Value* value)
    {
        detail::assert_type((unsafe::DataType*) jl_typeof(value), jl_module_type);
        return Module((jl_module_t*) value);
    }

    /// @brief box jluna::Module to Base.Module
    template<is<Module> T>
    inline unsafe::Value* box(T value)
    {
        return value.operator unsafe::Value*();
    }

    /// @brief type deduction
    template<>
    struct detail::as_julia_type_aux<Module>
    {
        static inline const std::string type_name = "Module";
    };

    template<is_boxable T>
    void Module::assign(const std::string& variable_name, T new_value)
    {
        static jl_function_t* assign_in_module = unsafe::get_function("jluna"_sym, "assign_in_module"_sym);

        if (detail::_num_threads != 1)
        {
            initialize_lock();
            _lock->lock();
        }

        unsafe::Module* me = value();
        auto* sym = jl_symbol(variable_name.c_str());

        if (jl_defines_or_exports_p(me, sym))
            jl_set_global(value(), jl_symbol(variable_name.c_str()), box<T>(new_value));
        else
        {
            JL_TRY
                jl_undefined_var_error(sym);
            JL_CATCH
                throw JuliaException((unsafe::Value*) jl_exception_occurred(), "in jluna::Module::assign: UndefVarError: " + variable_name + " not defined");
        }

        if (detail::_num_threads != 1)
            _lock->unlock();
    }

    template<is_boxable T>
    void Module::create_or_assign(const std::string& variable_name, T new_value)
    {
        static jl_function_t* assign_in_module = unsafe::get_function("jluna"_sym, "create_or_assign_in_module"_sym);

        if (detail::_num_threads != 1)
        {
            initialize_lock();
            _lock->lock();
        }
        gc_pause;
        jl_set_global(value(), jl_symbol(variable_name.c_str()), box<T>(new_value));
        gc_unpause;

        if (detail::_num_threads != 1)
            _lock->unlock();
    }

    inline Proxy Module::new_undef(const std::string& name)
    {
        static auto* undef_t = (unsafe::DataType*) jl_eval_string("return UndefInitializer");
        create_or_assign(name, jl_new_struct(undef_t));

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_bool(const std::string& name, bool v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_char(const std::string& name, char v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_uint8(const std::string& name, uint8_t v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_uint16(const std::string& name, uint16_t v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_uint32(const std::string& name, uint32_t v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_uint64(const std::string& name, uint64_t v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_int8(const std::string& name, int8_t v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_int16(const std::string& name, int16_t v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_int32(const std::string& name, int32_t v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_int64(const std::string& name, int64_t v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_float32(const std::string& name, float v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_float64(const std::string& name, double v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_string(const std::string& name, const std::string& v)
    {
        create_or_assign(name, v);

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    inline Proxy Module::new_symbol(const std::string& name, const std::string& v)
    {
        create_or_assign(name, (unsafe::Value*) jl_symbol(v.c_str()));

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    template<is_primitive T>
    Proxy Module::new_complex(const std::string& name, T real, T imag)
    {
        create_or_assign(name, box<std::complex<T>>(std::complex<T>(real, imag)));

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    template<is_boxable T>
    Proxy Module::new_vector(const std::string& name, const std::vector<T>& v)
    {
        create_or_assign(name, box<std::vector<T>>(v));

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    template<is_boxable Key_t, is_boxable Value_t>
    Proxy Module::new_dict(const std::string& name, const std::map<Key_t, Value_t>& v)
    {
        create_or_assign(name, box<std::map<Key_t, Value_t>>(v));

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    template<is_boxable Key_t, is_boxable Value_t>
    Proxy Module::new_dict(const std::string& name, const std::unordered_map<Key_t, Value_t>& v)
    {
        create_or_assign(name, box<std::unordered_map<Key_t, Value_t>>(v));

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    template<is_boxable T>
    Proxy Module::new_set(const std::string& name, const std::set<T>& v)
    {
        create_or_assign(name, box<std::set<T>>(v));

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    template<is_boxable T1, is_boxable T2>
    Proxy Module::new_pair(const std::string& name, T1 first, T2 second)
    {
        create_or_assign(name, box<std::pair<T1, T2>>(std::pair<T1, T2>(first, second)));

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    template<is_boxable... Ts>
    Proxy Module::new_tuple(const std::string& name, Ts... args)
    {
        create_or_assign(name, box<std::tuple<Ts...>>(std::make_tuple(args...)));

        auto* sym = jl_symbol(name.c_str());
        return Proxy(jl_get_global(value(), sym), sym);
    }

    template<is_boxable T, uint64_t N, is<uint64_t>... Dims>
    Array<T, N> Module::new_array(const std::string& name, Dims... dims)
    {
        static_assert(sizeof...(Dims) == N, "wrong number of dimension initializers");

        std::stringstream str;
        str << name << " = " << as_julia_type<Array<T, N>>::type_name << "(undef,";

        auto add = [&](uint64_t dim, uint64_t i){
            str << dim << (i != sizeof...(Dims) ? ", " : ")");
        };

        {
            uint64_t i = 0;
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
        auto* sym = jl_symbol(variable_name.c_str());
        auto* me = value();

        if (jl_defines_or_exports_p(me, sym))
            return unbox<T>(jl_get_binding(me, sym)->value);
        else
        {
            JL_TRY
                jl_undefined_var_error(sym);
            JL_CATCH
                throw JuliaException((unsafe::Value*) jl_exception_occurred(), "in jluna::Module::assign: UndefVarError: " + variable_name + " not defined");

            return jl_nothing;
        }
    }

    inline Proxy Module::get(const std::string& variable_name)
    {
        return Proxy(get<unsafe::Value*>(variable_name), jl_symbol(variable_name.c_str()));
    }

    namespace detail
    {
        inline void initialize_modules()
        {
            Main = Module(jl_main_module);
            Core = Module(jl_core_module);
            Base = Module(jl_base_module);
        }
    }
}