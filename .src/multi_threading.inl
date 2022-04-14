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
        std::optional<T> out;
        _mutex.lock();
        if (_value != nullptr)
            out = std::optional<T>(*(_value.get()));
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

    namespace detail
    {
        struct FutureHandler
        {
            template<typename T>
            static inline void update_future(Future<T>& future, T value)
            {
                future.set_value(value);
            }
        };
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

    template<typename T>
    Task<T>::Task(size_t id)
        : _threadpool_id(id), _future(std::make_unique<Future<T>>())
    {}

    template<typename T>
    Task<T>::~Task()
    {
        unsafe::gc_release(_value_id);

        ThreadPool::_storage_lock.lock();
        ThreadPool::_storage.erase(_threadpool_id);
        ThreadPool::_storage_lock.unlock();
    }

    template<typename T>
    void Task<T>::initialize(std::function<unsafe::Value*()>* in)
    {
        static auto* make_task = unsafe::get_function("jluna"_sym, "make_task"_sym);
        _value = jluna::safe_call(make_task, box(reinterpret_cast<size_t>(in)));
        _value_id = unsafe::gc_preserve(_value);
    }

    template<typename T>
    void Task<T>::join()
    {
        static auto* wait = unsafe::get_function(jl_base_module, "wait"_sym);
        jluna::safe_call(wait, _value);
    }

    template<typename T>
    void Task<T>::schedule()
    {
        static auto* schedule = unsafe::get_function(jl_base_module, "schedule"_sym);
        jluna::safe_call(schedule, _value);
    }

    template<typename T>
    bool Task<T>::is_done() const
    {
        static auto* istaskdone = unsafe::get_function(jl_base_module, "istaskdone"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskdone, _value));
    }

    template<typename T>
    bool Task<T>::is_failed() const
    {
        static auto* istaskfailed = unsafe::get_function(jl_base_module, "istaskfailed"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskfailed, _value));
    }

    template<typename T>
    bool Task<T>::is_running() const
    {
        static auto* istaskstarted = unsafe::get_function(jl_base_module, "istaskstarted"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskstarted, _value));
    }

    template<typename T>
    Future<T>& Task<T>::result()
    {
        return std::ref(*_future.get());
    }

    // ###

    template<is_not<void> Return_t, typename Lambda_t, typename... Args_t>
    Task<Return_t>& ThreadPool::create(Lambda_t lambda, Args_t... args)
    {
        return create(static_cast<std::function<Return_t(Args_t...)>>(lambda), args...);
    }

    template<is<void> Return_t, typename Lambda_t, typename... Args_t>
    Task<unsafe::Value*>& ThreadPool::create(Lambda_t lambda, Args_t... args)
    {
        return create(static_cast<std::function<Return_t(Args_t...)>>(lambda), args...);
    }

    template<typename... Args_t>
    Task<unsafe::Value*>& ThreadPool::create(const std::function<void(Args_t...)>& lambda, Args_t... args)
    {
        _storage_lock.lock();

        Task<unsafe::Value*>* task = new Task<unsafe::Value*>(_current_id);
        _storage.emplace(_current_id,
            std::make_pair(
            std::unique_ptr<TaskSuper>(task),
            std::make_unique<std::function<unsafe::Value*()>>([lambda, future = std::ref(task->result()) ,args...]() -> unsafe::Value* {
                lambda(args...);
                detail::FutureHandler::update_future<unsafe::Value*>(future, jl_nothing);
                return jl_nothing;
        })));
        auto& it = _storage.find(_current_id)->second;
        auto* out = reinterpret_cast<Task<unsafe::Value*>*>(it.first.get());
        out->initialize(it.second.get());
        _current_id += 1;
        _storage_lock.unlock();
        return std::ref(*out);
    }

    template<is_not<void> Return_t, typename... Args_t>
    Task<Return_t>& ThreadPool::create(const std::function<Return_t(Args_t...)>& lambda, Args_t... args)
    {
        _storage_lock.lock();

        Task<Return_t>* task = new Task<Return_t>(_current_id);
        _storage.emplace(_current_id,
            std::make_pair(
            std::unique_ptr<TaskSuper>(task),
            std::make_unique<std::function<unsafe::Value*()>>([lambda, future = std::ref(task->result()) ,args...]() -> unsafe::Value* {
                auto res = lambda(args...);
                detail::FutureHandler::update_future<Return_t>(future, res);
                return box(res);
        })));
        auto& it = _storage.find(_current_id)->second;
        auto* out = reinterpret_cast<Task<Return_t>*>(it.first.get());
        out->initialize(it.second.get());
        _current_id += 1;
        _storage_lock.unlock();
        return std::ref(*out);
    }

    // ###

    inline void yield()
    {
        static auto* jl_yield = unsafe::get_function(jl_base_module, "yield"_sym);
        jluna::safe_call(jl_yield);
    }
}