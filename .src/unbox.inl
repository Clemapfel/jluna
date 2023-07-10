// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <.src/common.hpp>

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
                return static_cast<T>(jl_unbox_bool(in));
            else if (jl_isa(in, (unsafe::Value*) jl_int8_type))
                return static_cast<T>(jl_unbox_int8(in));
            else if (jl_isa(in, (unsafe::Value*) jl_int16_type))
                return static_cast<T>(jl_unbox_int16(in));
            else if (jl_isa(in, (unsafe::Value*) jl_int32_type))
                return static_cast<T>(jl_unbox_int32(in));
            else if (jl_isa(in, (unsafe::Value*) jl_int64_type))
                return static_cast<T>(jl_unbox_int64(in));
            else if (jl_isa(in, (unsafe::Value*) jl_uint8_type))
                return static_cast<T>(jl_unbox_uint8(in));
            else if (jl_isa(in, (unsafe::Value*) jl_uint16_type))
                return static_cast<T>(jl_unbox_uint16(in));
            else if (jl_isa(in, (unsafe::Value*) jl_uint32_type))
                return static_cast<T>(jl_unbox_uint32(in));
            else if (jl_isa(in, (unsafe::Value*) jl_uint64_type))
                return static_cast<T>(jl_unbox_uint64(in));
            else if (jl_isa(in, (unsafe::Value*) jl_float32_type))
                return static_cast<T>(jl_unbox_float32(in));
            else if (jl_isa(in, (unsafe::Value*) jl_float64_type))
                return static_cast<T>(jl_unbox_float64(in));
            else if (jl_isa(in, (unsafe::Value*) jl_float16_type))
                return static_cast<T>(jl_unbox_float32(detail::convert(jl_float32_type, in)));
            else if (jl_isa(in, (unsafe::Value*) jl_char_type))
                return static_cast<T>(jl_unbox_int32(detail::convert(jl_int32_type, in)));
            else
            {
                if (not first_attempt)
                    return 0;

                in = detail::convert(as_julia_type<T>::type(), in);
                first_attempt = false;
                goto retry;
            }

            return 0;
        }
    }


    template<is<unsafe::Value*> T>
    T unbox(unsafe::Value* in)
    {
        return in;
    }

    template<is<bool> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<void*> T>
    T unbox(unsafe::Value* value)
    {
        return jl_unbox_voidpointer(value);
    }

    template<is<char> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<uint8_t> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<uint16_t> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<uint32_t> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<uint64_t> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<int8_t> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<int16_t> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<int32_t> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<int64_t> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<float> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<double> T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::smart_unbox_primitive<T>(value);
        detail::gc_pop(1);
        return out;
    }

    template<is<std::string> T>
    T unbox(unsafe::Value* value)
    {
        return std::string(detail::to_string(value));
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::complex<Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        gc_pause;
        static auto* type = (jl_datatype_t*) jl_eval_string(("return " + as_julia_type<std::complex<Value_t>>::type_name).c_str());
        auto* res = detail::convert(type, value);

        auto* re = jl_get_nth_field(res, 0);
        auto* im = jl_get_nth_field(res, 1);

        auto out = std::complex<Value_t>(unbox<Value_t>(re), unbox<Value_t>(im));
        gc_unpause;
        return out;
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::vector<Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        gc_pause;
        auto* in = (jl_array_t*) value;

        std::vector<Value_t> out;
        out.reserve(in->length);

        for (uint64_t i = 0; i < in->length; ++i)
            out.emplace_back(unbox<Value_t>(jl_arrayref(in, i)));

        gc_unpause;
        return out;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<std::is_same_v<T, std::map<Key_t, Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        static jl_function_t* iterate = jl_get_function(jl_base_module, "iterate");

        gc_pause;
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

        gc_unpause;
        return out;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<std::is_same_v<T, std::unordered_map<Key_t, Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        static jl_function_t* iterate = jl_get_function(jl_base_module, "iterate");

        gc_pause;
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

        gc_unpause;
        return out;
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::set<Value_t>>, bool>>
    T unbox(unsafe::Value* value)
    {
        gc_pause;
        static jl_function_t* serialize = unsafe::get_function("jluna"_sym, "serialize"_sym);
        auto* as_array = (jl_array_t*) jl_call1(serialize, value);

        T out;
        for (uint64_t i = 0; i < jl_array_len(as_array); ++i)
            out.insert(unbox<Value_t>(jl_arrayref(as_array, i)));

        gc_unpause;
        return out;
    }

    template<is_pair T>
    T unbox(unsafe::Value* value)
    {
        gc_pause;

        auto* first = jl_get_nth_field(value, 0);
        auto* second = jl_get_nth_field(value, 1);

        jluna::is_pair auto out = T(unbox<typename T::first_type>(first), unbox<typename T::second_type>(second));
        gc_unpause;
        return out;
    }

    namespace detail    // helper functions for tuple unboxing
    {
        template<typename Tuple_t, typename Value_t, uint64_t i>
        void unbox_tuple_aux_aux(Tuple_t& tuple, unsafe::Value* value)
        {
            std::get<i>(tuple) = unbox<std::tuple_element_t<i, Tuple_t>>(jl_get_nth_field(value, i));
        }

        template<typename Tuple_t, typename Value_t, std::uint64_t... is>
        void unbox_tuple_aux(Tuple_t& tuple, unsafe::Value* value, std::index_sequence<is...>)
        {
            (unbox_tuple_aux_aux<Tuple_t, Value_t, is>(tuple, value), ...);
        }

        template<typename... Ts>
        std::tuple<Ts...> unbox_tuple(unsafe::Value* value)
        {
            std::tuple<Ts...> out;
            (unbox_tuple_aux<std::tuple<Ts...>, Ts>(out, value, std::index_sequence_for<Ts...>{}), ...);
            return out;
        }

        template<typename... Ts>
        std::tuple<Ts...> unbox_tuple_pre(unsafe::Value* v, std::tuple<Ts...>)
        {
            return unbox_tuple<Ts...>(v);
        }
    }

    class Symbol;
    template<is<Symbol> T>
    T unbox(unsafe::Value*);

    class Module;
    template<is<Module> T>
    T unbox(unsafe::Value*);

    class Type;
    template<is<Type> T>
    T unbox(unsafe::Value*);

    template<is_tuple T>
    T unbox(unsafe::Value* value)
    {
        detail::gc_push(value);
        auto out = detail::unbox_tuple_pre(value, T());
        detail::gc_pop(1);
        return out;
    }
}