Library Index
=============

This section provides a complete list of all objects and functions in jluna, ordered by header name.

.. note::
    Use the "search" bar on the left of the page or the table of contents in the top right to find any particular function quickly!

--------------

Box
***

.. doxygenfunction:: jluna::docs_only::box(T value)

--------------

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

--------------

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

Safe
****

Safe: Initialize
^^^^^^^^^^^^^^^^

.. doxygenvariable:: jluna::JULIA_NUM_THREADS_AUTO
.. doxygenfunction:: jluna::initialize

--------------

Safe: Call Function
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::safe_call

--------------

Safe: Eval
^^^^^^^^^^

.. doxygenfunction:: jluna::safe_eval

--------------

.. doxygenfunction:: jluna::safe_eval_file

--------------

Safe: Miscellaneous
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::as_julia_pointer

--------------

.. doxygenfunction:: jluna::println

--------------

.. doxygenfunction:: jluna::undef

--------------

.. doxygenfunction:: jluna::nothing

--------------

.. doxygenfunction:: jluna::missing

--------------

.. doxygenfunction:: jluna::collect_garbage

-------------

--------------


Unsafe
******

GC
^^
.. doxygenfunction:: jluna::unsafe::docs_only::gc_preserve(T* value)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::gc_preserve(Ts... value)

--------------

.. doxygenfunction:: jluna::unsafe::gc_release(size_t id)

--------------

.. doxygenfunction:: jluna::unsafe::gc_release(std::vector<size_t> &ids)

--------------

.. doxygenfunction:: jluna::unsafe::gc_disable

--------------

.. doxygenfunction:: jluna::unsafe::gc_enable

-------------

Unsafe: Get / Call Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::unsafe::get_function(unsafe::Module* module, unsafe::Symbol* name)

--------------

.. doxygenfunction:: jluna::unsafe::get_function(unsafe::Symbol* module_name, unsafe::Symbol* function_name)

--------------

.. doxygenfunction:: jluna::unsafe::call(unsafe::Function* function, Args_t... args)

--------------

.. doxygenfunction:: jluna::unsafe::call(unsafe::DataType* type, Args_t... args)

-------------

Unsafe: Get / Set Values
^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::unsafe::get_value(unsafe::Module* module, unsafe::Symbol* name)

--------------

.. doxygenfunction:: jluna::unsafe::get_value(unsafe::Symbol* module_name, unsafe::Symbol* variable_name)

--------------

.. doxygenfunction:: jluna::unsafe::set_value(unsafe::Module* module, unsafe::Symbol* name, unsafe::Value* value)

--------------

.. doxygenfunction:: jluna::unsafe::set_value(unsafe::Symbol* module_name, unsafe::Symbol* variable_name)

--------------

Unsafe: Get / Set Fields
^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::unsafe::get_field

--------------

.. doxygenfunction:: jluna::unsafe::set_field

-------------

Unsafe: Expressions
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::operator""_eval

--------------

.. doxygenfunction:: jluna::operator""_sym

--------------

.. doxygenfunction:: jluna::unsafe::eval

--------------

.. doxygenfunction:: jluna::unsafe::Expr

-------------

Proxy
*****

.. doxygenclass:: jluna::Proxy
    :members:

--------------


.. doxygenclass:: jluna::Proxy::ProxyValue
    :members:

-------------

Module
******

.. doxygenclass:: jluna::Module
    :members:

--------------

.. doxygenvariable:: jluna::Main
.. doxygenvariable:: jluna::Base
.. doxygenvariable:: jluna::Core

-------------

Unsafe: Arrays
^^^^^^^^^^^^^^

.. doxygenfunction:: jluna::unsafe::docs_only::new_array(unsafe::Value* value_type, size_t one_d)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::new_array(unsafe::Value* value_type, size_t one_d, size_t two_d)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::new_array(unsafe::Value* value_type, Dims... size_per_dimension);

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::new_array_from_data(unsafe::Value* value_type, void* data, size_t one_d)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::new_array_from_data(unsafe::Value* value_type, void* data, Dims... size_per_dimension)

--------------

.. doxygenfunction:: jluna::unsafe::sizehint

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::resize_array(unsafe::Array* array, Dims...)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::resize_array(unsafe::Array* array, size_t one_d)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::resize_array(unsafe::Array* array, size_t one_d, size_t two_d)

--------------

.. doxygenfunction:: jluna::unsafe::override_array

--------------

.. doxygenfunction:: jluna::unsafe::get_array_size(unsafe::Array*)

--------------

.. doxygenfunction:: jluna::unsafe::get_array_size(unsafe::Array*, size_t dimension_index)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::get_index(unsafe::Array*, Index... index_per_dimension)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::get_index(unsafe::Array*, size_t)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::get_index(unsafe::Array*, size_t, size_t)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::set_index(unsafe::Array*, unsafe::Value* value, Index... index_per_dimension)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::set_index(unsafe::Array*, unsafe::Value* value, size_t)

--------------

.. doxygenfunction:: jluna::unsafe::docs_only::set_index(unsafe::Array*, unsafe::Value* value, size_t, size_t)

--------------

.. doxygenfunction:: jluna::unsafe::get_array_data

--------------

.. doxygenfunction:: jluna::unsafe::swap_array_data

--------------

.. doxygenfunction:: jluna::unsafe::set_array_data

--------------

.. doxygenfunction:: jluna::unsafe::push_front

--------------

.. doxygenfunction:: jluna::unsafe::push_back

--------------

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
.. doxygentypedef:: jluna::VectorAny

--------------

Vector
^^^^^^

.. doxygenclass:: jluna::Vector
    :members:

--------------

cppcall
*******

.. doxygenfunction:: jluna::as_julia_function

--------------

.. doxygenfunction:: jluna::register_function(std::function<Return_t()>)

--------------

.. doxygenfunction:: jluna::register_function(std::function<Return_t(Arg1_t)> f)

--------------

.. doxygenfunction:: jluna::register_function(std::function<Return_t(Arg1_t, Arg2_t)> f)

--------------

.. doxygenfunction:: jluna::register_function(std::function<Return_t(Arg1_t, Arg2_t, Arg3_t)> f)

--------------

Exceptions
**********

.. doxygenclass:: jluna::JuliaException
    :members:

--------------

.. doxygenstruct:: jluna::JuliaUninitializedException
    :members:

--------------

.. doxygenfunction:: jluna::forward_last_exception

--------------

.. doxygenfunction:: jluna::throw_if_uninitialized

--------------

Generator Expression
********************

.. doxygenclass:: jluna::GeneratorExpression
    :members:

--------------

.. doxygenclass:: jluna::GeneratorExpression::ForwardIterator
    :members:

--------------

Multi Threading
***************

Future
^^^^^^

.. doxygenclass:: jluna::Future
    :members:

--------------

Task
^^^^

.. doxygenclass:: jluna::Task
    :members:

--------------

.. doxygenfunction:: jluna::yield

--------------

ThreadPool
^^^^^^^^^^

.. doxygenclass:: jluna::ThreadPool
    :members:

--------------

Mutex
^^^^^

.. doxygenclass:: jluna::Mutex
    :members:

-------------

Symbol
******

.. doxygenclass:: jluna::Symbol
    :members:

-------------

Type
****

.. doxygenclass:: jluna::Type
    :members:

--------------

.. doxygenvariable:: jluna::AbstractArray_t
.. doxygenvariable:: jluna::AbstractChar_t
.. doxygenvariable:: jluna::AbstractFloat_t
.. doxygenvariable:: jluna::AbstractString_t
.. doxygenvariable:: jluna::Any_t
.. doxygenvariable:: jluna::Array_t
.. doxygenvariable:: jluna::Bool_t
.. doxygenvariable:: jluna::Char_t
.. doxygenvariable:: jluna::DataType_t
.. doxygenvariable:: jluna::DenseArray_t
.. doxygenvariable:: jluna::Exception_t
.. doxygenvariable:: jluna::Expr_t
.. doxygenvariable:: jluna::Float16_t
.. doxygenvariable:: jluna::Float32_t
.. doxygenvariable:: jluna::Float64_t
.. doxygenvariable:: jluna::Function_t
.. doxygenvariable:: jluna::GlobalRef_t
.. doxygenvariable:: jluna::IO_t
.. doxygenvariable:: jluna::Int128_t
.. doxygenvariable:: jluna::Int16_t
.. doxygenvariable:: jluna::Int32_t
.. doxygenvariable:: jluna::Int64_t
.. doxygenvariable:: jluna::Int8_t
.. doxygenvariable:: jluna::Integer_t
.. doxygenvariable:: jluna::LineNumberNode_t
.. doxygenvariable:: jluna::Method_t
.. doxygenvariable:: jluna::Missing_t
.. doxygenvariable:: jluna::Module_t
.. doxygenvariable:: jluna::NTuple_t
.. doxygenvariable:: jluna::NamedTuple_t
.. doxygenvariable:: jluna::Nothing_t
.. doxygenvariable:: jluna::Number_t
.. doxygenvariable:: jluna::Pair_t
.. doxygenvariable:: jluna::Ptr_t
.. doxygenvariable:: jluna::QuoteNode_t
.. doxygenvariable:: jluna::Real_t
.. doxygenvariable:: jluna::Ref_t
.. doxygenvariable:: jluna::Signed_t
.. doxygenvariable:: jluna::String_t
.. doxygenvariable:: jluna::Symbol_t
.. doxygenvariable:: jluna::Task_t
.. doxygenvariable:: jluna::Tuple_t
.. doxygenvariable:: jluna::Type_t
.. doxygenvariable:: jluna::TypeVar_t
.. doxygenvariable:: jluna::UInt128_t
.. doxygenvariable:: jluna::UInt16_t
.. doxygenvariable:: jluna::UInt32_t
.. doxygenvariable:: jluna::UInt64_t
.. doxygenvariable:: jluna::UInt8_t
.. doxygenvariable:: jluna::UndefInitializer_t
.. doxygenvariable:: jluna::Union_t
.. doxygenvariable:: jluna::UnionAll_t
.. doxygenvariable:: jluna::UnionEmpty_t
.. doxygenvariable:: jluna::Unsigned_t
.. doxygenvariable:: jluna::VecElement_t
.. doxygenvariable:: jluna::WeakRef_t

-------------

Typedefs
********

.. doxygentypedef:: jluna::Bool
.. doxygentypedef:: jluna::Char
.. doxygentypedef:: jluna::Int8
.. doxygentypedef:: jluna::Int16
.. doxygentypedef:: jluna::Int32
.. doxygentypedef:: jluna::Int64
.. doxygentypedef:: jluna::UInt8
.. doxygentypedef:: jluna::UInt16
.. doxygentypedef:: jluna::UInt32
.. doxygentypedef:: jluna::UInt64
.. doxygentypedef:: jluna::Float32
.. doxygentypedef:: jluna::Float64
.. doxygentypedef:: jluna::Nothing
.. doxygentypedef:: jluna::unsafe::Value
.. doxygentypedef:: jluna::unsafe::Function
.. doxygentypedef:: jluna::unsafe::Symbol
.. doxygentypedef:: jluna::unsafe::Module
.. doxygentypedef:: jluna::unsafe::Expression
.. doxygentypedef:: jluna::unsafe::Array
.. doxygentypedef:: jluna::unsafe::DataType

--------------

.. doxygenstruct:: jluna::as_julia_type
    :members:

--------------

.. code-block:: cpp
    :caption: Concept: is :code:`as_julia_type` defined for type :code:`T`

    template<typename T>
    concept to_julia_type_convertable = requires(T)
    {
        as_julia_type<T>::type_name;
    };

-------------

Usertype
********

.. doxygendefine:: set_usertype_enabled

.. doxygenclass:: jluna::Usertype
    :members:
