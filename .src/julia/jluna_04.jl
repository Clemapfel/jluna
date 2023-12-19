
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

