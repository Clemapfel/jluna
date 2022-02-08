// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <julia.h>

#include <sstream>
#include <iostream>

#include <include/state.hpp>
#include <include/exceptions.hpp>
#include <include/julia_extension.hpp>
#include <include/proxy.hpp>
#include <.src/include_julia.inl>
#include <include/module.hpp>
#include <include/type.hpp>

namespace jluna::detail
{
    static void on_exit()
    {
        jl_eval_string(R"([JULIA][LOG] Shutting down...)");
        jl_eval_string("jluna.memory_handler.force_free()");
        jl_atexit_hook(0);
    }
}

namespace jluna::State
{
    void initialize()
    {
        initialize("");
    }

    void initialize(const std::string& path)
    {
        if (path.empty())
            jl_init();
        else
            jl_init_with_image(path.c_str(), NULL);

        jl_eval_string(jluna::detail::include);
        forward_last_exception();


        jl_eval_string(R"(
            begin
                local version = tryparse(Float32, SubString(string(VERSION), 1, 3))
                if (version < 1.7)
                    local message = "jluna requires julia v1.7.0 or higher, but v" * string(VERSION) * " was detected. Please download the latest julia release at https://julialang.org/downloads/#current_stable_release, set JULIA_PATH accordingly, then recompile jluna using cmake. For more information, visit https://github.com/Clemapfel/jluna/blob/master/README.md#troubleshooting"
                    throw(AssertionError(message))
                end
            end
        )");
        forward_last_exception();

        jl_eval_string(("jluna._cppcall.eval(:(_library_name = \"" + std::string(RESOURCE_PATH) + "/libjluna_c_adapter.so\"))").c_str());
        forward_last_exception();

        jl_eval_string(R"(
            if isdefined(Main, :jluna) & jluna._cppcall.verify_library()
                print("[JULIA][LOG] ")
                Base.printstyled("initialization successfull.\n"; color = :green)
            else
                print("[JULIA]")
                Base.printstyled("[ERROR] initialization failed.\n"; color = :red)
                throw(AssertionError(("[JULIA][ERROR] initialization failed.")))
            end
        )");
        forward_last_exception();

        detail::initialize_modules();
        detail::initialize_types();

        std::atexit(&jluna::detail::on_exit);
    }
    
    Proxy script(const std::string& command) noexcept
    {
        jluna::throw_if_uninitialized();

        std::stringstream str;
        str << "jluna.exception_handler.unsafe_call(quote " << command << " end)" << std::endl;
        return Proxy(jl_eval_string(str.str().c_str()), nullptr);
    }
    
    Proxy safe_script(const std::string& command)
    {
        jluna::throw_if_uninitialized();

        std::stringstream str;
        str << "jluna.exception_handler.safe_call(quote " << command << " end)" << std::endl;
        auto* result = jl_eval_string(str.str().c_str());
        if (jl_exception_occurred() or jl_unbox_bool(jl_eval_string("jluna.exception_handler.has_exception_occurred()")))
        {
            std::cerr << "exception in jluna::State::safe_script for expression:\n\"" << command << "\"\n" << std::endl;
            forward_last_exception();
        }
        return Proxy(result, nullptr);
    }
    
    template<typename T>
    T safe_return(const std::string& full_name)
    {
        auto* res = jl_eval_string(("return " + full_name).c_str());
        forward_last_exception();
        return unbox<T>(res);
    }

    void collect_garbage()
    {
        jluna::throw_if_uninitialized();

        static jl_function_t* gc = jl_get_function((jl_module_t*) jl_eval_string("return Base.GC"), "gc");

        bool before = jl_gc_is_enabled();

        jl_gc_enable(true);
        jl_call0(gc);
        jl_gc_enable(before);
    }

    void set_garbage_collector_enabled(bool b)
    {
        jluna::throw_if_uninitialized();

        jl_gc_enable(b);
    }
}

namespace jluna::State::detail
{
    size_t create_reference(Any* in)
    {
        jluna::throw_if_uninitialized();
        static Function* create_reference = jl_find_function("jluna.memory_handler", "create_reference");

        if (in == nullptr)
            return 0;

        size_t res = -1;
        auto before = jl_gc_is_enabled();
        jl_gc_enable(false);
        Any* value;
        res = jl_unbox_uint64(jluna::safe_call(create_reference, in));
        jl_gc_enable(before);

        return res;
    }

    Any * get_reference(size_t key)
    {
        static Function* get_reference = jl_find_function("jluna.memory_handler", "get_reference");
        return jluna::safe_call(get_reference, jl_box_uint64(reinterpret_cast<size_t>(key)));
    }

    void free_reference(size_t key)
    {
        if (key == 0)
            return;

        jluna::throw_if_uninitialized();
        static Function* free_reference = jl_find_function("jluna.memory_handler", "free_reference");

        auto before = jl_gc_is_enabled();
        jl_gc_enable(false);
        jluna::safe_call(free_reference, jl_box_uint64(reinterpret_cast<size_t>(key)));
        jl_gc_enable(before);
    }

    void initialize_types()
    {
        AbstractArray_t = Type((jl_datatype_t*) jl_eval_string("return Core.AbstractArray"));
        AbstractChar_t = Type((jl_datatype_t*) jl_eval_string("return Core.AbstractChar"));
        AbstractFloat_t = Type((jl_datatype_t*) jl_eval_string("return Core.AbstractFloat"));
        AbstractString_t = Type((jl_datatype_t*) jl_eval_string("return Core.AbstractString"));
        Any_t = Type((jl_datatype_t*) jl_eval_string("return Core.Any"));
        Array_t = Type((jl_datatype_t*) jl_eval_string("return Core.Array"));
        Bool_t = Type((jl_datatype_t*) jl_eval_string("return Core.Bool"));
        Char_t = Type((jl_datatype_t*) jl_eval_string("return Core.Char"));
        DataType_t = Type((jl_datatype_t*) jl_eval_string("return Core.DataType"));
        DenseArray_t = Type((jl_datatype_t*) jl_eval_string("return Core.DenseArray"));
        Exception_t = Type((jl_datatype_t*) jl_eval_string("return Core.Exception"));
        Expr_t = Type((jl_datatype_t*) jl_eval_string("return Core.Expr"));
        Float16_t = Type((jl_datatype_t*) jl_eval_string("return Core.Float16"));
        Float32_t = Type((jl_datatype_t*) jl_eval_string("return Core.Float32"));
        Float64_t = Type((jl_datatype_t*) jl_eval_string("return Core.Float64"));
        Function_t = Type((jl_datatype_t*) jl_eval_string("return Core.Function"));
        GlobalRef_t = Type((jl_datatype_t*) jl_eval_string("return Core.GlobalRef"));
        IO_t = Type((jl_datatype_t*) jl_eval_string("return Core.IO"));
        Int128_t = Type((jl_datatype_t*) jl_eval_string("return Core.Int128"));
        Int16_t = Type((jl_datatype_t*) jl_eval_string("return Core.Int16"));
        Int32_t = Type((jl_datatype_t*) jl_eval_string("return Core.Int32"));
        Int64_t = Type((jl_datatype_t*) jl_eval_string("return Core.Int64"));
        Int8_t = Type((jl_datatype_t*) jl_eval_string("return Core.Int8"));
        Integer_t = Type((jl_datatype_t*) jl_eval_string("return Core.Integer"));
        LineNumberNode_t = Type((jl_datatype_t*) jl_eval_string("return Core.LineNumberNode"));
        Method_t = Type((jl_datatype_t*) jl_eval_string("return Core.Method"));
        Module_t = Type((jl_datatype_t*) jl_eval_string("return Core.Module"));
        NTuple_t = Type((jl_datatype_t*) jl_eval_string("return Core.NTuple"));
        NamedTuple_t = Type((jl_datatype_t*) jl_eval_string("return Core.NamedTuple"));
        Nothing_t = Type((jl_datatype_t*) jl_eval_string("return Core.Nothing"));
        Number_t = Type((jl_datatype_t*) jl_eval_string("return Core.Number"));
        Pair_t = Type((jl_datatype_t*) jl_eval_string("return Core.Pair"));
        Ptr_t = Type((jl_datatype_t*) jl_eval_string("return Core.Ptr"));
        QuoteNode_t = Type((jl_datatype_t*) jl_eval_string("return Core.QuoteNode"));
        Real_t = Type((jl_datatype_t*) jl_eval_string("return Core.Real"));
        Ref_t = Type((jl_datatype_t*) jl_eval_string("return Core.Ref"));
        Signed_t = Type((jl_datatype_t*) jl_eval_string("return Core.Signed"));
        String_t = Type((jl_datatype_t*) jl_eval_string("return Core.String"));
        Symbol_t = Type((jl_datatype_t*) jl_eval_string("return Core.Symbol"));
        Task_t = Type((jl_datatype_t*) jl_eval_string("return Core.Task"));
        Tuple_t = Type((jl_datatype_t*) jl_eval_string("return Core.Tuple"));
        Type_t = Type((jl_datatype_t*) jl_eval_string("return Core.Type"));
        TypeVar_t = Type((jl_datatype_t*) jl_eval_string("return Core.TypeVar"));
        UInt128_t = Type((jl_datatype_t*) jl_eval_string("return Core.UInt128"));
        UInt16_t = Type((jl_datatype_t*) jl_eval_string("return Core.UInt16"));
        UInt32_t = Type((jl_datatype_t*) jl_eval_string("return Core.UInt32"));
        UInt64_t = Type((jl_datatype_t*) jl_eval_string("return Core.UInt64"));
        UInt8_t = Type((jl_datatype_t*) jl_eval_string("return Core.UInt8"));
        UndefInitializer_t = Type((jl_datatype_t*) jl_eval_string("return Core.UndefInitializer"));
        Union_t = Type((jl_datatype_t*) jl_eval_string("return Core.Union"));
        UnionAll_t = Type((jl_datatype_t*) jl_eval_string("return Core.UnionAll"));
        Unsigned_t = Type((jl_datatype_t*) jl_eval_string("return Core.Unsigned"));
        VecElement_t = Type((jl_datatype_t*) jl_eval_string("return Core.VecElement"));
        WeakRef_t = Type((jl_datatype_t*) jl_eval_string("return Core.WeakRef"));
    }

    void initialize_modules()
    {
        Main = Module(jl_main_module);
        Core = Module(jl_core_module);
        Base = Module(jl_base_module);
    }
}

