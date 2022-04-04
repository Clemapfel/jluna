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
#include <.src/include_julia.inl>

namespace jluna::detail
{

}

namespace jluna::State
{

    
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

    Proxy safe_eval(const std::string& command)
    {
        jl_value_t* value = _content.value();
        bool is_module = jl_isa(value, (unsafe::Value*) jl_module_type);
        return Proxy(safe_eval(command, is_module ? value : jl_main_module));
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

    void set_garbage_collector_enabled(bool b)
    {
        jluna::throw_if_uninitialized();

        jl_gc_enable(b);
    }
}

namespace jluna::State::detail
{

}

