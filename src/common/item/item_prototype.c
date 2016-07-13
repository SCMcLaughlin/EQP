
#include "item_prototype.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"

ItemPrototype* item_proto_create(Basic* basic)
{
    ItemPrototype* proto = eqp_alloc_type(basic, ItemPrototype);
    
    item_proto_set_defaults(proto);
    return proto;
}

void item_proto_destroy(ItemPrototype* proto)
{
    free(proto);
}

void item_proto_set_defaults(ItemPrototype* proto)
{
    memset(proto, 0, sizeof(ItemPrototype));
    
    proto->isDroppable      = true;
    proto->isPermanent      = true;
    proto->itemSkillId      = 10; //fixme: make enum; 10 = armor
    proto->raceBitfield     = 0xffff;
    proto->classBitfield    = 0xffff;
    proto->spellId          = 0xffff;
    proto->iconId           = 500; // Just for the sake of avoiding invisible items...
}
