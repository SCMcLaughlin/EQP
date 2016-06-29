
#include "eqp_basic.h"

void basic_preinit(Basic* basic)
{
    exception_init(basic);
}

void basic_init(Basic* basic, int sourceId, IpcBuffer* logIpc)
{
    basic->sourceId = sourceId;
    log_init(&basic->logIpc, logIpc);
}

void basic_copy(Basic* dst, Basic* src)
{
    dst->sourceId   = src->sourceId;
    dst->logIpc.ipc = src->logIpc.ipc;
}

void basic_deinit(Basic* basic)
{
    exception_deinit(basic);
}
