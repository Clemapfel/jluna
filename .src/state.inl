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
    inline Proxy new_undef(const std::string& name)
    {
        State::safe_script("Main.eval(:(" + name + " = undef))");
        return Main[name];
    }

    inline Proxy new_bool(const std::string& name, bool value)
    {
        jluna::detail::create_or_assign(name, jl_box_bool(value));
    }
    
    inline Proxy new_char(const std::string& name, char value)
    {
        return jluna::detail::create_or_assign(name, box<char>(value));
    }
    
    inline Proxy new_uint8(const std::string& name, uint8_t value)
    {
        return jluna::detail::create_or_assign(name, box<uint8_t>(value));
    }
    
    inline Proxy new_uint16(const std::string& name, uint16_t value)
    {
        return jluna::detail::create_or_assign(name, box<uint16_t>(value));
    }
    
    inline Proxy new_uint32(const std::string& name, uint32_t value)
    {
        return jluna::detail::create_or_assign(name, box<uint32_t>(value));
    }
    
    inline Proxy new_uint64(const std::string& name, uint64_t value)
    {
        return jluna::detail::create_or_assign(name, box<uint64_t>(value));
    }
    
    inline Proxy new_int8(const std::string& name, int8_t value)
    {
        return jluna::detail::create_or_assign(name, box<int8_t>(value));
    }
    
    inline Proxy new_int16(const std::string& name, int16_t value)
    {
        return jluna::detail::create_or_assign(name, box<int16_t>(value));
    }
    
    inline Proxy new_int32(const std::string& name, int32_t value)
    {
        return jluna::detail::create_or_assign(name, box<int32_t>(value));
    }
    
    inline Proxy new_int64(const std::string& name, int64_t value)
    {
        return jluna::detail::create_or_assign(name, box<int64_t>(value));
    }
    
    inline Proxy new_float32(const std::string& name, float value)
    {
        return jluna::detail::create_or_assign(name, box<float>(value));
    }
    
    inline Proxy new_float64(const std::string& name, double value)
    {
        return jluna::detail::create_or_assign(name, box<double>(value));
    }
    
    inline Proxy new_string(const std::string& name, const std::string& value)
    {
        return jluna::detail::create_or_assign(name, box<std::string>(value));
    }
    
    inline Proxy new_symbol(const std::string& name, const std::string& value)
    {
        return jluna::detail::create_or_assign(name, (jl_value_t*) jl_symbol(value.c_str()));
    }

    template<IsNumerical T>
    Proxy new_complex(const std::string& name, T real, T imag)
    {
        return jluna::detail::create_or_assign(name, box<std::complex<T>>(std::complex<T>(real, imag)));
    }

    template<Boxable T>
    Proxy new_vector(const std::string& name, const std::vector<T>& value)
    {
        return jluna::detail::create_or_assign(name, box<std::vector<T>>(value));
    }

    template<Boxable Key_t, Boxable Value_t>
    Proxy new_iddict(const std::string& name, const std::map<Key_t, Value_t>& value)
    {
        return jluna::detail::create_or_assign(name, box<std::map<Key_t, Value_t>>(value));
    }

    template<Boxable Key_t, Boxable Value_t>
    Proxy new_dict(const std::string& name, const std::unordered_map<Key_t, Value_t>& value)
    {
        return jluna::detail::create_or_assign(name, box<std::unordered_map<Key_t, Value_t>>(value));
    }

    template<Boxable T>
    Proxy new_set(const std::string& name, const std::set<T>& value)
    {
        return jluna::detail::create_or_assign(name, box<std::set<T>>(value));
    }

    template<Boxable T1, Boxable T2>
    Proxy new_pair(const std::string& name, T1 first, T2 second)
    {
        return jluna::detail::create_or_assign(name, box<std::pair<T1, T2>>(std::pair<T1, T2>(first, second)));
    }

    template<Boxable... Ts>
    Proxy new_tuple(const std::string& name, Ts... args)
    {
        return jluna::detail::create_or_assign(name, box<std::tuple<Ts...>>(std::make_tuple(args...)));
    }

    template<Boxable T, size_t N, Is<size_t>... Dims>
    Array<T, N> new_array(const std::string& name, Dims... dims)
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
        return Main["name"];
    }
}