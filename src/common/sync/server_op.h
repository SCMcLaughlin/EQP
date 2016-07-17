
#ifndef EQP_SERVER_OP_H
#define EQP_SERVER_OP_H

#include "define.h"

ENUM_DEFINE(ServerOp)
{
    ServerOp_None,
    ServerOp_Shutdown,
    ServerOp_KeepAlive,
    /* Logging */
    ServerOp_LogOpen,
    ServerOp_LogClose,
    ServerOp_LogMessage,
    /* Console */
    ServerOp_ConsoleCommand,
    ServerOp_ConsoleMessage,
    ServerOp_ConsoleFinish,
    /* Zoning (CharSelect/ZoneCluster) */
    ServerOp_ClientZoning,
    ServerOp_ClientZoningReject,
    ServerOp_ZoneAddress,
    /* ZoneCluster */
    ServerOp_StartZone,
    /* Items */
    ServerOp_ItemSharedMemoryOpen,
    ServerOp_COUNT
};

#endif//EQP_SERVER_OP_H
