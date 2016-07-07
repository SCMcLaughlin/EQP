
local ffi = require "ffi"

require "ZoneCluster_cdefs"

ffi.cdef[[
typedef struct NpcPrototype NpcPrototype;

NpcPrototype*   npc_proto_create(ZC* zc);
void            npc_proto_destroy(NpcPrototype* proto);

void            npc_proto_set_name(ZC* zc, NpcPrototype* proto, const char* name, uint32_t len);
void            npc_proto_set_adhoc_id(NpcPrototype* proto, uint32_t id);
void            npc_proto_set_level(NpcPrototype* proto, uint8_t level);
void            npc_proto_set_class(NpcPrototype* proto, uint8_t class);
void            npc_proto_set_race(NpcPrototype* proto, uint16_t race);
void            npc_proto_set_gender(NpcPrototype* proto, uint8_t gender);
void            npc_proto_set_face(NpcPrototype* proto, uint8_t face);
void            npc_proto_set_body_type(NpcPrototype* proto, uint8_t bodyType);
void            npc_proto_set_texture(NpcPrototype* proto, uint8_t textureId);
void            npc_proto_set_helm_texture(NpcPrototype* proto, uint8_t textureId);
void            npc_proto_set_max_hp(NpcPrototype* proto, int64_t hp);
void            npc_proto_set_max_mana(NpcPrototype* proto, int64_t mana);
void            npc_proto_set_max_endurance(NpcPrototype* proto, int64_t endur);
void            npc_proto_set_size(NpcPrototype* proto, float size);
void            npc_proto_set_running_speed(NpcPrototype* proto, float speed);
void            npc_proto_set_walking_speed(NpcPrototype* proto, float speed);
void            npc_proto_set_aggro_radius(NpcPrototype* proto, float radius);
void            npc_proto_set_material(NpcPrototype* proto, uint8_t slot, uint8_t materialId);
void            npc_proto_set_tint(NpcPrototype* proto, uint8_t slot, uint8_t red, uint8_t green, uint8_t blue);
void            npc_proto_set_primary_model_id(NpcPrototype* proto, uint32_t modelId);
void            npc_proto_set_secondary_model_id(NpcPrototype* proto, uint32_t modelId);
void            npc_proto_set_primary_message_skill(NpcPrototype* proto, uint8_t skill);
void            npc_proto_set_secondary_message_skill(NpcPrototype* proto, uint8_t skill);
void            npc_proto_set_swings_per_round(NpcPrototype* proto, int8_t swings);
void            npc_proto_set_melee_damage(NpcPrototype* proto, int64_t minDmg, int64_t maxDmg);
void            npc_proto_set_ac(NpcPrototype* proto, uint32_t ac);
void            npc_proto_set_str(NpcPrototype* proto, int value);
void            npc_proto_set_sta(NpcPrototype* proto, int value);
void            npc_proto_set_dex(NpcPrototype* proto, int value);
void            npc_proto_set_agi(NpcPrototype* proto, int value);
void            npc_proto_set_wis(NpcPrototype* proto, int value);
void            npc_proto_set_int(NpcPrototype* proto, int value);
void            npc_proto_set_cha(NpcPrototype* proto, int value);
void            npc_proto_set_sv_magic(NpcPrototype* proto, int value);
void            npc_proto_set_sv_fire(NpcPrototype* proto, int value);
void            npc_proto_set_sv_cold(NpcPrototype* proto, int value);
void            npc_proto_set_sv_poison(NpcPrototype* proto, int value);
void            npc_proto_set_sv_disease(NpcPrototype* proto, int value);

const char*     npc_proto_get_name_cstr(NpcPrototype* proto);
uint32_t        npc_proto_get_adhoc_id(NpcPrototype* proto);
uint8_t         npc_proto_get_level(NpcPrototype* proto);
uint8_t         npc_proto_get_class(NpcPrototype* proto);
uint16_t        npc_proto_get_race(NpcPrototype* proto);
uint8_t         npc_proto_get_gender(NpcPrototype* proto);
uint8_t         npc_proto_get_face(NpcPrototype* proto);
uint8_t         npc_proto_get_body_type(NpcPrototype* proto);
uint8_t         npc_proto_get_texture(NpcPrototype* proto);
uint8_t         npc_proto_get_helm_texture(NpcPrototype* proto);
int64_t         npc_proto_get_max_hp(NpcPrototype* proto);
int64_t         npc_proto_get_max_mana(NpcPrototype* proto);
int64_t         npc_proto_get_max_endurance(NpcPrototype* proto);
float           npc_proto_get_size(NpcPrototype* proto);
float           npc_proto_get_running_speed(NpcPrototype* proto);
float           npc_proto_get_walking_speed(NpcPrototype* proto);
float           npc_proto_get_aggro_radius(NpcPrototype* proto);
uint8_t         npc_proto_get_material(NpcPrototype* proto, uint8_t slot);
uint32_t        npc_proto_get_tint(NpcPrototype* proto, uint8_t slot);
uint32_t        npc_proto_get_primary_model_id(NpcPrototype* proto);
uint32_t        npc_proto_get_secondary_model_id(NpcPrototype* proto);
uint8_t         npc_proto_get_primary_message_skill(NpcPrototype* proto);
uint8_t         npc_proto_get_secondary_message_skill(NpcPrototype* proto);
int8_t          npc_proto_get_swings_per_round(NpcPrototype* proto);
int64_t         npc_proto_get_min_damage(NpcPrototype* proto);
int64_t         npc_proto_get_max_damage(NpcPrototype* proto);
]]

return ffi.C
