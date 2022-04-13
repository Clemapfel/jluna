// 
// Copyright 2022 Clemens Cords
// Created on 12.04.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    template<Unboxable T>
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

    template<typename Return_t,
            LambdaType<Return_t> Lambda_t,
            std::enable_if_t<std::is_same_v<Return_t, void>, bool>
    >
    Task ThreadPool::create(Lambda_t lambda)
    {
        _storage_lock.lock();
        auto res = _storage.emplace({_current_id,
              std::make_unique<std::function<unsafe::Value*>>([lambda]() -> unsafe::Value*
        {
            lambda();
            return jl_nothing;
        })});
        _storage_lock.unlock();

        return Task(&res.second);
    }

    template<typename Return_t,
            LambdaType<Return_t> Lambda_t,
            std::enable_if_t<
                (Boxable<Return_t> and Unboxable<Return_t>)
                and not std::is_same_v<Return_t, void>,
            bool>>
    Task ThreadPool::create(Lambda_t lambda)
    {
        _storage_lock.lock();
        auto res = _storage.emplace({_current_id,
              std::make_unique<std::function<unsafe::Value*>>([lambda]() -> unsafe::Value*
        {
            return box(lambda());
        })});

        return Task(&res.second);
    }

    template<typename Return_t,
        LambdaType<Return_t> Lambda_t,
        std::enable_if_t<
            (Boxable<Return_t> and Unboxable<Return_t>) or
            std::is_same_v<Return_t, void>,
        bool>>
    Task ThreadPool::create_and_schedule(Lambda_t lambda)
    {
        auto out = create(lambda);
        out.schedule();
        return out;
    }
}