
#include "item.h"
#include "zone_cluster.h"

Item* item_create(ZC* zc, ItemPrototype* proto, uint16_t charges, uint16_t stackAmount)
{
    Item* item = eqp_alloc_type(B(zc), Item);
    
    zc_lua_object_set_index(&item->luaObj, 0);
    
    item->prototype     = proto;
    item->charges       = charges;
    item->stackAmount   = stackAmount;
    item->isBag         = false; //fixme
    item->isCustomized  = false;
    
    return item;
}

void item_destroy(ZC* zc, Item* item)
{
    zc_lua_destroy_object(zc, &item->luaObj);
    
    if (item->isCustomized && item->prototype)
    {
        free(item->prototype);
        item->prototype = NULL;
    }
    
    free(item);
}

ItemPrototype* item_get_prototype(Item* item)
{
    return item->prototype;
}

ItemPrototype* item_get_prototype_for_editing(ZC* zc, Item* item)
{
    ItemPrototype* proto;
    
    if (item->isCustomized)
        return item->prototype;
    
    proto = item_proto_copy(B(zc), item->prototype);
    
    item->prototype     = proto;
    item->isCustomized  = true;
    
    return proto;
}

uint16_t item_get_charges(Item* item)
{
    return item->charges;
}

uint16_t item_get_stack_amount(Item* item)
{
    return item->stackAmount;
}

int item_is_bag(Item* item)
{
    return item->isBag;
}
