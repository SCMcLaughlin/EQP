
#include "eqp_login.h"

void login_init(R(Login*) login, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath)
{
    server_list_init(B(login), &login->serverList);
    client_list_init(B(login), &login->clientList);
    
    shm_viewer_init(&login->shmViewerSelf);
    shm_viewer_open(B(login), &login->shmViewerSelf, ipcPath, sizeof(IpcBuffer));
    login->ipc = shm_viewer_memory_type(&login->shmViewerSelf, IpcBuffer);
    
    shm_viewer_init(&login->shmViewerMaster);
    shm_viewer_open(B(login), &login->shmViewerMaster, masterIpcPath, sizeof(IpcBuffer));
    login->ipcMaster = shm_viewer_memory_type(&login->shmViewerMaster, IpcBuffer);
    
    shm_viewer_init(&login->shmViewerLogWriter);
    shm_viewer_open(B(login), &login->shmViewerLogWriter, logWriterIpcPath, sizeof(IpcBuffer));
    // Tell the log writer to open our log file
    ipc_buffer_write(B(login), shm_viewer_memory_type(&login->shmViewerLogWriter, IpcBuffer), ServerOp_LogOpen, EQP_SOURCE_ID_LOGIN, 0, NULL);
    
    core_init(C(login), EQP_SOURCE_ID_LOGIN, shm_viewer_memory_type(&login->shmViewerLogWriter, IpcBuffer));
    
    login->socket = udp_socket_create(B(login));
    udp_socket_open(login->socket, EQP_LOGIN_PORT);
    
    tcp_server_init(login, &login->tcpServer);
    tcp_server_open(&login->tcpServer, EQP_LOGIN_PORT);
    
    login->crypto = login_crypto_create(B(login));
}

void login_deinit(R(Login*) login)
{
    core_deinit(C(login));
    
    shm_viewer_close(&login->shmViewerSelf);
    shm_viewer_close(&login->shmViewerMaster);
    shm_viewer_close(&login->shmViewerLogWriter);
    
    server_list_deinit(&login->serverList);
    client_list_deinit(&login->clientList);
    
    if (login->socket)
    {
        udp_socket_destroy(login->socket);
        login->socket = NULL;
    }
    
    tcp_server_deinit(&login->tcpServer);
}

static int login_check_ipc(R(Login*) login)
{
    R(IpcBuffer*) ipc = login->ipc;
    
    if (ipc_buffer_try(B(login), ipc))
    {
        for (;;)
        {
            IpcPacket packet;
            
            if (!ipc_buffer_read(B(login), ipc, &packet))
                break;
            
            if (ipc_packet_opcode(&packet) == ServerOp_Shutdown)
                return true;
            
            ipc_packet_deinit(&packet);
        }
    }
    
    return false;
}

static void login_send_keep_alive(R(Login*) login)
{
    ipc_buffer_write(B(login), login->ipcMaster, ServerOp_KeepAlive, EQP_SOURCE_ID_LOGIN, 0, NULL);
}

void login_main_loop(R(Login*) login)
{
    R(UdpSocket*) socket        = login->socket;
    R(TcpServer*) server        = &login->tcpServer;
    uint64_t lastKeepAliveTime  = 0;
    uint64_t curTime;
    
    for (;;)
    {
        udp_socket_recv(socket);
        db_thread_execute_query_callbacks(core_db_thread(C(login)));
        udp_socket_send(socket);
        udp_socket_check_timeouts(socket);
        
        tcp_server_accept_new_connections(server);
        tcp_server_recv(server);
        
        if (login_check_ipc(login))
        {
            log_format(B(login), LogInfo, "Shutting down cleanly");
            break;
        }
        
        curTime = clock_milliseconds();
        if ((curTime - lastKeepAliveTime) >= EQP_LOGIN_KEEP_ALIVE_DELAY_MILLISECONDS)
        {
            login_send_keep_alive(login);
            lastKeepAliveTime = curTime;
        }
        
        clock_sleep_milliseconds(50);
    }
}
