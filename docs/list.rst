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
.. doxygenfunction:: jluna::unsafe::unsafe_box

--------------

Concept: is_boxable
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp
    :caption: **Concept**: :code:`T` is boxable if :code:`box(T)` is defined or :code:`T` can decay to :code:`unsafe::Value*` directly

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

.. doxygenfunction:: jluna::docs_only::unbox(unsafe::Value*)
.. doxygenfunction:: jluna::unsafe::unsafe_unbox

--------------

Concept: is_unboxable
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp
    :caption: **Concept**: :code:`T` is unboxable if :code:`unbox(T)` is defined

    template<typename T>
    concept is_unboxable = requires(T t, jl_value_t* v)
    {
        {unbox<T>(v)};
    };

-------------

Unsafe
******

GC
^^
.. doxygenfunction:: jluna::unsafe::docs_only::gc_preserve(T* value)
.. doxygenfunction:: jluna::unsafe::docs_only::gc_preserve(Ts... value)
.. doxygenfunction:: jluna::unsafe::gc_release(size_t id)
.. doxygenfunction:: jluna::unsafe::gc_release(std::vector<size_t> &ids)
.. doxygenfunction:: jluna::unsafe::gc_disable
.. doxygenfunction:: jluna::unsafe::gc_enable

-------------

Unsafe: Get / Call Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::unsafe::get_function(unsafe::Module* module, unsafe::Symbol* name)
.. doxygenfunction:: jluna::unsafe::get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name)
.. doxygenfunction:: jluna::unsafe::call(unsafe::Function* function, Args_t... args);
.. doxygenfunction:: jluna::unsafe::call(unsafe::DataType* type, Args_t... args)

-------------

Unsafe: Get / Set Values
^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::unsafe::get_value(unsafe::Module* module, unsafe::Symbol* name);
.. doxygenfunction:: jluna::unsafe::get_value(unsafe::Symbol* module_name, unsafe::Symbol* variable_name);
.. doxygenfunction:: jluna::unsafe::set_value(unsafe::Module* module, unsafe::Symbol* name, unsafe::Value* value)
.. doxygenfunction:: jluna::unsafe::set_value(unsafe::Symbol* module_name, unsafe::Symbol* variable_name)

-------------

Unsafe: Get / Set Fields
^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::unsafe::get_field
.. doxygenfunction:: jluna::unsafe::set_field

-------------

Unsafe: Expressions
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::operator""_eval
.. doxygenfunction:: jluna::operator""_sym
.. doxygenfunction:: jluna::unsafe::eval
.. doxygenfunction:: jluna::unsafe::Expr

-------------

Unsafe: Arrays
^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::unsafe::docs_only::new_array(unsafe::Value* value_type, size_t one_d)
.. doxygenfunction:: jluna::unsafe::docs_only::new_array(unsafe::Value* value_type, size_t one_d, size_t two_d)
.. doxygenfunction:: jluna::unsafe::docs_only::new_array(unsafe::Value* value_type, Dims... size_per_dimension);
.. doxygenfunction:: jluna::unsafe::docs_only::new_array_from_data(unsafe::Value* value_type, void* data, size_t one_d)
.. doxygenfunction:: jluna::unsafe::docs_only::new_array_from_data(unsafe::Value* value_type, void* data, Dims... size_per_dimension)
.. doxygenfunction:: jluna::unsafe::sizehint
.. doxygenfunction:: jluna::unsafe::docs_only::resize_array(unsafe::Array* array, Dims...)
.. doxygenfunction:: jluna::unsafe::docs_only::resize_array(unsafe::Array* array, size_t one_d)
.. doxygenfunction:: jluna::unsafe::docs_only::resize_array(unsafe::Array* array, size_t one_d, size_t two_d)
.. doxygenfunction:: jluna::unsafe::override_array
.. doxygenfunction:: jluna::unsafe::get_array_size(unsafe::Array*)
.. doxygenfunction:: jluna::unsafe::get_array_size(unsafe::Array*, size_t dimension_index)
.. doxygenfunction:: jluna::unsafe::docs_only::get_index(unsafe::Array*, Index... index_per_dimension)
.. doxygenfunction:: jluna::unsafe::docs_only::get_index(unsafe::Array*, size_t)
.. doxygenfunction:: jluna::unsafe::docs_only::get_index(unsafe::Array*, size_t, size_t)
.. doxygenfunction:: jluna::unsafe::docs_only::set_index(unsafe::Array*, unsafe::Value* value, Index... index_per_dimension)
.. doxygenfunction:: jluna::unsafe::docs_only::set_index(unsafe::Array*, unsafe::Value* value, size_t)
.. doxygenfunction:: jluna::unsafe::docs_only::set_index(unsafe::Array*, unsafe::Value* value, size_t, size_t)
.. doxygenfunction:: jluna::unsafe::get_array_data
.. doxygenfunction:: jluna::unsafe::swap_array_data
.. doxygenfunction:: jluna::unsafe::set_array_data
.. doxygenfunction:: jluna::unsafe::push_front
.. doxygenfunction:: jluna::unsafe::push_back

-------------
