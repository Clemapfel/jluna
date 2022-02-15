// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    template<Is<Any*> T>
    T unbox(Any* in)
    {
        return in;
    }

    template<Is<bool> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Bool", value);
        return jl_unbox_bool(res);
    }

    template<Is<char> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("UInt8", value);
        return static_cast<char>(jl_unbox_uint8(res));
    }

    template<Is<uint8_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("UInt8", value);
        return jl_unbox_uint8(res);
    }

    template<Is<uint16_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("UInt16", value);
        return jl_unbox_uint16(res);
    }

    template<Is<uint32_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("UInt32", value);
        return jl_unbox_uint32(res);
    }

    template<Is<uint64_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("UInt64", value);
        return jl_unbox_uint64(res);
    }

    template<Is<int8_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Int8", value);
        return jl_unbox_int8(res);
    }

    template<Is<int16_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Int16", value);
        return jl_unbox_int16(res);
    }

    template<Is<int32_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Int32", value);
        return jl_unbox_int32(res);
    }

    template<Is<int64_t> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Int64", value);
        return jl_unbox_int64(res);
    }

    template<Is<float> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Float32", value);
        return jl_unbox_float32(res);
    }

    template<Is<double> T>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert("Float64", value);
        return jl_unbox_float64(res);
    }

    template<Is<std::string> T>
    T unbox(Any* value)
    {
        return jl_to_string(value);
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::complex<Value_t>>, bool>>
    T unbox(Any* value)
    {
        auto* res = jl_try_convert(to_julia_type<std::complex<Value_t>>::type_name.c_str(), value);

        auto* re = jl_get_nth_field(value, 0);
        auto* im = jl_get_nth_field(value, 1);

        return std::complex<Value_t>(unbox<Value_t>(re), unbox<Value_t>(im));
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::vector<Value_t>>, bool>>
    T unbox(Any* value)
    {
        jl_pause_gc;

        static jl_function_t* getindex = jl_get_function(jl_base_module, "getindex");

        std::vector<Value_t> out;
        out.reserve(jl_array_len(value));

        for (size_t i = 0; i < jl_array_len(value); ++i)
            out.push_back(unbox<Value_t>(jl_call2(getindex, value, jl_box_uint64(i+1))));

        jl_unpause_gc;
        return out;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<std::is_same_v<T, std::map<Key_t, Value_t>>, bool>>
    T unbox(Any* value)
    {
        jl_pause_gc;
        auto* res = jl_try_convert(to_julia_type<std::map<Key_t, Value_t>>::type_name.c_str(), value);

        static jl_function_t* serialize = jl_find_function("jluna", "serialize");

        jl_array_t* as_array = (jl_array_t*) safe_call(serialize, value);

        T out;
        for (size_t i = 0; i < jl_array_len(as_array); ++i)
            out.insert(unbox<std::pair<Key_t, Value_t>>(jl_arrayref(as_array, i)));

        jl_unpause_gc;
        return out;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<std::is_same_v<T, std::unordered_map<Key_t, Value_t>>, bool>>
    T unbox(Any* value)
    {
        jl_pause_gc;
        auto* res = jl_try_convert(to_julia_type<std::unordered_map<Key_t, Value_t>>::type_name.c_str(), value);

        static jl_function_t* serialize = jl_find_function("jluna", "serialize");

        jl_array_t* as_array = (jl_array_t*) safe_call(serialize, value);

        T out;
        for (size_t i = 0; i < jl_array_len(as_array); ++i)
            out.insert(unbox<std::pair<Key_t, Value_t>>(jl_arrayref(as_array, i)));

        jl_unpause_gc;
        return out;
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::set<Value_t>>, bool>>
    T unbox(Any* value)
    {
        jl_pause_gc;

        static jl_function_t* serialize = jl_find_function("jluna", "serialize");
        jl_array_t* as_array = (jl_array_t*) jl_call1(serialize, value);

        T out;
        for (size_t i = 0; i < jl_array_len(as_array); ++i)
            out.insert(unbox<Value_t>(jl_arrayref(as_array, i)));

        jl_unpause_gc;
        return out;
    }

    template<typename T, typename T1, typename T2, std::enable_if_t<std::is_same_v<T, std::pair<T1, T2>>, bool>>
    T unbox(Any* value)
    {
        jl_pause_gc;

        auto* first = jl_get_nth_field(value, 0);
        auto* second = jl_get_nth_field(value, 1);

        auto res = std::pair<T1, T2>(unbox<T1>(first), unbox<T2>(second));
        jl_unpause_gc;

        r
    }

    namespace detail    // helper functions for tuple unboxing
    {
        template<typename Tuple_t, typename Value_t, size_t i>
        void unbox_tuple_aux_aux(Tuple_t& tuple, jl_value_t* value)
        {
            static jl_function_t* tuple_at = (jl_function_t*) jl_eval_string("jluna.tuple_at");
            auto* v = safe_call(tuple_at, value, jl_box_uint64(i + 1));
            std::get<i>(tuple) = unbox<std::tuple_element_t<i, Tuple_t>>(v);
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
        std::tuple<Ts...> unbox_tuple_pre(jl_value_t* v, std::tuple<Ts...>)
        {
            return unbox_tuple<Ts...>(v);
        }
    }

    template<IsTuple T>
    T unbox(Any* value)
    {
        value = jl_try_convert(to_julia_type<T>::type_name.c_str(), value);

        return detail::unbox_tuple_pre(value, T());
    }
}