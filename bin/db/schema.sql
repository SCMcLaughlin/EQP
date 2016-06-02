
-- Enable WAL mode
PRAGMA journal_mode = WAL;
    
-- Only used by the minimal, localhost-only login server
CREATE TABLE local_login (
    username    TEXT PRIMARY KEY,
    password    BLOB,
    salt        BLOB
);

CREATE TABLE account (
    login_server_id             INT PRIMARY KEY,
    name                        TEXT,
    most_recent_character_name  TEXT,
    most_recent_ip_address      TEXT,
    status                      INT DEFAULT 0,
    gm_speed                    BOOLEAN DEFAULT 0,
    gm_hide                     BOOLEAN DEFAULT 0,
    shared_platinum             INT DEFAULT 0,
    suspended_until             INT DEFAULT 0,
    creation_time               DATE DEFAULT 0
);

CREATE TRIGGER trigger_account_creation_time AFTER INSERT ON account
BEGIN
    UPDATE account SET creation_time = datetime('now') WHERE login_server_id = new.login_server_id;
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
