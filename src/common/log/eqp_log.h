
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
#define EQP_LOG_SHM_PATH "shm/eqp-log-writer-"

STRUCT_DECLARE(Basic);

ENUM_DEFINE(LogType)
{
    LogNone,
    LogFatal,
    LogError,
    LogInfo,
    LogSql
};

STRUCT_DEFINE(LogIpc)
{
    IpcBuffer*  ipc;
};

void    log_init(R(LogIpc*) log, R(IpcBuffer*) ipc);
void    log_format(R(Basic*) basic, LogType type, R(const char*) fmt, ...);
void    log_from_format(R(Basic*) basic, int sourceId, LogType type, R(const char*) fmt, ...);
void    log_from_vformat(R(Basic*) basic, int sourceId, LogType type, R(const char*) fmt, va_list args);

#endif//EQP_LOG_H
