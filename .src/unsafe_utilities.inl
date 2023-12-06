// 
// Copyright 2022 Clemens Cords
// Created on 21.03.22 by clem (mail@clemens-cords.com)
//

namespace jluna::unsafe
{
    #ifdef _MSC_VER
        // silence false positive conversion warning on MSVC
        #pragma warning(push)
        #pragma warning(disable:4267)
    #endif

    template<is_julia_value_pointer... Args_t>
    unsafe::Value* call(Function* function, Args_t... args)
    {
        std::array<jl_value_t*, sizeof...(Args_t)> wrapped = {(unsafe::Value*) args...};
        return jl_call(function, wrapped.data(), wrapped.size());
    }

    #ifdef _MSC_VER
        #pragma warning(pop)
    #endif

    template<is_julia_value_pointer... Args_t>
    unsafe::Value* call(DataType* type, Args_t... args)
    {
        return jl_new_struct(type, args...);
    }

    template<is_julia_value_pointer... Args_t>
    unsafe::Expression* Expr(unsafe::Symbol* first, Args_t... other)
    {
        static unsafe::Function* expr = unsafe::get_function(jl_base_module, "Expr"_sym);
        return (unsafe::Expression*) call(expr, first, other...);
    }

    namespace detail
    {
        inline bool gc_initialized = false;

        inline std::nullptr_t gc_init()
        {
            if (gc_initialized)
                return nullptr;

            jl_eval_string(R"(
                if !isdefined(Main, :__jluna_heap) 

                    const __jluna_heap = Dict{UInt64, Base.RefValue{Any}}();
                    const __jluna_heap_index = Base.RefValue(UInt64(0));
                    const __jluna_heap_lock = Base.ReentrantLock()

                    function __jluna_add_to_heap(ptr::UInt64)
                        lock(__jluna_heap_lock)
                        global __jluna_heap_index[] += 1
                        __jluna_heap[__jluna_heap_index[]] = Ref{Any}(unsafe_pointer_to_objref(Ptr{Any}(ptr)))
                        unlock(__jluna_heap_lock);
                        return __jluna_heap_index[];
                    end

                    function __jluna_delete_from_heap(ptr::UInt64)
                        lock(__jluna_heap_lock)
                        delete!(__jluna_heap, ptr)
                        unlock(__jluna_heap_lock);
                    end
                end
            )");

            detail::gc_initialized = true;
            return nullptr;
        }
    }

    template<is_julia_value T>
    uint64_t gc_preserve(T* in)
    {
        auto* value = (unsafe::Value*) in;
        bool before = jl_gc_is_enabled();
        jl_gc_enable(false);

        detail::gc_init();

        static unsafe::Function* jluna_add_to_heap = get_function(jl_main_module, "__jluna_add_to_heap"_sym);

        auto res = jl_unbox_uint64(call(jluna_add_to_heap, jl_box_uint64((UInt64) value)));
        jl_gc_enable(before);
        return res;
    }

    template<is_julia_value_pointer... Ts, std::enable_if_t<(sizeof...(Ts) > 2), bool>>
    std::vector<uint64_t> gc_preserve(Ts... values)
    {
        std::vector<uint64_t> out;
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
            for (uint64_t i = 0; i < types.size(); ++i)
                types.at(i) = (jl_value_t*) jl_uint64_type;

            return jl_apply_tuple_type_v(types.data(), types.size());
        }();

        #if JULIA_VERSION_MAJOR >= 2 or JULIA_VERSION_MINOR >= 10
            auto* tuple = jl_new_struct((jl_datatype_t*) tuple_type, jl_box_uint64(size_per_dimension)...);
        #else
            auto* tuple = jl_new_struct(tuple_type, jl_box_uint64(size_per_dimension)...);
        #endif

        auto* res = jl_new_array(jl_apply_array_type(value_type, sizeof...(Dims)), tuple);
        jl_gc_enable(before);
        return res;
    }

    template<typename... Dims, std::enable_if_t<(sizeof...(Dims) > 1), bool>>
    unsafe::Array* new_array_from_data(unsafe::Value* value_type, void* data, Dims... size_per_dimension)
    {
        bool before = jl_gc_is_enabled();
        jl_gc_enable(false);
        static auto* tuple_type = [&](){
            std::array<jl_value_t*, sizeof...(Dims)> types;
            for (uint64_t i = 0; i < types.size(); ++i)
                types.at(i) = (jl_value_t*) jl_uint64_type;

            return jl_apply_tuple_type_v(types.data(), types.size());
        }();

        #if JULIA_VERSION_MAJOR >= 2 or JULIA_VERSION_MINOR >= 10
            auto* tuple = jl_new_struct((jl_datatype_t*) tuple_type, jl_box_uint64(size_per_dimension)...);
        #else
            auto* tuple = jl_new_struct(tuple_type, jl_box_uint64(size_per_dimension)...);
        #endif

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
            for (uint64_t i = 0; i < types.size(); ++i)
                types.at(i) = (jl_value_t*) jl_uint64_type;

            return jl_apply_tuple_type_v(types.data(), types.size());
        }();

        static jl_function_t* get_value_type_of_array = get_function("jluna"_sym, "get_value_type_of_array"_sym);

        #if JULIA_VERSION_MAJOR >= 2 or JULIA_VERSION_MINOR >= 10
            auto* tuple = jl_new_struct((jl_datatype_t*) tuple_type, jl_box_uint64(size_per_dimension)...);
        #else
            auto* tuple = jl_new_struct(tuple_type, jl_box_uint64(size_per_dimension)...);
        #endif

        auto* res = jl_reshape_array(jl_apply_array_type(unsafe::call(get_value_type_of_array, array), sizeof...(Dims)), array, tuple);
        override_array(array, res);
        jl_gc_enable(before);
    }

    template<typename... Index, std::enable_if_t<(sizeof...(Index) > 2), bool>>
    unsafe::Value* get_index(unsafe::Array* array, Index... index_per_dimension)
    {
        std::array<uint64_t, sizeof...(Index)> indices = {uint64_t(index_per_dimension)...};
        uint64_t index = 0;
        uint64_t mul = 1;

        for (uint64_t i = 0; i < static_cast<uint64_t>(array->flags.ndims); ++i)
        {
            index += (indices.at(i)) * mul;
            uint64_t dim = jl_array_dim(array, i);
            mul *= dim;
        }

        return jl_arrayref(array, index);
    }

    inline unsafe::Value* get_index(unsafe::Array* array, uint64_t i)
    {
        return jl_arrayref(array, i);
    }

    inline unsafe::Value* get_index(unsafe::Array* array, uint64_t i, uint64_t j)
    {
        return jl_arrayref(array, i + jl_array_dim(array, 0) * j);
    }

    template<typename... Index, std::enable_if_t<(sizeof...(Index) > 2), bool>>
    void set_index(unsafe::Array* array, unsafe::Value* new_value, Index... index_per_dimension)
    {
        std::array<uint64_t, sizeof...(Index)> indices = {uint64_t(index_per_dimension)...};
        uint64_t index = 0;
        uint64_t mul = 1;

        for (uint64_t i = 0; i < static_cast<uint64_t>(array->flags.ndims); ++i)
        {
            index += (indices.at(i)) * mul;
            uint64_t dim = jl_array_dim(array, i);
            mul *= dim;
        }

        jl_arrayset(array, new_value, index);
    }

    inline void set_index(unsafe::Array* array, unsafe::Value* value, uint64_t i)
    {
        jl_arrayset(array, value, i);
    }

    template<typename T>
    void set_array_data(unsafe::Array* array, T* new_data, uint64_t new_size)
    {
        array->data = new_data;
        jl_gc_wb(array, new_data);
        array->length = new_size;
    }

    inline void push_back(unsafe::Array* arr, unsafe::Value* value)
    {
        jl_array_grow_end(arr, 1);
        jl_arrayset(arr, value, jl_array_len(arr) - 1);
    }

    inline void push_front(unsafe::Array* arr, unsafe::Value* value)
    {
        jl_array_grow_beg(arr, 1);
        jl_arrayset(arr, value, 0);
    }

    template<is_julia_value T>
    unsafe::Value* unsafe_box(T* in)
    {
        return in;
    }

    template<is<bool> T>
    unsafe::Value* unsafe_box(bool in)
    {
        return jl_box_bool(in);
    }

    template<is<char> T>
    unsafe::Value* unsafe_box(char in)
    {
        return jl_box_char(in);
    }

    template<is<int8_t> T>
    unsafe::Value* unsafe_box(int8_t in)
    {
        return jl_box_int8(in);
    }

    template<is<int16_t> T>
    unsafe::Value* unsafe_box(int16_t in)
    {
        return jl_box_int16(in);
    }

    template<is<int32_t> T>
    unsafe::Value* unsafe_box(int32_t in)
    {
        return jl_box_int32(in);
    }
    
    template<is<int64_t> T>
    unsafe::Value* unsafe_box(int64_t in)
    {
        return jl_box_int64(in);
    }

    template<is<uint8_t> T>
    unsafe::Value* unsafe_box(uint8_t in)
    {
        return jl_box_uint8(in);
    }

    template<is<uint16_t> T>
    unsafe::Value* unsafe_box(uint16_t in)
    {
        return jl_box_uint16(in);
    }

    template<is<uint32_t> T>
    unsafe::Value* unsafe_box(uint32_t in)
    {
        return jl_box_uint32(in);
    }

    template<is<uint64_t> T>
    unsafe::Value* unsafe_box(uint64_t in)
    {
        return jl_box_uint64(in);
    }

    template<is<float> T>
    unsafe::Value* unsafe_box(float in)
    {
        return jl_box_float32(in);
    }
    
    template<is<double> T>
    unsafe::Value* unsafe_box(double in)
    {
        return jl_box_float64(in);
    }

    template<is<std::string> T>
    unsafe::Value* unsafe_box(const T& in)
    {
        return jl_array_to_string(jl_ptr_to_array_1d(jl_apply_array_type((jl_value_t*) jl_char_type, 1), in.data(), in.size(), 0));
    }

    template<is<bool> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return jl_unbox_bool(in);
    }

    template<is<char> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return char(jl_unbox_uint32(in));
    }

    template<is<int8_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<int8_t*>(in));
    }
    
    template<is<int16_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<int16_t*>(in));
    }
    
    template<is<int32_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<int32_t*>(in));
    }
    
    template<is<int64_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<int64_t*>(in));
    }
    
    template<is<uint8_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<uint8_t*>(in));
    }
    
    template<is<uint16_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<uint16_t*>(in));
    }
    
    template<is<uint32_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<uint32_t*>(in));
    }
    
    template<is<uint64_t> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<uint64_t*>(in));
    }
    
    template<is<Float32> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<Float32*>(in));
    }
    
    template<is<Float64> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return *(reinterpret_cast<Float64*>(in));
    }

    template<is<std::string> T>
    T unsafe_unbox(unsafe::Value* in)
    {
        return std::string(jl_string_ptr(in));
    }
}