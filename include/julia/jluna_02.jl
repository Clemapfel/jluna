#
# Copyright 2022 Clemens Cords
# Created on 03.05.2022 by clem (mail@clemens-cords.com)
#

"""
    `new_array(::Type, dims::Int64...) -> Array{Type, length(dims))`
    """
    function new_array(value_type::Type, lengths::Int64...)

        length = 1;
        for i in lengths
            length *= i
        end

        out = Array{value_type, 1}(undef, length)
        return reshape(out, lengths...)
    end

    """
    `get_value_type_of_array(::Array{T}) -> Type`

    forward value type of array
    """
    function get_value_type_of_array(_::Array{T}) ::Type where T

        return T
    end

    """
    `new_vector(::Integer, ::T) -> Vector{T}`

    create vector by deducing argument type
    """
    function new_vector(size::Integer, _::T) where T
        return Vector{T}(undef, size)
    end

    """
    `new_vector(::Integer, ::T) -> Vector{T}`

    create vector by deducing argument type
    """
    function new_vector(size::Integer, type::Type)
        return Vector{type}(undef, size)
    end

    """
    `new_complex(:T, :T) -> Complex{T}`

    wrap complex ctor
    """
    function new_complex(real::T, imag::T) ::Complex{T} where T
        return Complex{T}(real, imag)
    end

    """
    `invoke(function::Any, arguments::Any...) -> Any`

    wrap function call for non-function objects
    """
    function invoke(x::Any, args...) ::Any
        return x(args...)
    end

    """
    `create_or_assign(::Symbol, ::T) -> T`

    assign variable in main, or if none exist, create it and assign
    """
    function create_or_assign(symbol::Symbol, value::T) ::T where T

        return Main.eval(Expr(:(=), symbol, value))
    end

    """
    `serialize(<:AbstractDict{T, U}) -> Vector{Pair{T, U}}`

    transform dict into array
    """
    function serialize(x::T) ::Vector{Pair{Key_t, Value_t}} where {Key_t, Value_t, T <: AbstractDict{Key_t, Value_t}}

        out = Vector{Pair{Key_t, Value_t}}()
        for e in x
            push!(out, e)
        end
        return out;
    end

    """
    `serialize(::Set{T}) -> Vector{T}`

    transform dict into array
    """
    function serialize(x::T) ::Vector{U} where {U, T <: AbstractSet{U}}

        out = Vector{U}()

        for e in x
            push!(out, e)
        end

        return out;
    end

    """
    `new_dict(key_t::Type, value_t::Type, ::Integer) -> Dict{key_t, value_t}`

    create new dict from type, also provides sizehint
    """
    function new_dict(key_t::Type, value_t::Type, sizehint_maybe::Integer = 0)

        out = Dict{key_t, value_t}();
        sizehint!(out, sizehint_maybe);
        return out;
    end


    """
    `new_set(::Type, ::Integer) -> Set`

    create new set from type, also provides sizehint
    """
    function new_set(value_t::Type, sizehint_maybe::Integer = 0)

        out = Set{value_t}();
        sizehint!(out, sizehint_maybe);
        return out;
    end

    """
    `forward_as_pointer(t::Type, ::Ptr{Cvoid}) -> Ptr{t}`
    """
    function forward_as_pointer(type::Type, ptr::Ptr{Cvoid}) ::Ptr{type}
        return Ptr{type}(ptr)
    end

    """
    `new_lock() -> Base.ReentrantLock`
    """
    function new_lock()
        return Base.ReentrantLock()
    end

    module gc_sentinel

        struct ListNode{T}
            _previous::Union{Base.RefValue{ListNode{T}}, Nothing}
            _value::Union{T, Nothing}
            _is_root::Bool

            function ListNode{T}() where T
                new(nothing, nothing, true)
            end

            function ListNode{T}(previous::Base.RefValue{ListNode{T}}, value) where T
                return new(Ref{ListNode{T}}(previous.x), value, false)
            end
        end

        mutable struct List{T}

            _front::Base.RefValue{ListNode{T}}

            function List{T}() where T
                return new(Ref{ListNode{T}}(ListNode{T}()))
            end
        end

        function append!(list::List{T}, value::T) where T
           list._front = Ref{ListNode{T}}(ListNode{T}(list._front, value))
        end

        function front(list::List{T}) ::T where T
            return list._front[]._value
        end

        function pop!(list::List{T}) ::Nothing where T

            if !list._front[]._is_root
                list._front = list._front[]._previous
            end

            return nothing
        end

        function isempty(list::List{T}) ::Bool where T
            return list._front[]._is_root
        end

        # ---

        const _gc_stack = NTuple{Threads.nthreads(), List{Base.RefValue{Any}}}([List{Base.RefValue{Any}}() for i in 1:Threads.nthreads()])

        function gc_push(ptr::Ptr{Cvoid}) ::Nothing
            append!(_gc_stack[Threads.threadid()], Ref{Any}(unsafe_pointer_to_objref(ptr)))
            return nothing
        end

        function gc_pop() ::Nothing
            pop!(_gc_stack[Threads.threadid()])
        end

        function shutdown() ::Nothing

            for i in 1:Threads.nthreads()
                while !isempty(_gc_stack[i])
                    pop!(_gc_stack[i])
                end
            end
            return nothing
        end
    end