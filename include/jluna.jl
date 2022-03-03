#
# Copyright 2021 Clemens Cords
# Created on 26.12.2021 by clem (mail@clemens-cords.com)
#

"""
module with julia-functionality needed by the C++ side of jluna.
Most end-user should not call any function in this module, with `cppcall` being the only exception
"""
module jluna

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
        return Main.eval(:($symbol = $value))
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

        type = unroll_type(type)

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
    get_length_of_generator(::Base.Generator) -> Int64

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
    offers verbose exception interface. Any call with safe_call will store
    the last exception and full stack trace as string in _last_exception and
    _last_message respectively
    """
    module exception_handler

        """
        exception Placeholder that signals that no exception occurred
        """
        struct NoException <: Exception end
        export NoException

        """
        current state of the exception handler

        _last_exception <:Exception
        _last_message ::String
        """
        mutable struct State
            _last_exception
            _last_message::String
            _exception_occurred::Bool
        end

        const _state = Ref{State}(State(NoException(), "", false));

        """
        `safe_call(::Expr, ::Module = Main) -> Any`

        call any line of code, update the handler then forward the result, if any
        """
        function safe_call(expr::Expr, m::Module = Main) ::Any

            if expr.head == :block
                expr.head = :toplevel
            end

            result = undef
            try
                result = Base.eval(m, expr)
                update()
            catch exc
                result = nothing
                update(exc)
            end

            return result
        end

        """
        `unsafe_call(::Expr, ::Module = Main) -> Any`

        call any line of code without updating the handler
        """
        function unsafe_call(expr::Expr, m::Module = Main) ::Any

            if expr.head == :block
                expr.head = :toplevel
            end

            return Base.eval(m, expr);
        end

        """
        `safe_call(::Function, ::Any...) -> Any`

        call any function, update the handler then forward the result, if any
        """
        function safe_call(f::Any, args...)

            result = undef
            try
                result = f(args...)
                update()
            catch exc
                result = nothing
                update(exc)
            end

            return result;
        end

        """
        `update(<:Exception) -> Nothing`

        update the handler after an exception was thrown
        """
        function update(exception::Exception) ::Nothing

            try
            global _state[]._last_message = sprint(Base.showerror, exception, catch_backtrace())
            global _state[]._last_exception = exception
            global _state[]._exception_occurred = true
            catch e end
            return nothing
        end

        """
        `update() -> Nothing`

        update the handler after *no* exception was thrown
        """
        function update() ::Nothing

            global _state[]._last_message = ""
            global _state[]._last_exception = NoException()
            global _state[]._exception_occurred = false
            return nothing
        end

        """
        `has_exception_occurred() -> Bool`

        is last exception type no "jluna.exception_handler.NoException"
        """
        function has_exception_occurred() ::Bool

            return _state[]._exception_occurred
        end

        """
        `get_last_message() -> String`

        wrapper for _state access
        """
        function get_last_message() ::String
            return _state[]._last_message
        end

        """
        `get_last_exception() -> Exception`

        wrapper for _state access
        """
        function get_last_exception() ::Exception
            return _state[]._last_exception
        end
    end

    """
    offers julia-side memory management for C++ jluna
    """
    module memory_handler

        const _current_id = Ref(UInt64(0));
        const _refs = Ref(Dict{UInt64, Base.RefValue{Any}}())
        const _ref_counter = Ref(Dict{UInt64, UInt64}())

        const _ref_id_marker = '#'
        const _refs_expression = Meta.parse("jluna.memory_handler._refs[]")

        # proxy id that is actually an expression, the ID of topmodule Main is
        ProxyID = Union{Expr, Symbol, Nothing}

        # make as unnamed
        make_unnamed_proxy_id(id::UInt64) = return Expr(:ref, Expr(:ref, _refs_expression, id))

        # make as named with owner and symbol name
        make_named_proxy_id(id::Symbol, owner_id::ProxyID) ::ProxyID = return Expr(:(.), owner_id, QuoteNode(id))

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
                out = replace(out, reg => "<unnamed function proxy #" * string(tryparse(Int64, captures.captures[1])) * ">")
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
                println("\t", Int64(e.first), " => ", e.second[], " (", typeof(e.second[]), ")")
            end
        end

        """
        `create_reference(::UInt64, ::Any) -> UInt64`

        add reference to _refs
        """
        function create_reference(to_wrap::Any) ::UInt64

            global _current_id[] += 1;
            key::UInt64 = _current_id[];

            if (haskey(_refs[], key))
                _ref_counter[][key] += 1
            else
                _refs[][key] = Base.RefValue{Any}(to_wrap)
                _ref_counter[][key] = 1
            end

            return key;
        end

        create_reference(_::Nothing) ::UInt64 = return 0

        """
        `set_reference(::UInt64, ::T) -> Nothing`

        update the value of a reference in _refs without adding a new entry or changing it's key, ref pointers C++ side stay valid
        """
        function set_reference(key::UInt64, new_value::T) ::Base.RefValue{Any} where T

            _refs[][key] = Base.RefValue{Any}(new_value)
            return _refs[][key]
        end

        """
        `get_reference(::Int64) -> Any`

        access reference in _refs
        """
        function get_reference(key::UInt64) ::Any

            if (key == 0)
                return nothing
            end

           return _refs[][key]
        end

        """
        `free_reference(::UInt64) -> Nothing`

        free reference from _refs
        """
        function free_reference(key::UInt64) ::Nothing

            if (key == 0)
                return nothing;
            end

            if _refs[][key][] isa Module
                return
            end

            global _ref_counter[][key] -= 1
            count::UInt64 = _ref_counter[][key]

            if (count == 0)
                delete!(_ref_counter[], key)
                delete!(_refs[], key)
            end

            return nothing;
        end

        """
        `force_free() -> Nothing`

        immediately deallocate all C++-managed values
        """
        function force_free() ::Nothing

            for k in keys(_refs)
                delete!(_refs[], k)
                delete!(_ref_counter[], k)
            end

            return nothing;
        end
    end

    module _cppcall

        mutable struct State
            _arguments::Tuple
            _result::Any

            State() = new((), nothing)
        end

        _library_name = "<call jluna::State::initialize before using cppcall>"
        _state = Base.Ref{_cppcall.State}(State())

        """
        Wrapper object for unnamed functions, frees function once object is destroyed
        """
        mutable struct UnnamedFunctionProxy{N, Return_t}

            _id::Symbol
            _call::Function
            _n_args::Int64

            function UnnamedFunctionProxy{N, Return_t}(id::Symbol) where {N, Return_t}

                @assert(-1 <= N <= 4)

                _id = id
                x = new{N, Return_t}(id, function (xs...) Main.cppcall(_id, xs...) end, N)

                finalizer(function (t::UnnamedFunctionProxy{N, Return_t})
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

            global _cppcall._state[]._result = x
            return nothing
        end

        """
        `get_result() -> Any`

        access _cppcall result
        """
        function get_result() ::Any

            return _cppcall._state[]._result
        end

        """
        `set_arguments(xs...) -> Nothing`

        modify _cppcall state argument tuple
        """
        function set_arguments(xs...) ::Nothing

            global _cppcall._state[]._arguments = xs
            return nothing
        end

        """
        `get_result() -> Tuple`

        access _cppcall state argument tuple
        """
        function get_arguments() ::Tuple

            return _cppcall._state[]._arguments
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

    # internal id of proxies, starts at Max(Int32) to avoid id collision with UnnamedFunctionProxy
    const _proxy_id = Base.Ref{UInt64}(0x00000000ffffffff)

    # obfuscate internal state to encourage using operator[] sytanx
    struct ProxyInternal

        _fields::Dict{Symbol, Union{Any, Missing}}
        ProxyInternal() = new(Dict{Symbol, Union{Any, Missing}}())
    end

    # proxy as deepcopy of cpp-side usertype object
    struct Proxy

        _id::UInt64
        _typename::Symbol
        _value::ProxyInternal

        function Proxy(name::Symbol)
            global _proxy_id.x += 1
            new(_proxy_id.x, name, ProxyInternal())
        end
    end
    export proxy
    new_proxy(name::Symbol) = return Proxy(name)
end

using Main.jluna;

"""
`setindex!(::Proxy, <:Any, ::Symbol) -> Nothing`

extend base.setindex!
"""
function Base.setindex!(proxy::Main.jluna.Proxy, value, key::Symbol) ::Nothing
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

    return jluna._cppcall.get_result()
end
export cppcall