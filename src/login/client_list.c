
#include "client_list.h"
#include "eqp_basic.h"

void client_list_init(R(Basic*) basic, R(ClientList*) list)
{
    list->array = array_create_type(basic, ClientByAccountId);
}

void client_list_deinit(R(ClientList*) list)
{
    // UdpSocket is responsible for destroying the clients themselves (via client_on_disconnect)
    if (list->array)
        array_destroy(list->array);
}

void client_list_add(R(Basic*) basic, R(ClientList*) list, R(LoginClient*) client)
{
    ClientByAccountId add;
    
    add.accountId   = login_client_account_id(client);
    add.client      = client;
    
    array_push_back(basic, &list->array, &add);
}

void client_list_remove(R(ClientList*) list, R(LoginClient*) client)
{
    uint32_t accountId          = login_client_account_id(client);
    R(ClientByAccountId*) array = array_data_type(list->array, ClientByAccountId);
    uint32_t n                  = array_count(list->array);
    uint32_t i                  = 0;
    
    while (i < n)
    {
        if (array[i].accountId == accountId)
        {
            array_swap_and_pop(list->array, i);
            n--;
            continue;
        }
        
        i++;
    }
}

LoginClient* client_list_get(R(ClientList*) list, uint32_t accountId)
{
    R(ClientByAccountId*) array = array_data_type(list->array, ClientByAccountId);
    uint32_t n                  = array_count(list->array);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        R(ClientByAccountId*) by = &array[i];
        
        if (by->accountId == accountId)
            return by->client;
    }
    
    return NULL;
}
