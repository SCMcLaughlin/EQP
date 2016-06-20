
-- Enable WAL mode
PRAGMA journal_mode = WAL;
    
CREATE TABLE login (
    username    TEXT PRIMARY KEY,
    password    BLOB,
    salt        BLOB
);

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
-- The material fields look like 'how not to design a database 101',
-- but we really have no reason to split them into their own table... 
-- only used for char select, anyway.
CREATE TABLE character (
    character_id    INTEGER PRIMARY KEY,
    fk_account_id   INT,
    name            TEXT,
    level           INT     DEFAULT 1,
    class           INT     DEFAULT 1,
    race            INT     DEFAULT 1,
    zone_id         INT     DEFAULT 1,
    gender          INT     DEFAULT 0,
    face            INT     DEFAULT 0,
    x               REAL    DEFAULT 0,
    y               REAL    DEFAULT 0,
    z               REAL    DEFAULT 0,
    material0       INT     DEFAULT 0,
    material1       INT     DEFAULT 0,
    material2       INT     DEFAULT 0,
    material3       INT     DEFAULT 0,
    material4       INT     DEFAULT 0,
    material5       INT     DEFAULT 0,
    material6       INT     DEFAULT 0,
    material7       INT     DEFAULT 0,
    material8       INT     DEFAULT 0,
    tint0           INT     DEFAULT 0,
    tint1           INT     DEFAULT 0,
    tint2           INT     DEFAULT 0,
    tint3           INT     DEFAULT 0,
    tint4           INT     DEFAULT 0,
    tint5           INT     DEFAULT 0,
    tint6           INT     DEFAULT 0
);

CREATE INDEX index_character_account_id ON character (fk_account_id);
CREATE INDEX index_character_name ON character (name);

CREATE TABLE inventory (
    character_id    INT,
    slot_id         INT,
    item_id         INT,
    stack_amount    INT DEFAULT 1,
    charges         INT DEFAULT 0,
    
    PRIMARY KEY(character_id, slot_id)
);

--SELECT inv.slot_id, inv.item_id, aug.aug_slot, aug.aug_item_id FROM inventory inv
--JOIN inventory_augment aug ON aug.character_id = inv.character_id AND aug.inventory_slot_id = inv.slot_id
--WHERE inv.character_id = ?
--ORDER BY inv.slot_id
CREATE TABLE inventory_augment (
    character_id        INT,
    inventory_slot_id   INT,
    aug_slot            INT,
    aug_item_id         INT,
    
    PRIMARY KEY(character_id, inventory_slot_id, aug_slot)
);
