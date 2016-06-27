
#ifndef EQP_SPELLBOOK_H
#define EQP_SPELLBOOK_H

#include "define.h"
#include "eqp_array.h"

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
    MemorizedSpell  memorized[9];
};

void    spellbook_add_from_database(R(ZC*) zc, R(Spellbook*) spellbook, uint32_t slotId, uint32_t spellId);

#define spellbook_is_empty(book) ((book)->knownSpells == NULL)

#endif//EQP_SPELLBOOK_H
