
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

