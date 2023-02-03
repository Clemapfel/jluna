//
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

#include <include/safe_utilities.hpp>
#include <include/unsafe_utilities.hpp>
#include <.src/include_julia.inl>
#include <include/type.hpp>
#include <include/module.hpp>
#include <mutex>

namespace jluna
{
    namespace detail
    {
        static inline std::mutex initialize_lock = std::mutex();
    }

    void initialize(
        size_t n_threads,
        bool suppress_log,
        const std::string& jluna_shared_library_path,
        const std::string& julia_bindir,
        const std::string& image_path
    )
    {
        static bool is_initialized = false;

        detail::initialize_lock.lock();

        if (is_initialized)
        {
            detail::initialize_lock.unlock();
            return;
        }

        #ifdef _WIN32
        {
            std::stringstream str;
            str << "JULIA_NUM_THREADS=" << (n_threads == 0 ? "auto" : std::to_string(n_threads)) << std::endl;
            _putenv(str.str().c_str());
        }
        #else
            setenv("JULIA_NUM_THREADS", std::string(n_threads == 0 ? "auto" : std::to_string(n_threads)).c_str(), 1);
        #endif

        detail::_num_threads = n_threads;
        if (julia_bindir.empty() and image_path.empty())
            jl_init();
        else if (image_path.empty())
            jl_init_with_image(julia_bindir.c_str(), nullptr);
        else if (julia_bindir.empty())
            jl_init_with_image(nullptr, image_path.c_str());
        else
            jl_init_with_image(julia_bindir.c_str(), image_path.c_str());

        forward_last_exception();

        bool success = jl_unbox_bool(jl_eval_string(detail::julia_source));
        forward_last_exception();

        assert(success);

        jl_eval_string(R"(
            begin
                local version = tryparse(Float32, SubString(string(VERSION), 1, 3))
                if (version < 1.7)
                    local message = "jluna requires julia v1.7.0 or higher, but v" * string(VERSION) * " was detected. Please download the latest julia release at https://julialang.org/downloads/#current_stable_release, set JULIA_BINDIR accordingly, then recompile jluna using cmake. For more information, visit https://github.com/Clemapfel/jluna/blob/master/README.md#troubleshooting"
                    throw(AssertionError(message))
                end
            end
        )");
        forward_last_exception();

        std::stringstream str;
        str << "jluna.cppcall.eval(:(const _lib = \""
            << (jluna_shared_library_path == "" ? jluna::detail::shared_library_name : jluna_shared_library_path)
            << "\"))";

        jl_eval_string(str.str().c_str());
        forward_last_exception();

        detail::initialize_modules();
        detail::initialize_types();

        if (suppress_log)
        {
            safe_eval(R"(
                if !isdefined(Main, :jluna)
                    print("[JULIA]")
                    Base.printstyled("[ERROR] initialization failed.\n"; color = :red)
                    throw(AssertionError(("[JULIA][ERROR] initialization failed.")))
                end
            )");
        }
        else
        {
            safe_eval(R"(
                if isdefined(Main, :jluna)
                    print("[JULIA][LOG] ")
                    Base.printstyled("initialization successful (" * string(Threads.nthreads()) * " thread(s)).\n"; color = :green)
                else
                    print("[JULIA]")
                    Base.printstyled("[ERROR] initialization failed.\n"; color = :red)
                    throw(AssertionError(("[JULIA][ERROR] initialization failed.")))
                end
            )");
        }

        std::atexit(&jluna::detail::on_exit);
        is_initialized = true;

        detail::initialize_lock.unlock();
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
        auto* jl_string = box<std::string>(path);
        auto jl_string_id = unsafe::gc_preserve(jl_string);

        static auto* include = unsafe::get_function(jl_base_module, "include"_sym);
        auto* out = jluna::safe_call(include, (unsafe::Value*) module, jl_string);
        unsafe::gc_release(jl_string_id);
        return out;
    }

    void collect_garbage()
    {
        jl_gc_collect(JL_GC_FULL);
    }

    unsafe::Value* undef()
    {
        static auto* type = jl_eval_string("return UndefInitializer");
        return jl_new_bits(type, nullptr);
    }

    unsafe::Value* nothing()
    {
        return jl_new_bits((unsafe::Value*) jl_nothing_type, nullptr);
    }

    unsafe::Value* missing()
    {
        static auto* type = jl_eval_string("return Missing");
        return jl_new_bits(type, nullptr);
    }
}

namespace jluna::detail
{
    void on_exit()
    {
        jl_eval_string(R"([JULIA][LOG] Shutting down...)");
        jl_eval_string("jluna.memory_handler.force_free()");
        jl_eval_string("jluna.gc_sentinel.shutdown()");
        jl_atexit_hook(0);
    }

    size_t create_reference(unsafe::Value* in)
    {
        throw_if_uninitialized();
        static unsafe::Function* create_reference = unsafe::get_function((unsafe::Module*) jl_eval_string("jluna.memory_handler"), "create_reference"_sym);

        size_t res = -1;

        gc_pause;
        res = jl_unbox_uint64(jluna::safe_call(create_reference, jl_box_voidpointer((void*) in)));
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
        Missing_t = Type(unroll("Base.Missing"));
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

