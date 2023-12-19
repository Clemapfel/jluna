module jluna

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

