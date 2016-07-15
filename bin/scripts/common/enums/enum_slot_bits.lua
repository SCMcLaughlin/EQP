
local bit       = require "bit"
local slotId    = require "enum_slot_ids"

local function b(n)
    return bit.lshift(1, n - 1)
end

local ear       = b(slotId.Ear1) + b(slotId.Ear2)
local finger    = b(slotId.Fingers1) + b(slotId.Fingers2)
local wrist     = b(slotId.Wrist1) + b(slotId.Wrist2)
local weapon    = b(slotId.Primary) + b(slotId.Secondary)

local slotBits = {
    Ears        = ear,
    Ear         = ear,
    EARS        = ear,
    EAR         = ear,
    Head        = b(slotId.Head),
    HEAD        = b(slotId.Head),
    Face        = b(slotId.Face),
    FACE        = b(slotId.Face),
    Neck        = b(slotId.Neck),
    NECK        = b(slotId.Neck),
    Shoulders   = b(slotId.Shoulders),
    SHOULDERS   = b(slotId.Shoulders),
    Arms        = b(slotId.Arms),
    ARMS        = b(slotId.Arms),
    Back        = b(slotId.Back),
    BACK        = b(slotId.Back),
    Wrists      = wrist,
    Wrist       = wrist,
    WRISTS      = wrist,
    WRIST       = wrist,
    Range       = b(slotId.Range),
    RANGE       = b(slotId.Range),
    Primary     = b(slotId.Primary),
    PRIMARY     = b(slotId.Primary),
    Secondary   = b(slotId.Secondary),
    SECONDARY   = b(slotId.Secondary),
    Weapon      = weapon,
    WEAPON      = weapon,
    Fingers     = finger,
    Finger      = finger,
    FINGERS     = finger,
    FINGER      = finger,
    Chest       = b(slotId.Chest),
    CHEST       = b(slotId.Chest),
    Legs        = b(slotId.Legs),
    Leg         = b(slotId.Legs),
    LEGS        = b(slotId.Legs),
    LEG         = b(slotId.Legs),
    Feet        = b(slotId.Feet),
    FEET        = b(slotId.Feet),
    Waist       = b(slotId.Waist),
    WAIST       = b(slotId.Waist),
    Ammo        = b(slotId.Ammo),
    AMMO        = b(slotId.Ammo),
}

return slotBits
