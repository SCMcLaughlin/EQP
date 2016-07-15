
local itemTypeIds   = require "enum_item_type_ids"
local classIds      = require "enum_class_ids"
local raceBits      = require "enum_race_bits"
local slotBits      = require "enum_slot_bits"
local ItemPrototype = require "ItemPrototype"
local bit           = require "bit"

local tonumber = tonumber

local sizes = {
    tiny    = 0,
    small   = 1,
    medium  = 2,
    large   = 3,
    giant   = 4,
}

local function loretext(proto, str)
    proto:setLoreText(str)
end

local function tag(proto, str)
    for tag in str:gmatch("%w+") do
        tag = tag:lower()
        
        if tag == "magic" then
            proto:setIsMagic(true)
        elseif tag == "lore" then
            proto:setIsLore(true)
        elseif tag == "nodrop" or tag == "notrade" then
            proto:setIsDroppable(false)
        elseif tag == "norent" then
            proto:setIsPermanent(false)
        end
    end
end

local stats = {
    str         = ItemPrototype.setStr,
    dex         = ItemPrototype.setDex,
    agi         = ItemPrototype.setAgi,
    sta         = ItemPrototype.setSta,
    int         = ItemPrototype.setInt,
    wis         = ItemPrototype.setWis,
    cha         = ItemPrototype.setCha,
    hp          = ItemPrototype.setHp,
    mana        = ItemPrototype.setMana,
    svm         = ItemPrototype.setSvMagic,
    svmagic     = ItemPrototype.setSvMagic,
    mr          = ItemPrototype.setSvMagic,
    svf         = ItemPrototype.setSvFire,
    svfire      = ItemPrototype.setSvFire,
    fr          = ItemPrototype.setSvFire,
    svc         = ItemPrototype.setSvCold,
    svcold      = ItemPrototype.setSvCold,
    cr          = ItemPrototype.setSvCold,
    svp         = ItemPrototype.setSvPoison,
    svpoison    = ItemPrototype.setSvPoison,
    pr          = ItemPrototype.setSvPoison,
    svd         = ItemPrototype.setSvDisease,
    svdisease   = ItemPrototype.setSvDisease,
    dr          = ItemPrototype.setSvDisease,
    ac          = ItemPrototype.setAc,
    damage      = ItemPrototype.setDamage,
    dmg         = ItemPrototype.setDamage,
    delay       = ItemPrototype.setDelay,
    dly         = ItemPrototype.setDelay,
    range       = ItemPrototype.setRange,
}

local function stat(proto, str)
    local val = 0
    
    for s in str:gmatch("%-?%w+") do
        local v = tonumber(s)
        
        if v then
            val = v
        else
            print(s, val)
            local func = stats[s:lower()]
            if func then
                func(proto, val)
            end
        end
    end
end

local function slot(proto, str)
    if str == "ALL" then
        return proto:setSlotBitfield(bit.lshift(1, 21) - 1)
    end
    
    if str == "NONE" then
        return proto:setSlotBitfield(0)
    end
    
    local val = 0
    
    for s in str:gmatch("%w+") do
        local v = slotBits[s]
        
        if v then
            val = bit.bor(val, v)
        end
    end
    
    proto:setSlotBitfield(val)
end

local function class(proto, str)
    if str == "ALL" then
        return proto:setClassBitfield(0xffff)
    end
    
    if str == "NONE" then
        return proto:setClassBitfield(0)
    end
    
    local val = 0
    
    for c in str:gmatch("%w+") do
        local v = classIds[c]
        
        if v then
            val = bit.bor(val, bit.lshift(1, v - 1))
        end
    end
    
    proto:setClassBitfield(val)
end

local function race(proto, str)
    if str == "ALL" then
        return proto:setRaceBitfield(0xffff)
    end
    
    if str == "NONE" then
        return proto:setRaceBitfield(0)
    end
    
    local val = 0
    
    for r in str:gmatch("%w+") do
        local v = raceBits[r]
        
        if v then
            val = bit.bor(val, bit.lshift(1, v - 1))
        end
    end
    
    proto:setRaceBitfield(val)
end

local handlers = {
    name = function(proto, str)
        proto:setName(str)
    end,
    
    lore        = loretext,
    loretext    = loretext,
    
    type = function(proto, str)
        local v = tonumber(str) or itemTypeIds[str:lower()]
        
        if v then
            proto:setItemTypeId(v)
        end
    end,
    
    size = function(proto, str)
        local v = sizes[str:lower()]
        
        if v then
            proto:setSize(v)
        end
    end,
    
    weight = function(proto, n)
        proto:setWeight(n * 10)
    end,
    
    tag     = tag,
    tags    = tag,
    stat    = stat,
    stats   = stat,
    damage  = ItemPrototype.setDamage,
    dmg     = ItemPrototype.setDamage,
    delay   = ItemPrototype.setDelay,
    dly     = ItemPrototype.setDelay,
    range   = ItemPrototype.setRange,
    
    slot    = slot,
    slots   = slot,
    class   = class,
    classes = class,
    race    = race,
    races   = race,
    
    icon    = ItemPrototype.setIconId,
    iconid  = ItemPrototype.setIconId,
    
    model = function(proto, str)
        local id = tonumber(str) or str:match("IT(%d+)")
        
        if id then
            proto:setModelId(id)
        end
    end,
    
    material = ItemPrototype.setMaterial,
    
    tint = function(proto, str)
        local n = tonumber(str)
        
        if n then
            proto:setTint(n)
        end
    end,
}

return handlers
