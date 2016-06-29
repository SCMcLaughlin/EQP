
#include "spellbook.h"
#include "zone_cluster.h"

void spellbook_deinit(Spellbook* spellbook)
{
    if (spellbook->knownSpells)
    {
        array_destroy(spellbook->knownSpells);
        spellbook->knownSpells = NULL;
    }
}

void spellbook_add_from_database(ZC* zc, Spellbook* spellbook, uint32_t slotId, uint32_t spellId)
{
    SpellbookSlot slot;
    
    slot.slotId     = slotId;
    slot.spellId    = spellId;
    
    if (!spellbook->knownSpells)
        spellbook->knownSpells = array_create_type(B(zc), SpellbookSlot);
    
    array_push_back(B(zc), &spellbook->knownSpells, &slot);
}

void spellbook_add_memorized_from_database(ZC* zc, Spellbook* spellbook, uint32_t slotId, uint32_t spellId, uint64_t recastTimestamp)
{
    MemorizedSpell* memmed;
    (void)zc; //will be used to look up spell
    
    if (slotId >= 9)
        return;
    
    memmed = &spellbook->memorized[slotId];
    
    memmed->spellId                 = spellId;
    memmed->castingTimeMilliseconds = 0; //fixme: look up the spell's casting time, when spells exist...
    memmed->recastTimestamp         = recastTimestamp;
}
