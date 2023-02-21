// 
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>
#include <include/unsafe_utilities.hpp>
#include <.src/include_julia.inl>

#include <mutex>

namespace jluna::detail
{
    static inline size_t _num_threads = 1;

    void initialize_modules();
    void initialize_types();
    void on_exit();

    constexpr char _id_marker = '#';

    size_t create_reference(unsafe::Value* in);
    unsafe::Value* get_reference(size_t key);
    void free_reference(size_t key);

    inline std::mutex initialize_lock = std::mutex();
}

namespace jluna
{
    template<is_julia_value_pointer... Args_t>
    unsafe::Value* safe_call(unsafe::Function* function, Args_t... in)
    {
        throw_if_uninitialized();

        static auto* jl_safe_call = unsafe::get_function("jluna"_sym, "safe_call"_sym);

        static std::array<unsafe::Value*, sizeof...(Args_t) + 1> args;
        static auto set = [&](size_t i, unsafe::Value* x) {args[i] = x;};

        args[0] = (unsafe::Value*) function;

        size_t i = 1;
        (set(i++, (unsafe::Value*) in), ...);

        auto* tuple_res = jl_call(jl_safe_call, args.data(), args.size());

        if (jl_unbox_bool(jl_get_nth_field(tuple_res, 1)))
            throw JuliaException(jl_get_nth_field(tuple_res, 2), jl_string_ptr(jl_get_nth_field(tuple_res, 3)));

        auto* res = jl_get_nth_field(tuple_res, 0);
        return res;
    }

    template<is_julia_value_pointer... Ts>
    void println(Ts... in)
    {
        static auto* jl_println = unsafe::get_function(jl_base_module, "println"_sym);
        safe_call(jl_println, in...);
    }

    inline unsafe::Value* as_julia_pointer(unsafe::Value* in)
    {
        static auto* forward_as_pointer = unsafe::get_function("jluna"_sym, "forward_as_pointer"_sym);
        return safe_call(forward_as_pointer, jl_typeof(in), jl_box_voidpointer((void*) in));
    }

    inline void initialize(
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
            /*
            std::stringstream str;
            str << "JULIA_NUM_THREADS=" << (n_threads == 0 ? "auto" : std::to_string(n_threads)) << std::endl;
            auto i = _putenv(str.str().c_str());
            assert(i == 1);
            */
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

        bool success = jl_unbox_bool(jl_eval_string(detail::julia_source.c_str()));
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
}