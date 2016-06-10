
#ifndef EQP_SERVER_LIST_H
#define EQP_SERVER_LIST_H

#include "define.h"
#include "eqp_string.h"
#include "eqp_array.h"

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
    String*         name;
    String*         ipAddress;
};

STRUCT_DEFINE(ServerList)
{
    Basic*  basic;
    Array*  array;
};

void        server_list_init(R(Basic*) basic, R(ServerList*) list);
void        server_list_deinit(R(ServerList*) list);

void        server_list_add(R(ServerList*) list, R(ServerListing*) server);

#define     server_list_count(list) array_count((list)->array)
#define     server_list_data(list) array_data_type((list)->array, ServerListing)

#define     server_listing_status(server) ((server)->status)
#define     server_listing_rank(server) ((server)->rank)
#define     server_listing_player_count(server) ((server)->playerCount)
#define     server_listing_name(server) ((server)->name)
#define     server_listing_ip_address(server) ((server)->ipAddress)
#define     server_listing_strings_length(server) ((server)->nameAndIpLength)

#endif//EQP_SERVER_LIST_H
