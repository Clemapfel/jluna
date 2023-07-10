#ifdef __cplusplus

#include <include/julia_wrapper.hpp>
#include <include/cppcall.hpp>

#include <iostream>
#include <thread>

#include <.src/c_adapter.hpp>

jluna::unsafe::Value* jluna_make(void* function_ptr, int n_args)
{
    gc_pause;
    static auto* make = (jl_function_t*) jl_eval_string("return jluna.cppcall.make_unnamed_function");
    auto* res = jluna::safe_call(make, jl_box_voidpointer(function_ptr), jl_box_int64(n_args));
    gc_unpause;
    return res;
}

void jluna_free_lambda(void* in, int n_args)
{
    if (n_args == 0)
        delete (jluna::detail::lambda_0_arg*) in;
    else if (n_args == 1)
        delete (jluna::detail::lambda_1_arg*) in;
    else if (n_args == 2)
        delete (jluna::detail::lambda_2_arg*) in;
    else if (n_args == 3)
        delete (jluna::detail::lambda_3_arg*) in;
    else
        std::cerr << "[C++][WARNING] In c_adapter::jluna_free_lambda: Unreachable reached" << std::endl;
}

jluna::unsafe::Value* jluna_invoke_lambda_0(void* function_ptr)
{
    return (*reinterpret_cast<jluna::detail::lambda_0_arg*>(function_ptr))();
}

jluna::unsafe::Value* jluna_invoke_lambda_1(void* function_ptr, jluna::unsafe::Value* x)
{
    return (*reinterpret_cast<jluna::detail::lambda_1_arg*>(function_ptr))(x);
}

jluna::unsafe::Value* jluna_invoke_lambda_2(void* function_ptr, jluna::unsafe::Value* x, jluna::unsafe::Value* y)
{
    return (*reinterpret_cast<jluna::detail::lambda_2_arg*>(function_ptr))(x, y);
}

jluna::unsafe::Value* jluna_invoke_lambda_3(void* function_ptr, jluna::unsafe::Value* x, jluna::unsafe::Value* y, jluna::unsafe::Value* z)
{
    return (*reinterpret_cast<jluna::detail::lambda_3_arg*>(function_ptr))(x, y, z);
}

void* jluna_to_pointer(jl_value_t* in)
{
    return (void*) in;
}

uint64_t jluna_invoke_from_task(uint64_t function_ptr)
{
    return reinterpret_cast<uint64_t>(
        (*reinterpret_cast<std::function<jluna::unsafe::Value*()>*>(function_ptr))()
    );
}

bool jluna_verify()
{
    return true;
}


#endif
