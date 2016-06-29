
#include "zone_cluster.h"
#include "master_ipc.h"
#include "eqp_master.h"
#include "client.h"

ZoneCluster* zone_cluster_create(MasterIpcThread* ipcThread, uint16_t id, uint16_t port, uint16_t maxZones)
{
    ZoneCluster* zc = eqp_alloc_type(B(ipcThread), ZoneCluster);
    
    proc_init(&zc->proc);
    
    zc->portHostByteOrder   = port;
    zc->id                  = id;
    zc->maxZones            = maxZones;
    zc->reservedZones       = 0;
    zc->zoneCount           = 0;
    zc->ipcThread           = ipcThread;
    
    master_start_zone_cluster(master_ipc_thread_master(ipcThread), zc, id, port);
    
    return zc;
}

void zone_cluster_start_zone(ZoneCluster* zc, int sourceId)
{
    Basic* basic    = B(zc->ipcThread);
    int zoneId      = sourceId % EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET;
    int instId      = sourceId / EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET;
    
    log_format(basic, LogInfo, "zone_cluster%u: starting sourceId %i (%s_instance%i)", zc->id, sourceId,
        zone_short_name_by_id(zoneId), instId);
    
    proc_ipc_send(basic, zone_cluster_proc(zc), ServerOp_StartZone, sourceId, 0, NULL);
}

void zone_cluster_ipc_send(ZoneCluster* zc, ServerOp opcode, int sourceId, uint32_t length, const void* data)
{
    proc_ipc_send(B(zc->ipcThread), zone_cluster_proc(zc), opcode, sourceId, length, data);
}

void zone_cluster_inform_of_client_zoning_in(ZoneCluster* zc, Client* client, int sourceId)
{
    Server_ClientZoning zoning;
    
    zoning.accountId        = client_account_id(client);
    zoning.ipAddress        = client_ip_address(client);
    zoning.characterId      = client_character_id(client);
    zoning.isLocal          = client_is_local(client);
    zoning.isFromCharSelect = client_is_from_char_select(client);
    snprintf(zoning.accountName, sizeof(zoning.accountName), "%s", client_account_name(client));
    snprintf(zoning.characterName, sizeof(zoning.characterName), "%s", client_name(client));
    
    zone_cluster_ipc_send(zc, ServerOp_ClientZoning, sourceId, sizeof(zoning), &zoning);
}

int zone_cluster_has_free_space(ZoneCluster* zc)
{
    // Use signed ints to avoid underflow when reserved > max
    int maxZones    = zc->maxZones;
    int reserved    = zc->reservedZones;
    int count       = zc->zoneCount;
    
    return (count < (maxZones - reserved));
}

void zone_cluster_set_max_zones(ZoneCluster* zc, uint16_t maxZones)
{
    zc->maxZones = maxZones;
}
