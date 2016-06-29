
#include "client_list.h"
#include "eqp_basic.h"

void client_list_init(Basic* basic, ClientList* list)
{
    list->array = array_create_type(basic, ClientByAccountId);
}

void client_list_deinit(ClientList* list)
{
    // UdpSocket is responsible for destroying the clients themselves (via client_on_disconnect)
    if (list->array)
        array_destroy(list->array);
}

void client_list_add(Basic* basic, ClientList* list, LoginClient* client)
{
    ClientByAccountId add;
    
    add.accountId   = login_client_account_id(client);
    add.client      = client;
    
    array_push_back(basic, &list->array, &add);
}

void client_list_remove(ClientList* list, LoginClient* client)
{
    uint32_t accountId          = login_client_account_id(client);
    ClientByAccountId* array    = array_data_type(list->array, ClientByAccountId);
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

LoginClient* client_list_get(ClientList* list, uint32_t accountId)
{
    ClientByAccountId* array    = array_data_type(list->array, ClientByAccountId);
    uint32_t n                  = array_count(list->array);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        ClientByAccountId* by = &array[i];
        
        if (by->accountId == accountId)
            return by->client;
    }
    
    return NULL;
}
