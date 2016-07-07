
#include "npc_prototype.h"
#include "zone_cluster.h"
#include "mob.h"

STRUCT_DEFINE(NpcPrototype)
{
    String*     name;
    String*     clientFriendlyName;
    uint32_t    adHocId;
    uint8_t     level;
    uint8_t     class;
    uint8_t     gender;
    uint8_t     face;
    uint16_t    race;
    uint8_t     texture;
    uint8_t     helmTexture;
    Stats       stats;
    float       size;
    float       runningSpeed;
    float       walkingSpeed;
    float       aggroRadius;
    uint8_t     materials[7];
    uint32_t    tints[7];
    uint32_t    primaryModelId;
    uint32_t    secondaryModelId;
    uint8_t     bodyType;
    uint8_t     primaryMessageSkill;
    uint8_t     secondaryMessageSkill;
    int8_t      swingsPerRound; // -1 as code for client-style melee rounds..?
    int64_t     minDamage;
    int64_t     maxDamage;
};

NpcPrototype* npc_proto_create(ZC* zc)
{
    NpcPrototype* proto = eqp_alloc_type(B(zc), NpcPrototype);
    
    memset(proto, 0, sizeof(NpcPrototype));
    
    proto->level                    = 1;
    proto->class                    = Class_Warrior;
    proto->gender                   = Gender_Neuter;
    proto->race                     = Race_Human;
    proto->stats.maxHp              = 10;
    proto->size                     = 4.0f;
    proto->runningSpeed             = 0.7f;
    proto->walkingSpeed             = 0.46f;
    proto->aggroRadius              = 50.0f;
    proto->primaryMessageSkill      = Skill_HandToHand;
    proto->secondaryMessageSkill    = Skill_HandToHand;
    proto->swingsPerRound           = 1;
    proto->minDamage                = 1;
    proto->maxDamage                = 2;
    
    return proto;
}

void npc_proto_destroy(NpcPrototype* proto)
{
    if (proto->name)
        string_drop(proto->name);
    
    if (proto->clientFriendlyName)
        string_drop(proto->clientFriendlyName);
    
    free(proto);
}

String* npc_proto_determine_client_friendly_name(ZC* zc, String* name)
{
    uint32_t n      = string_length(name);
    const char* str = string_data(name);
    String* clean   = string_create_with_capacity(B(zc), n + 1);
    int changes     = 0;
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        int c = str[i];
        
        if (c == ' ')
        {
            c = '_';
            changes++;
        }
        
        string_add_char(B(zc), &clean, c);
    }
    
    if (changes == 0)
    {
        string_destroy(clean);
        clean = name;
        string_grab(name);
    }
    
    return clean;
}

String* npc_proto_get_name(NpcPrototype* proto)
{
    return proto->name;
}

String* npc_proto_get_client_friendly_name(NpcPrototype* proto)
{
    return proto->clientFriendlyName;
}

Stats* npc_proto_get_stats(NpcPrototype* proto)
{
    return &proto->stats;
}

void npc_proto_set_name(ZC* zc, NpcPrototype* proto, const char* str, uint32_t len)
{
    if (str)
    {
        String* name = string_create_from_cstr(B(zc), str, len);
        
        proto->name                 = name;
        proto->clientFriendlyName   = npc_proto_determine_client_friendly_name(zc, name);
    }
    else
    {
        if (proto->name)
            string_drop(proto->name);
    
        if (proto->clientFriendlyName)
            string_drop(proto->clientFriendlyName);
        
        proto->name                 = NULL;
        proto->clientFriendlyName   = NULL;
    }
}

void npc_proto_set_adhoc_id(NpcPrototype* proto, uint32_t id)
{
    proto->adHocId = id;
}

void npc_proto_set_level(NpcPrototype* proto, uint8_t level)
{
    if (level == 0)
        level = 1;
    
    proto->level = level;
}

void npc_proto_set_class(NpcPrototype* proto, uint8_t class)
{
    proto->class = class;
}

void npc_proto_set_race(NpcPrototype* proto, uint16_t race)
{
    proto->race = race;
}

void npc_proto_set_gender(NpcPrototype* proto, uint8_t gender)
{
    if (gender > Gender_Neuter)
        gender = Gender_Neuter;
    
    proto->gender = gender;
}

void npc_proto_set_face(NpcPrototype* proto, uint8_t face)
{
    proto->face = face;
}

void npc_proto_set_body_type(NpcPrototype* proto, uint8_t bodyType)
{
    proto->bodyType = bodyType;
}

void npc_proto_set_texture(NpcPrototype* proto, uint8_t textureId)
{
    proto->texture = textureId;
}

void npc_proto_set_helm_texture(NpcPrototype* proto, uint8_t textureId)
{
    proto->helmTexture = textureId;
}

void npc_proto_set_max_hp(NpcPrototype* proto, int64_t hp)
{
    proto->stats.maxHp = hp;
}

void npc_proto_set_max_mana(NpcPrototype* proto, int64_t mana)
{
    proto->stats.maxMana = mana;
}

void npc_proto_set_max_endurance(NpcPrototype* proto, int64_t endur)
{
    proto->stats.maxEndurance = endur;
}

void npc_proto_set_size(NpcPrototype* proto, float size)
{
    proto->size = size;
}

void npc_proto_set_running_speed(NpcPrototype* proto, float speed)
{
    proto->runningSpeed = speed;
}

void npc_proto_set_walking_speed(NpcPrototype* proto, float speed)
{
    proto->walkingSpeed = speed;
}

void npc_proto_set_aggro_radius(NpcPrototype* proto, float radius)
{
    proto->aggroRadius = radius;
}

void npc_proto_set_material(NpcPrototype* proto, uint8_t slot, uint8_t materialId)
{
    if (slot >= 7)
        return;
    
    proto->materials[slot] = materialId;
}

void npc_proto_set_tint(NpcPrototype* proto, uint8_t slot, uint8_t red, uint8_t green, uint8_t blue)
{
    if (slot >= 7)
        return;
    
    proto->tints[slot] = red | ((uint32_t)green) << 8 | ((uint32_t)blue) << 16;
}

void npc_proto_set_primary_model_id(NpcPrototype* proto, uint32_t modelId)
{
    proto->primaryModelId = modelId;
}

void npc_proto_set_secondary_model_id(NpcPrototype* proto, uint32_t modelId)
{
    proto->secondaryModelId = modelId;
}

void npc_proto_set_primary_message_skill(NpcPrototype* proto, uint8_t skill)
{
    proto->primaryMessageSkill = skill;
}

void npc_proto_set_secondary_message_skill(NpcPrototype* proto, uint8_t skill)
{
    proto->secondaryMessageSkill = skill;
}

void npc_proto_set_swings_per_round(NpcPrototype* proto, int8_t swings)
{
    proto->swingsPerRound = swings;
}

void npc_proto_set_melee_damage(NpcPrototype* proto, int64_t minDmg, int64_t maxDmg)
{
    proto->minDamage = minDmg;
    proto->maxDamage = maxDmg;
}

void npc_proto_set_ac(NpcPrototype* proto, uint32_t ac)
{
    proto->stats.AC = ac;
}

void npc_proto_set_str(NpcPrototype* proto, int value)
{
    proto->stats.STR = value;
}

void npc_proto_set_sta(NpcPrototype* proto, int value)
{
    proto->stats.STA = value;
}

void npc_proto_set_dex(NpcPrototype* proto, int value)
{
    proto->stats.DEX = value;
}

void npc_proto_set_agi(NpcPrototype* proto, int value)
{
    proto->stats.AGI = value;
}

void npc_proto_set_wis(NpcPrototype* proto, int value)
{
    proto->stats.WIS = value;
}

void npc_proto_set_int(NpcPrototype* proto, int value)
{
    proto->stats.INT = value;
}

void npc_proto_set_cha(NpcPrototype* proto, int value)
{
    proto->stats.CHA = value;
}

const char* npc_proto_get_name_cstr(NpcPrototype* proto)
{
    String* name = proto->name;
    return name ? string_data(name) : NULL;
}

uint32_t npc_proto_get_adhoc_id(NpcPrototype* proto)
{
    return proto->adHocId;
}

uint8_t npc_proto_get_level(NpcPrototype* proto)
{
    return proto->level;
}

uint8_t npc_proto_get_class(NpcPrototype* proto)
{
    return proto->class;
}

uint16_t npc_proto_get_race(NpcPrototype* proto)
{
    return proto->race;
}

uint8_t npc_proto_get_gender(NpcPrototype* proto)
{
    return proto->gender;
}

uint8_t npc_proto_get_face(NpcPrototype* proto)
{
    return proto->face;
}

uint8_t npc_proto_get_body_type(NpcPrototype* proto)
{
    return proto->bodyType;
}

uint8_t npc_proto_get_texture(NpcPrototype* proto)
{
    return proto->texture;
}

uint8_t npc_proto_get_helm_texture(NpcPrototype* proto)
{
    return proto->helmTexture;
}

int64_t npc_proto_get_max_hp(NpcPrototype* proto)
{
    return proto->stats.maxHp;
}

int64_t npc_proto_get_max_mana(NpcPrototype* proto)
{
    return proto->stats.maxMana;
}

int64_t npc_proto_get_max_endurance(NpcPrototype* proto)
{
    return proto->stats.maxEndurance;
}

float npc_proto_get_size(NpcPrototype* proto)
{
    return proto->size;
}

float npc_proto_get_running_speed(NpcPrototype* proto)
{
    return proto->runningSpeed;
}

float npc_proto_get_walking_speed(NpcPrototype* proto)
{
    return proto->walkingSpeed;
}

float npc_proto_get_aggro_radius(NpcPrototype* proto)
{
    return proto->aggroRadius;
}

uint8_t npc_proto_get_material(NpcPrototype* proto, uint8_t slot)
{
    if (slot >= 7)
        return 0;
    
    return proto->materials[slot];
}

uint32_t npc_proto_get_tint(NpcPrototype* proto, uint8_t slot)
{
    if (slot >= 7)
        return 0;
    
    return proto->tints[slot];
}

uint32_t npc_proto_get_primary_model_id(NpcPrototype* proto)
{
    return proto->primaryModelId;
}

uint32_t npc_proto_get_secondary_model_id(NpcPrototype* proto)
{
    return proto->secondaryModelId;
}

uint8_t npc_proto_get_primary_message_skill(NpcPrototype* proto)
{
    return proto->primaryMessageSkill;
}

uint8_t npc_proto_get_secondary_message_skill(NpcPrototype* proto)
{
    return proto->secondaryMessageSkill;
}

int8_t npc_proto_get_swings_per_round(NpcPrototype* proto)
{
    return proto->swingsPerRound;
}

int64_t npc_proto_get_min_damage(NpcPrototype* proto)
{
    return proto->minDamage;
}

int64_t npc_proto_get_max_damage(NpcPrototype* proto)
{
    return proto->maxDamage;
}
