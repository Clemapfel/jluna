module _cppcall

    mutable struct State
        _arguments::Tuple
        _result::Any

        State() = new((), nothing)
    end

    #_library_name = "<variable created during jluna::initialize>"
    const _state = Base.Threads.resize_nthreads!([Ref(State())])

    const get_state_lock = Base.ReentrantLock()
    function get_state()
        lock(get_state_lock)
        result = _state[Threads.threadid()]
        unlock(get_state_lock)
        return result
    end

    const cppcall_lock = Base.ReentrantLock()

    """
    Wrapper object for unnamed functions, frees function once object is destroyed
    """
    mutable struct UnnamedFunctionProxy{N, Return_t}

        _id::Symbol
        _call::Function
        _n_args::Int64

        function UnnamedFunctionProxy{N, Return_t}(id::Symbol) where {N, Return_t}

            global unnamed_function_proxy_ctor_lock = Base.ReentrantLock()

            @assert(-1 <= N <= 4)

            _id = id
            x = new{N, Return_t}(id, function (xs...) Main.cppcall(_id, xs...) end, N)

            @lock unnamed_function_proxy_ctor_lock finalizer(function (t::UnnamedFunctionProxy{N, Return_t})
                ccall((:free_function, _cppcall._library_name), Cvoid, (Csize_t,), hash(t._id))
            end, x)

            return x
        end
    end

    # call with signature (1x Any) -> [Any / Nothing]
    function (instance::UnnamedFunctionProxy{0, T})() ::T where T
        return instance._call();
    end

    # call with signature (1x Any) -> [Any / Nothing]
    function (instance::UnnamedFunctionProxy{1, T})(arg1) ::T where T
        return instance._call(arg1);
    end

    # call with signature (2x Any) -> [Any / Nothing]
    function (instance::UnnamedFunctionProxy{2, T})(arg1, arg2) ::T where T
        return instance._call(arg1, arg2);
    end

    # call with signature (3x Any) -> [Any / Nothing]
    function (instance::UnnamedFunctionProxy{3, T})(arg1, arg2, arg3) ::T where T
        return instance._call(arg1, arg2, arg3);
    end

    # call with signature (4x Any) -> [Any / Nothing]
    function (instance::UnnamedFunctionProxy{4, T})(arg1, arg2, arg3, arg4) ::T where T
        return instance._call(arg1, arg2, arg3, arg4);
    end

    # call with other signature
    function (instance::UnnamedFunctionProxy{Int64(-1), T})(args::Vector) ::T where T
        return instance._call(args...);
    end

    # ctor wrapper for jluna
    new_unnamed_function(s::Symbol, N::Integer, T::Type) = return UnnamedFunctionProxy{N, T}(s)

    """
    an exception thrown when trying to invoke cppcall with a function name that
    has not yet been registered via jluna::register_function
    """
    mutable struct UnregisteredFunctionNameException <: Exception

        _function_name::Symbol
    end
    Base.showerror(io::IO, e::UnregisteredFunctionNameException) = print(io, "cppcall.UnregisteredFunctionNameException: no C++ function with name :" * string(e._function_name) * " registered")

    """
    an exception thrown when the number of arguments does not match the number of arguments
    expected by the registered lambda
    """
    mutable struct TupleSizeMismatchException <: Exception

        _function_name::Symbol
        _expected::Int64
        _got::Int64
    end
    Base.showerror(io::IO, e::TupleSizeMismatchException) = print(io, "cppcall.TupleSizeMismatchException: C++ function with name :" * string(e._function_name) * " expects " * string(e._expected) * " arguments but was called with " * string(e._got))

    """
    `set_result(::Any) -> Nothing`

    modify _cppcall state result
    """
    function set_result(x::Any) ::Nothing

        global _cppcall.get_state().x._result = x
        return nothing
    end

    """
    `get_result() -> Any`

    access _cppcall result
    """
    function get_result() ::Any

        return _cppcall.get_state().x._result
    end

    """
    `set_arguments(xs...) -> Nothing`

    modify _cppcall state argument tuple
    """
    function set_arguments(xs...) ::Nothing

        global _cppcall.get_state().x._arguments = xs
        return nothing
    end

    """
    `get_result() -> Tuple`

    access _cppcall state argument tuple
    """
    function get_arguments() ::Tuple

        return _cppcall.get_state().x._arguments
    end

    """
    `verify_library() -> Bool`

    check if c_adapter library is available
    """
    function verify_library() ::Bool

        if isfile(_cppcall._library_name)
            return true
        end

        message = "when trying to initialize jluna.cppcall: "
        message *= "cannot find " * _cppcall._library_name

        println(sprint(Base.showerror, AssertionError(message), backtrace()))
        return false
    end
end

function make_task(ptr::UInt64)
    return Task() do;
        res_ptr = ccall((:invoke, _cppcall._library_name), Csize_t, (Csize_t,), ptr);
        return unsafe_pointer_to_objref(Ptr{Any}(res_ptr))
    end

    #res.sticky = false
end

# obfuscate internal state to encourage using operator[] sytanx
struct ProxyInternal

    _fieldnames_in_order::Vector{Symbol}
    _fields::Dict{Symbol, Union{Any, Missing}}
    ProxyInternal() = new(Vector{Symbol}(), Dict{Symbol, Union{Any, Missing}}())
end

# proxy as deepcopy of cpp-side usertype object
struct Proxy

    _typename::Symbol
    _value::ProxyInternal

    Proxy(name::Symbol) = new(name, ProxyInternal())
end
export proxy
new_proxy(name::Symbol) = return Proxy(name)

const implement_lock = Base.ReentrantLock()
function implement(template::Proxy, m::Module = Main) ::Type

    lock(implement_lock)

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

    result = m.eval(template._typename)
    unlock(implement_lock)

    return result
end
export implement