
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

