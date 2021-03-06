
#ifndef EQP_CLIENT_H
#define EQP_CLIENT_H

#include "define.h"
#include "network_interface.h"
#include "protocol_handler.h"
#include "mob.h"
#include "server_structs.h"
#include "expansion.h"
#include "eqp_string.h"
#include "inventory.h"
#include "skills.h"
#include "spellbook.h"
#include "structs.h"
#include "bit.h"
#include "timer.h"
#include "client_save.h"

#define EQP_CLIENT_ZONE_IN_EXPECTED_TIMEOUT TIMER_SECONDS(30)
#define EQP_CLIENT_ZONE_IN_NO_AUTH_TIMEOUT  TIMER_SECONDS(30)
#define EQP_CLIENT_AUTO_SAVE_DELAY          TIMER_MINUTES(3)

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(Zone);

STRUCT_DEFINE(Client)
{
    // Mob must be the first member of this struct
    Mob                 mob;
    ProtocolHandler*    handler;
    int                 expansion;
    atomic_int          refCount;
    
    uint32_t    isStubClient        : 1;
    uint32_t    isLocal             : 1;
    uint32_t    isFromCharSelect    : 1;
    uint32_t    isAutoSplitEnabled  : 1;
    uint32_t    isPermanentlyPvP    : 1;
    uint32_t    isCurrentlyPvP      : 1;
    uint32_t    isGM                : 1;
    uint32_t    isAfk               : 1;
    uint32_t    isLinkdead          : 1;
    uint32_t    isSneaking          : 1;
    uint32_t    isHiding            : 1;
    
    union
    {
        struct
        {
            uint8_t connection      : 1;
            uint8_t stats           : 1;
            uint8_t inventory       : 1;
            uint8_t skills          : 1;
            uint8_t spellbook       : 1;
            uint8_t memmedSpells    : 1;
            uint8_t bindPoints      : 1;
            uint8_t unused          : 1;
        };
        
        uint8_t total;
    } loaded;
    
    uint8_t     anonSetting;
    uint8_t     drunkeness;
    
    int         zoneClientIndex;
    
    Inventory   inventory;
    Skills      skills;
    Spellbook   spellbook;
    
    Coin        coins;
    Coin        coinsBank;
    Coin        coinsCursor;
    
    String*     surname;
    uint8_t     guildRank;
    int         guildId;
    
    uint64_t    harmtouchTimestamp; //reminder: set to current time if db value is 0
    uint64_t    disciplineTimestamp;
    
    uint32_t    hungerLevel;    // More hungry as this value approaches 0
    uint32_t    thirstLevel;
    
    uint16_t    trainingPoints;
    int64_t     experience;
    
    BindPoint   bindPoints[EQP_CLIENT_BIND_POINT_COUNT];
    
    int64_t     characterId;
    uint64_t    creationTimestamp;
    String*     accountName;
    uint32_t    accountId;
    uint32_t    ipAddress;
    
    Timer       timerAutoSave;
};

Client* client_create(ZC* zc, Zone* zone, Server_ClientZoning* zoning);
#define client_grab(cli) atomic_fetch_add(&(cli)->refCount, 1)
void    client_drop(Client* client);
void    client_forcibly_disconnect(Client* client);

void    client_catch_up_with_loading_progress(Client* client);
void    client_check_loading_finished(Client* client);
void    client_fill_in_missing_bind_points(Client* client);

void    client_on_unhandled_packet_opcode(Client* client, uint16_t opcode, Aligned* a);

int64_t client_calc_base_hp_trilogy(Client* client);

#define client_set_zone_index(cli, index)   ((cli)->zoneClientIndex = (index))
#define client_zone_index(cli)              ((cli)->zoneClientIndex)
#define client_set_linkdead(cli)            ((cli)->isLinkdead = true)

#define client_guild_id(cli)                ((cli)->guildId)
#define client_is_trilogy(cli)              ((cli)->expansion == ExpansionId_Trilogy)

#define client_is_from_char_select(cli)     ((cli)->isFromCharSelect)

#define client_set_handler(cli, h)          ((cli)->handler = (h))
#define client_handler(cli)                 ((cli)->handler)
#define client_name(cli)                    mob_name(&(cli)->mob)
#define client_name_cstr(cli)               mob_name_cstr(&(cli)->mob)
#define client_entity_id(cli)               mob_entity_id(&(cli)->mob)
#define client_level(cli)                   mob_level(&(cli)->mob)
#define client_class(cli)                   mob_class(&(cli)->mob)
#define client_base_race(cli)               mob_base_race(&(cli)->mob)
#define client_current_race(cli)            mob_current_race(&(cli)->mob)
#define client_base_gender(cli)             mob_base_gender(&(cli)->mob)
#define client_current_gender(cli)          mob_current_gender(&(cli)->mob)
#define client_face(cli)                    mob_face(&(cli)->mob)
#define client_deity(cli)                   mob_deity(&(cli)->mob)
#define client_x(cli)                       mob_x(&(cli)->mob)
#define client_y(cli)                       mob_y(&(cli)->mob)
#define client_z(cli)                       mob_z(&(cli)->mob)
#define client_heading(cli)                 mob_heading(&(cli)->mob)
#define client_current_hp(cli)              mob_current_hp(&(cli)->mob)
#define client_max_hp(cli)                  mob_max_hp(&(cli)->mob)
#define client_current_mana(cli)            mob_current_mana(&(cli)->mob)
#define client_max_mana(cli)                mob_max_mana(&(cli)->mob)
#define client_current_endurance(cli)       mob_current_endurance(&(cli)->mob)
#define client_max_endurance(cli)           mob_max_endurance(&(cli)->mob)
#define client_cur_str(cli)                 mob_cur_str(&(cli)->mob)
#define client_base_str(cli)                mob_base_str(&(cli)->mob)
#define client_cur_sta(cli)                 mob_cur_sta(&(cli)->mob)
#define client_base_sta(cli)                mob_base_sta(&(cli)->mob)
#define client_cur_dex(cli)                 mob_cur_dex(&(cli)->mob)
#define client_base_dex(cli)                mob_base_dex(&(cli)->mob)
#define client_cur_agi(cli)                 mob_cur_agi(&(cli)->mob)
#define client_base_agi(cli)                mob_base_agi(&(cli)->mob)
#define client_cur_int(cli)                 mob_cur_int(&(cli)->mob)
#define client_base_int(cli)                mob_base_int(&(cli)->mob)
#define client_cur_wis(cli)                 mob_cur_wis(&(cli)->mob)
#define client_base_wis(cli)                mob_base_wis(&(cli)->mob)
#define client_cur_cha(cli)                 mob_cur_cha(&(cli)->mob)
#define client_base_cha(cli)                mob_base_cha(&(cli)->mob)
#define client_current_walking_speed(cli)   mob_current_walking_speed(&(cli)->mob)
#define client_base_walking_speed(cli)      mob_base_walking_speed(&(cli)->mob)
#define client_current_running_speed(cli)   mob_current_running_speed(&(cli)->mob)
#define client_base_running_speed(cli)      mob_base_running_speed(&(cli)->mob)
#define client_current_size(cli)            mob_current_size(&(cli)->mob)
#define client_base_size(cli)               mob_base_size(&(cli)->mob)
#define client_spellbook_is_empty(cli)      spellbook_is_empty(&(cli)->spellbook)
#define client_zone(cli)                    mob_zone(&(cli)->mob)
#define client_zone_cluster(cli)            mob_zone_cluster(&(cli)->mob)

#define client_character_id(cli)            ((cli)->characterId)
#define client_account_name(cli)            ((cli)->accountName)
#define client_account_id(cli)              ((cli)->accountId)

#define client_texture(cli)                 mob_texture(&(cli)->mob)
#define client_helm_texture(cli)            mob_helm_texture(&(cli)->mob)
#define client_primary_model_id(cli)        mob_primary_model_id(&(cli)->mob)
#define client_secondary_model_id(cli)      mob_secondary_model_id(&(cli)->mob)

#define client_inventory(cli)               (&(cli)->inventory)
#define client_skills(cli)                  (&(cli)->skills)
#define client_spellbook(cli)               (&(cli)->spellbook)
#define client_bind_points(cli)             (&(cli)->bindPoints)

#define client_coins(cli)                   (&(cli)->coins)
#define client_coins_bank(cli)              (&(cli)->coinsBank)
#define client_coins_cursor(cli)            (&(cli)->coinsCursor)

#define client_harmtouch_timestamp(cli)     ((cli)->harmtouchTimestamp)
#define client_discipline_timestamp(cli)    ((cli)->disciplineTimestamp)

#define client_hunger_level(cli)            ((cli)->hungerLevel)
#define client_thirst_level(cli)            ((cli)->thirstLevel)
#define client_training_points(cli)         ((cli)->trainingPoints)
#define client_experience(cli)              ((cli)->experience)
#define client_drunkeness(cli)              ((cli)->drunkeness)

#define client_get_tint(cli, i)             mob_get_tint(&(cli)->mob, (i))
#define client_get_material(cli, i)         mob_get_material(&(cli)->mob, (i))
#define client_primary_model_id(cli)        mob_primary_model_id(&(cli)->mob)
#define client_secondary_model_id(cli)      mob_secondary_model_id(&(cli)->mob)

EQP_API int         client_expansion(Client* client);
EQP_API int         client_is_pvp(Client* client);
EQP_API int         client_is_gm(Client* client);
EQP_API int         client_is_afk(Client* client);
EQP_API int         client_is_linkdead(Client* client);
EQP_API uint8_t     client_anon_setting(Client* client);
EQP_API uint8_t     client_guild_rank(Client* client);
EQP_API const char* client_surname_cstr(Client* client);

EQP_API void        client_send_custom_message(Client* client, uint32_t chatChannel, const char* str, uint32_t len);

EQP_API void        client_set_bind_point(Client* client, uint32_t bindId, int zoneId, int instId, float x, float y, float z, float heading);

#endif//EQP_CLIENT_H
