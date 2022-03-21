// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <include/gc_sentinel.hpp>

namespace jluna
{
    namespace detail
    {
        template<typename T>
        T smart_unbox_primitive(unsafe::Value* in)
        {
            bool first_attempt = true;
            retry:

            if (jl_isa(in, (unsafe::Value*) jl_bool_type))
                return jl_unbox_bool(in);
            else if (jl_isa(in, (unsafe::Value*) jl_int8_type))
                return jl_unbox_int8(in);
            else if (jl_isa(in, (unsafe::Value*) jl_int16_type))
                return jl_unbox_int16(in);
            else if (jl_isa(in, (unsafe::Value*) jl_int32_type))
                return jl_unbox_int32(in);
            else if (jl_isa(in, (unsafe::Value*) jl_int64_type))
                return jl_unbox_int64(in);
            else if (jl_isa(in, (unsafe::Value*) jl_uint8_type))
                return jl_unbox_uint8(in);
            else if (jl_isa(in, (unsafe::Value*) jl_uint16_type))
                return jl_unbox_uint16(in);
            else if (jl_isa(in, (unsafe::Value*) jl_uint32_type))
                return jl_unbox_uint32(in);
            else if (jl_isa(in, (unsafe::Value*) jl_uint64_type))
                return jl_unbox_uint64(in);
            else if (jl_isa(in, (unsafe::Value*) jl_float32_type))
                return jl_unbox_float32(in);
            else if (jl_isa(in, (unsafe::Value*) jl_float64_type))
                return jl_unbox_float64(in);
            else if (jl_isa(in, (unsafe::Value*) jl_float16_type))
                return jl_unbox_float32(jl_try_convert(jl_float32_type, in));
            else if (jl_isa(in, (unsafe::Value*) jl_char_type))
                return jl_unbox_int32(jl_convert(jl_int32_type, in));
            else
            {
                if (not first_attempt)
                    return 0;

                in = jl_try_convert((jl_datatype_t*) jluna::safe_eval(to_julia_type<T>::type_name.c_str()), in);
                first_attempt = false;
                goto retry;
            }

            return 0;
        }
    }


    template<Is<unsafe::Value*> T>
    T unbox(unsafe::Value* in)
    {
        return in;
    }

    template<Is<bool> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<char> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<uint8_t> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<uint16_t> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<uint32_t> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<uint64_t> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<int8_t> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<int16_t> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<int32_t> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<int64_t> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<float> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<double> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res = detail::smart_unbox_primitive<T>(value);
        jl_gc_unpause;
        return res;
    }

    template<Is<std::string> T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto res =  std::string(jl_to_string(value));
        jl_gc_unpause;
        return res;
    } //°

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::complex<Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        static jl_datatype_t* type = (jl_datatype_t*) jl_eval_string(("return " + to_julia_type<std::complex<Value_t>>::type_name).c_str());
        auto* res = jl_try_convert(type, value);

        auto* re = jl_get_nth_field(value, 0);
        auto* im = jl_get_nth_field(value, 1);

        auto out = std::complex<Value_t>(unbox<Value_t>(re), unbox<Value_t>(im));
        jl_gc_unpause;
        return out;
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::vector<Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        jl_array_t* in = (jl_array_t*) value;

        std::vector<Value_t> out;
        out.reserve(in->length);

        for (size_t i = 0; i < in->length; ++i)
            out.emplace_back(unbox<Value_t>(jl_arrayref(in, i)));

        jl_gc_unpause;
        return out;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<std::is_same_v<T, std::map<Key_t, Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        static jl_function_t* iterate = jl_get_function(jl_base_module, "iterate");

        jl_gc_pause;
        auto out = std::map<Key_t, Value_t>();
        auto* it_res = jl_nothing;

        unsafe::Value* next_i = jl_box_int64(1);
        while(true)
        {
            it_res = jl_call2(iterate, value, next_i);
            if (it_res == jl_nothing)
                break;

            out.insert(unbox<std::pair<Key_t, Value_t>>(jl_get_nth_field(it_res, 0)));
            next_i = jl_get_nth_field(it_res, 1);
        }

        jl_gc_unpause;
        return out;
    } //°

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<std::is_same_v<T, std::unordered_map<Key_t, Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        static jl_function_t* iterate = jl_get_function(jl_base_module, "iterate");

        jl_gc_pause;
        auto out = std::unordered_map<Key_t, Value_t>();
        auto* it_res = jl_nothing;

        unsafe::Value* next_i = jl_box_int64(1);
        while(true)
        {
            it_res = jl_call2(iterate, value, next_i);
            if (it_res == jl_nothing)
                break;

            out.insert(unbox<std::pair<Key_t, Value_t>>(jl_get_nth_field(it_res, 0)));
            next_i = jl_get_nth_field(it_res, 1);
        }

        jl_gc_unpause;
        return out;
    } //°

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::set<Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;

        static jl_function_t* serialize = jl_find_function("jluna", "serialize");
        jl_array_t* as_array = (jl_array_t*) jl_call1(serialize, value);

        T out;
        for (size_t i = 0; i < jl_array_len(as_array); ++i)
            out.insert(unbox<Value_t>(jl_arrayref(as_array, i)));

        jl_gc_unpause;
        return out;
    }

    template<IsPair T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;

        auto* first = jl_get_nth_field(value, 0);
        auto* second = jl_get_nth_field(value, 1);

        auto res = T(unbox<typename T::first_type>(first), unbox<typename T::second_type>(second));
        jl_gc_unpause;

        return res;
    } //°

    namespace detail    // helper functions for tuple unboxing
    {
        template<typename Tuple_t, typename Value_t, size_t i>
        void unbox_tuple_aux_aux(Tuple_t& tuple, jl_value_t* value)
        {
            std::get<i>(tuple) = unbox<std::tuple_element_t<i, Tuple_t>>(jl_get_nth_field(value, i));
        }

        template<typename Tuple_t, typename Value_t, std::size_t... is>
        void unbox_tuple_aux(Tuple_t& tuple, jl_value_t* value, std::index_sequence<is...> _)
        {
            (unbox_tuple_aux_aux<Tuple_t, Value_t, is>(tuple, value), ...);
        }

        template<typename... Ts>
        std::tuple<Ts...> unbox_tuple(jl_value_t* value)
        {
            std::tuple<Ts...> out;
            (unbox_tuple_aux<std::tuple<Ts...>, Ts>(out, value, std::index_sequence_for<Ts...>{}), ...);
            return out;
        }

        template<typename... Ts>
        std::tuple<Ts...> unbox_tuple_pre(jl_value_t* v, std::tuple<Ts...> _)
        {
            return unbox_tuple<Ts...>(v);
        }
    }

    class Symbol;
    template<Is<Symbol> T>
    T unbox(unsafe::Value*);

    class Module;
    template<Is<Module> T>
    T unbox(unsafe::Value*);

    class Type;
    template<Is<Type> T>
    T unbox(unsafe::Value*);

    template<IsTuple T>
    T unbox(unsafe::Value* value)
    {
        jl_gc_pause;
        auto out = detail::unbox_tuple_pre(value, T());
        jl_gc_unpause;
        return out;
    }
}