// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#include <include/cppcall.hpp>
#include <include/exceptions.hpp>

namespace jluna
{
    namespace detail
    {
        template<typename Lambda_t, typename Return_t, typename... Args_t, std::enable_if_t<std::is_same_v<Return_t, void>, Bool> = true>
        jl_value_t* detail::invoke_lambda(const Lambda_t* func, Args_t... args)
        {
            (*func)(args...);
            return jl_nothing;
        }

        template<typename Lambda_t, typename Return_t, typename... Args_t, std::enable_if_t<std::is_same_v<Return_t, jl_value_t*>, Bool> = true>
        jl_value_t* detail::invoke_lambda(const Lambda_t* func, Args_t... args)
        {
            jl_value_t* res = (*func)(args...);
            return res;
        }
    }

    template<LambdaType<> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 0, [lambda](jl_value_t* tuple) -> jl_value_t* {

            return detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t>>(
                    &lambda
            );
        });
    }

    template<LambdaType<jl_value_t*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 1, [lambda](jl_value_t* tuple) -> jl_value_t* {

            return detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t, jl_value_t*>, jl_value_t*>(
                    &lambda,
                    jl_tupleref(tuple, 0)
            );
        });
    }

    template<LambdaType<jl_value_t*, jl_value_t*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 2, [lambda](jl_value_t* tuple) -> jl_value_t* {

            return detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t, jl_value_t*, jl_value_t*>, jl_value_t*, jl_value_t*>(
                    &lambda,
                    jl_tupleref(tuple, 0),
                    jl_tupleref(tuple, 1)
            );
        });
    }

    template<LambdaType<jl_value_t*, jl_value_t*, jl_value_t*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 3, [lambda](jl_value_t* tuple) -> jl_value_t* {

            return detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t, jl_value_t*, jl_value_t*, jl_value_t*>, jl_value_t*, jl_value_t*, jl_value_t*>(
                    &lambda,
                    jl_tupleref(tuple, 0),
                    jl_tupleref(tuple, 1),
                    jl_tupleref(tuple, 2)
            );
        });
    }

    template<LambdaType<jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 4, [lambda](jl_value_t* tuple) -> jl_value_t* {

            return detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t, jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*>, jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*>(
                    &lambda,
                    jl_tupleref(tuple, 0),
                    jl_tupleref(tuple, 1),
                    jl_tupleref(tuple, 2),
                    jl_tupleref(tuple, 3)
            );
        });
    }

    template<LambdaType<jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 5, [lambda](jl_value_t* tuple) -> jl_value_t* {

            return detail::invoke_lambda<Lambda_t, std::invoke_result_t<Lambda_t, jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*>, jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*, jl_value_t*>(
                    &lambda,
                    jl_tupleref(tuple, 0),
                    jl_tupleref(tuple, 1),
                    jl_tupleref(tuple, 2),
                    jl_tupleref(tuple, 3),
                    jl_tupleref(tuple, 4)
            );
        });
    }

    template<LambdaType<std::vector<jl_value_t*>> Lambda_t>
    void register_function(const std::string& name, const Lambda_t& lambda)
    {
        throw_if_uninitialized();

        c_adapter::register_function(name, 1, [lambda](jl_value_t* tuple) -> jl_value_t* {

            std::vector<jl_value_t*> wrapped;

            for (size_t i = 0; i < jl_tuple_len(tuple); ++i)
                wrapped.push_back(jl_tupleref(tuple, i));

            return detail::invoke_lambda<
                Lambda_t,
                std::invoke_result_t<Lambda_t, std::vector<jl_value_t*>>,
                std::vector<jl_value_t*>>(
                    &lambda, wrapped
            );
        });
    }
}

