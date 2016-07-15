
local ffi = require "ffi"

ffi.cdef[[
typedef struct ItemPrototype ItemPrototype;

ItemPrototype*  item_proto_create(void* basic);
void            item_proto_destroy(ItemPrototype* proto);

void            item_proto_set_defaults(ItemPrototype* proto);

void            item_proto_set_name(ItemPrototype* proto, const char* name);
void            item_proto_set_lore_text(ItemPrototype* proto, const char* loreText);
void            item_proto_set_model_id(ItemPrototype* proto, uint32_t modelId);
void            item_proto_set_is_lore(ItemPrototype* proto, int isLore);
void            item_proto_set_is_droppable(ItemPrototype* proto, int isDroppable);
void            item_proto_set_is_magic(ItemPrototype* proto, int isMagic);
void            item_proto_set_is_permanent(ItemPrototype* proto, int isPermanent);
void            item_proto_set_is_stackable(ItemPrototype* proto, int isStackable);
void            item_proto_set_size(ItemPrototype* proto, uint8_t size);
void            item_proto_set_weight(ItemPrototype* proto, uint8_t weight);
void            item_proto_set_item_type_id(ItemPrototype* proto, uint8_t itemTypeId);
void            item_proto_set_item_skill_id(ItemPrototype* proto, uint8_t itemSkillId);
void            item_proto_set_material(ItemPrototype* proto, uint8_t material);
void            item_proto_set_light(ItemPrototype* proto, uint8_t light);
void            item_proto_set_effect_type_id(ItemPrototype* proto, uint8_t effectTypeId);
void            item_proto_set_max_charges(ItemPrototype* proto, uint8_t maxCharges);
void            item_proto_set_str(ItemPrototype* proto, int16_t value);
void            item_proto_set_dex(ItemPrototype* proto, int16_t value);
void            item_proto_set_agi(ItemPrototype* proto, int16_t value);
void            item_proto_set_sta(ItemPrototype* proto, int16_t value);
void            item_proto_set_int(ItemPrototype* proto, int16_t value);
void            item_proto_set_wis(ItemPrototype* proto, int16_t value);
void            item_proto_set_cha(ItemPrototype* proto, int16_t value);
void            item_proto_set_sv_magic(ItemPrototype* proto, int16_t value);
void            item_proto_set_sv_fire(ItemPrototype* proto, int16_t value);
void            item_proto_set_sv_cold(ItemPrototype* proto, int16_t value);
void            item_proto_set_sv_poison(ItemPrototype* proto, int16_t value);
void            item_proto_set_sv_disease(ItemPrototype* proto, int16_t value);
void            item_proto_set_hp(ItemPrototype* proto, int16_t value);
void            item_proto_set_mana(ItemPrototype* proto, int16_t value);
void            item_proto_set_ac(ItemPrototype* proto, int16_t value);
void            item_proto_set_damage(ItemPrototype* proto, uint16_t value);
void            item_proto_set_delay(ItemPrototype* proto, uint16_t value);
void            item_proto_set_range(ItemPrototype* proto, uint16_t value);
void            item_proto_set_slot_bitfield(ItemPrototype* proto, uint32_t bits);
void            item_proto_set_race_bitfield(ItemPrototype* proto, uint32_t bits);
void            item_proto_set_class_bitfield(ItemPrototype* proto, uint32_t bits);
void            item_proto_set_spell_id(ItemPrototype* proto, uint32_t spellId);
void            item_proto_set_casting_time(ItemPrototype* proto, uint32_t castingTime);
void            item_proto_set_icon_id(ItemPrototype* proto, uint32_t iconId);
void            item_proto_set_cost(ItemPrototype* proto, uint32_t cost);
void            item_proto_set_tint(ItemPrototype* proto, uint32_t tint);

uint32_t        item_proto_get_item_id(ItemPrototype* proto);
const char*     item_proto_get_script_path(ItemPrototype* proto);
const char*     item_proto_get_name(ItemPrototype* proto);
const char*     item_proto_get_lore_text(ItemPrototype* proto);
uint32_t        item_proto_get_model_id(ItemPrototype* proto);
int             item_proto_is_lore(ItemPrototype* proto);
int             item_proto_is_droppable(ItemPrototype* proto);
int             item_proto_is_magic(ItemPrototype* proto);
int             item_proto_is_permanent(ItemPrototype* proto);
int             item_proto_is_stackable(ItemPrototype* proto);
uint8_t         item_proto_get_size(ItemPrototype* proto);
uint8_t         item_proto_get_weight(ItemPrototype* proto);
uint8_t         item_proto_get_item_type_id(ItemPrototype* proto);
uint8_t         item_proto_get_item_skill_id(ItemPrototype* proto);
uint8_t         item_proto_get_material(ItemPrototype* proto);
uint8_t         item_proto_get_light(ItemPrototype* proto);
uint8_t         item_proto_get_effect_type_id(ItemPrototype* proto);
uint8_t         item_proto_get_max_charges(ItemPrototype* proto);
int16_t         item_proto_get_str(ItemPrototype* proto);
int16_t         item_proto_get_dex(ItemPrototype* proto);
int16_t         item_proto_get_agi(ItemPrototype* proto);
int16_t         item_proto_get_sta(ItemPrototype* proto);
int16_t         item_proto_get_int(ItemPrototype* proto);
int16_t         item_proto_get_wis(ItemPrototype* proto);
int16_t         item_proto_get_cha(ItemPrototype* proto);
int16_t         item_proto_get_sv_magic(ItemPrototype* proto);
int16_t         item_proto_get_sv_fire(ItemPrototype* proto);
int16_t         item_proto_get_sv_cold(ItemPrototype* proto);
int16_t         item_proto_get_sv_poison(ItemPrototype* proto);
int16_t         item_proto_get_sv_disease(ItemPrototype* proto);
int16_t         item_proto_get_hp(ItemPrototype* proto);
int16_t         item_proto_get_mana(ItemPrototype* proto);
int16_t         item_proto_get_ac(ItemPrototype* proto);
uint16_t        item_proto_get_damage(ItemPrototype* proto);
uint16_t        item_proto_get_delay(ItemPrototype* proto);
uint16_t        item_proto_get_range(ItemPrototype* proto);
uint32_t        item_proto_get_slot_bitfield(ItemPrototype* proto);
uint32_t        item_proto_get_race_bitfield(ItemPrototype* proto);
uint32_t        item_proto_get_class_bitfield(ItemPrototype* proto);
uint32_t        item_proto_get_spell_id(ItemPrototype* proto);
uint32_t        item_proto_get_casting_time(ItemPrototype* proto);
uint32_t        item_proto_get_icon_id(ItemPrototype* proto);
uint32_t        item_proto_get_cost(ItemPrototype* proto);
uint32_t        item_proto_get_tint(ItemPrototype* proto);
]]

return ffi.C
