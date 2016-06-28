
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

#define make_ip(a, b, c, d) (((a << 0) & 0xff) | ((b << 8) & 0xff00) | ((c << 16) & 0xff0000) | ((d << 24) & 0xff000000))

int login_is_ip_address_local(uint32_t ip)
{
    uint32_t check;
    
    // Is it the loopback address 127.0.0.1?
    check = make_ip(127, 0, 0, 1);
    if (ip == check)
        goto ret_true;
    
    // Is it on the 192.168.x.x subnet?
    check = make_ip(192, 168, 0, 0);
    if ((ip & 0x0000ffff) == check)
        goto ret_true;
    
    // Is it on the 10.x.x.x subnet?
    check = make_ip(10, 0, 0, 0);
    if ((ip & 0x000000ff) == check)
        goto ret_true;
    
    // Is it on the 172.16.0.0 to 172.31.255.255 subnet (12-bit mask)?
    check = make_ip(172, 16, 0, 0);
    if ((ip & 0x0000f0ff) == check)
        goto ret_true;

    return false;
    
ret_true:
    return true;
}

#undef make_ip
