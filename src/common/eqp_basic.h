
#ifndef EQP_BASIC_H
#define EQP_BASIC_H

#include "define.h"
#include "exception.h"
#include "eqp_log.h"

/*
    The 'Basic' object contains the state that all running threads should
    maintain independent of one another.
*/

STRUCT_DEFINE(Basic)
{
    ExceptionState  exceptionState;
    int             sourceId;
    LogIpc          logIpc;
};

#define B(obj) ((Basic*)(obj))

void    basic_preinit(R(Basic*) basic);
void    basic_init(R(Basic*) basic, int sourceId, R(IpcBuffer*) logIpc);
void    basic_copy(R(Basic*) dst, R(Basic*) src);
void    basic_deinit(R(Basic*) basic);

#define basic_source_id(basic) ((basic)->sourceId)
#define basic_log_ipc(basic) (&(basic)->logIpc)

#endif//EQP_BASIC_H
