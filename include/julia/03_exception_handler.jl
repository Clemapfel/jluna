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
    `safe_call(::Expr, ::Module = Main) -> Any`

    call any line of code, update the handler then forward the result, if any
    """
    function safe_call(expr::Expr, m::Module = Main) ::Any

        if expr.head == :block
            expr.head = :toplevel
        end

        return safe_call(Base.eval, m, expr);
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

        if !(_state[]._last_exception isa NoException)

            global _state[]._last_message = ""
            global _state[]._last_exception = NoException()
            global _state[]._exception_occurred = false
        end
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