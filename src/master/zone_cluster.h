
#ifndef EQP_ZONE_CLUSTER_H
#define EQP_ZONE_CLUSTER_H

#include "define.h"
#include "child_process.h"
#include "eqp_alloc.h"
#include "source_id.h"

STRUCT_DECLARE(MasterIpcThread);

STRUCT_DEFINE(ZoneCluster)
{
    ChildProcess    proc;
    uint16_t        portHostByteOrder;
    uint16_t        id;
    uint16_t        maxZones;
    uint16_t        reservedZones;
    uint16_t        zoneCount;
};

STRUCT_DEFINE(ZoneClusterBySourceId)
{
    int             sourceId;
    ZoneCluster*    zoneCluster;
};

ZoneCluster*    zone_cluster_create(R(MasterIpcThread*) ipcThread, uint16_t id, uint16_t port, uint16_t maxZones);
#define         zone_cluster_destroy(zc) free(zc)

#define         zone_cluster_proc(zc) (&(zc)->proc)
#define         zone_cluster_source_id(zc) (((int)((zc)->id)) + EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET)
#define         zone_cluster_port(zc) ((zc)->portHostByteOrder)
#define         zone_cluster_id(zc) ((zc)->id)

#define         zone_cluster_increment_reserved_zones(zc) ((zc)->reservedZones++)
#define         zone_cluster_increment_zone_count(zc) ((zc)->zoneCount++)

EQP_API void    zone_cluster_set_max_zones(R(ZoneCluster*) zc, uint16_t maxZones);

#endif//EQP_ZONE_CLUSTER_H
