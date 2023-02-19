

module cppcall

    #const _lib = "<call jluna::initialize to initialize this field>";

    function verify_library() ::Bool
        return ccall((:jluna_verify, cppcall._lib), Bool, ());
    end

    """
    object that is callable like a function, but executes C++-side code
    """
    mutable struct UnnamedFunction{NArgs}

        _native_handle::Ptr{Cvoid}
        # points to C-side function

        _n_args::Cint
        #  0: (void) -> Any
        #  1: (Any) -> Any
        #  2: (Any, Any) -> Any
        #  3: (Any, Any, Any) -> Any
        # all others invalid

        function UnnamedFunction{N}(ptr::Ptr{Cvoid}) where N

            out = new{N}(ptr, N)
            finalizer(function (t::UnnamedFunction{N})
                ccall((:jluna_free_lambda, cppcall._lib), Cvoid, (Csize_t, Cint), t._native_handle, t._n_args)
            end, out);

            return out;
        end
    end

    """
    `make_unnamed_function(::Ptr{Cvoid}, n::Integer) -> UnnamedFunction{n}

    wrapper for UnnamedFunction ctor
    """
    function make_unnamed_function(ptr::Ptr{Cvoid}, n::Integer)
        return UnnamedFunction{n}(ptr)
    end

    """
    `invoke_function(::UnnamedFunction) -> Ptr{Any}`

    invoke function with 0 args
    """
    function invoke_function(f::UnnamedFunction{0}) ::Ptr{Any}
        return ccall((:jluna_invoke_lambda_0, cppcall._lib), Ptr{Any}, (Ptr{Cvoid},), f._native_handle);
    end

    # overload for 1 arg
    function invoke_function(f::UnnamedFunction{1}, arg1::Ptr{Any}) ::Ptr{Any}
        return ccall((:jluna_invoke_lambda_1, cppcall._lib), Ptr{Any}, (Ptr{Cvoid}, Ptr{Any}), f._native_handle, arg1);
    end

    # overload for 2 args
    function invoke_function(f::UnnamedFunction{2}, arg1::Ptr{Any}, arg2::Ptr{Any}) ::Ptr{Any}
        return ccall((:jluna_invoke_lambda_2, cppcall._lib), Ptr{Any}, (Ptr{Cvoid}, Ptr{Any}, Ptr{Any}), f._native_handle, arg1, arg2);
    end

    # overload for 3 args
    function invoke_function(f::UnnamedFunction{3}, arg1::Ptr{Any}, arg2::Ptr{Any}, arg3::Ptr{Any}) ::Ptr{Any}
        return ccall((:jluna_invoke_lambda_3, cppcall._lib), Ptr{Any}, (Ptr{Cvoid}, Ptr{Any}, Ptr{Any}, Ptr{Any}), f._native_handle, arg1, arg2, arg3);
    end

    """
    `to_pointer(::Any) -> Ptr{Any}`

    get pointer to any object (including immutable ones)
    """
    function to_pointer(x) ::Ptr{Any}
        return ccall((:jluna_to_pointer, cppcall._lib), Ptr{Cvoid}, (Any,), x)
    end

    """
    `from_pointer(::Ptr{Any}) -> Any`

    wrap unsafe_pointer_to_objref
    """
    function from_pointer(ptr::Ptr{Any})
        return unsafe_pointer_to_objref(ptr)
    end

    """
    `UnnamedFunction(xs...) -> Any`

    invoke UnnamedFunction, checks for correct number of arguments
    """
    function (f::UnnamedFunction{N})(xs...) where N

        n = length(xs)

        if n == N == 0
            return from_pointer(invoke_function(f));
        elseif n == N == 1
            return from_pointer(invoke_function(f, to_pointer(xs[1])));
        elseif n == N == 2
            return from_pointer(invoke_function(f, to_pointer(xs[1]), to_pointer(xs[2])));
        elseif n == N == 3
            return from_pt(invoke_function(f, to_pointer(xs[1]), to_pointer(xs[2]), to_pointer(xs[3])));
        elseif N != 0 && N != 1 && N != 2 & N != 3
            return from_pointer(invoke_function(f, to_pointer([xs...])));
        else
            throw(ErrorException(
                "MethodError: when trying to invoke <C++ Lambda#" * string(f._native_handle) * ">" *
                ": wrong number of arguments. expected " * string(N) * ", got " * string(n) * "."
               *  (n <= 3 ? "" : "\n\nTo create a C++-function that can take n > 3 arguments, simply make a 1-argument function with the only argument being an n-sized tuple or collection.")
            ))
        end
    end


    """
    `make_task(::UInt64) -> Task`
    """
    function make_task(ptr::UInt64)
        return Task() do;
            res_ptr = ccall((:jluna_invoke_from_task, _lib), Csize_t, (Csize_t,), ptr);
            return unsafe_pointer_to_objref(Ptr{Any}(res_ptr))
        end
    end
end

# obfuscate internal state to encourage using operator[] sytanx
struct ProxyInternal

    _fieldnames_in_order::Vector{Symbol}
    _fields::Dict{Symbol, Union{Any, Missing}}
    _lock::Base.ReentrantLock

    ProxyInternal() = new(Vector{Symbol}(), Dict{Symbol, Union{Any, Missing}}(), Base.ReentrantLock())
end

# proxy as deepcopy of cpp-side usertype object
struct Proxy

    _typename::Symbol
    _value::ProxyInternal

    Proxy(name::Symbol) = new(name, ProxyInternal())
end

"""
`new_proxy(::Symbol) -> Proxy`

wrap proxy ctor
"""
new_proxy(name::Symbol) = return Proxy(name)

"""
`implement(::Proxy, ::Module) -> Type`

translate a usertype proxy into an actual julia type
"""
function implement(template::Proxy, m::Module = Main) ::Type

    @lock template._value._lock begin
        out::Expr = :(mutable struct $(template._typename) end)
        deleteat!(out.args[3].args, 1)

        for name in template._value._fieldnames_in_order
            push!(out.args[3].args, Expr(:(::), name, :($(typeof(template._value._fields[name])))))
        end

        new_call::Expr = Expr(:(=), Expr(:call, template._typename), Expr(:call, :new))

        for name in template._value._fieldnames_in_order
            push!(new_call.args[2].args, template._value._fields[name])
        end

        push!(out.args[3].args, new_call)
        Base.eval(m, out)
    end
    return m.eval(template._typename)
end

"""
`setindex!(::Proxy, <:Any, ::Symbol) -> Nothing`
extend base.setindex!
"""
function Base.setindex!(proxy::Proxy, value, key::Symbol) ::Nothing

    @lock proxy._value._lock begin
        if (!haskey(proxy._value._fields, key))
            push!(proxy._value._fieldnames_in_order, key)
        end

        proxy._value._fields[key] = value
    end
    return nothing
end

"""
`getindex(::Proxy, ::Symbol) -> Any`
extend base.getindex
"""
function Base.getindex(proxy::Proxy, value, key::Symbol) #::Auto

    out::Any = undef
    @lock proxy._value._lock begin
        out = proxy._value._fields[key]
    end
    return out
end

end # end of module jluna
return true # used for testing