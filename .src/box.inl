// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <include/cppcall.hpp>

#include <iostream>


namespace jluna
{
    template<is_julia_value_pointer T>
    unsafe::Value* box(T value)
    {
        return (unsafe::Value*) value;
    }

    template<is<std::nullptr_t> T>
    unsafe::Value* box(T)
    {
        return jl_nothing;
    }

    template<is<bool> T>
    unsafe::Value* box(T value)
    {
        return jl_box_bool(value);
    }

    template<is<std::bool_constant<true>> T>
    unsafe::Value* box(T value)
    {
        return jl_box_bool(true);
    }

    template<is<std::bool_constant<false>> T>
    unsafe::Value* box(T value)
    {
        return jl_box_bool(false);
    }
    
    template<is<char> T>
    unsafe::Value* box(T value)
    {
        return detail::convert(jl_char_type, jl_box_int8((int8_t) value));
    }

    template<is<uint8_t> T>
    unsafe::Value* box(T value)
    {
        return jl_box_uint8((uint8_t) value);
    }

    template<is<uint16_t> T>
    unsafe::Value* box(T value)
    {
        return jl_box_uint16((uint16_t) value);
    }

    template<is<uint32_t> T>
    unsafe::Value* box(T value)
    {
        return jl_box_uint32((uint32_t) value);
    }

    template<is<uint64_t> T>
    unsafe::Value* box(T value)
    {
        return jl_box_uint64((uint64_t) value);
    }

    template<is<int8_t> T>
    unsafe::Value* box(T value)
    {
        return jl_box_int8((int8_t) value);
    }

    template<is<int16_t> T>
    unsafe::Value* box(T value)
    {
        return jl_box_int16((int16_t) value);
    }

    template<is<int32_t> T>
    unsafe::Value* box(T value)
    {
        return jl_box_int32((int32_t) value);
    }

    template<is<int64_t> T>
    unsafe::Value* box(T value)
    {
        return jl_box_int64((int64_t) value);
    }

    template<is<float> T>
    unsafe::Value* box(T value)
    {
        return jl_box_float32((float) value);
    }

    template<is<double> T>
    unsafe::Value* box(T value)
    {
        return jl_box_float64((double) value);
    }

    template<is<std::string> T>
    unsafe::Value* box(T value)
    {
        gc_pause;
        auto* array = unsafe::new_array_from_data((unsafe::Value*) to_julia_type<char>::type(), value.data(), value.size());
        auto* out = jl_array_to_string(array);
        gc_unpause;
        return out;
    }

    template<is<const char*> T>
    unsafe::Value* box(T value)
    {
        gc_pause;
        std::string as_string = value;
        auto* array = unsafe::new_array_from_data((unsafe::Value*) to_julia_type<char>::type(), as_string.data(), as_string.size());
        auto* out = jl_array_to_string(array);
        gc_unpause;
        return out;
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::complex<Value_t>>, bool>>
    unsafe::Value* box(T value)
    {
        static jl_function_t* complex = unsafe::get_function("jluna"_sym, "new_complex"_sym);
        return safe_call(complex, box<Value_t>(value.real()), box<Value_t>(value.imag()));
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::vector<Value_t>>, bool>>
    unsafe::Value* box(const T& value)
    {
        gc_pause;
        auto* out = unsafe::new_array((unsafe::Value*) to_julia_type<Value_t>::type(), value.size());
        for (size_t i = 0; i < value.size(); ++i)
        {
            auto* topush = box<Value_t>(value.at(i));
            jl_arrayset(out, topush, i);
        }

        gc_unpause;
        return (unsafe::Value*) out;
    }

    template<typename T, typename Key_t, typename Value_t, std::enable_if_t<
            std::is_same_v<T, std::unordered_map<Key_t, Value_t>> or
            std::is_same_v<T, std::map<Key_t, Value_t>>,
            bool>>
    unsafe::Value* box(T value)
    {
        static auto* new_dict = unsafe::get_function("jluna"_sym, "new_dict"_sym);
        static auto* setindex = unsafe::get_function(jl_base_module, "setindex!"_sym);

        gc_pause;

        auto* out = unsafe::call(new_dict, to_julia_type<Key_t>::type(), to_julia_type<Value_t>::type(), box(value.size()));
        for (auto& pair : value)
            safe_call(setindex, out, box<Value_t>(pair.second), box<Key_t>(pair.first));

        gc_unpause;
        return out;
    }

    template<typename T, typename Value_t, std::enable_if_t<std::is_same_v<T, std::set<Value_t>>, bool>>
    unsafe::Value* box(const T& value)
    {
        static auto* new_set = unsafe::get_function("jluna"_sym, "new_set"_sym);
        static auto* push = unsafe::get_function(jl_base_module, "push!"_sym);

        gc_pause;

        auto* out = unsafe::call(new_set, to_julia_type<Value_t>::type(), box(value.size()));

        for (auto& e : value)
            unsafe::call(push, out, box<Value_t>(e));

        gc_unpause;
        return out;
    }

    template<typename T, typename T1, typename T2, std::enable_if_t<std::is_same_v<T, std::pair<T1, T2>>, bool>>
    unsafe::Value* box(T value)
    {
        static auto* pair = unsafe::get_function(jl_base_module, "Pair"_sym);
        return unsafe::call(pair, box<T1>(value.first), box<T2>(value.second));
    }

    template<is_tuple T>
    unsafe::Value* box(T value)
    {
        gc_pause;

        auto* args_v = unsafe::new_array((unsafe::Value*) jl_any_type, std::tuple_size_v<T>);
        auto* args_t = unsafe::new_array((unsafe::Value*) jl_type_type, std::tuple_size_v<T>);

        {
            size_t i = 0;
            std::apply([&](auto... elements) {
                (jl_arrayset(args_v, box(elements), i++), ...);
            }, value);
        }

        for (size_t i = 0; i < std::tuple_size_v<T>; ++i)
            jl_arrayset(args_t, jl_typeof(jl_arrayref(args_v, i)), i);

        auto tuple_t = jl_apply_tuple_type_v((jl_value_t**) args_t->data, args_t->length);
        auto* out = jl_new_structv(tuple_t, (jl_value_t**) args_v->data, args_v->length);
        gc_unpause;
        return out;
    }

    template<LambdaType<> T>
    unsafe::Value* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<unsafe::Value*> T>
    unsafe::Value* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<unsafe::Value*, unsafe::Value*> T>
    unsafe::Value* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<unsafe::Value*, unsafe::Value*, unsafe::Value*> T>
    unsafe::Value* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<unsafe::Value*, unsafe::Value*, unsafe::Value*, unsafe::Value*> T>
    unsafe::Value* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }

    template<LambdaType<std::vector<unsafe::Value*>> T>
    unsafe::Value* box(T lambda)
    {
        return register_unnamed_function<T>(lambda);
    }
}