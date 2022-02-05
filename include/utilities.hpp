// 
// Copyright 2022 Clemens Cords
// Created on 31.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <include/proxy.hpp>

namespace jluna
{
    /// @brief Proxy of singleton Main, initialized by State::initialize
    inline Proxy Main;

    /// @brief Proxy of singleton Main.Base, initialized by State::initialize
    inline Proxy Base;

    /// @brief Proxy of singleton Main.Base.Core, initialized by State::initialize
    inline Proxy Core;
}

#include ".src/utilities.inl"