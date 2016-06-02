
#ifndef _EQP_ERROR_CODES_H_
#define _EQP_ERROR_CODES_H_

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
    ErrorConsole
};

#endif//_EQP_ERROR_CODES_H_
