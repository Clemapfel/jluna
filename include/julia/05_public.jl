using Main.jluna;

"""
`setindex!(::Proxy, <:Any, ::Symbol) -> Nothing`

extend base.setindex!
"""
function Base.setindex!(proxy::Main.jluna.Proxy, value, key::Symbol) ::Nothing

    if (!haskey(proxy._value._fields, key))
        push!(proxy._value._fieldnames_in_order, key)
    end

    proxy._value._fields[key] = value
    return nothing
end
export setindex!

"""
`getindex(::Proxy, ::Symbol) -> Any`

extend base.getindex
"""
function Base.getindex(proxy::Main.jluna.Proxy, value, key::Symbol) #::Auto
    return proxy._value._fields[key]
end
export getindex

"""
`cppall(::Symbol, ::Any...) -> Any`

Call a lambda registered via `jluna::State::register_function` using `xs...` as arguments.
After the C++-side function returns, return the resulting object
(or `nothing` if the C++ function returns `void`)

This function is not thread-safe and should not be used in a parallel context
"""
function cppcall(function_name::Symbol, xs...) ::Any

    global cppcall_lock = Base.ReentrantLock()
    lock(cppcall_lock)

    id = hash(function_name)

    if !ccall((:is_registered, jluna._cppcall._library_name), Bool, (Csize_t,), id)
        throw(jluna._cppcall.UnregisteredFunctionNameException(function_name))
    end

    n_args_expected = ccall((:get_n_args, jluna._cppcall._library_name), Csize_t, (Csize_t,), id)
    if length(xs) != n_args_expected
        throw(jluna._cppcall.TupleSizeMismatchException(function_name, n_args_expected, length(xs)))
    end

    jluna._cppcall.set_arguments(xs...)
    jluna._cppcall.set_result(nothing)

    ccall((:call_function, jluna._cppcall._library_name), Cvoid, (Csize_t,), id)

    result = jluna._cppcall.get_result()
    unlock(cppcall_lock)
    return result
end
export cppcall