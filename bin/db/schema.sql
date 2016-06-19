
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

CREATE TABLE account (
    name_id_pair                INT     PRIMARY KEY,
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
    UPDATE account SET creation_time = datetime('now') WHERE name_id_pair = new.name_id_pair;
END;

-- Use rowid as implicit autoincrement character id and PRIMARY KEY
CREATE TABLE character (
    name    TEXT
);

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
