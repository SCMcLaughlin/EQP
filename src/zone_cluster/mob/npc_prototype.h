
#ifndef EQP_NPC_PROTOTYPE_H
#define EQP_NPC_PROTOTYPE_H

#include "define.h"
#include "enums.h"
#include "skills.h"
#include "eqp_alloc.h"
#include "eqp_string.h"

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(NpcPrototype);
STRUCT_DECLARE(Stats);

EQP_API NpcPrototype*   npc_proto_create(ZC* zc);
EQP_API void            npc_proto_destroy(NpcPrototype* proto);

// If the name is already client-friendly, returns the passed-in string; otherwise, creates a new string.
String*                 npc_proto_determine_client_friendly_name(ZC* zc, String* name);

String*                 npc_proto_get_name(NpcPrototype* proto);
String*                 npc_proto_get_client_friendly_name(NpcPrototype* proto);

Stats*                  npc_proto_get_stats(NpcPrototype* proto);

EQP_API void            npc_proto_set_name(ZC* zc, NpcPrototype* proto, const char* name, uint32_t len);
EQP_API void            npc_proto_set_adhoc_id(NpcPrototype* proto, uint32_t id);
EQP_API void            npc_proto_set_level(NpcPrototype* proto, uint8_t level);
EQP_API void            npc_proto_set_class(NpcPrototype* proto, uint8_t class);
EQP_API void            npc_proto_set_race(NpcPrototype* proto, uint16_t race);
EQP_API void            npc_proto_set_gender(NpcPrototype* proto, uint8_t gender);
EQP_API void            npc_proto_set_face(NpcPrototype* proto, uint8_t face);
EQP_API void            npc_proto_set_body_type(NpcPrototype* proto, uint8_t bodyType);
EQP_API void            npc_proto_set_texture(NpcPrototype* proto, uint8_t textureId);
EQP_API void            npc_proto_set_helm_texture(NpcPrototype* proto, uint8_t textureId);
EQP_API void            npc_proto_set_max_hp(NpcPrototype* proto, int64_t hp);
EQP_API void            npc_proto_set_max_mana(NpcPrototype* proto, int64_t mana);
EQP_API void            npc_proto_set_max_endurance(NpcPrototype* proto, int64_t endur);
EQP_API void            npc_proto_set_size(NpcPrototype* proto, float size);
EQP_API void            npc_proto_set_running_speed(NpcPrototype* proto, float speed);
EQP_API void            npc_proto_set_walking_speed(NpcPrototype* proto, float speed);
EQP_API void            npc_proto_set_aggro_radius(NpcPrototype* proto, float radius);
EQP_API void            npc_proto_set_material(NpcPrototype* proto, uint8_t slot, uint8_t materialId);
EQP_API void            npc_proto_set_tint(NpcPrototype* proto, uint8_t slot, uint8_t red, uint8_t green, uint8_t blue);
EQP_API void            npc_proto_set_primary_model_id(NpcPrototype* proto, uint32_t modelId);
EQP_API void            npc_proto_set_secondary_model_id(NpcPrototype* proto, uint32_t modelId);
EQP_API void            npc_proto_set_primary_message_skill(NpcPrototype* proto, uint8_t skill);
EQP_API void            npc_proto_set_secondary_message_skill(NpcPrototype* proto, uint8_t skill);
EQP_API void            npc_proto_set_swings_per_round(NpcPrototype* proto, int8_t swings);
EQP_API void            npc_proto_set_melee_damage(NpcPrototype* proto, int64_t minDmg, int64_t maxDmg);
EQP_API void            npc_proto_set_ac(NpcPrototype* proto, uint32_t ac);
EQP_API void            npc_proto_set_str(NpcPrototype* proto, int value);
EQP_API void            npc_proto_set_sta(NpcPrototype* proto, int value);
EQP_API void            npc_proto_set_dex(NpcPrototype* proto, int value);
EQP_API void            npc_proto_set_agi(NpcPrototype* proto, int value);
EQP_API void            npc_proto_set_wis(NpcPrototype* proto, int value);
EQP_API void            npc_proto_set_int(NpcPrototype* proto, int value);
EQP_API void            npc_proto_set_cha(NpcPrototype* proto, int value);

EQP_API const char*     npc_proto_get_name_cstr(NpcPrototype* proto);
EQP_API uint32_t        npc_proto_get_adhoc_id(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_level(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_class(NpcPrototype* proto);
EQP_API uint16_t        npc_proto_get_race(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_gender(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_face(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_body_type(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_texture(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_helm_texture(NpcPrototype* proto);
EQP_API int64_t         npc_proto_get_max_hp(NpcPrototype* proto);
EQP_API int64_t         npc_proto_get_max_mana(NpcPrototype* proto);
EQP_API int64_t         npc_proto_get_max_endurance(NpcPrototype* proto);
EQP_API float           npc_proto_get_size(NpcPrototype* proto);
EQP_API float           npc_proto_get_running_speed(NpcPrototype* proto);
EQP_API float           npc_proto_get_walking_speed(NpcPrototype* proto);
EQP_API float           npc_proto_get_aggro_radius(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_material(NpcPrototype* proto, uint8_t slot);
EQP_API uint32_t        npc_proto_get_tint(NpcPrototype* proto, uint8_t slot);
EQP_API uint32_t        npc_proto_get_primary_model_id(NpcPrototype* proto);
EQP_API uint32_t        npc_proto_get_secondary_model_id(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_primary_message_skill(NpcPrototype* proto);
EQP_API uint8_t         npc_proto_get_secondary_message_skill(NpcPrototype* proto);
EQP_API int8_t          npc_proto_get_swings_per_round(NpcPrototype* proto);
EQP_API int64_t         npc_proto_get_min_damage(NpcPrototype* proto);
EQP_API int64_t         npc_proto_get_max_damage(NpcPrototype* proto);

//fixme: implement this Lua-side
//EQP_API void            npc_proto_calc_defaults_by_level(NpcPrototype* proto, uint8_t level); // If level = 0, will use the proto's currently set level

#endif//EQP_NPC_PROTOTYPE_H
