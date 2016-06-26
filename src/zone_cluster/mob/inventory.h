
#ifndef EQP_INVENTORY_H
#define EQP_INVENTORY_H

#include "define.h"
#include "eqp_array.h"

/*
    The Inventory is a flat, orderless, dynamically-sized array.
    Only slots that are, or were, in use by the client will be present in the array.
    
    Commonly looked-up slots (weapons) have their indices cached for extra-fast lookup.
    The cursor slot is treated specially (always slot 0) because of its frequent use in 
    swapping, and to allow a "cursor queue".
    
    The inventory is always saved to the database whole, in a single transaction.
    This is fast, since we only need to access the flat fields of the array, and it
    ensures that an inventory will always be consistent in the database (provided it is 
    kept consistent in memory, at least).
*/

STRUCT_DEFINE(InventorySlot)
{
    uint16_t    slotId;
    uint16_t    augSlotId;  // Augs use the same slotId as the item they are inserted into, but a non-zero augSlotId
    int16_t     stackAmount;
    int16_t     charges;
    uint32_t    itemId;
    void*       item;
};

STRUCT_DEFINE(Inventory)
{
    int     mainHandIndex;
    int     offHandIndex;
    int     rangeIndex;
    Array*  slots;
    Array*  cursorQueue;
};

#endif//EQP_INVENTORY_H
