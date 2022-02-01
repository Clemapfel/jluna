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
#include <include/utilities.hpp>
#include <.src/include_julia.inl>

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

        jl_eval_string(R"(
            if (tryparse(Float32, SubString(string(VERSION), 1, 3)) < 1.8)
                throw(ErrorException("[ERROR] jluna requires julia version 1.7.0 or higher"))
            end
        )");

        jl_eval_string(jluna::detail::include);
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

        jluna::Main = Proxy((Any*) jl_main_module, nullptr);
        jluna::Base = Main["Base"];
        jluna::Core = Main["Core"];

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

    Proxy new_undef(const std::string& variable_name)
    {
        State::safe_script(variable_name + " = undef");
        return Main[variable_name];
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
}

