
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
            
            if (server->name)
                string_destroy(server->name);
            
            if (server->ipAddress)
                string_destroy(server->ipAddress);
        }
        
        array_destroy(list->array);
        list->array = NULL;
    }
}

void server_list_add(R(ServerList*) list, R(ServerListing*) server)
{
    uint32_t len = 0;
    
    if (server->name)
        len += string_length(server->name) + 1;
    
    if (server->ipAddress)
        len += string_length(server->ipAddress) + 1;
    
    server->nameAndIpLength = len;
    
    array_push_back(list->basic, &list->array, server);
}
