
#include "mob.h"
#include "zone.h"

void mob_init_client(R(Mob*) mob, R(ZC*) zc, R(Zone*) zone)
{
    mob->zone           = zone;
    mob->zoneCluster    = zc;
}
