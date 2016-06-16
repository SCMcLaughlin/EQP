
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
    
    charSelect->L = lua_sys_open(B(charSelect));
    
    charSelect->socket = udp_socket_create(B(charSelect));
    udp_socket_open(charSelect->socket, EQP_CHAR_SELECT_PORT);
    
    charSelect->loginServerConnections = array_create_type(B(charSelect), TcpClient);
}

void char_select_deinit(R(CharSelect*) charSelect)
{
    core_deinit(C(charSelect));
    shm_viewer_close(&charSelect->shmViewerLogWriter);
    
    if (charSelect->L)
    {
        lua_close(charSelect->L);
        charSelect->L = NULL;
    }
    
    if (charSelect->socket)
    {
        udp_socket_destroy(charSelect->socket);
        charSelect->socket = NULL;
    }
    
    if (charSelect->loginServerConnections)
    {
        array_destroy(charSelect->loginServerConnections);
        charSelect->loginServerConnections = NULL;
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

void char_select_start_login_server_connections(R(CharSelect*) charSelect)
{
    R(lua_State*) L = charSelect->L;
    uint32_t n;
    uint32_t i;
    
    lua_sys_run_file(B(charSelect), L, EQP_LUA_SYS_LOGIN_CONFIG_SCRIPT, 1);
    
    n = lua_objlen(L, -1);
    
    for (i = 1; i <= n; i++)
    {
        R(TcpClient*) client;
        R(LoginServerConfig*) server = eqp_alloc_type(B(charSelect), LoginServerConfig);
        
        lua_pushinteger(L, i);
        lua_gettable(L, -2);
        
        server->longName    = lua_sys_field_to_string(B(charSelect), L, -1, "longname");
        server->shortName   = lua_sys_field_to_string(B(charSelect), L, -1, "shortname");
        server->host        = lua_sys_field_to_string(B(charSelect), L, -1, "host");
        server->port        = lua_sys_field_to_string(B(charSelect), L, -1, "port");
        server->username    = lua_sys_field_to_string(B(charSelect), L, -1, "username");
        server->password    = lua_sys_field_to_string(B(charSelect), L, -1, "password");
        
        // Push new TcpClient object
        client = array_push_back_type(B(charSelect), &charSelect->loginServerConnections, TcpClient);
        
        tcp_client_init(charSelect, client, server);
        tcp_client_start_connect_cycle(client);
        
        lua_pop(L, 1);
    }
    
    lua_clear(L);
}
