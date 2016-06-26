
#include "inventory.h"

#define CURSOR_INDEX 0

void inventory_init(R(Basic*) basic, R(Inventory*) inv)
{
    InventorySlot slot;
    
    inv->mainHandIndex  = -1;
    inv->offHandIndex   = -1;
    inv->rangeIndex     = -1;
    inv->slots          = array_create_type(basic, InventorySlot);
    inv->cursorQueue    = NULL; // Only created if needed; keeps checks fast, too
    
    // The cursor slot is always slot 0
    memset(&slot, 0, sizeof(slot));
    slot.slotId = InvSlot_Cursor;
    
    array_push_back(basic, &inv->slots, &slot);
}

void inventory_deinit(R(Inventory*) inv)
{
    if (inv->slots)
    {
        //fixme: free individual items
        
        array_destroy(inv->slots);
        inv->slots = NULL;
    }
    
    if (inv->cursorQueue)
    {
        //fixme: free individual items
        
        array_destroy(inv->cursorQueue);
        inv->cursorQueue = NULL;
    }
}

static void inventory_add_to_cursor_queue_from_database(R(Basic*) basic, R(Inventory*) inv, R(InventorySlot*) slot)
{
    if (!inv->cursorQueue)
        inv->cursorQueue = array_create_type(basic, InventorySlot);
    
    array_push_back(basic, &inv->cursorQueue, slot);
}

void inventory_add_from_database(R(Basic*) basic, R(Inventory*) inv, R(InventorySlot*) slot)
{
    uint16_t slotId = slot->slotId;
    
    if (slotId == InvSlot_Cursor)
    {
        R(InventorySlot*) cursor = array_get_type(inv->slots, 0, InventorySlot);
        
        // Do we already have something in the cursor slot?
        if (cursor->itemId)
        {
            inventory_add_to_cursor_queue_from_database(basic, inv, slot);
            return;
        }
    }
    
    array_push_back(basic, &inv->slots, slot);
}

#undef CURSOR_INDEX
