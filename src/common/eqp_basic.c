
#include "eqp_basic.h"

void basic_preinit(R(Basic*) basic)
{
    exception_init(basic);
}

void basic_init(R(Basic*) basic, int sourceId, R(IpcBuffer*) logIpc)
{
    basic->sourceId = sourceId;
    log_init(&basic->logIpc, logIpc);
}

void basic_copy(R(Basic*) dst, R(Basic*) src)
{
    dst->sourceId   = src->sourceId;
    dst->logIpc.ipc = src->logIpc.ipc;
}

void basic_deinit(R(Basic*) basic)
{
    exception_deinit(basic);
}
