
#include "eqp_login.h"

void login_init(R(Login*) login, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath)
{
    (void)ipcPath;
    (void)masterIpcPath;
    
    server_list_init(B(login), &login->serverList);
    client_list_init(B(login), &login->clientList);
    
    shm_viewer_init(&login->shmViewerLogWriter);
    shm_viewer_open(B(login), &login->shmViewerLogWriter, logWriterIpcPath, sizeof(IpcBuffer));
    // Tell the log writer to open our log file
    ipc_buffer_write(B(login), shm_viewer_memory_type(&login->shmViewerLogWriter, IpcBuffer), ServerOpLogOpen, EQP_SOURCE_ID_LOGIN, 0, NULL);
    
    core_init(C(login), EQP_SOURCE_ID_LOGIN, shm_viewer_memory_type(&login->shmViewerLogWriter, IpcBuffer));
    
    login->socket = udp_socket_create(B(login));
    udp_socket_open(login->socket, 5998);
    
    tcp_server_init(login, &login->tcpServer);
    tcp_server_open(&login->tcpServer, 5998);
    
    login->crypto = login_crypto_create(B(login));
}

void login_deinit(R(Login*) login)
{
    core_deinit(C(login));
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

void login_main_loop(R(Login*) login)
{
    R(UdpSocket*) socket = login->socket;
    R(TcpServer*) server = &login->tcpServer;
    
    for (;;)
    {
        udp_socket_recv(socket);
        db_thread_execute_query_callbacks(core_db_thread(C(login)));
        udp_socket_send(socket);
        udp_socket_check_timeouts(socket);
        
        tcp_server_accept_new_connections(server);
        tcp_server_recv(server);
        
        clock_sleep_milliseconds(50);
    }
}
