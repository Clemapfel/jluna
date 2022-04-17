#
# Copyright 2022 Clemens Cords
# Created on 17.04.2022 by clem (mail@clemens-cords.com)
#

_c_adapter_path = "<call jluna::initialize to initialize this field>";

"""
object that is callable like a function, but executes C++-side code
"""
struct UnnamedFunctionProxy{NArgs}

    _native_handle::UInt64

    function UnnamedFunctionProxy{NArgs}(id::UInt64)

        out = new{Nargs}(id)
        finalizer(function (t::LambdaProxy{N})
            ccall((:free_function, _cppcall._library_name), Cvoid, (Csize_t, Csize_t), t._native_handle, 0)
        end, out);
    end
end

"""
`make_unnamed_function_proxy(::UInt64, n::UInt64) -> UnnamedFunctionProxy{n}`
"""
make_unnamed_function_proxy(id::UInt64, n_args::UInt64) = return UnnamedFunctionProxy{n_args}(id)

"""
`invoke_function(id::UInt64, n::UInt64) -> Ptr{Any}`
"""
function invoke_function(id::UInt64, xs...)

    n = length(xs...)

    if n == 0
        return Ptr{Any}(ccall((:invoke_0_arg, _cppcall_library_name), Csize_t, (Csize_t, Csize_t), id))
    elseif n == 1
        return Ptr{Any}(ccall((:invoke_1_arg, _cppcall_library_name), Csize_t, (Csize_t, Csize_t), id, xs[1]))
    elseif n == 2
        return Ptr{Any}(ccall((:invoke_2_arg, _cppcall_library_name), Csize_t, (Csize_t, Csize_t), id, xs[1], xs[2]))
    elseif n == 3
        return Ptr{Any}(ccall((:invoke_3_arg, _cppcall_library_name), Csize_t, (Csize_t, Csize_t), id, xs[1], xs[2], xs[3]))
    else
        return Ptr{Any}(ccall((:invoke_3_arg, _cppcall_library_name), Csize_t, (Csize_t, Csize_t), id, Csize_t(pointer_from_objref([xs...]))))
    end
end

# call operator for (void) -> Any
function (f::UnnamedFunctionProxy{N})(xs...)

    n = length(xs...)

    if n == N == 0
        return unsafe_pointer_to_objref(invoke_function(f._native_handle));
    elseif n == N == 1
        return unsafe_pointer_to_objref(invoke_function(f._native_handle, xs[1]));
    elseif n == N == 2
        return unsafe_pointer_to_objref(invoke_function(f._native_handle, xs[1], xs[2]));
    elseif n == N == 3
        return unsafe_pointer_to_objref(invoke_function(f._native_handle, xs[1], xs[2], xs[3]));
    elseif N != 0 && N != 1 && N != 2 & N != 3
        return unsafe_pointer_to_objref(invoke_function(f._native_handle, xs...));
    else
        throw(ErrorException(
            "MethodError: when trying to invoke unnamedFunctionProxy #" * string(_native_handle) *
            ": wrong number of arguments. expected: " * string(N) * ", got: " * string(n)
        ))
    end
end

