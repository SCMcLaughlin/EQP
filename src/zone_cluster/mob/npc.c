
#include "npc.h"
#include "zone_cluster.h"
#include "zone.h"

Npc* npc_create(ZC* zc, Zone* zone, NpcPrototype* proto, float x, float y, float z, float heading)
{
    Npc* npc = eqp_alloc_type(B(zc), Npc);
    
    memset(npc, 0, sizeof(Npc));
    
    mob_init_npc(&npc->mob, zc, zone, proto, x, y, z, heading);
    
    return npc;
}

void npc_destroy(Npc* npc)
{
    mob_deinit(&npc->mob);
    
    free(npc);
}

uint32_t npc_adhoc_id(Npc* npc)
{
    return npc->adhocId;
}
