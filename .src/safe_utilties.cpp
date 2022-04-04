// 
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

#include <include/safe_utilities.hpp>
#include <include/unsafe_utilities.hpp>
#include <.src/include_julia.inl>
#include <include/type.hpp>
#include <include/module.hpp>

namespace jluna
{
    void set_c_adapter_path(const std::string& path)
    {
        jluna::detail::c_adapter_path_override = path;
    }

    void initialize()
    {
        initialize("");
    }

    void initialize(const std::string& path)
    {
        if (path.empty())
            jl_init();
        else
            jl_init_with_image(path.c_str(), nullptr);

        { // execute jluna julia code in pieces
            using namespace jluna::detail;
            std::stringstream str;
            str << module_start;
                str << include_01;
                str << include_02;
                str << include_03;
                str << include_04;
            str << module_end;

            str << include_05;

            jl_eval_string(str.str().c_str());
        }

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
        str << "jluna._cppcall.eval(:(_library_name = \"";
        str << (jluna::detail::c_adapter_path_override.empty() ?  jluna::detail::c_adapter_path : jluna::detail::c_adapter_path_override);
        str << "\"))";

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

    unsafe::Value* safe_eval(const std::string& code, unsafe::Module* module)
    {
        static auto* eval = unsafe::get_function(jl_base_module, "eval"_sym);

        const std::string a = "quote ";
        const std::string b = " end";

        auto* quote = jl_eval_string((a + code + b).c_str());
        if (quote == nullptr)
        {
            auto* exc = jl_exception_occurred();
            std::stringstream str;
            str << "In jluna::safe_eval: " << jl_typeof_str(exc) << " in expression\n\t" << code << "\n"
                << jl_string_ptr(jl_get_nth_field(exc, 0)) << std::endl;
            throw JuliaException(exc, str.str());
        }

        jl_set_nth_field(quote, 0, (unsafe::Value*) "toplevel"_sym);
        return safe_call(eval, module, quote);
    }

    unsafe::Value* safe_eval_file(const std::string& path, unsafe::Module* module)
    {
        static auto* include = unsafe::get_function(jl_base_module, "include"_sym);
        assert(false && "TODO");
    }

    void collect_garbage()
    {
        throw_if_uninitialized();

        static jl_function_t* gc = jl_get_function((jl_module_t*) jl_eval_string("return Base.GC"), "gc");

        bool before = jl_gc_is_enabled();
        jl_gc_enable(true);
        jl_call0(gc);
        jl_gc_enable(before);
    }
}

namespace jluna::detail
{
    void on_exit()
    {
        jl_eval_string(R"([JULIA][LOG] Shutting down...)");
        jl_eval_string("jluna.memory_handler.force_free()");
        jl_atexit_hook(0);
    }

    size_t create_reference(unsafe::Value* in)
    {
        throw_if_uninitialized();
        static unsafe::Function* create_reference = unsafe::get_function((unsafe::Module*) jl_eval_string("jluna.memory_handler"), "create_reference"_sym);

        size_t res = -1;

        gc_pause;
        unsafe::Value* value;
        res = jl_unbox_uint64(jluna::safe_call(create_reference, in));
        gc_unpause;
        return res;
    }

    unsafe::Value* get_reference(size_t key)
    {
        static unsafe::Function* get_reference = unsafe::get_function((unsafe::Module*) jl_eval_string("jluna.memory_handler"), "get_reference"_sym);
        return jluna::safe_call(get_reference, jl_box_uint64(static_cast<size_t>(key)));
    }

    void free_reference(size_t key)
    {
        throw_if_uninitialized();
        static unsafe::Function* free_reference = unsafe::get_function((unsafe::Module*) jl_eval_string("jluna.memory_handler"), "free_reference"_sym);
        jluna::safe_call(free_reference, jl_box_uint64(static_cast<size_t>(key)));
    }

    void initialize_types()
    {
        auto unroll = [](const std::string& name) -> jl_datatype_t*
        {
            return (jl_datatype_t*) jl_eval_string(("return jluna.unroll_type(" + name + ")").c_str());
        };

        gc_pause;
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
        gc_unpause;
    }

    void initialize_modules()
    {
        Main = Module(jl_main_module);
        Core = Module(jl_core_module);
        Base = Module(jl_base_module);
    }
}

