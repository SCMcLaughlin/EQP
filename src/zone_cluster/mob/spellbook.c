
#include "spellbook.h"
#include "zone_cluster.h"

void spellbook_add_from_database(R(ZC*) zc, R(Spellbook*) spellbook, uint32_t slotId, uint32_t spellId)
{
    SpellbookSlot slot;
    
    slot.slotId     = slotId;
    slot.spellId    = spellId;
    
    if (!spellbook->knownSpells)
        spellbook->knownSpells = array_create_type(B(zc), SpellbookSlot);
    
    array_push_back(B(zc), &spellbook->knownSpells, &slot);
}
