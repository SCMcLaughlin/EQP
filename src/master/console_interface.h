
#ifndef EQP_CONSOLE_INTERFACE_H
#define EQP_CONSOLE_INTERFACE_H

#include "define.h"
#include "ipc_buffer.h"
#include "server_op.h"
#include "source_id.h"
#include "child_process.h"
#include "eqp_alloc.h"
#include "aligned.h"
#include "lua_sys.h"
#include "exception.h"
#include "eqp_string.h"

STRUCT_DECLARE(Master);
STRUCT_DECLARE(MasterIpcThread);

STRUCT_DEFINE(Console)
{
    Master*             M;
    MasterIpcThread*    ipcThread;
    int                 sourceId;
    union
    {
        ChildProcess    procConsole;
        ChildProcess*   procPtr;
    };
    uint32_t            replyCharNameLength;
    char                replyCharName[32];
};

int console_receive(MasterIpcThread* ipcThread, Master* M, ChildProcess* proc, int sourceId, IpcPacket* packet);

EQP_API void console_reply(Console* console, const char* msg, uint32_t length);
EQP_API void console_finish(Console* console);

#endif//EQP_CONSOLE_INTERFACE_H
