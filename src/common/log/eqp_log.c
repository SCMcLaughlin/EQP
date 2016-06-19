
#include "eqp_log.h"
#include "eqp_basic.h"

#define ERR_STRFTIME "[log_construct_message] strftime() failed"

static uint32_t log_construct_message(R(Basic*) basic, R(char*) message, LogType type, R(const char*) fmt, va_list args)
{
    time_t rawTime      = time(NULL);
    struct tm* curTime  = localtime(&rawTime);
    size_t pos          = strftime(message, EQP_LOG_MESSAGE_SIZE, "[%Y-%m-%d : %H:%M:%S]", curTime);
    int wrote           = 0;
    
    if (pos == 0)
        exception_throw_literal(basic, ErrorFormatString, ERR_STRFTIME);
    
    switch (type)
    {
    case LogFatal:
        wrote = snprintf(message + pos, EQP_LOG_MESSAGE_SIZE - pos, "[FATAL] ");
        break;
    
    case LogError:
        wrote = snprintf(message + pos, EQP_LOG_MESSAGE_SIZE - pos, "[ERROR] ");
        break;
    
    case LogInfo:
        wrote = snprintf(message + pos, EQP_LOG_MESSAGE_SIZE - pos, "[INFO] ");
        break;
    
    case LogSql:
        wrote = snprintf(message + pos, EQP_LOG_MESSAGE_SIZE - pos, "[SQL] ");
        break;
    
    case LogNone:
    default:
        wrote = snprintf(message + pos, EQP_LOG_MESSAGE_SIZE - pos, " ");
        break;
    }
    
    if (wrote > 0 && (size_t)wrote < (EQP_LOG_MESSAGE_SIZE - pos))
    {
        pos += (size_t)wrote;
        
        wrote = vsnprintf(message + pos, EQP_LOG_MESSAGE_SIZE - pos, fmt, args);
        
        if (wrote > 0 && (size_t)wrote < (EQP_LOG_MESSAGE_SIZE - pos))
        {
            pos += (size_t)wrote;
            return pos;
        }
    }
    
    return 0;
}

void log_init(R(LogIpc*) log, R(IpcBuffer*) ipc)
{
    log->ipc = ipc;
}

void log_format(R(Basic*) basic, LogType type, R(const char*) fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_from_vformat(basic, basic_source_id(basic), type, fmt, args);
    va_end(args);
}

void log_from_format(R(Basic*) basic, int sourceId, LogType type, R(const char*) fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_from_vformat(basic, sourceId, type, fmt, args);
    va_end(args);
}

void log_from_vformat(R(Basic*) basic, int sourceId, LogType type, R(const char*) fmt, va_list args)
{
    char message[EQP_LOG_MESSAGE_SIZE];
    uint32_t length = log_construct_message(basic, message, type, fmt, args);
    
    if (length)
        ipc_buffer_write(basic, basic_log_ipc(basic)->ipc, ServerOpLogMessage, sourceId, length, message);
}

#undef ERR_STRFTIME
