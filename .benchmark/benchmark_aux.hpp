// 
// Copyright 2022 Clemens Cords
// Created on 08.03.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <limits>
#include <random>

namespace jluna
{
    constexpr static inline size_t seed = 12345;

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    T generate_number(T t_min = std::numeric_limits<T>::min(), T t_max = std::numeric_limits<T>::max())
    {
        static std::uniform_real_distribution<T> dist = std::uniform_real_distribution<T>(t_min, t_max);
        static std::mt19937 engine = std::mt19937(seed);

        return dist(engine);
    }

    template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    T generate_number(T t_min = std::numeric_limits<T>::min(), T t_max = std::numeric_limits<T>::max())
    {
        static std::uniform_int_distribution<T> dist = std::uniform_int_distribution<T>(t_min, t_max);
        static std::mt19937 engine = std::mt19937(seed + 1000);

        return dist(engine);
    }

    std::string generate_string(size_t length)
    {
        static std::uniform_int_distribution<char> dist = std::uniform_int_distribution<char>(65, 90);
        static std::mt19937 engine = std::mt19937(seed);

        std::string out;
        out.reserve(length);

        for (size_t i = 0; i < length; ++i)
            out.push_back(dist(engine));

        return out;
    }
}