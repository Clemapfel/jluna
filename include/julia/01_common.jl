"""
`get_value_type_of_array(::Array{T}) -> Type`

forward value type of array
"""
function get_value_type_of_array(_::Array{T}) ::Type where T

    return T
end

"""
`get_reference_value(::Base.RefValue{T}) -> T`

forward value of reference
"""
function get_reference_value(ref::Base.RefValue{T}) ::T where T

    return ref[];
end

setindex!(str::String, c::Char, i::Int64) = setindex!(collect(str), c, i)

"""
`is_number_only(::String) -> Bool`

check whether a string can be transformed into a base 10 number
"""
function is_number_only(x::String) ::Bool

    for s in x
        if s != '0' || s != '1' || s != '2' || s != '3' || s != '4' || s != '5' || s != '6' || s != '7' || s != '8' || s != '9'
            return false
        end
    end

    return true;
end

"""
`is_method_available(::Function, ::Any) -> Bool`

check if method of given function is available for a specific variable
"""
function is_method_available(f::Function, variable) ::Bool

    return hasmethod(f, Tuple{typeof(variable)})
end

"""
`get_function(::Symbol, ::Module) -> Function`

exception-safe function access wrapper
"""
function get_function(function_name::Symbol, m::Module) ::Function

    return m.eval(function_name)
end
export get_function

"""
`exists(<:AbstractArray, ::Any) -> Bool`

check if element exists in array
"""
function exists(array::T, v::Any) ::Bool where T <: AbstractArray

    return !isempty(findall(x -> x == v, array))
end

"""
`tuple_at(::Tuple, ::Integer) -> Any`

get nth element of tuple
"""
function tuple_at(x::Tuple, i::Integer)
    return x[i]
end

"""
`tuple_length(::Tuple) -> Integer`

get length of tuple
"""
function tuple_length(x::Tuple{N}) where N
    return N
end

"""
`make_new(::String, xs...) -> Any`

parse string to type, then call ctor with given args
"""
function make_new(name::String, xs...) ::Any

    return Main.eval(:($(Meta.parse(name))($(xs...))))
end

"""
`make_vector(::T...) -> Vector{T}`

wrap vector ctor in varargs argument, used by box/unbox
"""
function make_vector(args::T...) ::Vector{T} where T

    return [args...]
end

"""
`make_vector(t::Type) -> Vector{t}`

create empty vector of given type
"""
function make_vector(t::Type) ::Vector{t}

    return Vector{t}()
end

"""
`make_vector(::Type{T}, ::Any...) -> Vector{T}`

create vector by converting all elements to target type
"""
function make_vector(type::Type{T}, xs...) where T
   return convert.(T, [xs...])
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
`make_set(::T...) -> Set{T}`

wrap set ctor in varargs argument, used by box/unbox
"""
function make_set(args::T...) ::Set{T} where T

    return Set([args...]);
end

"""
`make_set(::Type{T}, ::Any...) -> Set{T}`


"""
function make_set(type::Type{T}, xs...) ::Set{T} where T

    return Set(make_vector(type, xs...));
end

"""
`make_pair(::T, ::U) -> Pair{T, U}`

wrap pair ctor
"""
function make_pair(a::T, b::U) ::Pair{T, U} where {T, U}

    return a => b
end

"""
`make_complex(:T, :T) -> Complex{T}`

wrap complex ctor
"""
function make_complex(real::T, imag::T) ::Complex{T} where T
    return Complex{T}(real, imag)
end

"""
`assert_isa(::T, ::Symbol) -> Nothing`

throw assertion if x is not of named type
"""
function assert_isa(x::Any, type::Type) ::Nothing

    if !(x isa type)
        throw(AssertionError("expected " * string(type) * " but got an object of type " * string(typeof(x))));
    end

    return nothing
end

"""
`convert(::T, symbol::Symbol) -> Any`

convert value type, declared via symbol
"""
function convert(x::T, symbol::Symbol) ::Any where T

    type = Main.eval(symbol);
    @assert type isa Type

    if T isa type
        return T
    end

    return Base.convert(type, x)
end

"""
`string_to_type` -> Type

evaluate string to type name if possible
"""
function string_to_type(str::String) ::Type

    res = Main.eval(Meta.parse(str))
    if !(res isa Type)
       throw(UndefVarError(Symbol(str)))
    end
    return res
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