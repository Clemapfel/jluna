

"""
offers julia-side memory management for C++ jluna
"""
module memory_handler

    const _current_id = Ref(UInt64(0)) # modified atomically through locks
    const _refs = Ref(Dict{UInt64, Base.RefValue{Any}}())
    const _ref_counter = Ref(Dict{UInt64, UInt64}())

    const _refs_lock = Base.ReentrantLock()
    const _refs_counter_lock = Base.ReentrantLock()

    const _ref_id_marker = '#'
    const _refs_expression = Meta.parse("jluna.memory_handler._refs[]")

    # proxy id that is actually an expression, the ID of topmodule Main is
    ProxyID = Union{Expr, Symbol, Nothing}

    # make as unnamed
    make_unnamed_proxy_id(id::UInt64) = return Expr(:ref, Expr(:ref, _refs_expression, id))

    # make as named with owner and symbol name
    function make_named_proxy_id(id::Symbol, owner_id::ProxyID) ::ProxyID

        if owner_id == :Main
            return id
        else
            return Expr(:(.), owner_id, QuoteNode(id))
        end
    end

    # make as named with main as owner and symbol name
    make_named_proxy_id(id::Symbol, owner_id::Nothing) ::ProxyID = return id

    # make as named with owner and array index name
    make_named_proxy_id(id::Number, owner_id::ProxyID) ::ProxyID = return Expr(:ref, owner_id, convert(Int64, id))

    # assign to proxy id
    function assign(new_value::T, name::ProxyID) where T

        if new_value isa Symbol || new_value isa Expr
            return Main.eval(Expr(:(=), name, QuoteNode(new_value)))
        else
            return Main.eval(Expr(:(=), name, new_value));
        end
    end

    # eval proxy id
    evaluate(name::ProxyID) ::Any = return Main.eval(name)
    evaluate(name::Symbol) ::Any = return Main.eval(:($name))

    """
    `get_name(::ProxyID) -> String`

    parse name from proxy id
    """
    function get_name(id::ProxyID) ::String

        if length(id.args) == 0
            return "Main"
        end

        current = id
        while current.args[1] isa Expr && length(current.args) >= 2

            if current.args[2] isa UInt64
                current.args[2] = convert(Int64, current.args[2])
            end

            current = current.args[1]
        end

        out = string(id)
        reg = r"\Q((jluna.memory_handler._refs[])[\E(.*)\Q])[]\E"
        captures = match(reg, out)

        if captures != nothing
            out = replace(out, reg => "<unnamed proxy #" * string(tryparse(Int64, captures.captures[1])) * ">")
        end

        return out;
    end

    get_name(::Nothing) ::String = return "Main"
    get_name(s::Symbol) ::String = return string(s)
    get_name(i::Integer) ::String = return "[" * string(i) * "]"

    """
    `print_refs() -> Nothing`

    pretty print _ref state, for debugging
    """
    function print_refs() ::Nothing

        println("jluna.memory_handler._refs: ");
        for e in _refs[]
            println("\t", Int64(e.first), " => ", e.second[], " (", typeof(e.second[]), ") ")
        end
    end

    """
    `create_reference(::Ptr{Cvoid}, ::Any) -> UInt64`

    add reference to _refs
    """
    function create_reference(to_wrap::Ptr{Cvoid}) ::UInt64

        lock(_refs_lock)
        lock(_refs_counter_lock)

        global _current_id[] += 1
        key::UInt64 = _current_id[];

        if (haskey(_refs[], key))
            _ref_counter[][key] += 1
        else
            _refs[][key] = Base.RefValue{Any}(unsafe_pointer_to_objref(to_wrap))
            _ref_counter[][key] = 1
        end

        unlock(_refs_lock)
        unlock(_refs_counter_lock)

        return key;
    end

    create_reference(_::Nothing) ::UInt64 = return 0

    """
    `set_reference(::UInt64, ::T) -> Nothing`

    update the value of a reference in _refs without adding a new entry or changing it's key, ref pointers C++ side stay valid
    """
    function set_reference(key::UInt64, new_value::T) ::Base.RefValue{Any} where T

        lock(_refs_lock)
        result = begin _refs[][key] = Base.RefValue{Any}(new_value) end
        unlock(_refs_lock)
        return result
    end

    """
    `get_reference(::Int64) -> Any`

    access reference in _refs
    """
    function get_reference(key::UInt64) ::Any

        if (key == 0)
            return nothing
        end

        lock(_refs_lock)
        result = _refs[][key]
        unlock(_refs_lock)
        return result
    end

    """
    `free_reference(::UInt64) -> Nothing`

    free reference from _refs
    """
    function free_reference(key::UInt64) ::Nothing

        if (key == 0)
            return nothing;
        end

        lock(_refs_lock)

        if _refs[][key][] isa Module
            unlock(_refs_lock)
            return
        end

        lock(_refs_counter_lock)
        global _ref_counter[][key] -= 1
        count::UInt64 = _ref_counter[][key]

        if (count == 0)
            delete!(_ref_counter[], key)
            delete!(_refs[], key)
        end

        unlock(_refs_lock)
        unlock(_refs_counter_lock)
        return nothing;
    end

    """
    `force_free() -> Nothing`

    immediately deallocate all C++-managed values
    """
    function force_free() ::Nothing

        lock(_refs_lock)
        lock(_refs_counter_lock)

        for k in keys(_refs)
            delete!(_refs[], k)
            delete!(_ref_counter[], k)
        end

        unlock(_refs_lock)
        unlock(_refs_counter_lock)
        return nothing;
    end

    ### GC Sentinel: protects values from being collected

"""
    # declare abstract type
    abstract type AbstractGCSentinel end

    # global storage, one sentinel per thread
    const _sentinels = NTuple{Threads.nthreads(), Vector{Base.Ref{Union{AbstractGCSentinel, Nothing}}}}([[] for _ in 1:(Threads.nthreads())])

    # sentinel, holds N values
    mutable struct GCSentinel{N} <: AbstractGCSentinel
        _values::NTuple{N, Base.RefValue{Any}}
        _n_preserved::Int64
        _index::Int64
    end

    # update sentinel
    function new_sentinel(n::Integer) ::Nothing

        index = length(_sentinels[Threads.threadid()]);
        push!(_sentinels[Threads.threadid()], GCSentinel{n}(NTuple{N, Base.Ref{Any}}([undef for _ in 1:n]), 1, length))

        finalizer(last(_sentinels[Threads.threadid()]), function (x::GCSentinel{n})
            deleteat!(_sentinels[Threads.threadid()], x._index)
        end)
        return nothing
    end

    # add value to sentinel
    function preserve(ptr::Ptr{Cvoid}) where N

        sentinel_ref = _sentinels[Threads.threadid()]
        sentinel_ref[]._values[sentinel_ref[]._n_preserved][] = Ref{Any}(unsafe_pointer_to_objref(ptr))
        sentinel_ref[]._n_preserved += 1
        return nothing
    end

    # release sentinel, all held values can be collected
    function release()
        _sentinels[Threads.threadid()][] = nothing
    end
    """
end

