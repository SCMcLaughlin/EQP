
local ItemPrototype = require "ItemPrototype"

ItemPrototype{
    name        = "Example One",
    lore        = "An example item",
    type        = "Armor",
    size        = "Small",
    weight      = 1.5,
    tag         = "[MAGIC][LORE][NO DROP]",
    stat        = "5 STR DEX AGI 10 WIS INT 50 HP 25 Mana",
    slot        = "ALL",
    class       = "WAR PAL SHD RNG ROG BRD MNK CLE",
    race        = "ALL",
    icon        = 500,
    model       = 1,
    material    = 3,
    tint        = 0x9051cc00,
}

function event_spawn(zone, item, owner, isEquipped)

end
