
#ifndef EQP_CLIENT_H
#define EQP_CLIENT_H

#include "define.h"
#include "network_interface.h"
#include "protocol_handler.h"
#include "mob.h"
#include "server_structs.h"
#include "expansion.h"
#include "eqp_string.h"
#include "inventory.h"
#include "skills.h"

#define EQP_CLIENT_ZONE_IN_EXPECTED_TIMEOUT TIMER_SECONDS(30)

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(Zone);

STRUCT_DEFINE(Client)
{
    // Mob must be the first member of this struct
    Mob                 mob;
    ProtocolHandler*    handler;
    int                 expansion;
    atomic_int          refCount;
    
    uint32_t    isStubClient    : 1;
    uint32_t    isLocal         : 1;
    uint32_t    bools_unused    : 30;
    
    union
    {
        struct
        {
            uint8_t connection  : 1;
            uint8_t stats       : 1;
            uint8_t inventory   : 1;
            uint8_t skills      : 1;
            uint8_t unused      : 4;
        };
        
        uint8_t total;
    } loaded;
    
    int         zoneClientIndex;
    
    Inventory   inventory;
    Skills      skills;
    
    String*     surname;
    uint8_t     guildRank;
    int         guildId;
    
    int64_t     experience;
    int64_t     characterId;
    String*     accountName;
    uint32_t    accountId;
    uint32_t    ipAddress;
};

Client* client_create(R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning);
#define client_grab(cli) atomic_fetch_add(&(cli)->refCount, 1)
void    client_drop(R(Client*) client);

void    client_catch_up_with_loading_progress(R(Client*) client);
void    client_check_loading_finished(R(Client*) client);

#define client_set_handler(cli, handler) ((cli)->handler = (handler))
#define client_handler(cli) ((cli)->handler)
#define client_name(cli) mob_name(&(cli)->mob)
#define client_zone_cluster(cli) mob_zone_cluster(&(cli)->mob)

#endif//EQP_CLIENT_H
