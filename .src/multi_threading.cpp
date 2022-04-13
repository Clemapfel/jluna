// 
// Copyright 2022 Clemens Cords
// Created on 12.04.22 by clem (mail@clemens-cords.com)
//

#include <include/multi_threading.hpp>
#include <include/unsafe_utilities.hpp>

namespace jluna
{
    /*
    Task::Task(std::function<unsafe::Value*()>* in, size_t id)
        : _threadpool_id(id)
    {
        static auto* make_task = unsafe::get_function("jluna"_sym, "make_task"_sym);
        _value = jluna::safe_call(make_task, box(reinterpret_cast<size_t>(in)));
        _value_id = unsafe::gc_preserve(_value);
    }

    Task::~Task()
    {
        unsafe::gc_release(_value_id);

        ThreadPool::_storage_lock.lock();
        ThreadPool::_storage.erase(_threadpool_id);
        ThreadPool::_storage_lock.unlock();
    }

    Task::operator _jl_value_t*()
    {
        return _value;
    }

    void Task::join()
    {
        static auto* wait = unsafe::get_function(jl_base_module, "wait"_sym);
        jluna::safe_call(wait, _value);
    }

    void Task::schedule()
    {
        static auto* schedule = unsafe::get_function(jl_base_module, "schedule"_sym);
        jluna::safe_call(schedule, _value);
    }
     */
}

