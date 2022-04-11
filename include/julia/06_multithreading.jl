
"""
Lock that will stall both Julia and C++ threads
"""
mutable struct Mutex

    _native_handle::Csize_t

    function Mutex()
        out = new(ccall((:new_mutex, jluna._cppcall._library_name), Csize_t, ()))
        println(Threads.threadid(), " created mutex #", out._native_handle)
        finalizer(function (this::Mutex)
            ccall((:delete_mutex, jluna._cppcall._library_name), Cvoid, (Csize_t,), this._native_handle)
        end, out)
        return out
    end
end

new_lock() = Base.ReentrantLock();
task_lock = Base.ReentrantLock();

function Base.lock(mutex::Mutex)

    @async try
        ccall((:lock_mutex, jluna._cppcall._library_name), Cvoid, (Csize_t,), mutex._native_handle)
    finally end

    jl_lock = unsafe_pointer_to_objref(Ptr{Base.ReentrantLock}(ccall((:get_lock, jluna._cppcall._library_name), Csize_t, (Csize_t,), mutex._native_handle)))
    Base.lock(jl_lock)
end

function Base.unlock(mutex::Mutex)
    @async try
        ccall((:lock_mutex, jluna._cppcall._library_name), Cvoid, (Csize_t,), mutex._native_handle)
    finally end

    jl_lock = unsafe_pointer_to_objref(Ptr{Base.ReentrantLock}(ccall((:get_lock, jluna._cppcall._library_name), Csize_t, (Csize_t,), mutex._native_handle)))
    Base.unlock(jl_lock)
end

function Base.trylock(mutex::Mutex)
    @async try
        ccall((:lock_mutex, jluna._cppcall._library_name), Cvoid, (Csize_t,), mutex._native_handle)
    finally end

    jl_lock = unsafe_pointer_to_objref(Ptr{Base.ReentrantLock}(ccall((:get_lock, jluna._cppcall._library_name), Csize_t, (Csize_t,), mutex._native_handle)))
    Base.trylock(jl_lock)
end
