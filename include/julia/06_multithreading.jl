
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

const _mutex_lock_lock = Base.ReentrantLock();
const _mutex_unlock_lock = Base.ReentrantLock();

function Base.lock(mutex::Mutex)

    println(Threads.threadid(), " attempt lock");
    lock(unsafe_pointer_to_objref(Ptr{Base.ReentrantLock}(mutex._native_handle)));
    println(Threads.threadid(), " lock done");
end

function Base.unlock(mutex::Mutex)

    println(Threads.threadid(), " attempt unlock");
    unlock(unsafe_pointer_to_objref(Ptr{Base.ReentrantLock}(mutex._native_handle)));
    println(Threads.threadid(), " unlock done");
end

function Base.trylock(mutex::Mutex)
    #@lock _mutex_llock ccall((:try_lock_mutex, jluna._cppcall._library_name), Cvoid, (Csize_t,), mutex._native_handle)
end
