
#ifndef EQP_SERVER_OP_H
#define EQP_SERVER_OP_H

#include "define.h"

ENUM_DEFINE(ServerOp)
{
    ServerOpNone,
    ServerOpShutdown,
    // Logging
    ServerOpLogOpen,
    ServerOpLogClose,
    ServerOpLogMessage,
    ServerOpCOUNT
};

#endif//EQP_SERVER_OP_H
