#ifdef __cplusplus

#include <include/julia_wrapper.hpp>
#include <include/mutex.hpp>

#include <iostream>
#include <thread>

#include <.src/c_adapter.hpp>

extern "C"
{
    namespace jluna::c_adapter
    {
        void call_function(size_t id)
        {
            static jl_function_t* get_args = jl_get_function((jl_module_t*) jl_eval_string("return Main.jluna._cppcall"), "get_arguments");
            static jl_function_t* set_result = jl_get_function((jl_module_t*) jl_eval_string("return Main.jluna._cppcall"), "set_result");
            jl_value_t* tuple = jl_call0(get_args);
            jl_value_t* res = nullptr;

            res = _functions.at(id).first(tuple);

            if (res == nullptr) // catch returning nullptr
                res = jl_nothing;

            jl_call1(set_result, res);
        }

        size_t hash(const std::string& str)
        {
            return jl_symbol(str.c_str())->hash;
        }

        void register_function(const std::string& name, size_t n_args, std::function<jl_value_t*(jl_value_t*)>&& lambda)
        {
            [[unlikely]]
            if (name.find('.') != std::string::npos)
            {
                std::string str = "In register_function(\"" + name + "\"): function names cannot begin with \'#\' or contain \'.\' in any place";
                throw std::invalid_argument(str.c_str());
            }

            _functions.insert({hash(name), std::make_pair(lambda, n_args)});
        }

        void unregister_function(const std::string& name)
        {
            _functions.erase(hash(name));
        }

        bool is_registered(size_t id)
        {
            auto it = _functions.find(id);
            return it != _functions.end();
        }

        size_t get_n_args(size_t id)
        {
            return _functions.at(id).second;
        }

        void free_function(size_t id)
        {
            _functions.erase(id);
        }

        size_t new_mutex()
        {
            auto* mutex = new jluna::Mutex();
            return reinterpret_cast<size_t>(mutex);
        }

        void delete_mutex(size_t ptr)
        {
            delete reinterpret_cast<jluna::Mutex*>(ptr);
        }

        void lock_mutex(size_t ptr)
        {
            reinterpret_cast<jluna::Mutex*>(ptr)->lock();
        }

        void unlock_mutex(size_t ptr)
        {
            reinterpret_cast<jluna::Mutex*>(ptr)->unlock();
        }

        void try_lock_mutex(size_t ptr)
        {
            reinterpret_cast<jluna::Mutex*>(ptr)->try_lock();
        }

        size_t get_lock(size_t ptr)
        {
            return reinterpret_cast<size_t>(reinterpret_cast<jluna::Mutex*>(ptr)->operator jl_value_t *());
        }
    }
}

#endif
