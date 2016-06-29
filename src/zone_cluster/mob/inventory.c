
#include "inventory.h"

#define CURSOR_INDEX 0

void inventory_init(Basic* basic, Inventory* inv)
{
    InventorySlot slot;
    
    inv->primaryIndex   = -1;
    inv->secondaryIndex = -1;
    inv->rangeIndex     = -1;
    inv->ammoIndex      = -1;
    inv->slots          = array_create_type(basic, InventorySlot);
    inv->cursorQueue    = NULL; // Only created if needed; keeps checks fast, too
    
    // The cursor slot is always slot 0
    memset(&slot, 0, sizeof(slot));
    slot.slotId = InvSlot_Cursor;
    
    array_push_back(basic, &inv->slots, &slot);
}

void inventory_deinit(Inventory* inv)
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

static void inventory_add_to_cursor_queue_from_database(Basic* basic, Inventory* inv, InventorySlot* slot)
{
    if (!inv->cursorQueue)
        inv->cursorQueue = array_create_type(basic, InventorySlot);
    
    array_push_back(basic, &inv->cursorQueue, slot);
}

void inventory_add_from_database(Basic* basic, Inventory* inv, InventorySlot* slot)
{
    uint16_t slotId = slot->slotId;
    int count;
    
    if (slotId == InvSlot_Cursor)
    {
        InventorySlot* cursor = array_get_type(inv->slots, 0, InventorySlot);
        
        // Do we already have something in the cursor slot?
        if (cursor->itemId)
        {
            inventory_add_to_cursor_queue_from_database(basic, inv, slot);
            return;
        }
    }
    
    count = array_count(inv->slots);
    
    switch (slotId)
    {
    case InvSlot_Range:
        inv->rangeIndex = count;
        break;
    
    case InvSlot_Primary:
        inv->primaryIndex = count;
        break;
    
    case InvSlot_Secondary:
        inv->secondaryIndex = count;
        break;
    
    default:
        break;
    }
    
    array_push_back(basic, &inv->slots, slot);
}

void inventory_iterator_init(InventoryIterator* itr, uint32_t slotFrom, uint32_t slotTo)
{
    if (slotFrom > slotTo)
    {
        uint32_t tmp    = slotFrom;
        slotFrom        = slotTo;
        slotTo          = tmp;
    }
    
    itr->index      = 0;
    itr->slotFrom   = slotFrom;
    itr->slotTo     = slotTo;
    itr->slot       = NULL;
}

int inventory_iterate_no_augs(Inventory* inv, InventoryIterator* itr)
{
    InventorySlot* array    = array_data_type(inv->slots, InventorySlot);
    uint32_t n              = array_count(inv->slots);
    uint32_t i              = itr->index;
    uint32_t from;
    uint32_t to;
    
    if (i >= n)
        goto finished;
    
    from    = itr->slotFrom;
    to      = itr->slotTo;
    
    for (; i < n; i++)
    {
        InventorySlot* slot = &array[i];
        uint32_t slotId     = slot->slotId;
        
        if (slotId >= from && slotId <= to && slot->itemId && slot->augSlotId == 0)
        {
            itr->index  = i + 1;
            itr->slot   = slot;
            return true;
        }
    }
    
finished:
    return false;
}

#undef CURSOR_INDEX
