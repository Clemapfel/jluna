//
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

#include <include/safe_utilities.hpp>
#include <include/unsafe_utilities.hpp>
#include <include/type.hpp>
#include <include/module.hpp>
#include <mutex>

namespace jluna
{
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

    uint64_t create_reference(unsafe::Value* in)
    {
        throw_if_uninitialized();
        static unsafe::Function* create_reference = unsafe::get_function((unsafe::Module*) jl_eval_string("jluna.memory_handler"), "create_reference"_sym);

        uint64_t res = -1;

        gc_pause;
        res = jl_unbox_uint64(jluna::safe_call(create_reference, jl_box_voidpointer((void*) in)));
        gc_unpause;
        return res;
    }

    unsafe::Value* get_reference(uint64_t key)
    {
        static unsafe::Function* get_reference = unsafe::get_function((unsafe::Module*) jl_eval_string("jluna.memory_handler"), "get_reference"_sym);
        return jluna::safe_call(get_reference, jl_box_uint64(static_cast<uint64_t>(key)));
    }

    void free_reference(uint64_t key)
    {
        throw_if_uninitialized();
        static unsafe::Function* free_reference = unsafe::get_function((unsafe::Module*) jl_eval_string("jluna.memory_handler"), "free_reference"_sym);
        jluna::safe_call(free_reference, jl_box_uint64(static_cast<uint64_t>(key)));
    }
}

