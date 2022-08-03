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
            friend class jluna::ThreadPool;
            TaskValue(size_t);
            ~TaskValue();

            void free() override;
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
    void detail::TaskValue<T>::free()
    {
        unsafe::gc_release(_value_id);
    }

    template<typename T>
    detail::TaskValue<T>::~TaskValue()
    {}

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
    Task<T>::Task(detail::TaskValue<T>* ptr)
        : _value(ptr)
    {}

    template<typename T>
    Task<T>::~Task()
    {
        //ThreadPool::free(_value->_threadpool_id);
    }

    template<typename T>
    Task<T>::Task(Task&& other)
    {
        _value = std::move(other._value);
        other._is_valid = false;
    }

    template<typename T>
    Task<T>& Task<T>::operator=(Task&& other)
    {
        _value = std::move(other._value);
        other._is_valid = false;
    }

    template<typename T>
    void Task<T>::join()
    {
        static auto* wait = unsafe::get_function(jl_base_module, "wait"_sym);
        jluna::safe_call(wait, _value->_value);
    }

    template<typename T>
    void Task<T>::schedule()
    {
        static auto* schedule = unsafe::get_function(jl_base_module, "schedule"_sym);
        jluna::safe_call(schedule, _value->_value);
    }

    template<typename T>
    bool Task<T>::is_done() const
    {
        static auto* istaskdone = unsafe::get_function(jl_base_module, "istaskdone"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskdone, _value->_value));
    }

    template<typename T>
    bool Task<T>::is_failed() const
    {
        static auto* istaskfailed = unsafe::get_function(jl_base_module, "istaskfailed"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskfailed, _value->_value));
    }

    template<typename T>
    bool Task<T>::is_running() const
    {
        static auto* istaskstarted = unsafe::get_function(jl_base_module, "istaskstarted"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskstarted, _value->_value));
    }

    template<typename T>
    Future<T>& Task<T>::result()
    {
        return std::ref(*(_value->_future.get()));
    }

    // void specialization for nicer syntax
    template<>
    class Task<void>
    {
        friend class ThreadPool;

        public:
            ~Task();

            Task& operator=(const Task&) = delete;
            Task(const Task&) = delete;
            Task(Task&& other);
            Task& operator=(Task&& other);

            operator unsafe::Value*();
            void join();
            void schedule();
            bool is_done() const;
            bool is_failed() const;
            bool is_running() const;
            Future<unsafe::Value*>& result();

        protected:
            Task(detail::TaskValue<unsafe::Value*>*);

        private:
            bool _is_valid = true;
            detail::TaskValue<unsafe::Value*>* _value; // lifetime managed by threadpool
    };

    inline Task<void>::Task(detail::TaskValue<unsafe::Value*>* value)
        : _value(value)
    {}

    inline Task<void>::~Task()
    {
        if (_is_valid)
            ThreadPool::free(_value->_threadpool_id);
    }

    inline Task<void>::Task(Task&& other)
    {
        _value = std::move(other._value);
        other._is_valid = false;
    }

    inline Task<void>& Task<void>::operator=(Task&& other)
    {
        _value = std::move(other._value);
        other._is_valid = false;
    }

    inline void Task<void>::join()
    {
        static auto* wait = unsafe::get_function(jl_base_module, "wait"_sym);
        jluna::safe_call(wait, _value->_value);
    }

    inline void Task<void>::schedule()
    {
        static auto* schedule = unsafe::get_function(jl_base_module, "schedule"_sym);
        jluna::safe_call(schedule, _value->_value);
    }

    inline bool Task<void>::is_done() const
    {
        static auto* istaskdone = unsafe::get_function(jl_base_module, "istaskdone"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskdone, _value->_value));
    }

    inline bool Task<void>::is_failed() const
    {
        static auto* istaskfailed = unsafe::get_function(jl_base_module, "istaskfailed"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskfailed, _value->_value));
    }

    inline bool Task<void>::is_running() const
    {
        static auto* istaskstarted = unsafe::get_function(jl_base_module, "istaskstarted"_sym);
        return jl_unbox_bool(jluna::safe_call(istaskstarted, _value->_value));
    }

    inline Future<unsafe::Value*>& Task<void>::result()
    {
        return std::ref(*(_value->_future.get()));
    }

    // ###

    template<typename Signature, typename Lambda_t, typename... Args_t, typename T>
    Task<T> ThreadPool::create(Lambda_t f, Args_t... args)
    {
        return ThreadPool::create(std::function<Signature>(f), args...);
    }

    template<typename... Args_t>
    Task<void> ThreadPool::create(const std::function<void(Args_t...)>& lambda, Args_t... args)
    {
        _storage_lock.lock();

        detail::TaskValue<unsafe::Value*>* task = new detail::TaskValue<unsafe::Value*>(_current_id);
        _storage.emplace(_current_id,
            std::make_pair(
            task,
            std::make_unique<std::function<unsafe::Value*()>>([lambda, task, future = std::ref(*(task->_future.get())) ,args...]() -> unsafe::Value* {
                lambda(args...);
                detail::FutureHandler::update_future<unsafe::Value*>(future, jl_nothing);
                return jl_nothing;
        })));
        auto& it = _storage.find(_current_id)->second;
        task->initialize(it.second.get());

        _current_id += 1;
        _storage_lock.unlock();
        return Task<void>(task);
    }

    template<is_not<void> Return_t, typename... Args_t>
    Task<Return_t> ThreadPool::create(const std::function<Return_t(Args_t...)>& lambda, Args_t... args)
    {
        _storage_lock.lock();

        detail::TaskValue<Return_t>* task = new detail::TaskValue<Return_t>(_current_id);
        _storage.emplace(_current_id,
            std::make_pair(
            task,
            std::make_unique<std::function<unsafe::Value*()>>([lambda, future = std::ref(*(task->_future.get())) ,args...]() -> unsafe::Value* {
                auto res = lambda(args...);
                detail::FutureHandler::update_future<Return_t>(future, res);
                return box<Return_t>(res);
        })));
        auto& it = _storage.find(_current_id)->second;
        task->initialize(it.second.get());

        _current_id += 1;
        _storage_lock.unlock();
        return Task<Return_t>(task);
    }

    inline size_t ThreadPool::n_threads()
    {
        static auto* nthreads = unsafe::get_function("Threads"_sym, "nthreads"_sym);
        return unbox<Int64>(jl_call0(nthreads));
    }

    inline size_t ThreadPool::thread_id()
    {
        static auto* threadid = unsafe::get_function("Threads"_sym, "threadid"_sym);
        return unbox<Int64>(jl_call0(threadid));
    }

    inline void ThreadPool::free(size_t id)
    {
        _storage_lock.lock();
        auto it = _storage.find(id);
        auto* ptr = it->second.first;
        ptr->free();
        delete it->second.first;
        _storage.erase(id);
        _storage_lock.unlock();
    }

    // ###

    inline void yield()
    {
        static auto* jl_yield = unsafe::get_function(jl_base_module, "yield"_sym);
        jluna::safe_call(jl_yield);
    }

    // ###
}