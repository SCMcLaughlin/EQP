
#ifndef EQP_CLIENT_SAVE_H
#define EQP_CLIENT_SAVE_H

#include "define.h"
#include "structs.h"
#include "eqp_alloc.h"
#include "database.h"
#include "query.h"
#include "transaction.h"
#include "spellbook.h"
#include "skills.h"

STRUCT_DECLARE(Client);

STRUCT_DEFINE(ClientSave)
{
    ZC*             zc;
    int64_t         characterId;
    char            name[32];
    char            surname[32];
    uint8_t         level;
    uint8_t         class;
    uint8_t         gender;
    uint8_t         face;
    uint16_t        race;
    uint16_t        deity;
    uint16_t        zoneId;
    uint16_t        instanceId;
    float           x;
    float           y;
    float           z;
    float           heading;
    int             hp;
    int             mana;
    int             endurance;
    int             STR;
    int             DEX;
    int             AGI;
    int             STA;
    int             INT;
    int             WIS;
    int             CHA;
    Coin            coins;
    Coin            coinsBank;
    Coin            coinsCursor;
    int             guildRank;
    int             guildId;
    uint64_t        harmtouchTimestamp;
    uint64_t        disciplineTimestamp;
    uint32_t        hungerLevel;
    uint32_t        thirstLevel;
    uint16_t        trainingPoints;
    int64_t         experience;
    int             anonSetting;
    int             isGM;
    int             drunkeness;
    uint8_t         material[7];
    uint32_t        tint[7];
    uint32_t        primaryModelId;
    uint32_t        secondaryModelId;
    
    MemorizedSpell  memmedSpell[EQP_MEMORIZED_SPELL_SLOTS];
    Skills          skill;
    BindPoint       bindPoints[EQP_CLIENT_BIND_POINT_COUNT];
    
    uint32_t        invCount;
    uint32_t        spellbookCount;
};

STRUCT_DEFINE(ClientSave_Inv)
{
    uint16_t    slotId;
    uint16_t    augSlotId;
    uint16_t    stackAmount;
    uint16_t    charges;
    uint32_t    itemId;
};

/*
    Takes a snapshot of the client's current state and queues it to be commited to the database
    in one atomic transaction. If an onCompletion callback is provided, it is the callback's
    responsibility to free the query's ClientSave userdata (which can be used to identify
    which client just finished saving). If no callback is provided, the snapshot userdata is
    automatically freed.

    The ClientSave userdata does not contain any direct pointer to the Client becasue it is
    not assumed that the Client will still exist when the save is finished committing.
*/
void    client_save(Client* client, QueryCallback onCompletion);

#endif//EQP_CLIENT_SAVE_H
