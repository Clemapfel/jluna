// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <include/julia_wrapper.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

#include <include/state.hpp>
#include <include/exceptions.hpp>
#include <include/julia_extension.hpp>
#include <include/proxy.hpp>
#include <include/module.hpp>
#include <include/type.hpp>
#include "include_julia.inl"

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

        std::stringstream str;
        str << "jluna._cppcall.eval(:(_library_name = \"" << jluna::detail::c_adapter_path << "/libjluna_c_adapter.so\"))";

        jl_eval_string(str.str().c_str());
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

        jl_gc_pause;
        auto* result = jl_eval_string(str.str().c_str());
        if (jl_exception_occurred() or jl_unbox_bool(jl_eval_string("jluna.exception_handler.has_exception_occurred()")))
        {
            std::cerr << "exception in jluna::State::safe_script for expression:\n\"" << command << "\"\n" << std::endl;
            forward_last_exception();
        }
        jl_gc_unpause;
        return Proxy(result, nullptr);
    }

    Proxy eval(const std::string& command) noexcept
    {
        jluna::throw_if_uninitialized();

        static jl_function_t* unsafe_call = jl_find_function("jluna.exception_handler", "unsafe_call");
        jl_gc_pause;
        auto* res = jl_call1(unsafe_call, jl_quote(command.c_str()));
        jl_gc_unpause;

        return Proxy(res);
    }

    Proxy safe_eval(const std::string& command)
    {
        jluna::throw_if_uninitialized();

        static jl_function_t* safe_call = jl_find_function("jluna.exception_handler", "safe_call");
        static jl_function_t* has_exception_occurred = jl_find_function("jluna.exception_handler", "has_exception_occurred");

        jl_gc_pause;
        auto* result = jl_call1(safe_call, jl_quote(command.c_str()));

        if (jl_exception_occurred() or jl_unbox_bool(jl_call0(has_exception_occurred)))
        {
            std::cerr << "exception in jluna::State::safe_eval for expression:\n\"" << command << "\"\n" << std::endl;
            forward_last_exception();
        }
        jl_gc_unpause;
        return Proxy(result, nullptr);
    }

    Proxy eval_file(const std::string& path) noexcept
    {
        std::fstream file;
        file.open(path);

        std::stringstream str;
        str << file.rdbuf();

        file.close();
        return eval(str.str());
    }

    Proxy safe_eval_file(const std::string& path) noexcept
    {
        std::fstream file;
        file.open(path);

        std::stringstream str;
        str << file.rdbuf();

        file.close();
        return safe_eval(str.str());
    }
    
    template<typename T>
    T safe_return(const std::string& full_name)
    {
        jl_gc_pause;
        auto* res = jl_eval_string(("return " + full_name).c_str());
        forward_last_exception();
        jl_gc_unpause;
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

        jl_gc_pause;
        Any* value;
        res = jl_unbox_uint64(jluna::safe_call(create_reference, in));
        jl_gc_unpause;

        return res;
    }

    Any * get_reference(size_t key)
    {
        static Function* get_reference = jl_find_function("jluna.memory_handler", "get_reference");
        return jluna::safe_call(get_reference, jl_box_uint64(static_cast<size_t>(key)));
    }

    void free_reference(size_t key)
    {
        if (key == 0)
            return;

        jluna::throw_if_uninitialized();
        static Function* free_reference = jl_find_function("jluna.memory_handler", "free_reference");

        jl_gc_pause;
        jluna::safe_call(free_reference, jl_box_uint64(static_cast<size_t>(key)));
        jl_gc_unpause;
    }

    void initialize_types()
    {
        auto unroll = [](const std::string& name) -> jl_datatype_t* 
        {
            return (jl_datatype_t*) jl_eval_string(("return jluna.unroll_type(" + name + ")").c_str());
        };

        jl_gc_pause;
        AbstractArray_t = Type(unroll("Core.AbstractArray"));
        AbstractChar_t = Type(unroll("Core.AbstractChar"));
        AbstractFloat_t = Type(unroll("Core.AbstractFloat"));
        AbstractString_t = Type(unroll("Core.AbstractString"));
        Any_t = Type(unroll("Core.Any"));
        Array_t = Type(unroll("Core.Array"));
        Bool_t = Type(unroll("Core.Bool"));
        Char_t = Type(unroll("Core.Char"));
        DataType_t = Type(unroll("Core.DataType"));
        DenseArray_t = Type(unroll("Core.DenseArray"));
        Exception_t = Type(unroll("Core.Exception"));
        Expr_t = Type(unroll("Core.Expr"));
        Float16_t = Type(unroll("Core.Float16"));
        Float32_t = Type(unroll("Core.Float32"));
        Float64_t = Type(unroll("Core.Float64"));
        Function_t = Type(unroll("Core.Function"));
        GlobalRef_t = Type(unroll("Core.GlobalRef"));
        IO_t = Type(unroll("Core.IO"));
        Int128_t = Type(unroll("Core.Int128"));
        Int16_t = Type(unroll("Core.Int16"));
        Int32_t = Type(unroll("Core.Int32"));
        Int64_t = Type(unroll("Core.Int64"));
        Int8_t = Type(unroll("Core.Int8"));
        Integer_t = Type(unroll("Core.Integer"));
        LineNumberNode_t = Type(unroll("Core.LineNumberNode"));
        Method_t = Type(unroll("Core.Method"));
        Module_t = Type(unroll("Core.Module"));
        NTuple_t = Type(unroll("Core.NTuple"));
        NamedTuple_t = Type(unroll("Core.NamedTuple"));
        Nothing_t = Type(unroll("Core.Nothing"));
        Number_t = Type(unroll("Core.Number"));
        Pair_t = Type(unroll("Core.Pair"));
        Ptr_t = Type(unroll("Core.Ptr"));
        QuoteNode_t = Type(unroll("Core.QuoteNode"));
        Real_t = Type(unroll("Core.Real"));
        Ref_t = Type(unroll("Core.Ref"));
        Signed_t = Type(unroll("Core.Signed"));
        String_t = Type(unroll("Core.String"));
        Symbol_t = Type(unroll("Core.Symbol"));
        Task_t = Type(unroll("Core.Task"));
        Tuple_t = Type(unroll("Core.Tuple"));
        Type_t = Type(unroll("Core.Type"));
        TypeVar_t = Type(unroll("Core.TypeVar"));
        UInt128_t = Type(unroll("Core.UInt128"));
        UInt16_t = Type(unroll("Core.UInt16"));
        UInt32_t = Type(unroll("Core.UInt32"));
        UInt64_t = Type(unroll("Core.UInt64"));
        UInt8_t = Type(unroll("Core.UInt8"));
        UndefInitializer_t = Type(unroll("Core.UndefInitializer"));
        Union_t = Type(unroll("Core.Union"));
        UnionAll_t = Type(unroll("Core.UnionAll"));
        UnionEmpty_t = Type(unroll("Union{}"));
        Unsigned_t = Type(unroll("Core.Unsigned"));
        VecElement_t = Type(unroll("Core.VecElement"));
        WeakRef_t = Type(unroll("Core.WeakRef"));
        jl_gc_unpause;
    }

    void initialize_modules()
    {
        Main = Module(jl_main_module);
        Core = Module(jl_core_module);
        Base = Module(jl_base_module);
    }
}

