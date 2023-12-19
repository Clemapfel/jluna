// 
// Copyright 2022 Clemens Cords
// Created on 04.04.22 by clem (mail@clemens-cords.com)
//

namespace jluna
{
    template<is_unboxable T>
    GeneratorExpression::ForwardIterator::operator T()
    {
        return unbox<T>(this->operator*());
    }
}