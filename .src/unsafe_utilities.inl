// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

namespace jluna::unsafe
{
    template<IsJuliaValuePointer... Args_t>
    unsafe::Value* call(Function* function, Args_t... args)
    {
        std::array<jl_value_t*, sizeof...(Args_t)> wrapped = {(unsafe::Value*) args...};
        return jl_call(function, wrapped.data(), wrapped.size());
    }

    template<IsJuliaValuePointer... Args_t>
    unsafe::Value* call(DataType* type, Args_t... args)
    {
        return jl_new_struct(type, args...);
    }

    template<IsJuliaValuePointer... Args_t>
    unsafe::Expression* Expr(unsafe::Symbol* first, Args_t... other)
    {
        static unsafe::Function* expr = unsafe::get_function(jl_base_module, "Expr"_sym);
        return (unsafe::Expression*) call(expr, first, other...);
    }

    namespace detail
    {
        inline std::nullptr_t gc_init()
        {
            static bool initialized = false;

            if (initialized)
                return nullptr;

            jl_eval_string(R"(
                __jluna_heap = Dict{UInt64, Base.RefValue{Any}}();
                __jluna_heap_index = Base.RefValue(UInt64(0));

                function __jluna_add_to_heap(ptr::UInt64)
                    global __jluna_heap_index[] += 1
                    __jluna_heap[__jluna_heap_index[]] = Ref{Any}(unsafe_pointer_to_objref(Ptr{Any}(ptr)))
                    return __jluna_heap_index[];
                end
            )");
            initialized = true;
            return nullptr;
        }
    }

    template<IsJuliaValue T>
    size_t gc_preserve(T* in)
    {
        auto* value = (unsafe::Value*) in;
        bool before = jl_gc_is_enabled();
        jl_gc_enable(false);
        
        static auto _ = detail::gc_init();
        static unsafe::Function* jluna_add_to_heap = get_function(jl_main_module, "__jluna_add_to_heap"_sym);

        auto res = jl_unbox_uint64(call(jluna_add_to_heap, jl_box_uint64((UInt64) value)));
        jl_gc_enable(before);
        return res;
    }

    template<IsJuliaValuePointer... Ts, std::enable_if_t<(sizeof...(Ts) > 2), bool>>
    std::vector<size_t> gc_preserve(Ts... values)
    {
        std::vector<size_t> out;
        out.reserve(sizeof...(Ts));
        (out.push_back(gc_preserve(values)), ...);
        return out;
    }

    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool>>
    unsafe::Array* new_array(unsafe::Value* value_type, Dims... size_per_dimension)
    {
        bool before = jl_gc_is_enabled();
        jl_gc_enable(false);
        static auto* tuple_type = [&](){
            std::array<jl_value_t*, sizeof...(Dims)> types;
            for (size_t i = 0; i < types.size(); ++i)
                types.at(i) = (jl_value_t*) jl_uint64_type;

            return jl_apply_tuple_type_v(types.data(), types.size());
        }();
        auto* tuple = jl_new_struct(tuple_type, jl_box_uint64(size_per_dimension)...);
        auto* res = jl_new_array(jl_apply_array_type(value_type, sizeof...(Dims)), tuple);
        jl_gc_enable(before);
        return res;
    }

    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool>>
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, Dims... size_per_dimension)
    {
        bool before = jl_gc_is_enabled();
        jl_gc_enable(false);
        static auto* tuple_type = [&](){
            std::array<jl_value_t*, sizeof...(Dims)> types;
            for (size_t i = 0; i < types.size(); ++i)
                types.at(i) = (jl_value_t*) jl_uint64_type;

            return jl_apply_tuple_type_v(types.data(), types.size());
        }();
        auto* tuple = jl_new_struct(tuple_type, jl_box_uint64(size_per_dimension)...);
        auto* res = jl_ptr_to_array(jl_apply_array_type(value_type, sizeof...(Dims)), data, tuple, 0);
        jl_gc_enable(before);
        return res;
    }

    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 2), bool>>
    void resize_array(unsafe::Array* array, Dims... size_per_dimension)
    {
        bool before = jl_gc_is_enabled();
        jl_gc_enable(false);
        static auto* tuple_type = [&](){
            std::array<jl_value_t*, sizeof...(Dims)> types;
            for (size_t i = 0; i < types.size(); ++i)
                types.at(i) = (jl_value_t*) jl_uint64_type;

            return jl_apply_tuple_type_v(types.data(), types.size());
        }();

        static jl_function_t* get_value_type_of_array = get_function("jluna"_sym, "get_value_type_of_array"_sym);

        auto* tuple = jl_new_struct(tuple_type, jl_box_uint64(size_per_dimension)...);
        auto* res = jl_reshape_array(jl_apply_array_type(unsafe::call(get_value_type_of_array, array), sizeof...(Dims)), array, tuple);
        override_array(array, res);
        jl_gc_enable(before);
    }

    template<typename... Index, std::enable_if_t<(sizeof...(Index) > 2), bool>>
    unsafe::Value* get_index(unsafe::Array* array, Index... index_per_dimension)
    {
        std::array<size_t, sizeof...(Index)> indices = {size_t(index_per_dimension)...};
        size_t index = 0;
        size_t mul = 1;

        for (size_t i = 0; i < array->flags.ndims; ++i)
        {
            index += (indices.at(i)) * mul;
            size_t dim = jl_array_dim(array, i);
            mul *= dim;
        }

        return jl_arrayref(array, index);
    }

    inline unsafe::Value* get_index(unsafe::Array* array, size_t i)
    {
        return jl_arrayref(array, i);
    }

    inline unsafe::Value* get_index(unsafe::Array* array, size_t i, size_t j)
    {
        return jl_arrayref(array, i + jl_array_dim(array, 0) * j);
    }

    template<typename... Index, std::enable_if_t<(sizeof...(Index) > 2), bool>>
    void set_index(unsafe::Array* array, unsafe::Value* new_value, Index... index_per_dimension)
    {
        std::array<size_t, sizeof...(Index)> indices = {size_t(index_per_dimension)...};
        size_t index = 0;
        size_t mul = 1;

        for (size_t i = 0; i < array->flags.ndims; ++i)
        {
            index += (indices.at(i)) * mul;
            size_t dim = jl_array_dim(array, i);
            mul *= dim;
        }

        jl_arrayset(array, new_value, index);
    }

    inline void set_index(unsafe::Array* array, unsafe::Value* value, size_t i)
    {
        jl_arrayset(array, value, i);
    }

    template<typename T>
    void set_array_data(unsafe::Array* array, T* new_data, size_t new_size)
    {
        array->data = new_data;
        jl_gc_wb(array, new_data);
        array->length = new_size;
    }

    template<IsJuliaValue T>
    unsafe::Value* unsafe_box(T* in)
    {
        return in;
    }

    template<Is<bool> T>
    unsafe::Value* unsafe_box(bool in)
    {
        return jl_box_bool(in);
    }

    template<Is<char> T>
    unsafe::Value* unsafe_box(char in)
    {
        return jl_box_char(in);
    }

    template<Is<int8_t> T>
    unsafe::Value* unsafe_box(int8_t in)
    {
        return jl_box_int8(in);
    }

    template<Is<int16_t> T>
    unsafe::Value* unsafe_box(int16_t in)
    {
        return jl_box_int16(in);
    }

    template<Is<int32_t> T>
    unsafe::Value* unsafe_box(int32_t in)
    {
        return jl_box_int32(in);
    }
    
    template<Is<int64_t> T>
    unsafe::Value* unsafe_box(int64_t in)
    {
        return jl_box_int64(in);
    }

    template<Is<uint8_t> T>
    unsafe::Value* unsafe_box(uint8_t in)
    {
        return jl_box_uint8(in);
    }

    template<Is<uint16_t> T>
    unsafe::Value* unsafe_box(uint16_t in)
    {
        return jl_box_uint16(in);
    }

    template<Is<uint32_t> T>
    unsafe::Value* unsafe_box(uint32_t in)
    {
        return jl_box_uint32(in);
    }

    template<Is<uint64_t> T>
    unsafe::Value* unsafe_box(uint64_t in)
    {
        return jl_box_uint64(in);
    }

    template<Is<float> T>
    unsafe::Value* unsafe_box(float in)
    {
        return jl_box_float32(in);
    }
    
    template<Is<double> T>
    unsafe::Value* unsafe_box(double in)
    {
        return jl_box_float64(in);
    }

    template<Is<std::string> T>
    unsafe::Value* unsafe_box(const T& in)
    {
        return jl_array_to_string(jl_ptr_to_array_1d(jl_apply_array_type((jl_value_t*) jl_char_type, 1), in.data(), in.size(), 0));
    }

    template<Is<bool> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return jl_unbox_bool(in);
    }

    template<Is<char> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return char(jl_unbox_uint32(in));
    }

    template<Is<int8_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<int8_t*>(in));
    }
    
    template<Is<int16_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<int16_t*>(in));
    }
    
    template<Is<int32_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<int32_t*>(in));
    }
    
    template<Is<int64_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<int64_t*>(in));
    }
    
    template<Is<uint8_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<uint8_t*>(in));
    }
    
    template<Is<uint16_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<uint16_t*>(in));
    }
    
    template<Is<uint32_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<uint32_t*>(in));
    }
    
    template<Is<uint64_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<uint64_t*>(in));
    }
    
    template<Is<Float32> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<Float32*>(in));
    }
    
    template<Is<Float64> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<Float64*>(in));
    }

    template<Is<std::string> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return std::string(jl_string_ptr(in));
    }
}