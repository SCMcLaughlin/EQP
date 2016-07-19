
#ifndef EQP_ITEM_H
#define EQP_ITEM_H

#include "define.h"
#include "item_prototype.h"
#include "eqp_alloc.h"
#include "lua_object.h"

STRUCT_DECLARE(ZC);

STRUCT_DEFINE(Item)
{
    // LuaObject must be the first member of this struct
    LuaObject       luaObj;
    ItemPrototype*  prototype;
    uint16_t        charges;
    uint16_t        stackAmount;
    bool            isBag;
    bool            isCustomized;
};

Item*                   item_create(ZC* zc, ItemPrototype* proto, uint16_t charges, uint16_t stackAmount);
void                    item_destroy(ZC* zc, Item* item);

EQP_API ItemPrototype*  item_get_prototype(Item* item);
EQP_API ItemPrototype*  item_get_prototype_for_editing(ZC* zc, Item* item);

EQP_API uint16_t        item_get_charges(Item* item);
EQP_API uint16_t        item_get_stack_amount(Item* item);
EQP_API int             item_is_bag(Item* item);

#endif//EQP_ITEM_H
