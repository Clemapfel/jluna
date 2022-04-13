// 
// Copyright 2022 Clemens Cords
// Created on 12.04.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    template<is_unboxable T>
    T Task::result()
    {
        static auto* getproperty = unsafe::get_function(jl_base_module, "getproperty"_sym);
        static auto* sym = []() -> unsafe::Symbol* {
            auto* res = "result"_sym;
            size_t _ = unsafe::gc_preserve(res);   // intentional memory leak
            return res;
        }();

        return unbox<T>(unsafe::call(getproperty, _value, sym));
    }

    template<typename Lambda_t, std::enable_if_t<std::is_void_v<typename function_traits<forward_as_function_v<Lambda_t>>::return_t>, bool>>
    Task ThreadPool::create(Lambda_t lambda)
    {
        _storage_lock.lock();
        _storage.emplace(_current_id, std::make_unique<std::function<unsafe::Value*()>>([lambda]() -> unsafe::Value* {
            lambda();
            return jl_nothing;
        }));
        auto out = Task(_storage.at(_current_id).get(), _current_id);
        _current_id += 1;
        _storage_lock.unlock();
        return out;
    }

    template<typename Lambda_t, std::enable_if_t<not std::is_void_v<typename function_traits<forward_as_function_v<Lambda_t>>::return_t>, bool>>
    Task ThreadPool::create(Lambda_t lambda)
    {
        _storage_lock.lock();
        _storage.emplace(_current_id, std::make_unique<std::function<unsafe::Value*()>>([lambda]() -> unsafe::Value* {
            return box(lambda());
        }));
        auto out = Task(_storage.at(_current_id).get(), _current_id);
        _current_id += 1;
        _storage_lock.unlock();
        return out;
    }

    template<typename Lambda_t>
    Task ThreadPool::create_and_schedule(Lambda_t lambda)
    {
        auto task = create(lambda);
        task.schedule();
        return task;
    }
}