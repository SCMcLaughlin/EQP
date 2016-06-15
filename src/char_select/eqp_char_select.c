
#include "eqp_char_select.h"

void char_select_init(R(CharSelect*) charSelect, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath)
{
    (void)ipcPath;
    (void)masterIpcPath;
    
    shm_viewer_init(&charSelect->shmViewerLogWriter);
    shm_viewer_open(B(charSelect), &charSelect->shmViewerLogWriter, logWriterIpcPath, sizeof(IpcBuffer));
    // Tell the log writer to open our log file
    ipc_buffer_write(B(charSelect), shm_viewer_memory_type(&charSelect->shmViewerLogWriter, IpcBuffer), ServerOpLogOpen, EQP_SOURCE_ID_CHAR_SELECT, 0, NULL);
    
    core_init(C(charSelect), EQP_SOURCE_ID_CHAR_SELECT, shm_viewer_memory_type(&charSelect->shmViewerLogWriter, IpcBuffer));
    
    charSelect->socket = udp_socket_create(B(charSelect));
    udp_socket_open(charSelect->socket, 9000);
}

void char_select_deinit(R(CharSelect*) charSelect)
{
    core_deinit(C(charSelect));
    shm_viewer_close(&charSelect->shmViewerLogWriter);
    
    if (charSelect->socket)
    {
        udp_socket_destroy(charSelect->socket);
        charSelect->socket = NULL;
    }
}

void char_select_main_loop(R(CharSelect*) charSelect)
{
    R(UdpSocket*) socket = charSelect->socket;
    
    for (;;)
    {
        udp_socket_recv(socket);
        
        db_thread_execute_query_callbacks(core_db_thread(C(charSelect)));
        timer_pool_execute_callbacks(&charSelect->timerPool);
        
        udp_socket_send(socket);
        udp_socket_check_timeouts(socket);
        
        clock_sleep_milliseconds(50);
    }
}
