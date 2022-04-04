// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <include/exceptions.hpp>
#include <include/gc_sentinel.hpp>
#include <.src/c_adapter.hpp>
#include <.src/common.hpp>

namespace jluna
{
    namespace detail
    {
        template<typename Lambda_t, typename Return_t, typename... Args_t, std::enable_if_t<std::is_same_v<Return_t, void>, bool> = true>
        unsafe::Value* invoke_lambda(const Lambda_t* func, Args_t... args)
        {
            (*func)(args...);
            return jl_nothing;
        }

        template<typename Lambda_t, typename Return_t, typename... Args_t, std::enable_if_t<not std::is_same_v<Return_t, void>, bool> = true>
        unsafe::Value* invoke_lambda(const Lambda_t* func, Args_t... args)
        {
            auto res = (*func)(args...);
            return box(res);
        }

        static inline size_t _unnamed_function_id = 1;
    }

    template<LambdaType<> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 0, [lambda](unsafe::Value* tuple) -> unsafe::Value* {

            auto gc = GCSentinel();
            auto out = detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t>>(
                    &lambda
            );
            return out;
        });
    }

    template<LambdaType<unsafe::Value*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 1, [lambda](unsafe::Value* tuple) -> unsafe::Value* {

            auto gc = GCSentinel();
            auto out = detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t, unsafe::Value*>, unsafe::Value*>(
                    &lambda,
                    jl_get_nth_field(tuple, 0)
            );
            return out;
        });
    }

    template<LambdaType<unsafe::Value*, unsafe::Value*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 2, [lambda](unsafe::Value* tuple) -> unsafe::Value* {

            auto gc = GCSentinel();
            auto out = detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t, unsafe::Value*, unsafe::Value*>, unsafe::Value*, unsafe::Value*>(
                    &lambda,
                    jl_get_nth_field(tuple, 0),
                    jl_get_nth_field(tuple, 1)
            );
            return out;
        });
    }

    template<LambdaType<unsafe::Value*, unsafe::Value*, unsafe::Value*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 3, [lambda](unsafe::Value* tuple) -> unsafe::Value* {

            auto gc = GCSentinel();
            auto out = detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t, unsafe::Value*, unsafe::Value*, unsafe::Value*>, unsafe::Value*, unsafe::Value*, unsafe::Value*>(
                    &lambda,
                    jl_get_nth_field(tuple, 0),
                    jl_get_nth_field(tuple, 1),
                    jl_get_nth_field(tuple, 2)
            );
            return out;
        });
    }

    template<LambdaType<unsafe::Value*, unsafe::Value*, unsafe::Value*, unsafe::Value*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 4, [lambda](unsafe::Value* tuple) -> unsafe::Value* {

            auto gc = GCSentinel();
            auto out = detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t, unsafe::Value*, unsafe::Value*, unsafe::Value*, unsafe::Value*>, unsafe::Value*, unsafe::Value*, unsafe::Value*, unsafe::Value*>(
                    &lambda,
                    jl_get_nth_field(tuple, 0),
                    jl_get_nth_field(tuple, 1),
                    jl_get_nth_field(tuple, 2),
                    jl_get_nth_field(tuple, 3)
            );
            return out;
        });
    }

    template<LambdaType<std::vector<unsafe::Value*>> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 1, [lambda](unsafe::Value* tuple) -> unsafe::Value* {

            std::vector<unsafe::Value*> wrapped;

            for (size_t i = 0; i < detail::tuple_length(tuple); ++i)
                wrapped.push_back(jl_get_nth_field(tuple, i));

            auto gc = GCSentinel();
            auto out = detail::invoke_lambda<
                Lambda_t,
                std::invoke_result_t<Lambda_t, std::vector<unsafe::Value*>>,
                std::vector<unsafe::Value*>>(
                    &lambda, wrapped
            );
            return out;
        });
    }

    template<LambdaType<> Lambda_t>
    unsafe::Function* register_unnamed_function(const Lambda_t& lambda)
    {
        std::string id = "#" + std::to_string(detail::_unnamed_function_id++);
        register_function(id, lambda);

        static jl_function_t* new_unnamed_function = unsafe::get_function("jluna._cppcall"_sym, "new_unnamed_function"_sym);

        auto gc = GCSentinel();

        unsafe::Value* res;
        if (std::is_same_v<std::invoke_result_t<Lambda_t>, void>)
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(0), (unsafe::Value*) jl_nothing_type);
        else
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(0), (unsafe::Value*) jl_any_type);

        return res;
    }

    template<LambdaType<unsafe::Value*> Lambda_t>
    unsafe::Function* register_unnamed_function(const Lambda_t& lambda)
    {
        std::string id = "#" + std::to_string(detail::_unnamed_function_id++);
        register_function(id, lambda);

        static jl_function_t* new_unnamed_function = unsafe::get_function("jluna._cppcall"_sym, "new_unnamed_function"_sym);

        auto gc = GCSentinel();

        unsafe::Value* res;
        if (std::is_same_v<std::invoke_result_t<Lambda_t, unsafe::Value*>, void>)
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(1), (unsafe::Value*) jl_nothing_type);
        else
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(1), (unsafe::Value*) jl_any_type);

        return res;
    }

    template<LambdaType<unsafe::Value*, unsafe::Value*> Lambda_t>
    unsafe::Function* register_unnamed_function(const Lambda_t& lambda)
    {
        std::string id = "#" + std::to_string(detail::_unnamed_function_id++);
        register_function(id, lambda);

        static jl_function_t* new_unnamed_function = unsafe::get_function("jluna._cppcall"_sym, "new_unnamed_function"_sym);

        auto gc = GCSentinel();

        unsafe::Value* res;
        if (std::is_same_v<std::invoke_result_t<Lambda_t, unsafe::Value*, unsafe::Value*>, void>)
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(2), (unsafe::Value*) jl_nothing_type);
        else
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(2), (unsafe::Value*) jl_any_type);

        return res;
    }

    template<LambdaType<unsafe::Value*, unsafe::Value*, unsafe::Value*> Lambda_t>
    unsafe::Function* register_unnamed_function(const Lambda_t& lambda)
    {
        std::string id = "#" + std::to_string(detail::_unnamed_function_id++);
        register_function(id, lambda);

        static jl_function_t* new_unnamed_function = unsafe::get_function("jluna._cppcall"_sym, "new_unnamed_function"_sym);

        auto gc = GCSentinel();

        unsafe::Value* res;
        if (std::is_same_v<std::invoke_result_t<Lambda_t, unsafe::Value*, unsafe::Value*, unsafe::Value*>, void>)
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(3), (unsafe::Value*) jl_nothing_type);
        else
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(3), (unsafe::Value*) jl_any_type);

        return res;
    }

    template<LambdaType<unsafe::Value*, unsafe::Value*, unsafe::Value*, unsafe::Value*> Lambda_t>
    unsafe::Function* register_unnamed_function(const Lambda_t& lambda)
    {
        std::string id = "#" + std::to_string(detail::_unnamed_function_id++);
        register_function(id, lambda);
        static jl_function_t* new_unnamed_function = unsafe::get_function("jluna._cppcall"_sym, "new_unnamed_function"_sym);

        auto gc = GCSentinel();

        unsafe::Value* res;
        if (std::is_same_v<std::invoke_result_t<Lambda_t, unsafe::Value*, unsafe::Value*, unsafe::Value*, unsafe::Value*>, void>)
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(4), (unsafe::Value*) jl_nothing_type);
        else
            res = jl_call3(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(4), (unsafe::Value*) jl_any_type);

        return res;
    }

    template<LambdaType<std::vector<unsafe::Value*>> Lambda_t>
    unsafe::Function* register_unnamed_function(const Lambda_t& lambda)
    {
        std::string id = "#" + std::to_string(detail::_unnamed_function_id++);
        register_function(id, lambda);

        static jl_function_t* new_unnamed_function = unsafe::get_function("jluna._cppcall"_sym, "new_unnamed_function"_sym);

        auto gc = GCSentinel();
        auto* res = jl_call2(new_unnamed_function, (unsafe::Value*) jl_symbol(id.c_str()), jl_box_int64(-1));
        return res;
    }
}

