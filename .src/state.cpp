// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <julia.h>

#include <include/state.hpp>
#include <include/exceptions.hpp>
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


        jl_eval_string(detail::include);
        forward_last_exception();

        jl_eval_string(R"(
            if isdefined(Main, :jluna) # && _cppcall.verify_library()
                print("[JULIA][LOG] ")
                Base.printstyled("initialization successfull.\n"; color = :green)
            else
                print("[JULIA]")
                Base.printstyled("[ERROR] initialization failed.\n"; color = :red)
                throw(AssertionError(("[JULIA][ERROR] initialization failed.")))
            end
        )");
        forward_last_exception();

        std::atexit(&detail::on_exit);
    }
}

