
#include "exception.h"
#include "eqp_basic.h"
#include "eqp_string.h"

void exception_init(Basic* basic)
{
    ExceptionState* es = &basic->exceptionState;
    
    memset(es, 0, sizeof(ExceptionState));
    
    es->errMsg = string_create(basic);
}

void exception_deinit(Basic* basic)
{
    ExceptionState* es = &basic->exceptionState;
    
    if (es->errMsg)
    {
        free(es->errMsg);
        es->errMsg = NULL;
    }
}

void exception_throw(Basic* basic, ErrorCode errcode)
{
    ExceptionState* es = &basic->exceptionState;
    
    es->state = errcode;
    longjmp(es->topScope->jmpBuf, (int)errcode);
}

void exception_throw_message(Basic* basic, ErrorCode errcode, const char* msg, uint32_t len)
{
    string_set_from_cstr(basic, &basic->exceptionState.errMsg, msg, len ? len : strlen(msg));
    exception_throw(basic, errcode);
}

void exception_throw_format(Basic* basic, ErrorCode errcode, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    string_set_from_vformat(basic, &basic->exceptionState.errMsg, fmt, args);
    va_end(args);
    exception_throw(basic, errcode);
}

void exception_begin_try(Basic* basic, ExceptionScope* scope)
{
    ExceptionState* es = &basic->exceptionState;
    
    // Link in the new exception scope
    scope->prev     = es->topScope;
    es->topScope    = scope;
}

void exception_handled(Basic* basic)
{
    String* errMsg = basic->exceptionState.errMsg;
    basic->exceptionState.state = Try;
    
    if (errMsg)
        string_clear(errMsg);
}

void exception_end_try(Basic* basic)
{
    ExceptionState* es = &basic->exceptionState;
    
    // Unlink the top exception scope
    es->topScope = es->topScope->prev;
    
    // Do we have an unhandled exception?
    if (es->state != Try)
    {
        // If so, rethrow
        longjmp(es->topScope->jmpBuf, (int)es->state);
    }
}

void exception_end_try_with_finally(Basic* basic)
{
    ExceptionState* es  = &basic->exceptionState;
    ExceptionScope* top = es->topScope;

    // Has the finally block just finished executing?
    if (es->inFinallyBlock)
    {
        ErrorCode err = es->state;
        
        // Clear finally flag and unlink top exception scope
        es->inFinallyBlock  = 0;
        es->topScope        = top->prev;
        
        // Do we have an unhandled exception to rethrow?
        if (err != Try)
            longjmp(es->topScope->jmpBuf, (int)err);
    }
    else
    {
        // Set the finally flag, and rethrow into the finally block
        es->inFinallyBlock = 1;
        longjmp(top->jmpBuf, (int)Finally);
    }
}

void exception_set_message(Basic* basic, const char* msg, uint32_t len)
{
    string_set_from_cstr(basic, &basic->exceptionState.errMsg, msg, len ? len : strlen(msg));
}

String* exception_get_message(Basic* basic)
{
    return basic->exceptionState.errMsg;
}
