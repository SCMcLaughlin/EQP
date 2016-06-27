
-- Enable WAL mode
PRAGMA journal_mode = WAL;

CREATE TABLE login (
    username    TEXT PRIMARY KEY,
    password    BLOB,
    salt        BLOB
);

CREATE TABLE parameter (
    key     TEXT PRIMARY KEY,
    value
);

INSERT INTO
    parameter (key, value) 
VALUES
    ('eqBaseUnixSeconds', strftime('%s', 'now')),
    ('messageOfTheDay', 'Welcome to EQP!');

-- If we are connected to two separate login servers, we need to make sure that
-- an account (from login server A) with an id of 1 and name "abc" is treated as
-- separate from an account (from login server B) with an id of 1 and name "xyz".
-- To do this, we make sure to use both the id number AND the account name,
-- rather than just the id number.
CREATE TABLE account_name_id_pair (
    id      INT,
    name    TEXT,
    PRIMARY KEY (id, name)
);

-- fk_name_id_pair is the rowid from a account_name_id_pair entry
CREATE TABLE account (
    fk_name_id_pair             INT     PRIMARY KEY,
    most_recent_character_name  TEXT,
    most_recent_ip_address      TEXT,
    status                      INT     DEFAULT 0,
    gm_speed                    BOOLEAN DEFAULT 0,
    gm_hide                     BOOLEAN DEFAULT 0,
    shared_platinum             INT     DEFAULT 0,
    suspended_until             INT     DEFAULT 0,
    creation_time               DATE    DEFAULT 0
);

CREATE TRIGGER trigger_account_creation_time AFTER INSERT ON account
BEGIN
    UPDATE account SET creation_time = datetime('now') WHERE fk_name_id_pair = new.fk_name_id_pair;
END;

-- character_id aliases the rowid, implicitly auto-increments.
--
-- This is gonna be a big one...
--
-- The material fields look like 'how not to design a database 101',
-- but we really have no reason to split them into their own table... 
-- only used for char select, anyway.
CREATE TABLE character (
    character_id            INTEGER PRIMARY KEY,
    fk_name_id_pair         INT,
    name                    TEXT    UNIQUE,
    surname                 TEXT,
    level                   INT     DEFAULT 1,
    class                   INT     DEFAULT 1,
    race                    INT     DEFAULT 1,
    zone_id                 INT     DEFAULT 1,
    instance_id             INT     DEFAULT 0,
    gender                  INT     DEFAULT 0,
    face                    INT     DEFAULT 0,
    deity                   INT     DEFAULT 140,
    x                       REAL    DEFAULT 0,
    y                       REAL    DEFAULT 0,
    z                       REAL    DEFAULT 0,
    heading                 REAL    DEFAULT 0,
    current_hp              INT     DEFAULT 10,
    current_mana            INT     DEFAULT 0,
    current_endurance       INT     DEFAULT 0,
    experience              INT     DEFAULT 0,
    training_points         INT     DEFAULT 5,
    base_str                INT     DEFAULT 0,
    base_sta                INT     DEFAULT 0,
    base_dex                INT     DEFAULT 0,
    base_agi                INT     DEFAULT 0,
    base_int                INT     DEFAULT 0,
    base_wis                INT     DEFAULT 0,
    base_cha                INT     DEFAULT 0,
    fk_guild_id             INT     DEFAULT 0,
    guild_rank              INT     DEFAULT 0,
    harmtouch_timestamp     INT     DEFAULT 0,
    discipline_timestamp    INT     DEFAULT 0,
    pp                      INT     DEFAULT 0,
    gp                      INT     DEFAULT 0,
    sp                      INT     DEFAULT 0,
    cp                      INT     DEFAULT 0,
    pp_cursor               INT     DEFAULT 0,
    gp_cursor               INT     DEFAULT 0,
    sp_cursor               INT     DEFAULT 0,
    cp_cursor               INT     DEFAULT 0,
    pp_bank                 INT     DEFAULT 0,
    gp_bank                 INT     DEFAULT 0,
    sp_bank                 INT     DEFAULT 0,
    cp_bank                 INT     DEFAULT 0,
    hunger                  INT     DEFAULT 4500,
    thirst                  INT     DEFAULT 4500,
    is_gm                   BOOLEAN DEFAULT 0,
    anon                    INT     DEFAULT 0,
    drunkeness              INT     DEFAULT 0,
    creation_time           INT,
    material0               INT     DEFAULT 0,
    material1               INT     DEFAULT 0,
    material2               INT     DEFAULT 0,
    material3               INT     DEFAULT 0,
    material4               INT     DEFAULT 0,
    material5               INT     DEFAULT 0,
    material6               INT     DEFAULT 0,
    material7               INT     DEFAULT 0,
    material8               INT     DEFAULT 0,
    tint0                   INT     DEFAULT 0,
    tint1                   INT     DEFAULT 0,
    tint2                   INT     DEFAULT 0,
    tint3                   INT     DEFAULT 0,
    tint4                   INT     DEFAULT 0,
    tint5                   INT     DEFAULT 0,
    tint6                   INT     DEFAULT 0
);

CREATE INDEX index_character_account_id_and_name ON character (fk_name_id_pair, name);

CREATE TRIGGER trigger_character_creation_time AFTER INSERT ON character
BEGIN
    UPDATE character SET creation_time = datetime('now') WHERE character_id = new.character_id;
END;

CREATE TABLE inventory (
    character_id    INT,
    slot_id         INT,
    aug_slot_id     INT     DEFAULT 0,
    stack_amount    INT     DEFAULT 0,
    charges         INT     DEFAULT 65535,
    is_bag          BOOLEAN DEFAULT 0,
    item_id         INT
);

-- Why not PRIMARY KEY (character_id, slot_id, aug_slot_id)?
-- Because the "cursor queue" will expect multiple entries to
-- have slot_id = 30 and aug_slot_id = 0
CREATE INDEX index_inventory_character_id ON inventory (character_id);

CREATE TABLE skill (
    character_id    INT,
    skill_id        INT,
    value           INT,
    
    PRIMARY KEY (character_id, skill_id)
);

CREATE TABLE spellbook (
    character_id    INT,
    slot_id         INT,
    spell_id        INT,
    
    PRIMARY KEY (character_id, slot_id)
);

CREATE TABLE memmed_spells (
    character_id                    INT,
    slot_id                         INT,
    spell_id                        INT,
    recast_timestamp_milliseconds   INT,
    
    PRIMARY KEY (character_id, slot_id)
);

CREATE TABLE bind_point (
    character_id    INT,
    bind_id         INT,
    zone_id         INT,
    x               REAL,
    y               REAL,
    z               REAL,
    heading         REAL,
    
    PRIMARY KEY (character_id, bind_id)
);

CREATE TABLE guild (
    guild_id    INTEGER PRIMARY KEY,
    name        TEXT    UNIQUE
);
