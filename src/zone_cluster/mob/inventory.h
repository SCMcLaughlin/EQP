
#ifndef EQP_INVENTORY_H
#define EQP_INVENTORY_H

#include "define.h"
#include "eqp_array.h"
#include "item.h"

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
    uint32_t    itemId;
    Item*       item;
};

STRUCT_DEFINE(Inventory)
{
    int     primaryIndex;
    int     secondaryIndex;
    int     rangeIndex;
    int     ammoIndex;
    Array*  slots;
    Array*  cursorQueue;
};

ENUM_DEFINE(InvSlot)
{
    InvSlot_Charm,
    InvSlot_Ear1,
    InvSlot_Head,
    InvSlot_Face,
    InvSlot_Ear2,
    InvSlot_Neck,
    InvSlot_Shoulders,
    InvSlot_Arms,
    InvSlot_Back,
    InvSlot_Wrist1,
    InvSlot_Wrist2,
    InvSlot_Range,
    InvSlot_Hands,
    InvSlot_Primary,
    InvSlot_Secondary,
    InvSlot_Fingers1,
    InvSlot_Fingers2,
    InvSlot_Chest,
    InvSlot_Legs,
    InvSlot_Feet,
    InvSlot_Waist,
    InvSlot_Ammo,
    InvSlot_MainInventory0,
    InvSlot_MainInventory1,
    InvSlot_MainInventory2,
    InvSlot_MainInventory3,
    InvSlot_MainInventory4,
    InvSlot_MainInventory5,
    InvSlot_MainInventory6,
    InvSlot_MainInventory7,
    InvSlot_Cursor,
    
    InvSlot_EquipForStatsBegin              = InvSlot_Charm,
    InvSlot_EquipForStatsEnd                = InvSlot_Waist,
    InvSlot_EquipMainAndCursorBegin         = InvSlot_Charm,
    InvSlot_EquipMainAndCursorEnd           = InvSlot_Cursor,
    InvSlot_EquipMainAndCursorNoCharmBegin  = InvSlot_Ear1,
    InvSlot_EquipMainAndCursorNoCharmEnd    = InvSlot_Cursor,
    InvSlot_MainInventoryBegin              = InvSlot_MainInventory0,
    InvSlot_MainInventoryEnd                = InvSlot_MainInventory7,
    InvSlot_BagSlotsBegin                   = 251,
    InvSlot_BagSlotsEnd                     = 330,
    InvSlot_BagsSlotsIncludingCursorBegin   = 251,
    InvSlot_BagsSlotsIncludingCursorEnd     = 340,
    InvSlot_BankBegin                       = 2000,
    InvSlot_BankEnd                         = 2007,
    InvSlot_BankExtendedBegin               = InvSlot_BankBegin,
    InvSlot_BankExtendedEnd                 = 2015,
    InvSlot_BankBagSlotsBegin               = 2031,
    InvSlot_BankBagSlotsEnd                 = 2111,
    InvSlot_BankExtendedBagSlotsBegin       = InvSlot_BankBagSlotsBegin,
    InvSlot_BankExtendedBagSlotsEnd         = 2191,
};

STRUCT_DEFINE(InventoryIterator)
{
    uint32_t        index;
    uint32_t        slotFrom;
    uint32_t        slotTo;
    InventorySlot*  slot;
};

void    inventory_init(Basic* basic, Inventory* inv);
void    inventory_deinit(Inventory* inv);

void    inventory_add_from_database(Basic* basic, Inventory* inv, InventorySlot* slot);

void    inventory_iterator_init(InventoryIterator* itr, uint32_t slotFrom, uint32_t slotTo);
int     inventory_iterate_no_augs(Inventory* inv, InventoryIterator* itr);

#define inventory_array(inv) array_data_type((inv)->slots, InventorySlot)
#define inventory_array_count(inv) array_count((inv)->slots)

#endif//EQP_INVENTORY_H
