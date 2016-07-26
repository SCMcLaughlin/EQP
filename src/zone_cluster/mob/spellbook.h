
#ifndef EQP_SPELLBOOK_H
#define EQP_SPELLBOOK_H

#include "define.h"
#include "eqp_array.h"

#define EQP_MEMORIZED_SPELL_SLOTS 9

STRUCT_DECLARE(ZC);

STRUCT_DEFINE(SpellbookSlot)
{
    uint32_t    slotId;
    uint32_t    spellId;
};

STRUCT_DEFINE(MemorizedSpell)
{
    uint32_t    spellId;
    uint32_t    castingTimeMilliseconds;
    uint64_t    recastTimestamp;
};

STRUCT_DEFINE(Spellbook)
{
    Array*          knownSpells;    // Deliberately NULL if the client has no spells
    MemorizedSpell  memorized[EQP_MEMORIZED_SPELL_SLOTS];
};

void    spellbook_deinit(Spellbook* spellbook);
void    spellbook_add_from_database(ZC* zc, Spellbook* spellbook, uint32_t slotId, uint32_t spellId);
void    spellbook_add_memorized_from_database(ZC* zc, Spellbook* spellbook, uint32_t slotId, uint32_t spellId, uint64_t recastTimestamp);

#define spellbook_is_empty(book) ((book)->knownSpells == NULL)
#define spellbook_array(book) array_data_type((book)->knownSpells, SpellbookSlot)
#define spellbook_array_count(book) array_count((book)->knownSpells)
#define spellbook_memorized(book) (&(book)->memorized)

#endif//EQP_SPELLBOOK_H
