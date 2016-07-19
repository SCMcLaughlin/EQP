
#ifndef EQP_EXCEPTION_H
#define EQP_EXCEPTION_H

#include "define.h"
#include "error_codes.h"
#include <setjmp.h>

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(String);

STRUCT_DEFINE(ExceptionScope)
{
    jmp_buf         jmpBuf;
    ExceptionScope* prev;
};

STRUCT_DEFINE(ExceptionState)
{
    ErrorCode       state;
    int             inFinallyBlock;
    ExceptionScope* topScope;
    String*         errMsg;
};

void    exception_init(Basic* basic);
void    exception_deinit(Basic* basic);
void    exception_throw(Basic* basic, ErrorCode errcode);
void    exception_throw_message(Basic* basic, ErrorCode errcode, const char* msg, uint32_t len);
#define exception_throw_literal(basic, errcode, literal) exception_throw_message((basic), (errcode), (literal), sizeof((literal)) - 1)
void    exception_throw_format(Basic* basic, ErrorCode errcode, const char* fmt, ...);
void    exception_begin_try(Basic* basic, ExceptionScope* scope);
#define exception_try(basic) setjmp((basic)->exceptionState.topScope->jmpBuf)
void    exception_handled(Basic* basic);
void    exception_end_try(Basic* basic);
void    exception_end_try_with_finally(Basic* basic);
void    exception_set_message(Basic* basic, const char* msg, uint32_t len);
String* exception_get_message(Basic* basic);
#define exception_message(basic) exception_get_message((basic))

#endif//EQP_EXCEPTION_H
