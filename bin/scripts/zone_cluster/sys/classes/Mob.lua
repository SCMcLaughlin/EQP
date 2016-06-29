
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C         = require "Mob_cdefs"
local ffi       = require "ffi"
local class     = require "class"
local LuaObject = require "LuaObject"
local Enum      = require "Enum"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local getClassNameById      = Enum.getClassNameById
local getRaceNameById       = Enum.getRaceNameById
local getDeityNameById      = Enum.getDeityNameById
local getBodyTypeNameById   = Enum.getBodyTypeNameById
local toLuaString           = ffi.string
local tonumber              = tonumber
--------------------------------------------------------------------------------

local Mob = class("Mob", LuaObject)

function Mob:getName()
    return toLuaString(C.mob_name_cstr(self:ptr()))
end

function Mob:getClientFriendlyName()
    return toLuaString(C.mob_client_friendly_name_cstr(self:ptr()))
end

function Mob:getEntityId()
    return C.mob_entity_id(self:ptr())
end

function Mob:getMobTypeInt()
    -- Derived classes will provide a "getClassName" method, better to use that on the Lua side
    return C.mob_get_type(self:ptr())
end

function Mob:getLevel()
    return C.mob_level(self:ptr())
end

function Mob:getClassId()
    return C.mob_class(self:ptr())
end

function Mob:getClassName()
    return getClassNameById(self:getClassId())
end

function Mob:getBaseRaceId()
    return C.mob_base_race(self:ptr())
end

function Mob:getRaceId()
    return C.mob_current_race(self:ptr())
end

function Mob:getBaseRaceName()
    return getRaceNameById(self:getBaseRace())
end

function Mob:getRaceName()
    return getRaceNameById(self:getRace())
end

function Mob:getBaseGenderId()
    return C.mob_base_gender(self:ptr())
end

function Mob:getGenderId()
    return C.mob_current_gender(self:ptr())
end

function Mob:getFaceId()
    return C.mob_face(self:ptr())
end

function Mob:getDeityId()
    return C.mob_deity(self:ptr())
end

function Mob:getX()
    return C.mob_x(self:ptr())
end

function Mob:getY()
    return C.mob_y(self:ptr())
end

function Mob:getZ()
    return C.mob_z(self:ptr())
end

function Mob:getHeading()
    return C.mob_heading(self:ptr())
end

function Mob:getLoc()
    return self:getX(), self:getY(), self:getZ(), self:getHeading()
end

function Mob:getHpRatio()
    return C.mob_hp_ratio(self:ptr())
end

function Mob:getHp64()
    -- LuaJIT boxes 64bit integers as "CData" values; they can be passed around to other C functions,
    -- but they don't behave like normal Lua numbers. You can do some math on them, but it'll be integer arithmatic
    -- rather than Lua's standard floating point math.
    return C.mob_current_hp(self:ptr())
end

function Mob:getHp()
    -- See the comment for Mob:getHp64(). Converting the value to a Lua number loses some precision
    -- (essentially, the value cap becomes 54 bits instead of 64, assuming the number never becomes decimalized).
    return tonumber(C.mob_current_hp(self:ptr()))
end

function Mob:getMaxHp64()
    return C.mob_max_hp(self:ptr())
end

function Mob:getMaxHp()
    return tonumber(C.mob_max_hp(self:ptr()))
end

function Mob:getMana64()
    return C.mob_current_mana(self:ptr())
end

function Mob:getMana()
    return tonumber(C.mob_current_mana(self:ptr()))
end

function Mob:getMaxMana64()
    return C.mob_max_mana(self:ptr())
end

function Mob:getMaxMana()
    return tonumber(C.mob_max_mana(self:ptr()))
end

function Mob:getEndurance64()
    return C.mob_current_endurance(self:ptr())
end

function Mob:getEndurance()
    return tonumber(C.mob_current_endurance(self:ptr()))
end

function Mob:getMaxEndurance64()
    return C.mob_max_endurance(self:ptr())
end

function Mob:getMaxEndurance()
    return tonumber(C.mob_max_endurance(self:ptr()))
end

function Mob:getStr()
    return C.mob_cur_str(self:ptr())
end

function Mob:getBaseStr()
    return C.mob_base_str(self:ptr())
end

function Mob:setBaseStr(val)
    C.mob_set_base_str(self:ptr(), val)
end

function Mob:getSta()
    return C.mob_cur_sta(self:ptr())
end

function Mob:getBaseSta()
    return C.mob_base_sta(self:ptr())
end

function Mob:setBaseSta(val)
    C.mob_set_base_sta(self:ptr(), val)
end

function Mob:getDex()
    return C.mob_cur_dex(self:ptr())
end

function Mob:getBaseDex()
    return C.mob_base_dex(self:ptr())
end

function Mob:setBaseDex(val)
    C.mob_set_base_dex(self:ptr(), val)
end

function Mob:getAgi()
    return C.mob_cur_agi(self:ptr())
end

function Mob:getBaseAgi()
    return C.mob_base_agi(self:ptr())
end

function Mob:setBaseAgi(val)
    C.mob_set_base_agi(self:ptr(), val)
end

function Mob:getInt()
    return C.mob_cur_int(self:ptr())
end

function Mob:getBaseInt()
    return C.mob_base_int(self:ptr())
end

function Mob:setBaseInt(val)
    C.mob_set_base_int(self:ptr(), val)
end

function Mob:getWis()
    return C.mob_cur_wis(self:ptr())
end

function Mob:getBaseWis()
    return C.mob_base_wis(self:ptr())
end

function Mob:setBaseWis(val)
    C.mob_set_base_wis(self:ptr(), val)
end

function Mob:getCha()
    return C.mob_cur_cha(self:ptr())
end

function Mob:getBaseCha()
    return C.mob_base_cha(self:ptr())
end

function Mob:setBaseCha(val)
    C.mob_set_base_cha(self:ptr(), val)
end

function Mob:getWalkingSpeed()
    return C.mob_current_walking_speed(self:ptr())
end

function Mob:getBaseWalkingSpeed()
    return C.mob_base_walking_speed(self:ptr())
end

function Mob:setBaseWalkingSpeed(val)
    C.mob_set_base_walking_speed(self:ptr(), val)
end

function Mob:getRunningSpeed()
    return C.mob_current_running_speed(self:ptr())
end

function Mob:getBaseRunningSpeed()
    return C.mob_base_running_speed(self:ptr())
end

function Mob:setBaseRunningSpeed(val)
    C.mob_set_base_running_speed(self:ptr(), val)
end

function Mob:getSize()
    return C.mob_current_size(self:ptr())
end

function Mob:getBaseSize()
    return C.mob_base_size(self:ptr())
end

function Mob:getBodyTypeId()
    return C.mob_body_type(self:ptr())
end

function Mob:getBodyTypeName()
    return getBodyTypeNameById(self:getBodyTypeId())
end

function Mob:isInvisible()
    return C.mob_is_invisible(self:ptr()) ~= 0
end

function Mob:isInvisibleVsUndead()
    return C.mob_is_invisible_vs_undead(self:ptr()) ~= 0
end

function Mob:isInvisibleVsAnimals()
    return C.mob_is_invisible_vs_animals(self:ptr())
end

function Mob:isInvisibleTo(target)
    return C.mob_is_invisible_to_mob(self:ptr(), target:ptr())
end

function Mob:getUprightStateId()
    --fixme: add an enum for this?
    return C.mob_upright_state(self:ptr())
end

function Mob:getLightLevel()
    return C.mob_light_level(self:ptr())
end

function Mob:getTextureId()
    return C.mob_texture(self:ptr())
end

function Mob:getHelmTextureId()
    return C.mob_helm_texture(self:ptr())
end

return Mob
