
#ifndef EQP_LOG_H
#define EQP_LOG_H

#include "define.h"
#include "terminal.h"
#include "exception.h"
#include "ipc_buffer.h"
#include "server_op.h"
#include "source_id.h"
#include <time.h>

#define EQP_LOG_MESSAGE_SIZE 2048

STRUCT_DECLARE(Basic);

ENUM_DEFINE(LogType)
{
    LogNone,
    LogFatal,
    LogError,
    LogInfo,
    LogSql,
    LogNetwork,
    LogLua
};

STRUCT_DEFINE(LogIpc)
{
    IpcBuffer*  ipc;
};

void    log_init(LogIpc* log, IpcBuffer* ipc);
void    log_format(Basic* basic, LogType type, const char* fmt, ...);
void    log_from_format(Basic* basic, int sourceId, LogType type, const char* fmt, ...);
void    log_from_vformat(Basic* basic, int sourceId, LogType type, const char* fmt, va_list args);

#endif//EQP_LOG_H
