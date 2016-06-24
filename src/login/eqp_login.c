
#include "eqp_login.h"

void login_init(R(Login*) login, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath)
{
    server_list_init(B(login), &login->serverList);
    client_list_init(B(login), &login->clientList);
    
    ipc_set_open(B(login), &login->ipcSet, EQP_SOURCE_ID_LOGIN, ipcPath, masterIpcPath, logWriterIpcPath);
    
    core_init(C(login), EQP_SOURCE_ID_LOGIN, ipc_set_log_writer_ipc(&login->ipcSet));
    
    login->socket = udp_socket_create(B(login));
    udp_socket_open(login->socket, EQP_LOGIN_PORT);
    
    tcp_server_init(login, &login->tcpServer);
    tcp_server_open(&login->tcpServer, EQP_LOGIN_PORT);
    
    login->crypto = login_crypto_create(B(login));
}

void login_deinit(R(Login*) login)
{
    core_deinit(C(login));
    ipc_set_deinit(&login->ipcSet);
    
    server_list_deinit(&login->serverList);
    client_list_deinit(&login->clientList);
    
    if (login->socket)
    {
        udp_socket_destroy(login->socket);
        login->socket = NULL;
    }
    
    tcp_server_deinit(&login->tcpServer);
}

void ipc_set_handle_packet(R(Basic*) basic, R(IpcPacket*) packet)
{
    // We need to define this function -- but Login doesn't actually 
    // care about any IPC packets beyond the shutdown signal.
    (void)basic;
    (void)packet;
}

void login_main_loop(R(Login*) login)
{
    R(UdpSocket*) socket        = login->socket;
    R(TcpServer*) server        = &login->tcpServer;
    R(IpcSet*) ipcSet           = &login->ipcSet;
    
    for (;;)
    {
        udp_socket_recv(socket);
        db_thread_execute_query_callbacks(core_db_thread(C(login)));
        udp_socket_send(socket);
        udp_socket_check_timeouts(socket);
        
        tcp_server_accept_new_connections(server);
        tcp_server_recv(server);
        
        if (ipc_set_receive(B(login), ipcSet))
        {
            log_format(B(login), LogInfo, "Shutting down cleanly");
            break;
        }
        
        ipc_set_keep_alive(B(login), ipcSet, EQP_SOURCE_ID_LOGIN);
        
        clock_sleep_milliseconds(50);
    }
}
