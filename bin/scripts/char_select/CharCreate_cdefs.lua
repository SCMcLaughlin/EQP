
local ffi = require "ffi"

ffi.cdef[[
typedef struct CharCreate CharCreate;

uint16_t    char_create_lua_race(CharCreate* ccl);
uint8_t     char_create_lua_class(CharCreate* ccl);
uint8_t     char_create_lua_gender(CharCreate* ccl);
uint16_t    char_create_lua_deity(CharCreate* ccl);
int         char_create_lua_is_trilogy(CharCreate* ccl);
uint8_t     char_create_lua_str(CharCreate* ccl);
uint8_t     char_create_lua_dex(CharCreate* ccl);
uint8_t     char_create_lua_agi(CharCreate* ccl);
uint8_t     char_create_lua_sta(CharCreate* ccl);
uint8_t     char_create_lua_int(CharCreate* ccl);
uint8_t     char_create_lua_wis(CharCreate* ccl);
uint8_t     char_create_lua_cha(CharCreate* ccl);

void        char_create_lua_set_starting_zone(CharCreate* ccl, int zoneId, float x, float y, float z, float heading);
void        char_create_lua_set_bind_point(CharCreate* ccl, int zoneId, float x, float y, float z, float heading, uint32_t index);
void        char_create_lua_add_starting_item(CharCreate* ccl, uint32_t slotId, uint32_t itemId, uint32_t stackAmount, uint32_t charges);
void        char_create_lua_set_race(CharCreate* ccl, uint16_t race);
void        char_create_lua_set_class(CharCreate* ccl, uint8_t class);
void        char_create_lua_set_gender(CharCreate* ccl, uint8_t gender);
void        char_create_lua_set_str(CharCreate* ccl, uint8_t val);
void        char_create_lua_set_dex(CharCreate* ccl, uint8_t val);
void        char_create_lua_set_agi(CharCreate* ccl, uint8_t val);
void        char_create_lua_set_sta(CharCreate* ccl, uint8_t val);
void        char_create_lua_set_int(CharCreate* ccl, uint8_t val);
void        char_create_lua_set_wis(CharCreate* ccl, uint8_t val);
void        char_create_lua_set_cha(CharCreate* ccl, uint8_t val);

int         char_create_lua_get_zone_id(CharCreate* ccl, const char* shortName);
]]

return ffi.C
