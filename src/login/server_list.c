
#include "server_list.h"
#include "eqp_login.h"

void server_list_init(R(Basic*) basic, R(ServerList*) list)
{
    list->basic = basic;
    list->array = array_create_type(basic, ServerListing);
}

void server_list_deinit(R(ServerList*) list)
{
    if (list->array)
    {
        R(ServerListing*) array = server_list_data(list);
        uint32_t n              = server_list_count(list);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            R(ServerListing*) server = &array[i];
            
            if (server->longName)
                string_destroy(server->longName);
            
            if (server->shortName)
                string_destroy(server->shortName);
            
            if (server->remoteIpAddress)
                string_destroy(server->remoteIpAddress);
            
            if (server->localIpAddress)
                string_destroy(server->localIpAddress);
        }
        
        array_destroy(list->array);
        list->array = NULL;
    }
}

uint32_t server_list_add(R(ServerList*) list, R(ServerListing*) server)
{
    server->nameLength          = (server->longName) ? string_length(server->longName) + 1 : 0;
    server->remoteAddressLength = (server->remoteIpAddress) ? string_length(server->remoteIpAddress) + 1 : 0;
    
    array_push_back(list->basic, &list->array, server);
    
    return array_count(list->array) - 1;
}

void server_list_remove_by_index(R(ServerList*) list, uint32_t index)
{
    R(ServerListing*) server = array_get_type(list->array, index, ServerListing);
    
    if (server == NULL)
        return;
    
    if (server->longName)
        string_destroy(server->longName);
    
    if (server->shortName)
        string_destroy(server->shortName);
    
    if (server->remoteIpAddress)
        string_destroy(server->remoteIpAddress);
    
    if (server->localIpAddress)
        string_destroy(server->localIpAddress);
    
    array_swap_and_pop(list->array, index);
}

void server_list_update_by_index(R(ServerList*) list, uint32_t index, int playerCount, int status)
{
    R(ServerListing*) server = array_get_type(list->array, index, ServerListing);
    
    if (server == NULL)
        return;
    
    server->status      = status;
    server->playerCount = playerCount;
}

void server_list_send_client_login_request_by_ip_address(R(Login*) login, R(const char*) ipAddress, uint32_t accountId)
{
    R(ServerList*) list     = login_server_list(login);
    R(ServerListing*) array = array_data_type(list->array, ServerListing);
    uint32_t n              = array_count(list->array);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        R(ServerListing*) server = &array[i];
        
        if (string_compare_cstr(server->remoteIpAddress, ipAddress) == 0)
        {
            tcp_server_send_client_login_request(login_tcp_server(login), (int)i, accountId);
            return;
        }
        
        if (string_compare_cstr(server->localIpAddress, ipAddress) == 0)
        {
            tcp_server_send_client_login_request(login_tcp_server(login), (int)i, accountId);
            return;
        }
    }
}
