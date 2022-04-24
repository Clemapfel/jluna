
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

    """
    `new_lock() -> Base.ReentrantLock`
    """
    function new_lock()
        return Base.ReentrantLock();
    end

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
        `create_reference(::UInt64, ::Any) -> UInt64`

        add reference to _refs
        """
        function create_reference(to_wrap::Any) ::UInt64

            lock(_refs_lock)
            lock(_refs_counter_lock)

            global _current_id[] += 1
            key::UInt64 = _current_id[];

            if (haskey(_refs[], key))
                _ref_counter[][key] += 1
            else
                _refs[][key] = Base.RefValue{Any}(to_wrap)
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

        # declare abstract type
        abstract type AbstractGCSentinel end

        # global storage, one sentinel per thread
        const _sentinels = NTuple{Threads.nthreads(), Base.Ref{Union{AbstractGCSentinel, Nothing}}}([nothing for _ in 1:(Threads.nthreads())])

        # sentinel, holds N values
        mutable struct GCSentinel{N} <: AbstractGCSentinel
            _values::NTuple{N, Base.RefValue{Any}}
            _n_preserved::Int64

            function GCSentinel{N}() where N
                return new{N}(NTuple{N, Base.Ref{Any}}([undef for _ in 1:N]), 1)
            end
        end

        # update sentinel
        function new_sentinel(n::Integer) ::Nothing
            _sentinels[Threads.threadid()].x = GCSentinel{n}()
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
    end

    module cppcall

        #const _c_adapter_path = "<call jluna::initialize to initialize this field>";

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
                    ccall((:free_lambda, cppcall._c_adapter_path), Cvoid, (Csize_t, Cint), t._native_handle, t._n_args)
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
            return ccall((:invoke_lambda_0, cppcall._c_adapter_path), Ptr{Any}, (Ptr{Cvoid},), f._native_handle);
        end

        # overload for 1 arg
        function invoke_function(f::UnnamedFunction{1}, arg1::Ptr{Any}) ::Ptr{Any}
            return ccall((:invoke_lambda_1, cppcall._c_adapter_path), Ptr{Any}, (Ptr{Cvoid}, Ptr{Any}), f._native_handle, arg1);
        end

        # overload for 2 args
        function invoke_function(f::UnnamedFunction{2}, arg1::Ptr{Any}, arg2::Ptr{Any}) ::Ptr{Any}
            return ccall((:invoke_lambda_2, cppcall._c_adapter_path), Ptr{Any}, (Ptr{Cvoid}, Ptr{Any}, Ptr{Any}), f._native_handle, arg1, arg2);
        end

        # overload for 3 args
        function invoke_function(f::UnnamedFunction{3}, arg1::Ptr{Any}, arg2::Ptr{Any}, arg3::Ptr{Any}) ::Ptr{Any}
            return ccall((:invoke_lambda_3, cppcall._c_adapter_path), Ptr{Any}, (Ptr{Cvoid}, Ptr{Any}, Ptr{Any}, Ptr{Any}), f._native_handle, arg1, arg2, arg3);
        end

        """
        `to_pointer(::Any) -> Ptr{Cvoid}`

        get pointer to any object (including immutable ones)
        """
        function to_pointer(x) ::Ptr{Any}
            return ccall((:to_pointer, cppcall._c_adapter_path), Ptr{Cvoid}, (Any,), x)
        end

        """
        `from_pointer(::Ptr{Cvoid}) -> Any`

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
                res_ptr = ccall((:invoke_from_task, _c_adapter_path), Csize_t, (Csize_t,), ptr);
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
end

return true # used for testing