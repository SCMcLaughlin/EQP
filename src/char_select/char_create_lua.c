
#include "char_create_lua.h"
#include "eqp_char_select.h"
#include "char_select_client.h"

CharCreateLua* char_create_lua_create(CharSelect* charSelect, CharSelectClient* client, int isTrilogy)
{
    CharCreateLua* ccl = eqp_alloc_type(B(charSelect), CharCreateLua);
    
    memset(ccl, 0, sizeof(CharCreateLua));
    
    ccl->isTrilogy      = isTrilogy;
    ccl->startingItems  = array_create_type(B(charSelect), CharCreateItem);
    ccl->client         = client;
    ccl->charSelect     = charSelect;
    
    return ccl;
}

void char_create_lua_destroy(CharCreateLua* ccl)
{
    if (ccl->startingItems)
    {
        array_destroy(ccl->startingItems);
        ccl->startingItems = NULL;
    }
    
    free(ccl);
}

int char_create_lua_has_bind_points_or_items(CharCreateLua* ccl)
{
    uint32_t i;
    
    for (i = 0; i < 5; i++)
    {
        if (ccl->bindPoints[i].zoneId)
            return true;
    }
    
    return !array_is_empty(ccl->startingItems);
}

uint16_t char_create_lua_race(CharCreateLua* ccl)
{
    return ccl->race;
}

uint8_t char_create_lua_class(CharCreateLua* ccl)
{
    return ccl->class;
}

uint8_t char_create_lua_gender(CharCreateLua* ccl)
{
    return ccl->gender;
}

uint16_t char_create_lua_deity(CharCreateLua* ccl)
{
    return ccl->deity;
}

int char_create_lua_is_trilogy(CharCreateLua* ccl)
{
    return ccl->isTrilogy;
}

uint8_t char_create_lua_str(CharCreateLua* ccl)
{
    return ccl->STR;
}

uint8_t char_create_lua_dex(CharCreateLua* ccl)
{
    return ccl->DEX;
}

uint8_t char_create_lua_agi(CharCreateLua* ccl)
{
    return ccl->AGI;
}

uint8_t char_create_lua_sta(CharCreateLua* ccl)
{
    return ccl->STA;
}

uint8_t char_create_lua_int(CharCreateLua* ccl)
{
    return ccl->INT;
}

uint8_t char_create_lua_wis(CharCreateLua* ccl)
{
    return ccl->WIS;
}

uint8_t char_create_lua_cha(CharCreateLua* ccl)
{
    return ccl->CHA;
}

void char_create_lua_set_starting_zone(CharCreateLua* ccl, int zoneId, float x, float y, float z, float heading)
{
    ccl->startingZone.zoneId    = zoneId;
    ccl->startingZone.x         = x;
    ccl->startingZone.y         = y;
    ccl->startingZone.z         = z;
    ccl->startingZone.heading   = heading;
}

void char_create_lua_set_bind_point(CharCreateLua* ccl, int zoneId, float x, float y, float z, float heading, uint32_t index)
{
    CharCreatePoint* p;
    
    if (index >= 5)
        return;
    
    p = &ccl->bindPoints[index];
    
    p->zoneId   = zoneId;
    p->x        = x;
    p->y        = y;
    p->z        = z;
    p->heading  = heading;
}

void char_create_lua_add_starting_item(CharCreateLua* ccl, uint32_t slotId, uint32_t itemId, uint32_t stackAmount, uint32_t charges)
{
    CharCreateItem* array   = array_data_type(ccl->startingItems, CharCreateItem);
    uint32_t n              = array_count(ccl->startingItems);
    CharCreateItem item;
    uint32_t i;
    
    item.slotId         = slotId;
    item.itemId         = itemId;
    item.stackAmount    = stackAmount;
    item.charges        = charges;
    
    for (i = 0; i < n; i++)
    {
        if (array[i].slotId == slotId)
        {
            memcpy(&array[i], &item, sizeof(item));
            return;
        }
    }
    
    array_push_back(B(ccl->charSelect), &ccl->startingItems, &item);
}

void char_create_lua_set_race(CharCreateLua* ccl, uint16_t race)
{
    ccl->race = race;
}

void char_create_lua_set_class(CharCreateLua* ccl, uint8_t class)
{
    ccl->class = class;
}

void char_create_lua_set_gender(CharCreateLua* ccl, uint8_t gender)
{
    ccl->gender = gender;
}

void char_create_lua_set_str(CharCreateLua* ccl, uint8_t val)
{
    ccl->STR = val;
}

void char_create_lua_set_dex(CharCreateLua* ccl, uint8_t val)
{
    ccl->DEX = val;
}

void char_create_lua_set_agi(CharCreateLua* ccl, uint8_t val)
{
    ccl->AGI = val;
}

void char_create_lua_set_sta(CharCreateLua* ccl, uint8_t val)
{
    ccl->STA = val;
}

void char_create_lua_set_int(CharCreateLua* ccl, uint8_t val)
{
    ccl->INT = val;
}

void char_create_lua_set_wis(CharCreateLua* ccl, uint8_t val)
{
    ccl->WIS = val;
}

void char_create_lua_set_cha(CharCreateLua* ccl, uint8_t val)
{
    ccl->CHA = val;
}

int char_create_lua_get_zone_id(CharCreateLua* ccl, const char* shortName)
{
    CharSelect* charSelect = char_create_lua_char_select(ccl);
    return char_select_get_zone_id(charSelect, shortName);
}
