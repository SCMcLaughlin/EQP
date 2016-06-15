
#ifndef EQP_SERVER_LIST_H
#define EQP_SERVER_LIST_H

#include "define.h"
#include "eqp_string.h"
#include "eqp_array.h"

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
    uint32_t        nameAndIpLength;    // Saves us some dereferences when calculating packet lengths
    String*         longName;
    String*         shortName;
    String*         remoteIpAddress;
    String*         localIpAddress;
};

STRUCT_DEFINE(ServerList)
{
    Basic*  basic;
    Array*  array;
};

void        server_list_init(R(Basic*) basic, R(ServerList*) list);
void        server_list_deinit(R(ServerList*) list);

uint32_t    server_list_add(R(ServerList*) list, R(ServerListing*) server);
void        server_list_remove_by_index(R(ServerList*) list, uint32_t index);
void        server_list_update_by_index(R(ServerList*) list, uint32_t index, int playerCount, int status);

void        server_list_send_client_login_request_by_ip_address(R(Login*) login, R(const char*) ipAddress, uint32_t accountId);

#define     server_list_count(list) array_count((list)->array)
#define     server_list_data(list) array_data_type((list)->array, ServerListing)

#define     server_listing_status(server) ((server)->status)
#define     server_listing_rank(server) ((server)->rank)
#define     server_listing_player_count(server) ((server)->playerCount)
#define     server_listing_name(server) ((server)->longName)
#define     server_listing_ip_address(server) ((server)->remoteIpAddress)
#define     server_listing_strings_length(server) ((server)->nameAndIpLength)

#endif//EQP_SERVER_LIST_H
