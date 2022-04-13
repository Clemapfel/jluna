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

    //template<is<void> Return_t>


    template<typename Lambda_t>
    Task ThreadPool::create_and_schedule(Lambda_t lambda)
    {
        auto task = create(lambda);
        task.schedule();
        return task;
    }
}