// 
// Copyright 2022 Clemens Cords
// Created on 12.04.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    namespace detail
    {
        template<typename Result_t>
        struct TaskValue : public detail::TaskSuper
        {
            friend class ThreadPool;
            TaskValue(size_t);
            ~TaskValue();

            void initialize(std::function<unsafe::Value*()>*);

            unsafe::Value* _value;
            size_t _value_id;
            size_t _threadpool_id;
            std::unique_ptr<Future<Result_t>> _future;
        };
    }

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
    detail::TaskValue<T>::TaskValue(size_t id)
        : _threadpool_id(id), _future(std::make_unique<Future<T>>())
    {}

    template<typename T>
    detail::TaskValue<T>::~TaskValue()
    {
        unsafe::gc_release(_value_id);

        ThreadPool::_storage_lock.lock();
        ThreadPool::_storage.erase(_threadpool_id);
        ThreadPool::_storage_lock.unlock();
    }

    template<typename T>
    void detail::TaskValue<T>::initialize(std::function<unsafe::Value*()>* in)
    {
        static auto* make_task = unsafe::get_function((unsafe::Module*) jl_eval_string("return jluna.cppcall"), "make_task"_sym);
        _value = unsafe::call(make_task, box(reinterpret_cast<size_t>(in)));

        static auto* setfield = unsafe::get_function(jl_base_module, "setfield!"_sym);
        static auto* sticky = jl_symbol("sticky");

        unsafe::call(setfield, _value, sticky, jl_box_bool(false));

        _value_id = unsafe::gc_preserve(_value);
    }
    
    template<typename T>
    Task<T>::Task(std::reference_wrapper<detail::TaskValue<T>> ref)
        : _ref(ref)
    {}

    template<typename T>
    void Task<T>::join()
    {
        static auto* wait = unsafe::get_function(jl_base_module, "wait"_sym);
        jluna::safe_call(wait, _ref.get()._value);
    }

    template<typename T>
    void Task<T>::schedule()
    {
        static auto* schedule = unsafe::get_function(jl_base_module, "schedule"_sym);
        jluna::safe_call(schedule, _ref.get()._value);
    }

    template<typename T>
    bool Task<T>::is_done() const
    {
        static auto* istaskdone = unsafe::get_function(jl_base_module, "istaskdone"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskdone, _ref.get()._value));
    }

    template<typename T>
    bool Task<T>::is_failed() const
    {
        static auto* istaskfailed = unsafe::get_function(jl_base_module, "istaskfailed"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskfailed, _ref.get()._value));
    }

    template<typename T>
    bool Task<T>::is_running() const
    {
        static auto* istaskstarted = unsafe::get_function(jl_base_module, "istaskstarted"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskstarted, _ref.get()._value));
    }

    template<typename T>
    Future<T>& Task<T>::result()
    {
        return std::ref(*(_ref.get()._future.get()));
    }

    // ###

    template<is_not<void> Return_t, typename Lambda_t, typename... Args_t>
    Task<Return_t> ThreadPool::create(Lambda_t lambda, Args_t... args)
    {
        return create(static_cast<std::function<Return_t(Args_t...)>>(lambda), args...);
    }

    template<is<void> Return_t, typename Lambda_t, typename... Args_t>
    Task<unsafe::Value*> ThreadPool::create(Lambda_t lambda, Args_t... args)
    {
        return create(static_cast<std::function<Return_t(Args_t...)>>(lambda), args...);
    }

    template<typename... Args_t>
    Task<unsafe::Value*> ThreadPool::create(const std::function<void(Args_t...)>& lambda, Args_t... args)
    {
        _storage_lock.lock();

        detail::TaskValue < unsafe::Value * > * task = new detail::TaskValue<unsafe::Value*>(_current_id);
        _storage.emplace(_current_id,
            std::make_pair(
            std::unique_ptr<detail::TaskSuper>(task),
            std::make_unique<std::function<unsafe::Value*()>>([lambda, future = std::ref(*(task->_future.get())) ,args...]() -> unsafe::Value* {
                lambda(args...);
                detail::FutureHandler::update_future<unsafe::Value*>(future, jl_nothing);
                return jl_nothing;
        })));
        auto& it = _storage.find(_current_id)->second;
        auto* out = reinterpret_cast<detail::TaskValue < unsafe::Value * > * > (it.first.get());
        out->initialize(it.second.get());
        _current_id += 1;
        _storage_lock.unlock();
        return Task<unsafe::Value*>(std::ref(*out));
    }

    template<is_not<void> Return_t, typename... Args_t>
    Task<Return_t> ThreadPool::create(const std::function<Return_t(Args_t...)>& lambda, Args_t... args)
    {
        _storage_lock.lock();

        detail::TaskValue<Return_t>* task = new detail::TaskValue<Return_t>(_current_id);
        _storage.emplace(_current_id,
            std::make_pair(
            std::unique_ptr<detail::TaskSuper>(task),
            std::make_unique<std::function<unsafe::Value*()>>([lambda, future = std::ref(*(task->_future.get())) ,args...]() -> unsafe::Value* {
                auto res = lambda(args...);
                detail::FutureHandler::update_future<Return_t>(future, res);
                return box(res);
        })));
        auto& it = _storage.find(_current_id)->second;
        auto* out = reinterpret_cast<detail::TaskValue<Return_t>*>(it.first.get());
        out->initialize(it.second.get());
        _current_id += 1;
        _storage_lock.unlock();
        return Task<Return_t>(std::ref(*out));
    }

    // ###

    inline void yield()
    {
        static auto* jl_yield = unsafe::get_function(jl_base_module, "yield"_sym);
        jluna::safe_call(jl_yield);
    }
}