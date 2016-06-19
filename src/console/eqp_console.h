
#ifndef EQP_CONSOLE_H
#define EQP_CONSOLE_H

#include "define.h"
#include "eqp_basic.h"
#include "eqp_clock.h"
#include "ipc_buffer.h"
#include "share_mem.h"
#include "server_op.h"
#include "source_id.h"

#ifdef EQP_LINUX
# include <dirent.h>
#endif

#define EQP_CONSOLE_PATH_ENV_VARIABLE "EQP_PATH"
#define EQP_CONSOLE_SHM_PATH "./shm/eqp-console-"
#define EQP_CONSOLE_TIMEOUT_MILLISECONDS 5000

STRUCT_DEFINE(Console)
{
    Basic       basicState;
    IpcBuffer*  ipcSend;
    IpcBuffer*  ipcRecv;
    ShmViewer   shmViewerConsole;
    ShmCreator  shmCreatorConsole;
    ShmViewer   shmViewerMaster;
};

void    console_init(R(Console*) console);
void    console_deinit(R(Console*) console);
int     console_send(R(Console*) console, int argc, R(const char**) argv);
void    console_recv(R(Console*) console);

#endif//EQP_CONSOLE_H
