
#ifndef EQP_ERROR_CODES_H
#define EQP_ERROR_CODES_H

#include "define.h"

ENUM_DEFINE(ErrorCode)
{
    Try,
    Finally,
    ErrorOutOfMemory,
    ErrorFormatString,
    ErrorSpawnProcess,
    ErrorDatabase,
    ErrorSql,
    ErrorSyncPrimitive,
    ErrorShareMemInit,
    ErrorConsole,
    ErrorLua,
    ErrorIpc,
    ErrorThread
};

#endif//EQP_ERROR_CODES_H
