
#ifndef EQP_CHAR_CREATE_LUA_H
#define EQP_CHAR_CREATE_LUA_H

#include "define.h"
#include "eqp_array.h"
#include "eqp_alloc.h"

STRUCT_DECLARE(CharSelect);
STRUCT_DECLARE(CharSelectClient);

STRUCT_DEFINE(CharCreatePoint)
{
    int     zoneId;
    float   x;
    float   y;
    float   z;
    float   heading;
};

STRUCT_DEFINE(CharCreateItem)
{
    uint32_t    slotId;
    uint32_t    itemId;
    uint32_t    stackAmount;
    uint32_t    charges;
};

STRUCT_DEFINE(CharCreateLua)
{
    int64_t             charId;
    uint32_t            accountId;
    char                name[32];
    uint8_t             face;
    uint8_t             currentHp;
    uint16_t            race;
    uint8_t             class;
    uint8_t             gender;
    uint16_t            deity;
    bool                isTrilogy;
    union
    {
        struct
        {
            uint8_t     STR;
            uint8_t     DEX;
            uint8_t     AGI;
            uint8_t     STA;
            uint8_t     INT;
            uint8_t     WIS;
            uint8_t     CHA;
        };
        
        uint8_t stats[7];
    };
    CharCreatePoint     startingZone;
    CharCreatePoint     bindPoints[5];
    Array*              startingItems;
    CharSelectClient*   client;
    CharSelect*         charSelect;
};

CharCreateLua*      char_create_lua_create(CharSelect* charSelect, CharSelectClient* client, int isTrilogy);
void                char_create_lua_destroy(CharCreateLua* ccl);

int                 char_create_lua_has_bind_points_or_items(CharCreateLua* ccl);

#define             char_create_lua_char_select(ccl) ((ccl)->charSelect)
#define             char_create_lua_client(ccl) ((ccl)->client)
#define             char_create_lua_character_id(ccl) ((ccl)->charId)
#define             char_create_lua_account_id(ccl) ((ccl)->accountId)
#define             char_create_lua_name(ccl) ((ccl)->name)
#define             char_create_lua_face(ccl) ((ccl)->face)
#define             char_create_lua_current_hp(ccl) ((ccl)->currentHp)

#define             char_create_lua_zone_id(ccl) ((ccl)->startingZone.zoneId)
#define             char_create_lua_zone_x(ccl) ((ccl)->startingZone.x)
#define             char_create_lua_zone_y(ccl) ((ccl)->startingZone.y)
#define             char_create_lua_zone_z(ccl) ((ccl)->startingZone.z)
#define             char_create_lua_zone_heading(ccl) ((ccl)->startingZone.heading)

EQP_API uint16_t    char_create_lua_race(CharCreateLua* ccl);
EQP_API uint8_t     char_create_lua_class(CharCreateLua* ccl);
EQP_API uint8_t     char_create_lua_gender(CharCreateLua* ccl);
EQP_API uint16_t    char_create_lua_deity(CharCreateLua* ccl);
EQP_API int         char_create_lua_is_trilogy(CharCreateLua* ccl);
EQP_API uint8_t     char_create_lua_str(CharCreateLua* ccl);
EQP_API uint8_t     char_create_lua_dex(CharCreateLua* ccl);
EQP_API uint8_t     char_create_lua_agi(CharCreateLua* ccl);
EQP_API uint8_t     char_create_lua_sta(CharCreateLua* ccl);
EQP_API uint8_t     char_create_lua_int(CharCreateLua* ccl);
EQP_API uint8_t     char_create_lua_wis(CharCreateLua* ccl);
EQP_API uint8_t     char_create_lua_cha(CharCreateLua* ccl);

#define             char_create_lua_set_character_id(ccl, id) ((ccl)->charId = (id))
#define             char_create_lua_set_account_id(ccl, id) ((ccl)->accountId = (id))
#define             char_create_lua_set_name(ccl, str) snprintf((ccl)->name, sizeof_field(CharCreateLua, name), "%s", (str))
#define             char_create_lua_set_face(ccl, id) ((ccl)->face = (id))
#define             char_create_lua_set_current_hp(ccl, hp) ((ccl)->currentHp = (hp))
#define             char_create_lua_set_deity(ccl, id) ((ccl)->deity = (id))

EQP_API void        char_create_lua_set_starting_zone(CharCreateLua* ccl, int zoneId, float x, float y, float z, float heading);
EQP_API void        char_create_lua_set_bind_point(CharCreateLua* ccl, int zoneId, float x, float y, float z, float heading, uint32_t index);
EQP_API void        char_create_lua_add_starting_item(CharCreateLua* ccl, uint32_t slotId, uint32_t itemId, uint32_t stackAmount, uint32_t charges);
EQP_API void        char_create_lua_set_race(CharCreateLua* ccl, uint16_t race);
EQP_API void        char_create_lua_set_class(CharCreateLua* ccl, uint8_t class);
EQP_API void        char_create_lua_set_gender(CharCreateLua* ccl, uint8_t gender);
EQP_API void        char_create_lua_set_str(CharCreateLua* ccl, uint8_t val);
EQP_API void        char_create_lua_set_dex(CharCreateLua* ccl, uint8_t val);
EQP_API void        char_create_lua_set_agi(CharCreateLua* ccl, uint8_t val);
EQP_API void        char_create_lua_set_sta(CharCreateLua* ccl, uint8_t val);
EQP_API void        char_create_lua_set_int(CharCreateLua* ccl, uint8_t val);
EQP_API void        char_create_lua_set_wis(CharCreateLua* ccl, uint8_t val);
EQP_API void        char_create_lua_set_cha(CharCreateLua* ccl, uint8_t val);

EQP_API int         char_create_lua_get_zone_id(CharCreateLua* ccl, const char* shortName);

#endif//EQP_CHAR_CREATE_LUA_H
