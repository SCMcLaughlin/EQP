
#include "server_list.h"

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
            
            if (server->ipAddress)
                string_destroy(server->ipAddress);
        }
        
        array_destroy(list->array);
        list->array = NULL;
    }
}

uint32_t server_list_add(R(ServerList*) list, R(ServerListing*) server)
{
    uint32_t len = 0;
    
    if (server->longName)
        len += string_length(server->longName) + 1;
    
    if (server->ipAddress)
        len += string_length(server->ipAddress) + 1;
    
    server->nameAndIpLength = len;
    
    array_push_back(list->basic, &list->array, server);
    
    return array_count(list->array) - 1;
}

void server_list_remove_by_index(R(ServerList*) list, uint32_t index)
{
    R(ServerListing*) server;
    
    if (index >= array_count(list->array))
        return;
    
    server = array_get_type(list->array, index, ServerListing);
    
    if (server->longName)
        string_destroy(server->longName);
    
    if (server->shortName)
        string_destroy(server->shortName);
    
    if (server->ipAddress)
        string_destroy(server->ipAddress);
    
    array_swap_and_pop(list->array, index);
}
