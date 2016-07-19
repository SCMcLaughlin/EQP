
#include "item_prototype.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"

ItemPrototype* item_proto_create(Basic* basic)
{
    ItemPrototype* proto = eqp_alloc_type(basic, ItemPrototype);
    item_proto_set_defaults(proto);
    return proto;
}

ItemPrototype* item_proto_copy(Basic* basic, ItemPrototype* orig)
{
    ItemPrototype* proto = eqp_alloc_type(basic, ItemPrototype);
    memcpy(proto, orig, sizeof(ItemPrototype));
    return proto;
}

void item_proto_destroy(ItemPrototype* proto)
{
    free(proto);
}

void item_proto_set_defaults(ItemPrototype* proto)
{
    memset(proto, 0, sizeof(ItemPrototype));
    
    proto->isDroppable      = true;
    proto->isPermanent      = true;
    proto->itemSkillId      = 10; //fixme: make enum; 10 = armor
    proto->raceBitfield     = 0xffff;
    proto->classBitfield    = 0xffff;
    proto->spellId          = 0xffff;
    proto->iconId           = 500; // Just for the sake of avoiding invisible items...
}

void item_proto_set_item_id(ItemPrototype* proto, uint32_t itemId)
{
    proto->itemId = itemId;
}

void item_proto_set_name(ItemPrototype* proto, const char* name)
{
    snprintf(proto->name, sizeof_field(ItemPrototype, name), "%s", name);
}

void item_proto_set_lore_text(ItemPrototype* proto, const char* loreText)
{
    snprintf(proto->name, sizeof_field(ItemPrototype, loreText), "%s", loreText);
}

void item_proto_set_script_path(ItemPrototype* proto, const char* path)
{
    snprintf(proto->scriptPath, sizeof_field(ItemPrototype, scriptPath), "%s", path);
}

void item_proto_set_model_id(ItemPrototype* proto, uint32_t modelId)
{
    proto->modelId = modelId;
}

void item_proto_set_is_lore(ItemPrototype* proto, int isLore)
{
    proto->isLore = isLore;
}

void item_proto_set_is_droppable(ItemPrototype* proto, int isDroppable)
{
    proto->isDroppable = isDroppable;
}

void item_proto_set_is_magic(ItemPrototype* proto, int isMagic)
{
    proto->isMagic = isMagic;
}

void item_proto_set_is_permanent(ItemPrototype* proto, int isPermanent)
{
    proto->isPermanent = isPermanent;
}

void item_proto_set_is_stackable(ItemPrototype* proto, int isStackable)
{
    proto->isStackable = isStackable;
}

void item_proto_set_size(ItemPrototype* proto, uint8_t size)
{
    proto->size = size;
}

void item_proto_set_weight(ItemPrototype* proto, uint8_t weight)
{
    proto->weight = weight;
}

void item_proto_set_item_type_id(ItemPrototype* proto, uint8_t itemTypeId)
{
    proto->itemTypeId = itemTypeId;
}

void item_proto_set_item_skill_id(ItemPrototype* proto, uint8_t itemSkillId)
{
    proto->itemSkillId = itemSkillId;
}

void item_proto_set_material(ItemPrototype* proto, uint8_t material)
{
    proto->material = material;
}

void item_proto_set_light(ItemPrototype* proto, uint8_t light)
{
    proto->light = light;
}

void item_proto_set_effect_type_id(ItemPrototype* proto, uint8_t effectTypeId)
{
    proto->effectTypeId = effectTypeId;
}

void item_proto_set_max_charges(ItemPrototype* proto, uint8_t maxCharges)
{
    proto->maxCharges = maxCharges;
}

void item_proto_set_str(ItemPrototype* proto, int16_t value)
{
    proto->STR = value;
}

void item_proto_set_dex(ItemPrototype* proto, int16_t value)
{
    proto->DEX = value;
}

void item_proto_set_agi(ItemPrototype* proto, int16_t value)
{
    proto->AGI = value;
}

void item_proto_set_sta(ItemPrototype* proto, int16_t value)
{
    proto->STA = value;
}

void item_proto_set_int(ItemPrototype* proto, int16_t value)
{
    proto->INT = value;
}

void item_proto_set_wis(ItemPrototype* proto, int16_t value)
{
    proto->WIS = value;
}

void item_proto_set_cha(ItemPrototype* proto, int16_t value)
{
    proto->CHA = value;
}

void item_proto_set_sv_magic(ItemPrototype* proto, int16_t value)
{
    proto->svMagic = value;
}

void item_proto_set_sv_fire(ItemPrototype* proto, int16_t value)
{
    proto->svFire = value;
}

void item_proto_set_sv_cold(ItemPrototype* proto, int16_t value)
{
    proto->svCold = value;
}

void item_proto_set_sv_poison(ItemPrototype* proto, int16_t value)
{
    proto->svPoison = value;
}

void item_proto_set_sv_disease(ItemPrototype* proto, int16_t value)
{
    proto->svDisease = value;
}

void item_proto_set_hp(ItemPrototype* proto, int16_t value)
{
    proto->hp = value;
}

void item_proto_set_mana(ItemPrototype* proto, int16_t value)
{
    proto->mana = value;
}

void item_proto_set_ac(ItemPrototype* proto, int16_t value)
{
    proto->ac = value;
}

void item_proto_set_damage(ItemPrototype* proto, uint16_t value)
{
    proto->damage = value;
}

void item_proto_set_delay(ItemPrototype* proto, uint16_t value)
{
    proto->delay = value;
}

void item_proto_set_range(ItemPrototype* proto, uint16_t value)
{
    proto->range = value;
}

void item_proto_set_slot_bitfield(ItemPrototype* proto, uint32_t bits)
{
    proto->slotBitfield = bits;
}

void item_proto_set_race_bitfield(ItemPrototype* proto, uint32_t bits)
{
    proto->raceBitfield = bits;
}

void item_proto_set_class_bitfield(ItemPrototype* proto, uint32_t bits)
{
    proto->classBitfield = bits;
}

void item_proto_set_spell_id(ItemPrototype* proto, uint32_t spellId)
{
    proto->spellId = spellId;
}

void item_proto_set_casting_time(ItemPrototype* proto, uint32_t castingTime)
{
    proto->castingTime = castingTime;
}

void item_proto_set_icon_id(ItemPrototype* proto, uint32_t iconId)
{
    proto->iconId = iconId;
}

void item_proto_set_cost(ItemPrototype* proto, uint32_t cost)
{
    proto->cost = cost;
}

void item_proto_set_tint(ItemPrototype* proto, uint32_t tint)
{
    proto->tint = tint;
}

uint32_t item_proto_get_item_id(ItemPrototype* proto)
{
    return proto->itemId;
}

const char* item_proto_get_name(ItemPrototype* proto)
{
    return proto->name;
}

const char* item_proto_get_lore_text(ItemPrototype* proto)
{
    return proto->loreText;
}

const char* item_proto_get_script_path(ItemPrototype* proto)
{
    return proto->scriptPath;
}

uint32_t item_proto_get_model_id(ItemPrototype* proto)
{
    return proto->modelId;
}

int item_proto_is_lore(ItemPrototype* proto)
{
    return proto->isLore;
}

int item_proto_is_droppable(ItemPrototype* proto)
{
    return proto->isDroppable;
}

int item_proto_is_magic(ItemPrototype* proto)
{
    return proto->isMagic;
}

int item_proto_is_permanent(ItemPrototype* proto)
{
    return proto->isPermanent;
}

int item_proto_is_stackable(ItemPrototype* proto)
{
    return proto->isStackable;
}

uint8_t item_proto_get_size(ItemPrototype* proto)
{
    return proto->size;
}

uint8_t item_proto_get_weight(ItemPrototype* proto)
{
    return proto->weight;
}

uint8_t item_proto_get_item_type_id(ItemPrototype* proto)
{
    return proto->itemTypeId;
}

uint8_t item_proto_get_item_skill_id(ItemPrototype* proto)
{
    return proto->itemSkillId;
}

uint8_t item_proto_get_material(ItemPrototype* proto)
{
    return proto->material;
}

uint8_t item_proto_get_light(ItemPrototype* proto)
{
    return proto->light;
}

uint8_t item_proto_get_effect_type_id(ItemPrototype* proto)
{
    return proto->effectTypeId;
}

uint8_t item_proto_get_max_charges(ItemPrototype* proto)
{
    return proto->maxCharges;
}

int16_t item_proto_get_str(ItemPrototype* proto)
{
    return proto->STR;
}

int16_t item_proto_get_dex(ItemPrototype* proto)
{
    return proto->DEX;
}

int16_t item_proto_get_agi(ItemPrototype* proto)
{
    return proto->AGI;
}

int16_t item_proto_get_sta(ItemPrototype* proto)
{
    return proto->STA;
}

int16_t item_proto_get_int(ItemPrototype* proto)
{
    return proto->INT;
}

int16_t item_proto_get_wis(ItemPrototype* proto)
{
    return proto->WIS;
}

int16_t item_proto_get_cha(ItemPrototype* proto)
{
    return proto->CHA;
}

int16_t item_proto_get_sv_magic(ItemPrototype* proto)
{
    return proto->svMagic;
}

int16_t item_proto_get_sv_fire(ItemPrototype* proto)
{
    return proto->svFire;
}

int16_t item_proto_get_sv_cold(ItemPrototype* proto)
{
    return proto->svCold;
}

int16_t item_proto_get_sv_poison(ItemPrototype* proto)
{
    return proto->svPoison;
}

int16_t item_proto_get_sv_disease(ItemPrototype* proto)
{
    return proto->svDisease;
}

int16_t item_proto_get_hp(ItemPrototype* proto)
{
    return proto->hp;
}

int16_t item_proto_get_mana(ItemPrototype* proto)
{
    return proto->mana;
}

int16_t item_proto_get_ac(ItemPrototype* proto)
{
    return proto->ac;
}

uint16_t item_proto_get_damage(ItemPrototype* proto)
{
    return proto->damage;
}

uint16_t item_proto_get_delay(ItemPrototype* proto)
{
    return proto->delay;
}

uint16_t item_proto_get_range(ItemPrototype* proto)
{
    return proto->range;
}

uint32_t item_proto_get_slot_bitfield(ItemPrototype* proto)
{
    return proto->slotBitfield;
}

uint32_t item_proto_get_race_bitfield(ItemPrototype* proto)
{
    return proto->raceBitfield;
}

uint32_t item_proto_get_class_bitfield(ItemPrototype* proto)
{
    return proto->classBitfield;
}

uint32_t item_proto_get_spell_id(ItemPrototype* proto)
{
    return proto->spellId;
}

uint32_t item_proto_get_casting_time(ItemPrototype* proto)
{
    return proto->castingTime;
}

uint32_t item_proto_get_icon_id(ItemPrototype* proto)
{
    return proto->iconId;
}

uint32_t item_proto_get_cost(ItemPrototype* proto)
{
    return proto->cost;
}

uint32_t item_proto_get_tint(ItemPrototype* proto)
{
    return proto->tint;
}
