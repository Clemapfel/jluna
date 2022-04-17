// 
// Copyright 2022 Clemens Cords
// Created on 17.04.22 by clem (mail@clemens-cords.com)
//

#include <include/typedefs.hpp>
#include <include/mutex.hpp>
#include <include/concepts.hpp>
#include <include/unsafe_utilities.hpp>
#include <include/safe_utilities.hpp>
#include <include/box>

#include <unordered_map>
#include <functional>

namespace jluna::detail
{
    using lambda_0_arg = std::function<unsafe::Value*()>;
    using lambda_1_arg = std::function<unsafe::Value*(unsafe::Value*)>;
    using lambda_2_arg = std::function<unsafe::Value*(unsafe::Value*, unsafe::Value*)>;
    using lambda_3_arg = std::function<unsafe::Value*(unsafe::Value*, unsafe::Value*, unsafe::Value*)>;
    using lambda_n_arg = std::function<unsafe::Value*(const std::vector<unsafe::Value*>&)>;

    static inline std::unordered_map<size_t, lambda_0_arg> _0_arg = {};
    static inline std::unordered_map<size_t, lambda_1_arg> _1_arg = {};
    static inline std::unordered_map<size_t, lambda_2_arg> _2_arg = {};
    static inline std::unordered_map<size_t, lambda_3_arg> _3_arg = {};
    static inline std::unordered_map<size_t, lambda_n_arg> _n_arg = {};

    static inline jluna::Mutex _lambda_0_lock = jluna::Mutex();
    static inline jluna::Mutex _lambda_1_lock = jluna::Mutex();
    static inline jluna::Mutex _lambda_2_lock = jluna::Mutex();
    static inline jluna::Mutex _lambda_3_lock = jluna::Mutex();
    static inline jluna::Mutex _lambda_n_lock = jluna::Mutex();

    static inline size_t _lambda_id = 0;

    void free_lambda(size_t id, int n_args)
    {
        switch(n_args)
        {
            case 0:
                _lambda_0_lock.lock();
                _0_arg.erase(id);
                _lambda_0_lock.unlock();
                break;
            case 1:
                _lambda_1_lock.lock();
                _1_arg.erase(id);
                _lambda_1_lock.unlock();
                break;
            case 2:
                _lambd_2_lock.lock();
                _2_arg.erase(id);
                _lambda_2_lock.unlock();
                break;
            case 3:
                _lambda_3_lock.lock();
                _3_arg.erase(id);
                _lambda_3_lock.unlock();
                break;
            case -1:
                _lambda_n_lock.lock();
                _n_arg.erase(id);
                _lambda_n_lock.unlock();
                break;
        }
    }

    unsafe::Value* invoke_0_args(size_t id)
    {
        gc_pause;
        _lambda_0_lock.lock();
        auto& f = _0_arg.at(id);
        _lambda_0_lock.unlock();

        auto* res = f();
        gc_unpause;
        return res;
    }

    unsafe::Value* invoke_1_args(size_t id, unsafe::Value* arg1)
    {
        gc_pause;
        _lambda_1_lock.lock();
        auto& f = _1_arg.at(id);
        _lambda_1_lock.unlock();

        auto* res = f(arg1);
        gc_unpause;
        return res;
    }

    unsafe::Value* invoke_2_args(size_t id, unsafe::Value* arg1, unsafe::Value* arg2)
    {
        gc_pause;
        _lambda_2_lock.lock();
        auto& f = _2_arg.at(id);
        _lambda_2_lock.unlock();

        auto* res = f(arg1, arg2);
        gc_unpause;
        return res;
    }

    unsafe::Value* invoke_3_args(size_t id, unsafe::Value* arg1, unsafe::Value* arg2, unsafe::Value* arg3)
    {
        gc_pause;
        _lambda_3_lock.lock();
        auto& f = _3_arg.at(id);
        _lambda_3_lock.unlock();

        auto* res = f(arg1, arg2, arg3);
        gc_unpause;
        return res;
    }

    unsafe::Value* invoke_n_args(size_t id, unsafe::Value** args_ptr, size_t n_args)
    {
        gc_pause;
        std::vector<unsafe::Value*> args;
        args.reserve(n_args);
        for (size_t i = 0; i < n_args; ++i)
            args.push_back(args_ptr[i]);

        _lambda_n_lock.lock();
        auto& f = _n_arg.at(id);
        _lambda_n_lock.lock();

        auto* res = f(args);
        gc_unpause;
        return res;
    }
}

namespace jluna
{
    template<is_function_with_n_args<0> Function_t>
    unsafe::Function* register_function(Function_t f)
    {
        using namespace detail;

        _lambda_0_lock.lock();
        _0_arg.emplace(_lambda_id, [f]() -> unsafe::Value* {

            return box(f());
        });

        gc_pause;
        static auto* new_unnamed_function = unsafe::get_function((unsafe::Module*) jl_eval_string("jluna._cppcall"), "new_unnamed_function");
        auto* out = jluna::safe_call(new_unnamed_function, box<UInt64>(_lambda_id), box<int>(0));
        _lambda_0_lock.unlock();
        gc_unpause;
        return out;
    }

}