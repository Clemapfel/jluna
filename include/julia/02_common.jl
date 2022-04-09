"""
`safe_call(::Function, args...) ::Tuple{Any, Bool, Exception, String)`

safely call any function, while forwarding any exception that may have occurred
"""
function safe_call(f::Function, args...)

    res::Any = undef

    backtrace::String = ""
    exception_occurred::Bool = false
    exception::Union{Exception, UndefInitializer} = undef

    try
        res = f(args...)
    catch e
        exception = e
        backtrace = sprint(Base.showerror, exception, catch_backtrace())
        exception_occurred = true
    end

    return (res, exception_occurred, exception, backtrace)
end

"""
`dot(::Array, field::Symbol) -> Any`

wrapped dot operator
"""
function dot(x::Array, field_name::Symbol) ::Any

    index_maybe = parse(Int, string(field_name));
    @assert index_maybe isa Integer
    return x[index_maybe];
end
export dot;

"""
`dot(::Module, field::Symbol) -> Any`

wrapped dot operator
"""
dot(x::Module, field_name::Symbol) = return x.eval(field_name);

"""
`dot(x::Any, field::Symbol) -> Any`

wrapped dot operator, x.field
"""
dot(x::Any, field_name::Symbol) = return eval(:($x.$field_name))

"""
`unroll_type(::Type) -> Type`

unroll type declaration
"""
function unroll_type(type::Type) ::Type

    while hasproperty(type, :body)
        type = type.body
    end

    return type
end

"""
`is_name_typename(::Type, ::Type) -> Bool`

unroll type declaration, then check if name is typename
"""
function is_name_typename(type_in::Type, type_comparison::Type) ::Bool
    return getproperty(type_in, :name) == Base.typename(type_comparison)
end

"""
`get_n_fields(::Type) -> Int64`
"""
function get_n_fields(type::Type) ::Int64
    return length(fieldnames(type))
end

"""
`get_fields(::Type) -> Vector{Pair{Symbol, Type}}`

get field symbols and types, used by jluna::Type::get_fields
"""
function get_fields(type::Type) ::Vector{Pair{Symbol, Type}}

    out = Vector{Pair{Symbol, Type}}();
    names = fieldnames(type)
    types = fieldtypes(type)

    for i in 1:(length(names))
        push!(out, names[i] => types[i])
    end

    return out
end

"""
`get_parameter(::Type) -> Vector{Pair{Symbol, Type}}`

get parameter symbols and upper type limits, used by jluna::Type::get_parameters
"""
function get_parameters(type::Type) ::Vector{Pair{Symbol, Type}}

    while !hasproperty(type, :parameters)
        type = type.body
    end

    out = Vector{Pair{Symbol, Type}}();
    parameters = getproperty(type, :parameters)

    for i in 1:(length(parameters))
        push!(out, parameters[i].name => parameters[i].ub)
    end

    return out
end

"""
`get_n_parameters(::Type) -> Int64`
"""
function get_n_parameters(type::Type) ::Int64

    type = unroll_type(type)

    return length(getproperty(type, :parameters))
end

"""
`assign_in_module(::Module, ::Symbol, ::T) -> T`

assign variable in other module, throws if variable does not exist
"""
function assign_in_module(m::Module, variable_name::Symbol, value::T) ::T where T

    if (!isdefined(m, variable_name))
        throw(UndefVarError(Symbol(string(m) * "." * string(variable_name))))
    end

    return Base.eval(m, :($variable_name = $value))
end

"""
`create_in_module(::Module, ::Symbol, ::T) -> T`

assign variable in other module, if variable does not exist, create then assign
"""
function create_or_assign_in_module(m::Module, variable_name::Symbol, value::T) ::T where T
    return Base.eval(m, :($variable_name = $value))
end

"""
`get_names(::Module) -> IdDict{Symbol, Any}`

access all module members as dict
"""
function get_names(m::Module) ::IdDict{Symbol, Any}

    out = IdDict{Symbol, Any}()

    for n in names(m; all = true)
        if string(n)[1] != '#'
            out[n] = m.eval(n)
        end
    end

    return out
end

"""
`get_nth_method(::Function, ::Integer) -> Method`

wrap method access, used by jlune::Method
"""
function get_nth_method(f::Function, i::Integer) ::Method

    return methods(f)[i]
end

"""
`get_return_type_of_nth_method(::Function, ::Integer) -> Type`

used by jluna::Function to deduce method signature
"""
function get_return_type_of_nth_method(f::Function, i::Integer) ::Type

    return Base.return_types(test)[i]
end

"""
`get_argument_type_of_nths_methods(::Function, ::Integer) -> Vector{Type}`

used by jluna::Function to deduce method signature
"""
function get_argument_types_of_nth_method(f::Function, i::Integer) ::Vector{Type}

    out = Vector{Type}()
    types = methods(f)[i].sig.types

    for i in 2:length(types)
        push!(out, types[i])
    end

    return out
end


"""
`get_length_of_generator(::Base.Generator) -> Int64`

deduce length of Base.Generator object
"""
function get_length_of_generator(gen::Base.Generator) ::Int64

    if (Base.haslength(gen))
        return length(gen)
    else
        # heuristically deduce length
        for i in Iterators.reverse(gen.iter.itr)
            if gen.iter.flt(i)
                return i
            end
        end
    end
end

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
