// 
// Copyright 2022 Clemens Cords
// Created on 02.02.22 by clem (mail@clemens-cords.com)
//

#include <include/utilities.hpp>
#include <include/julia_extension.hpp>
#include <include/proxy.hpp>
#include <include/array.hpp>

namespace jluna::State
{
    inline Proxy new_named_undef(const std::string& name)
    {
        State::safe_script("Main.eval(:(" + name + " = undef))");
        return Main[name];
    }

    inline Proxy new_named_bool(const std::string& name, bool value)
    {
        jluna::detail::create_or_assign(name, jl_box_bool(value));
    }
    
    inline Proxy new_named_char(const std::string& name, char value)
    {
        return jluna::detail::create_or_assign(name, box<char>(value));
    }
    
    inline Proxy new_named_uint8(const std::string& name, uint8_t value)
    {
        return jluna::detail::create_or_assign(name, box<uint8_t>(value));
    }
    
    inline Proxy new_named_uint16(const std::string& name, uint16_t value)
    {
        return jluna::detail::create_or_assign(name, box<uint16_t>(value));
    }
    
    inline Proxy new_named_uint32(const std::string& name, uint32_t value)
    {
        return jluna::detail::create_or_assign(name, box<uint32_t>(value));
    }
    
    inline Proxy new_named_uint64(const std::string& name, uint64_t value)
    {
        return jluna::detail::create_or_assign(name, box<uint64_t>(value));
    }
    
    inline Proxy new_named_int8(const std::string& name, int8_t value)
    {
        return jluna::detail::create_or_assign(name, box<int8_t>(value));
    }
    
    inline Proxy new_named_int16(const std::string& name, int16_t value)
    {
        return jluna::detail::create_or_assign(name, box<int16_t>(value));
    }
    
    inline Proxy new_named_int32(const std::string& name, int32_t value)
    {
        return jluna::detail::create_or_assign(name, box<int32_t>(value));
    }
    
    inline Proxy new_named_int64(const std::string& name, int64_t value)
    {
        return jluna::detail::create_or_assign(name, box<int64_t>(value));
    }
    
    inline Proxy new_named_float32(const std::string& name, float value)
    {
        return jluna::detail::create_or_assign(name, box<float>(value));
    }
    
    inline Proxy new_named_float64(const std::string& name, double value)
    {
        return jluna::detail::create_or_assign(name, box<double>(value));
    }
    
    inline Proxy new_named_string(const std::string& name, const std::string& value)
    {
        return jluna::detail::create_or_assign(name, box<std::string>(value));
    }
    
    inline Proxy new_named_symbol(const std::string& name, const std::string& value)
    {
        return jluna::detail::create_or_assign(name, (jl_value_t*) jl_symbol(value.c_str()));
    }

    template<IsNumerical T>
    Proxy new_named_complex(const std::string& name, T real, T imag)
    {
        return jluna::detail::create_or_assign(name, box<std::complex<T>>(std::complex<T>(real, imag)));
    }

    template<Boxable T>
    Proxy new_named_vector(const std::string& name, const std::vector<T>& value)
    {
        return jluna::detail::create_or_assign(name, box<std::vector<T>>(value));
    }

    template<Boxable Key_t, Boxable Value_t>
    Proxy new_named_iddict(const std::string& name, const std::map<Key_t, Value_t>& value)
    {
        return jluna::detail::create_or_assign(name, box<std::map<Key_t, Value_t>>(value));
    }

    template<Boxable Key_t, Boxable Value_t>
    Proxy new_named_dict(const std::string& name, const std::unordered_map<Key_t, Value_t>& value)
    {
        return jluna::detail::create_or_assign(name, box<std::unordered_map<Key_t, Value_t>>(value));
    }

    template<Boxable T>
    Proxy new_named_set(const std::string& name, const std::set<T>& value)
    {
        return jluna::detail::create_or_assign(name, box<std::set<T>>(value));
    }

    template<Boxable T1, Boxable T2>
    Proxy new_named_pair(const std::string& name, T1 first, T2 second)
    {
        return jluna::detail::create_or_assign(name, box<std::pair<T1, T2>>(std::pair<T1, T2>(first, second)));
    }

    template<Boxable... Ts>
    Proxy new_named_tuple(const std::string& name, Ts... args)
    {
        return jluna::detail::create_or_assign(name, box<std::tuple<Ts...>>(std::make_tuple(args...)));
    }

    template<Boxable T, size_t N, Is<size_t>... Dims>
    Array<T, N> new_named_array(const std::string& name, Dims... dims)
    {
        static_assert(sizeof...(Dims) == N, "wrong number of dimension initializers");

        std::stringstream str;
        str << name << " = " << to_julia_type<Array<T, N>>::type_name << "(undef,";

        auto add = [&](size_t dim, size_t i){
            str << dim << (i != sizeof...(Dims) ? ", " : ")");
        };

        {
            size_t i = 0;
            (add(dims, ++i), ...);
        }

        std::cout << str.str() << std::endl;

        State::safe_script(str.str());
        return Main[name];
    }

    // ###

    inline Proxy new_unnamed_undef(const std::string& name)
    {
        return Proxy(jl_undef_initializer());
    }

    inline Proxy new_unnamed_bool(bool value)
    {
        return Proxy(jl_box_bool(value));
    }

    inline Proxy new_unnamed_char(char value)
    {
        return Proxy(box<char>(value));
    }

    inline Proxy new_unnamed_uint8(uint8_t value)
    {
        return Proxy(box<uint8_t>(value));
    }

    inline Proxy new_unnamed_uint16(uint16_t value)
    {
        return Proxy(box<uint16_t>(value));
    }

    inline Proxy new_unnamed_uint32(uint32_t value)
    {
        return Proxy(box<uint32_t>(value));
    }

    inline Proxy new_unnamed_uint64(uint64_t value)
    {
        return Proxy(box<uint64_t>(value));
    }

    inline Proxy new_unnamed_int8(int8_t value)
    {
        return Proxy(box<int8_t>(value));
    }

    inline Proxy new_unnamed_int16(int16_t value)
    {
        return Proxy(box<int16_t>(value));
    }

    inline Proxy new_unnamed_int32(int32_t value)
    {
        return Proxy(box<int32_t>(value));
    }

    inline Proxy new_unnamed_int64(int64_t value)
    {
        return Proxy(box<int64_t>(value));
    }

    inline Proxy new_unnamed_float32(float value)
    {
        return Proxy(box<float>(value));
    }

    inline Proxy new_unnamed_float64(double value)
    {
        return Proxy(box<double>(value));
    }

    inline Proxy new_unnamed_string(const std::string& value)
    {
        return Proxy(box<std::string>(value));
    }

    inline Proxy new_unnamed_symbol(const std::string& value)
    {
        return Proxy((jl_value_t*) jl_symbol(value.c_str()));
    }

    template<IsNumerical T>
    Proxy new_unnamed_complex(T real, T imag)
    {
        return Proxy(box<std::complex<T>>(std::complex<T>(real, imag)));
    }

    template<Boxable T>
    Proxy new_unnamed_vector(const std::vector<T>& value)
    {
        return Proxy(box<std::vector<T>>(value));
    }

    template<Boxable Key_t, Boxable Value_t>
    Proxy new_unnamed_iddict(const std::map<Key_t, Value_t>& value)
    {
        return Proxy(box<std::map<Key_t, Value_t>>(value));
    }

    template<Boxable Key_t, Boxable Value_t>
    Proxy new_unnamed_dict(const std::unordered_map<Key_t, Value_t>& value)
    {
        return Proxy(box<std::unordered_map<Key_t, Value_t>>(value));
    }

    template<Boxable T>
    Proxy new_unnamed_set(const std::set<T>& value)
    {
        return Proxy(box<std::set<T>>(value));
    }

    template<Boxable T1, Boxable T2>
    Proxy new_unnamed_pair(T1 first, T2 second)
    {
        return Proxy(box<std::pair<T1, T2>>(std::pair<T1, T2>(first, second)));
    }

    template<Boxable... Ts>
    Proxy new_unnamed_tuple(Ts... args)
    {
        return Proxy(box<std::tuple<Ts...>>(std::make_tuple(args...)));
    }

    template<Boxable T, size_t N, Is<size_t>... Dims>
    Array<T, N> new_unnamed_array(Dims... dims)
    {
        static_assert(sizeof...(Dims) == N, "wrong number of dimension initializers");

        auto* make_new = jl_find_function("jluna", "make_new");
        auto* res = safe_call(
            make_new,
            box<std::string>(to_julia_type<Array<T, N>>::type_name),
            dims...
        );

        return Proxy(res);
    }
}