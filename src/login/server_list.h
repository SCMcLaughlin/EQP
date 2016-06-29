
#ifndef EQP_SERVER_LIST_H
#define EQP_SERVER_LIST_H

#include "define.h"
#include "eqp_string.h"
#include "eqp_array.h"
#include "login_client.h"

STRUCT_DECLARE(Login);

ENUM_DEFINE(ServerStatus)
{
    ServerStatus_Up,
    ServerStatus_Down,
    ServerStatus_Locked
};

ENUM_DEFINE(ServerRank)
{
    ServerRank_Standard,
    ServerRank_Preferred,
    ServerRank_Legends
};

STRUCT_DEFINE(ServerListing)
{
    uint32_t        status : 2;
    uint32_t        rank : 2;
    uint32_t        unused : 28;
    int             playerCount;
    uint32_t        nameLength;             // Saves us some dereferences when calculating packet lengths
    uint32_t        remoteAddressLength;    // Ditto
    String*         longName;
    String*         shortName;
    String*         remoteIpAddress;
    String*         localIpAddress;
    bool            isLocal;
};

STRUCT_DEFINE(ServerList)
{
    Basic*  basic;
    Array*  array;
};

void        server_list_init(Basic* basic, ServerList* list);
void        server_list_deinit(ServerList* list);

uint32_t    server_list_add(ServerList* list, ServerListing* server);
void        server_list_remove_by_index(ServerList* list, uint32_t index);
void        server_list_update_by_index(ServerList* list, uint32_t index, int playerCount, int status);

void        server_list_send_client_login_request_by_ip_address(Login* login, const char* ipAddress, uint32_t accountId);

#define     server_list_count(list) array_count((list)->array)
#define     server_list_data(list) array_data_type((list)->array, ServerListing)

#define     server_listing_status(server) ((server)->status)
#define     server_listing_rank(server) ((server)->rank)
#define     server_listing_player_count(server) ((server)->playerCount)
#define     server_listing_name(server) ((server)->longName)
#define     server_listing_remote_address(server) ((server)->remoteIpAddress)
#define     server_listing_local_address(server) ((server)->localIpAddress)
#define     server_listing_name_length(server) ((server)->nameLength)
#define     server_listing_remote_length(server) ((server)->remoteAddressLength)
#define     server_listing_is_local(server) ((server)->isLocal)

#endif//EQP_SERVER_LIST_H
