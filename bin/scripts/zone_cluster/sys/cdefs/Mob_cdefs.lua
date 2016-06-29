
local ffi = require "ffi"

ffi.cdef[[
const char* mob_name_cstr(void* mob);
const char* mob_client_friendly_name_cstr(void* mob);
int         mob_entity_id(void* mob);
int         mob_get_type(void* mob);
uint8_t     mob_level(void* mob);
uint8_t     mob_class(void* mob);
uint16_t    mob_base_race(void* mob);
uint16_t    mob_current_race(void* mob);
uint8_t     mob_base_gender(void* mob);
uint8_t     mob_current_gender(void* mob);
uint8_t     mob_face(void* mob);
uint16_t    mob_deity(void* mob);
float       mob_x(void* mob);
float       mob_y(void* mob);
float       mob_z(void* mob);
float       mob_heading(void* mob);
int8_t      mob_hp_ratio(void* mob);
int64_t     mob_current_hp(void* mob);
int64_t     mob_max_hp(void* mob);
int64_t     mob_current_mana(void* mob);
int64_t     mob_max_mana(void* mob);
int64_t     mob_current_endurance(void* mob);
int64_t     mob_max_endurance(void* mob);
int         mob_cur_str(void* mob);
int         mob_base_str(void* mob);
void        mob_set_base_str(void* mob, int value);
int         mob_cur_sta(void* mob);
int         mob_base_sta(void* mob);
void        mob_set_base_sta(void* mob, int value);
int         mob_cur_dex(void* mob);
int         mob_base_dex(void* mob);
void        mob_set_base_dex(void* mob, int value);
int         mob_cur_agi(void* mob);
int         mob_base_agi(void* mob);
void        mob_set_base_agi(void* mob, int value);
int         mob_cur_int(void* mob);
int         mob_base_int(void* mob);
void        mob_set_base_int(void* mob, int value);
int         mob_cur_wis(void* mob);
int         mob_base_wis(void* mob);
void        mob_set_base_wis(void* mob, int value);
int         mob_cur_cha(void* mob);
int         mob_base_cha(void* mob);
void        mob_set_base_cha(void* mob, int value);
float       mob_current_walking_speed(void* mob);
float       mob_base_walking_speed(void* mob);
void        mob_set_base_walking_speed(Mob* mob, float value);
float       mob_current_running_speed(void* mob);
float       mob_base_running_speed(void* mob);
void        mob_set_base_running_speed(Mob* mob, float value);
float       mob_current_size(void* mob);
float       mob_base_size(void* mob);
uint16_t    mob_body_type(void* mob);
int         mob_is_invisible(void* mob);
int         mob_is_invisible_vs_undead(void* mob);
int         mob_is_invisible_vs_animals(void* mob);
int         mob_is_invisible_to_mob(void* self, void* target);
uint8_t     mob_upright_state(void* mob);
uint8_t     mob_light_level(void* mob);
uint8_t     mob_texture(void* mob);
uint8_t     mob_helm_texture(void* mob);
]]

return ffi.C
