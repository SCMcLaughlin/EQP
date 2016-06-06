
#include "exception.h"
#include "eqp_basic.h"
#include "eqp_string.h"

void exception_init(R(Basic*) basic)
{
    R(ExceptionState*) es = &basic->exceptionState;
    
    memset(es, 0, sizeof(ExceptionState));
    
    es->errMsg = string_create(basic);
}

void exception_deinit(R(Basic*) basic)
{
    R(ExceptionState*) es = &basic->exceptionState;
    
    if (es->errMsg)
    {
        free(es->errMsg);
        es->errMsg = NULL;
    }
}

void exception_throw(R(Basic*) basic, ErrorCode errcode)
{
    R(ExceptionState*) es = &basic->exceptionState;
    
    es->state = errcode;
    longjmp(es->topScope->jmpBuf, (int)errcode);
}

void exception_throw_message(R(Basic*) basic, ErrorCode errcode, R(const char*) msg, uint32_t len)
{
    string_set_from_cstr(basic, &basic->exceptionState.errMsg, msg, len ? len : strlen(msg));
    exception_throw(basic, errcode);
}

void exception_throw_format(R(Basic*) basic, ErrorCode errcode, R(const char*) fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    string_set_from_vformat(basic, &basic->exceptionState.errMsg, fmt, args);
    va_end(args);
    exception_throw(basic, errcode);
}

int exception_try(R(Basic*) basic, R(ExceptionScope*) scope)
{
    ExceptionState* es = &basic->exceptionState;
    
    // Link in the new exception scope
    scope->prev     = es->topScope;
    es->topScope    = scope;
    
    return setjmp(scope->jmpBuf);
}

void exception_handled(R(Basic*) basic)
{
    String* errMsg = basic->exceptionState.errMsg;
    basic->exceptionState.state = Try;
    
    if (errMsg)
        string_clear(errMsg);
}

void exception_end_try(R(Basic*) basic)
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

void exception_end_try_with_finally(R(Basic*) basic)
{
    R(ExceptionState*) es   = &basic->exceptionState;
    R(ExceptionScope*) top  = es->topScope;
    
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

String* exception_get_message(R(Basic*) basic)
{
    return basic->exceptionState.errMsg;
}
