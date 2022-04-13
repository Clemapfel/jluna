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
    class Task
    {
        friend class ThreadPool;

        public:
            ~Task();

            operator unsafe::Value*();

            void join();
            void schedule();

            template<is_unboxable T>
            T result();

            bool is_done() const;
            bool is_failed() const;
            bool is_running() const;

        protected:
            /// @brief private ctor, use ThreadPool::create to create a task
            Task(std::function<unsafe::Value*()>*, size_t);

        private:
            unsafe::Value* _value;
            size_t _value_id;
            size_t _threadpool_id;
    };

    struct ThreadPool
    {
        friend class Task;

        /// @brief create a task around given lambda
        /// @tparam Return_t: return type of lambda, has to be either (un)boxable or void
        /// @param lambda
        /// @return task
        template<typename Lambda_t,
            std::enable_if_t<
                std::is_same_v<std::invoke_result_t<Lambda_t()>, void>,
            bool> = true>
        static Task create(Lambda_t);

        template<typename Lambda_t,
            std::enable_if_t<
                not std::is_same_v<std::invoke_result_t<Lambda_t()>, void>,
            bool> = true>
        static Task create(Lambda_t);

        /// @brief create a task around given lambda
        /// @tparam Return_t: return type of lambda, has to be either (un)boxable or void
        /// @param lambda
        /// @return task
        template<typename Lambda_t>
        static Task create_and_schedule(Lambda_t);

        protected:

        private:
            static inline size_t _current_id = 0;
            static inline std::mutex _storage_lock = std::mutex();
            static inline std::map<size_t, std::unique_ptr<std::function<unsafe::Value*()>>> _storage = {};
    };
}

#include <.src/multi_threading.inl>