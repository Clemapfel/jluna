Index
=====

This section provides a complete list of all objects and functions in jluna, ordered by header name.

.. note::
    Use the "search" bar on the left of the page or the table of contents in the top right to find any particular function quickly!

-------------

Array
*****

.. doxygenclass:: jluna::Array
    :members:

-------------

Array: Non-Const Iterator
^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenstruct:: jluna::Array::Iterator
    :members:

-------------

Array: Const Iterator
^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: jluna::Array::ConstIterator
    :members:

-------------

Array: Typedefs
^^^^^^^^^^^^^^^

.. doxygentypedef:: jluna::ArrayAny1d
.. doxygentypedef:: jluna::ArrayAny2d
.. doxygentypedef:: jluna::ArrayAny3d
.. doxygentypedef:: jluna::ArrayAny

--------------

Vector
^^^^^^

.. doxygenclass:: jluna::Vector
    :members:

--------------

Box
***

.. doxygenfunction:: jluna::docs_only::box(T value)

.. code-block:: cpp
    :caption: **concept**: is_boxable

    // T is boxable if box(T) is defined or value can decay to unsafe::Value* directly
    template<typename T>
    concept is_boxable = requires(T t)
    {
        box(t);
    } or requires(T t)
    {
        (unsafe::Value*) t;
    };

--------------

Unbox
*****

.. doxygenfunction:: jluna::docs_only::box(T value)

.. code-block:: cpp
    :caption: **concept**: is_unboxable

    // T is unboxable if unbox(T) is defined
    template<typename T>
    concept is_unboxable = requires(T t, jl_value_t* v)
    {
        {unbox<T>(v)};
    };

-------------

