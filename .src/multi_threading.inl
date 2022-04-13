// 
// Copyright 2022 Clemens Cords
// Created on 12.04.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    template<typename T>
    Future<T>::Future()
        : _mutex(), _cv_lock(), _cv()
    {}

    template<typename T>
    std::optional<T> Future<T>::get()
    {
        auto out = std::optional<T>();
        _mutex.lock();
        if (_value != nullptr)
            out.value() = std::copy(_value.get());
        _mutex.unlock();
        return out;
    }

    template<typename T>
    bool Future<T>::is_available()
    {
        bool out = false;
        _mutex.lock();
        if (_value != nullptr)
            out = true;
        _mutex.unlock();
        return out;
    }

    template<typename T>
    std::optional<T> Future<T>::wait()
    {
        _cv.wait(_cv_lock, [&](){
            return _value.get() != nullptr;
        });
        return get();
    }

    template<typename T>
    void Future<T>::set_value(T value)
    {
        _mutex.lock();
        _value = std::make_unique<T>(value);
        _cv.notify_all();
        _mutex.unlock();
    }

    // ###

    implement task, future already notifies waiting threads. Add Future.set_value to lambda wrappers
    in create

    // ###

    template<typename Return_t, typename Lambda_t, typename... Args_t>
    auto ThreadPool::create(Lambda_t lambda, Args_t... args)
    {
        return create(static_cast<std::function<Return_t(Args_t...)>>(lambda), args...);
    }

    template<typename... Args_t>
    auto ThreadPool::create(const std::function<void(Args_t...)>& lambda, Args_t... args)
    {
        _storage_lock.lock();
        _storage.emplace(_current_id, std::make_unique<std::function<unsafe::Value*()>>([lambda, args...]() -> unsafe::Value* {
            lambda(args...);
            return jl_nothing;
        }));
        auto out = Task<unsafe::Value*>(_storage.at(_current_id).get(), _current_id);
        _current_id += 1;
        _storage_lock.unlock();
        return out;
    }

    template<is_not<void> Return_t, typename... Args_t>
    auto ThreadPool::create(const std::function<Return_t()>& lambda, Args_t... args)
    {
        _storage_lock.lock();
        _storage.emplace(_current_id, std::make_unique<std::function<unsafe::Value*()>>([lambda, args...]() -> unsafe::Value* {
            return lambda(args...);
        }));
        auto out = Task<Return_t>(_storage.at(_current_id).get(), _current_id);
        _current_id += 1;
        _storage_lock.unlock();
        return out;
    }
}