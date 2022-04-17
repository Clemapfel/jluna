#ifdef __cplusplus

#include <include/julia_wrapper.hpp>
#include <include/cppcall.hpp>

#include <iostream>
#include <thread>

#include <.src/c_adapter.hpp>

extern "C"
{
    namespace jluna::c_adapter
    {

        unsafe::Value* make(void* function_ptr, int n_args)
        {
            static auto* make = (jl_function_t*) jl_eval_string("return jluna.cppcall.make_unnamed_function_proxy");
            return jluna::safe_call(make, (jl_value_t*) function_ptr, jl_box_int64(n_args));
        }

        void free_lambda(void* in, int n_args)
        {
            if (n_args == 0)
                delete (lambda_0_arg*) in;
            else if (n_args == 1)
                delete (lambda_1_arg*) in;
            else if (n_args == 2)
                delete (lambda_2_arg*) in;
            else if (n_args == 3)
                delete (lambda_3_arg*) in;
            else if (n_args == -1)
                delete (lambda_n_arg*) in;
            else
                throw std::invalid_argument("In c_adapter::free: " + std::to_string(n_args) + " is a invalid number of arguments");
        }

        unsafe::Value* invoke_lambda_0(void* function_ptr)
        {
            return (*reinterpret_cast<lambda_0_arg*>(function_ptr))();
        }

        unsafe::Value* invoke_lambda_1(void* function_ptr, unsafe::Value* x)
        {
            return (*reinterpret_cast<lambda_1_arg*>(function_ptr))(x);
        }

        unsafe::Value* invoke_lambda_2(void* function_ptr, unsafe::Value* x, unsafe::Value* y)
        {
            return (*reinterpret_cast<lambda_2_arg*>(function_ptr))(x, y);
        }

        unsafe::Value* invoke_lambda_3(void* function_ptr, unsafe::Value* x, unsafe::Value* y, unsafe::Value* z)
        {
            return (*reinterpret_cast<lambda_3_arg*>(function_ptr))(x, y, z);
        }

        unsafe::Value* invoke_lambda_n(void* function_ptr, unsafe::Array* vector)
        {
            std::vector<unsafe::Value*> args;
            args.reserve(jl_array_len(vector));

            for (size_t i = 0; i < jl_array_len(vector); ++i)
                args.push_back(jl_arrayref(vector, i));

            return (*reinterpret_cast<lambda_n_arg*>(function_ptr))(args);
        }

        jl_value_t* test()
        {
            return (jl_value_t*) jl_main_module;
        }
    }
}

#endif
