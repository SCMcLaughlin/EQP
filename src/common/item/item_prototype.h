
#ifndef EQP_ITEM_PROTOTYPE_H
#define EQP_ITEM_PROTOTYPE_H

#include "define.h"

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(ItemPrototype)
{
    uint32_t    itemId;
    char        name[64];
    char        loreText[64];
    uint32_t    modelId;
    uint8_t     isLore;
    uint8_t     isDroppable;
    uint8_t     isMagic;
    uint8_t     isPermanent;
    uint8_t     isStackable;
    uint8_t     size;
    uint8_t     weight;
    uint8_t     itemTypeId;
    uint8_t     itemSkillId;
    uint8_t     material;
    uint8_t     light;
    uint8_t     effectTypeId;
    uint8_t     maxCharges;
    int16_t     STR;
    int16_t     DEX;
    int16_t     AGI;
    int16_t     STA;
    int16_t     INT;
    int16_t     WIS;
    int16_t     CHA;
    int16_t     svMagic;
    int16_t     svFire;
    int16_t     svCold;
    int16_t     svPoison;
    int16_t     svDisease;
    int16_t     hp;
    int16_t     mana;
    int16_t     ac;
    uint16_t    damage;
    uint16_t    delay;
    uint16_t    range;
    uint16_t    scriptPathLength;
    uint32_t    slotBitfield;
    uint32_t    raceBitfield;
    uint32_t    classBitfield;
    uint32_t    spellId;
    uint32_t    castingTime;
    uint32_t    iconId;
    uint32_t    cost;
    uint32_t    tint;
    char        scriptPath[0];
};

EQP_API ItemPrototype*  item_proto_create(Basic* basic);
EQP_API void            item_proto_destroy(ItemPrototype* proto);

EQP_API void            item_proto_set_defaults(ItemPrototype* proto);

EQP_API void            item_proto_set_item_id(ItemPrototype* proto, uint32_t itemId);
EQP_API void            item_proto_set_name(ItemPrototype* proto, const char* name);
EQP_API void            item_proto_set_lore_text(ItemPrototype* proto, const char* loreText);
EQP_API void            item_proto_set_model_id(ItemPrototype* proto, uint32_t modelId);
EQP_API void            item_proto_set_is_lore(ItemPrototype* proto, int isLore);
EQP_API void            item_proto_set_is_droppable(ItemPrototype* proto, int isDroppable);
EQP_API void            item_proto_set_is_magic(ItemPrototype* proto, int isMagic);
EQP_API void            item_proto_set_is_permanent(ItemPrototype* proto, int isPermanent);
EQP_API void            item_proto_set_is_stackable(ItemPrototype* proto, int isStackable);
EQP_API void            item_proto_set_size(ItemPrototype* proto, uint8_t size);
EQP_API void            item_proto_set_weight(ItemPrototype* proto, uint8_t weight);
EQP_API void            item_proto_set_item_type_id(ItemPrototype* proto, uint8_t itemTypeId);
EQP_API void            item_proto_set_item_skill_id(ItemPrototype* proto, uint8_t itemSkillId);
EQP_API void            item_proto_set_material(ItemPrototype* proto, uint8_t material);
EQP_API void            item_proto_set_light(ItemPrototype* proto, uint8_t light);
EQP_API void            item_proto_set_effect_type_id(ItemPrototype* proto, uint8_t effectTypeId);
EQP_API void            item_proto_set_max_charges(ItemPrototype* proto, uint8_t maxCharges);
EQP_API void            item_proto_set_str(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_dex(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_agi(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_sta(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_int(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_wis(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_cha(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_sv_magic(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_sv_fire(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_sv_cold(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_sv_poison(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_sv_disease(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_hp(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_mana(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_ac(ItemPrototype* proto, int16_t value);
EQP_API void            item_proto_set_damage(ItemPrototype* proto, uint16_t value);
EQP_API void            item_proto_set_delay(ItemPrototype* proto, uint16_t value);
EQP_API void            item_proto_set_range(ItemPrototype* proto, uint16_t value);
EQP_API void            item_proto_set_slot_bitfield(ItemPrototype* proto, uint32_t bits);
EQP_API void            item_proto_set_race_bitfield(ItemPrototype* proto, uint32_t bits);
EQP_API void            item_proto_set_class_bitfield(ItemPrototype* proto, uint32_t bits);
EQP_API void            item_proto_set_spell_id(ItemPrototype* proto, uint32_t spellId);
EQP_API void            item_proto_set_casting_time(ItemPrototype* proto, uint32_t castingTime);
EQP_API void            item_proto_set_icon_id(ItemPrototype* proto, uint32_t iconId);
EQP_API void            item_proto_set_cost(ItemPrototype* proto, uint32_t cost);
EQP_API void            item_proto_set_tint(ItemPrototype* proto, uint32_t tint);

EQP_API uint32_t        item_proto_get_item_id(ItemPrototype* proto);
EQP_API const char*     item_proto_get_name(ItemPrototype* proto);
EQP_API const char*     item_proto_get_lore_text(ItemPrototype* proto);
EQP_API uint32_t        item_proto_get_model_id(ItemPrototype* proto);
EQP_API int             item_proto_is_lore(ItemPrototype* proto);
EQP_API int             item_proto_is_droppable(ItemPrototype* proto);
EQP_API int             item_proto_is_magic(ItemPrototype* proto);
EQP_API int             item_proto_is_permanent(ItemPrototype* proto);
EQP_API int             item_proto_is_stackable(ItemPrototype* proto);
EQP_API uint8_t         item_proto_get_size(ItemPrototype* proto);
EQP_API uint8_t         item_proto_get_weight(ItemPrototype* proto);
EQP_API uint8_t         item_proto_get_item_type_id(ItemPrototype* proto);
EQP_API uint8_t         item_proto_get_item_skill_id(ItemPrototype* proto);
EQP_API uint8_t         item_proto_get_material(ItemPrototype* proto);
EQP_API uint8_t         item_proto_get_light(ItemPrototype* proto);
EQP_API uint8_t         item_proto_get_effect_type_id(ItemPrototype* proto);
EQP_API uint8_t         item_proto_get_max_charges(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_str(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_dex(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_agi(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_sta(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_int(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_wis(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_cha(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_sv_magic(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_sv_fire(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_sv_cold(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_sv_poison(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_sv_disease(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_hp(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_mana(ItemPrototype* proto);
EQP_API int16_t         item_proto_get_ac(ItemPrototype* proto);
EQP_API uint16_t        item_proto_get_damage(ItemPrototype* proto);
EQP_API uint16_t        item_proto_get_delay(ItemPrototype* proto);
EQP_API uint16_t        item_proto_get_range(ItemPrototype* proto);
EQP_API uint32_t        item_proto_get_slot_bitfield(ItemPrototype* proto);
EQP_API uint32_t        item_proto_get_race_bitfield(ItemPrototype* proto);
EQP_API uint32_t        item_proto_get_class_bitfield(ItemPrototype* proto);
EQP_API uint32_t        item_proto_get_spell_id(ItemPrototype* proto);
EQP_API uint32_t        item_proto_get_casting_time(ItemPrototype* proto);
EQP_API uint32_t        item_proto_get_icon_id(ItemPrototype* proto);
EQP_API uint32_t        item_proto_get_cost(ItemPrototype* proto);
EQP_API uint32_t        item_proto_get_tint(ItemPrototype* proto);

#endif//EQP_ITEM_PROTOTYPE_H
