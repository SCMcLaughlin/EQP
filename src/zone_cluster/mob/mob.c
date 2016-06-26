
#include "mob.h"
#include "zone.h"
#include "zone_cluster.h"

void mob_init_client(R(Mob*) mob, R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning)
{
    mob->mobType        = MobType_Client;
    mob->entityId       = -1;
    mob->zoneMobIndex   = -1;
    
    mob->name           = string_create_from_cstr(B(zc), zoning->characterName, strlen(zoning->characterName));
    
    mob->zone           = zone;
    mob->zoneCluster    = zc;
}

void mob_deinit(R(Mob*) mob)
{
    if (mob->name)
    {
        string_destroy(mob->name);
        mob->name = NULL;
    }
    
    if (mob->zoneCluster)
    {
        R(ZC*) zc = mob->zoneCluster;
        
        zc_lua_destroy_object(zc, &mob->luaObj);
    }
}
