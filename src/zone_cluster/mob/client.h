
#ifndef EQP_CLIENT_H
#define EQP_CLIENT_H

#include "define.h"
#include "network_interface.h"
#include "protocol_handler.h"
#include "mob.h"
#include "server_structs.h"
#include "expansion.h"

#define EQP_CLIENT_ZONE_IN_EXPECTED_TIMEOUT TIMER_SECONDS(30)

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(Zone);

STRUCT_DEFINE(Client)
{
    // Mob must be the first member of this struct
    Mob                 mob;
    ProtocolHandler*    handler;
    atomic_int          refCount;
    
    int     expansion;
    
    uint32_t    bools_unused    : 32;
    
    union
    {
        struct
        {
            uint8_t stats       : 1;
            uint8_t inventory   : 1;
            uint8_t unused      : 6;
        };
        
        uint8_t total;
    } loaded;
    
    int     zoneClientIndex;
    
    String* surname;
};

Client* client_create(R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning);
#define client_grab(cli) atomic_fetch_add(&(cli)->refCount, 1)
void    client_drop(R(Client*) client);

#define client_zone_cluster(cli) mob_zone_cluster(&(cli)->mob)

#endif//EQP_CLIENT_H
