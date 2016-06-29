
#ifndef EQP_ZONE_CLUSTER_H
#define EQP_ZONE_CLUSTER_H

#include "define.h"
#include "child_process.h"
#include "eqp_alloc.h"
#include "source_id.h"
#include "zone_id.h"

STRUCT_DECLARE(MasterIpcThread);
STRUCT_DECLARE(Client);

STRUCT_DEFINE(ZoneCluster)
{
    ChildProcess        proc;
    uint16_t            portHostByteOrder;
    uint16_t            id;
    uint16_t            maxZones;
    uint16_t            reservedZones;
    uint16_t            zoneCount;
    MasterIpcThread*    ipcThread;
};

STRUCT_DEFINE(ZoneClusterBySourceId)
{
    int             sourceId;
    ZoneCluster*    zoneCluster;
};

ZoneCluster*    zone_cluster_create(MasterIpcThread* ipcThread, uint16_t id, uint16_t port, uint16_t maxZones);
#define         zone_cluster_destroy(zc) free(zc)

void            zone_cluster_start_zone(ZoneCluster* zc, int sourceId);

void            zone_cluster_ipc_send(ZoneCluster* zc, ServerOp opcode, int sourceId, uint32_t length, const void* data);
void            zone_cluster_inform_of_client_zoning_in(ZoneCluster* zc, Client* client, int sourceId);

#define         zone_cluster_proc(zc) (&(zc)->proc)
#define         zone_cluster_source_id(zc) (((int)((zc)->id)) + EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET)
#define         zone_cluster_port(zc) ((zc)->portHostByteOrder)
#define         zone_cluster_id(zc) ((zc)->id)

int             zone_cluster_has_free_space(ZoneCluster* zc);

#define         zone_cluster_increment_reserved_zones(zc) ((zc)->reservedZones++)
#define         zone_cluster_increment_zone_count(zc) ((zc)->zoneCount++)

EQP_API void    zone_cluster_set_max_zones(ZoneCluster* zc, uint16_t maxZones);

#endif//EQP_ZONE_CLUSTER_H
