
#ifndef _EQP_EXCEPTION_H_
#define _EQP_EXCEPTION_H_

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

void    exception_init(R(Basic*) basic);
void    exception_deinit(R(Basic*) basic);
void    exception_throw(R(Basic*) basic, ErrorCode errcode);
void    exception_throw_message(R(Basic*) basic, ErrorCode errcode, R(const char*) msg, uint32_t len);
void    exception_throw_format(R(Basic*) basic, ErrorCode errcode, R(const char*) fmt, ...);
int     exception_try(R(Basic*) basic, R(ExceptionScope*) scope);
void    exception_handled(R(Basic*) basic);
void    exception_end_try(R(Basic*) basic);
void    exception_end_try_with_finally(R(Basic*) basic);
String* exception_get_message(R(Basic*) basic);

#endif//_EQP_EXCEPTION_H_
