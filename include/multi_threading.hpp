// 
// Copyright 2022 Clemens Cords
// Created on 06.04.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/unsafe_utilities.hpp>
#include <include/concepts.hpp>
#include <include/unbox.hpp>
#include <include/box.hpp>

#include <thread>
#include <optional>
#include <condition_variable>


/* * * * * * * * * * * * * * * * * * * * * * * * * *
 * WARNING
 *
 * The Julia state CANNOT be accessed safely from
 * within a C-side thread (such as std::thread
 * or libuv). This is, because the Julia C-API
 * will force a segfault if a C-API function
 * is called from anywhere but master scope.
 * This is by design and not related to jluna.
 *
 * Instead, for multi-threaded C++-side tasks,
 * ONLY use the threadpool provided here. It allows
 * for safe access into both the Julia and
 * C++ state from within a thread.
 *
 * Julia-side threads/tasks issued through
 * the `Threads` library (via @spawn, @threads,
 * @async, etc.) are also safe.
 *
 * Do not use @threadcall, it uses the libuv
 * threadpool, not the native Julia one,
 * which means it will also segfault if calling
 * the Julia C-API.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace jluna
{
    namespace detail
    {
        // forward declarations
        struct FutureHandler;
        struct TaskSuper {
            virtual void free() {};
        };
        template<typename>struct TaskValue;
    }

    /// @brief the result of a thread
    template<typename Value_t>
    class Future
    {
        template<typename>
        friend struct TaskValue;
        friend struct detail::FutureHandler;

        public:
            /// @brief construct
            Future();

            /// @brief access the value, thread-safe
            /// @returns optional, contains no value if task is failed or not yet done
            std::optional<Value_t> get();

            /// @brief check if value is available, thread-safe
            /// @returns true if task .is_done() returns true, false otherwise
            bool is_available();

            /// @brief pause the current thread until the futures value becomes available
            /// @returns value, if task failed, optional will not contain a value
            std::optional<Value_t> wait();

        private:
            void set_value(Value_t);

            std::mutex _mutex;
            std::condition_variable _cv;
            std::unique_lock<std::mutex> _cv_lock;
            std::unique_ptr<Value_t> _value;
    };

    template<typename Result_t>
    class Task
    {
        friend class ThreadPool;

        public:
            /// @brief dtor
            ~Task();

            /// @brief copy ctor deleted
            Task(const Task&) = delete;

            /// @brief move ctor
            /// @param other: other task, will be unusable after
            Task(Task&& other) noexcept;

            /// @brief copy assignment deleted
            Task& operator=(const Task&) = delete;

            /// @brief move ctor
            /// @param other: other task, will be unusable after
            Task& operator=(Task&& other) noexcept;

            /// @brief access the Julia-side value of type Task, implicit
            operator unsafe::Value*();

            /// @brief stall the thread this function is called from until the task is done
            void join();

            /// @brief start the thread
            void schedule();

            /// @brief is task finished
            /// @returns true if .result() is available, false otherwise
            bool is_done() const;

            /// @brief is task failed
            /// @returns true if task has failed and exited, false otherwise
            bool is_failed() const;

            /// @brief is task active
            /// @returns true if .result() is not yet available and the task has not yielded(), false otherwise
            bool is_running() const;

            /// @brief access the tasks result
            /// @returns future
            Future<Result_t>& result();

        protected:
            /// @brief ctor private, use ThreadPool::create
            explicit Task(detail::TaskValue<Result_t>*);

        private:
            detail::TaskValue<Result_t>* _value = nullptr; // lifetime managed by threadpool
    };

    /// @brief threadpool that allows scheduled C++-side tasks to safely access the Julia State from within a thread.
    /// Pool cannot be resized, it will use the native Julia threads to execute any C++-side tasks
    /// @note during task creation, the copy ctor will be invoked for all arguments `args` and the functions return value. To avoid this, wrap the type in an std::ref
    class ThreadPool
    {
        template<typename>
        friend class Task;

        public:
            /// @brief create a task from a std::function returning void
            /// @param f: function returning void
            /// @param args: arguments
            /// @returns Task, not yet scheduled
            /// @note once the task is done, .result() will return a future with value of type jluna::Nothing_t
            template<typename... Args_t>
            [[nodiscard]] static Task<void> create(const std::function<void(Args_t...)>& f, Args_t... args);

            /// @brief create a task from a std::function returning non-void
            /// @param f: function
            /// @param args: arguments
            /// @returns Task, not yet scheduled
            template<is_not<void> Return_t, typename... Args_t>
            [[nodiscard]] static Task<Return_t> create(const std::function<Return_t(Args_t...)>& f, Args_t... args);

            /// @brief create a task from a lambda
            /// @param f: lambda
            /// @param args: arguments
            /// @returns Task, not yet scheduled
            template<typename Signature,
                typename Lambda_t,
                typename... Args_t,
                typename T = std::invoke_result_t<std::function<Signature>, Args_t...>
            >
            [[nodiscard]] static Task<T> create(Lambda_t f, Args_t... args);

            /// @brief get number of threads
            /// @returns number
            static uint64_t n_threads();

            /// @brief get id of current task
            /// @returns number
            static uint64_t thread_id();

        private:
            static void free(uint64_t id);

            static inline uint64_t _current_id = 0;
            static inline std::mutex _storage_lock = std::mutex();
            static inline std::map<uint64_t,
                std::pair<
                    detail::TaskSuper*,
                    std::unique_ptr<std::function<unsafe::Value*()>>
                >> _storage = {};
    };

    /// @brief pause the current task, has to be called from within a task allocated via ThreadPool::create
    void yield();
}

#include <.src/multi_threading.inl>