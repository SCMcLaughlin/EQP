
local C     = require "char_select/CharCreate_cdefs"
local ffi   = require "ffi"

local CharCreate = {}
CharCreate.__index = CharCreate

function CharCreate:getRaceId()
    return C.char_create_lua_race(self)
end

function CharCreate:getClassId()
    return C.char_create_lua_class(self)
end

function CharCreate:getGenderId()
    return C.char_create_lua_gender(self)
end

function CharCreate:getDeityId()
    return C.char_create_lua_deity(self)
end

function CharCreate:isTrilogy()
    return C.char_create_lua_is_trilogy(self) ~= 0
end

function CharCreate:getStr()
    return C.char_create_lua_str(self)
end

CharCreate.getSTR = CharCreate.getStr

function CharCreate:getDex()
    return C.char_create_lua_dex(self)
end

CharCreate.getDEX = CharCreate.getDex

function CharCreate:getAgi()
    return C.char_create_lua_agi(self)
end

CharCreate.getAGI = CharCreate.getAgi

function CharCreate:getSta()
    return C.char_create_lua_sta(self)
end

CharCreate.getSTA = CharCreate.getSta

function CharCreate:getInt()
    return C.char_create_lua_int(self)
end

CharCreate.getINT = CharCreate.getInt

function CharCreate:getWis()
    return C.char_create_lua_wis(self)
end

CharCreate.getWIS = CharCreate.getWis

function CharCreate:getCha()
    return C.char_create_lua_cha(self)
end

CharCreate.getCHA = CharCreate.getCha

--------------------------------------------------------------------------------

function CharCreate:setStartingZone(shortName, x, y, z, heading)
    local zoneId = C.char_create_lua_get_zone_id(self, shortName)
    
    if zoneId ~= 0 then
        C.char_create_lua_set_starting_zone(self, zoneId, x or 0, y or 0, z or 0, heading or 0)
    end
end

function CharCreate:setBindPoint(shortName, x, y, z, heading, index)
    local zoneId = C.char_create_lua_get_zone_id(self, shortName)
    
    if zoneId ~= 0 then
        C.char_create_lua_set_bind_point(self, zoneId, x or 0, y or 0, z or 0, heading or 0, index or 0)
    end
end

local function addItem(self, item, slotId, stackAmt, charges)
    local itemId = item:getItemId()
    C.char_create_lua_add_starting_item(self, slotId, itemId, stackAmt or 0, charges or 0xffff)
end

function CharCreate:addStartingItem(item, slotId)
    addItem(self, item, slotId)
end

function CharCreate:addStartingItemStack(item, slotId, amt)
    addItem(self, item, slotId, amt)
end

function CharCreate:addStartingItemCharges(item, slotId, amt)
    addItem(self, item, slotId, 0, amt)
end

function CharCreate:setRaceId(id)
    C.char_create_lua_set_race(self, id)
end

function CharCreate:setClassId(id)
    C.char_create_lua_set_class(self, id)
end

function CharCreate:setGenderId(id)
    C.char_create_lua_set_gender(self, id)
end

function CharCreate:setStr(val)
    C.char_create_lua_set_str(self, val)
end

CharCreate.setSTR = CharCreate.setStr

function CharCreate:setDex(val)
    C.char_create_lua_set_dex(self, val)
end

CharCreate.setDEX = CharCreate.setDex

function CharCreate:setAgi(val)
    C.char_create_lua_set_agi(self, val)
end

CharCreate.setAGI = CharCreate.setAgi

function CharCreate:setSta(val)
    C.char_create_lua_set_sta(self, val)
end

CharCreate.setSTA = CharCreate.setSta

function CharCreate:setInt(val)
    C.char_create_lua_set_int(self, val)
end

CharCreate.setINT = CharCreate.setInt

function CharCreate:setWis(val)
    C.char_create_lua_set_wis(self, val)
end

CharCreate.setWIS = CharCreate.setWis

function CharCreate:setCha(val)
    C.char_create_lua_set_cha(self, val)
end

CharCreate.setCHA = CharCreate.setCha

--------------------------------------------------------------------------------

ffi.metatype("CharCreate", CharCreate)
