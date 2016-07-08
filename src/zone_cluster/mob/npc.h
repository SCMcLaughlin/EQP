
#ifndef EQP_NPC_H
#define EQP_NPC_H

#include "define.h"
#include "mob.h"
#include "eqp_alloc.h"
#include "eqp_string.h"
#include "npc_prototype.h"

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(Zone);

STRUCT_DEFINE(Npc)
{
    Mob         mob;
    
    uint32_t    adhocId;
};

Npc*    npc_create(ZC* zc, Zone* zone, NpcPrototype* proto, float x, float y, float z, float heading);
void    npc_destroy(Npc* npc);

EQP_API uint32_t    npc_adhoc_id(Npc* npc);

#endif//EQP_NPC_H
