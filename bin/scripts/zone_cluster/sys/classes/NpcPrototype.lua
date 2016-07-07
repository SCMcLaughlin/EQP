
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C         = require "NpcPrototype_cdefs"
local ffi       = require "ffi"
local class     = require "class"
local Enum      = require "Enum"
local ZC        = require "ZC"
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
local floor                 = math.floor
--------------------------------------------------------------------------------

local NpcProto = class("NpcPrototype")

function NpcProto.new()
    local ptr = C.npc_proto_create(ZC:ptr())
    
    ffi.gc(ptr, C.npc_proto_destroy)
    
    return NpcProto:instance{_ptr = ptr}
end

function NpcProto:ptr()
    return self._ptr
end

function NpcProto:setName(name)
    C.npc_proto_set_name(ZC:ptr(), self:ptr(), name, #name)
end

function NpcProto:setId(id)
    C.npc_proto_set_adhoc_id(self:ptr(), id)
end

function NpcProto:setLevel(level)
    C.npc_proto_set_level(self:ptr(), level)
end

function NpcProto:setClassId(class)
    C.npc_proto_set_class(self:ptr(), class)
end

function NpcProto:setRaceId(race)
    C.npc_proto_set_race(self:ptr(), race)
end

function NpcProto:setGenderId(gender)
    C.npc_proto_set_gender(self:ptr(), gender)
end

function NpcProto:setFaceId(face)
    C.npc_proto_set_face(self:ptr(), face)
end

function NpcProto:setBodyTypeId(bodyType)
    C.npc_proto_set_body_type(self:ptr(), bodyType)
end

function NpcProto:setTextureId(tex)
    C.npc_proto_set_texture(self:ptr(), tex)
end

function NpcProto:setHelmTextureId(tex)
    C.npc_proto_set_helm_texture(self:ptr(), tex)
end

function NpcProto:setMaxHp(hp)
    C.npc_proto_set_max_hp(self:ptr(), hp)
end

function NpcProto:setMaxMana(mana)
    C.npc_proto_set_max_mana(self:ptr(), mana)
end

function NpcProto:setMaxEndurance(endur)
    C.npc_proto_set_max_endurance(self:ptr(), endur)
end

function NpcProto:setSize(size)
    C.npc_proto_set_size(self:ptr(), size)
end

function NpcProto:setRunningSpeed(speed)
    C.npc_proto_set_running_speed(self:ptr(), speed)
end

function NpcProto:setWalkingSpeed(speed)
    C.npc_proto_set_walking_speed(self:ptr(), speed)
end

function NpcProto:setAggroRadius(radius)
    C.npc_proto_set_aggro_radius(self:ptr(), radius)
end

function NpcProto:setMaterialId(slot, mat)
    C.npc_proto_set_material(self:ptr(), slot, mat)
end

function NpcProto:setTint(slot, red, green, blue)
    C.npc_proto_set_tint(self:ptr(), slot, red, green, blue)
end

function NpcProto:setPrimaryModelId(model)
    C.npc_proto_set_primary_model_id(self:ptr(), model)
end

function NpcProto:setSecondaryModelId(model)
    C.npc_proto_set_secondary_model_id(self:ptr(), model)
end

function NpcProto:setPrimaryMessageSkill(skill)
    C.npc_proto_set_primary_message_skill(self:ptr(), skill)
end

function NpcProto:setSecondaryMessageSkill(skill)
    C.npc_proto_set_secondary_message_skill(self:ptr(), skill)
end

function NpcProto:setSwingsPerRound(swings)
    C.npc_proto_set_swings_per_round(self:ptr(), swings)
end

function NpcProto:setMeleeDamage(min, max)
    min = min or 1
    max = max or 2
    
    if min > max then
        min, max = max, min
    end
    
    if min == 0 and max > 0 then
        min = max / 2
    end
    
    C.npc_proto_set_melee_damage(self:ptr(), min, max)
end

function NpcProto:setAc(ac)
    C.npc_proto_set_ac(self:ptr(), ac)
end

function NpcProto:setStr(value)
    C.npc_proto_set_str(self:ptr(), value)
end

function NpcProto:setSta(value)
    C.npc_proto_set_sta(self:ptr(), value)
end

function NpcProto:setDex(value)
    C.npc_proto_set_dex(self:ptr(), value)
end

function NpcProto:setAgi(value)
    C.npc_proto_set_agi(self:ptr(), value)
end

function NpcProto:setInt(value)
    C.npc_proto_set_int(self:ptr(), value)
end

function NpcProto:setWis(value)
    C.npc_proto_set_wis(self:ptr(), value)
end

function NpcProto:setCha(value)
    C.npc_proto_set_cha(self:ptr(), value)
end

function NpcProto:setSvMagic(value)
    C.npc_proto_set_sv_magic(self:ptr(), value)
end

function NpcProto:setSvFire(value)
    C.npc_proto_set_sv_fire(self:ptr(), value)
end

function NpcProto:setSvCold(value)
    C.npc_proto_set_sv_cold(self:ptr(), value)
end

function NpcProto:setSvPoison(value)
    C.npc_proto_set_sv_poison(self:ptr(), value)
end

function NpcProto:setSvDisease(value)
    C.npc_proto_set_sv_disease(self:ptr(), value)
end

function NpcProto:getName()
    return toLuaString(C.npc_proto_get_name_cstr(self:ptr())
end

function NpcProto:getId()
    return C.npc_proto_get_adhoc_id(self:ptr())
end

function NpcProto:getLevel()
    return C.npc_proto_get_level(self:ptr())
end

function NpcProto:getClassId()
    return C.npc_proto_get_class(self:ptr())
end

function NpcProto:getRaceId()
    return C.npc_proto_get_race(self:ptr())
end

function NpcProto:getGenderId()
    return C.npc_proto_get_gender(self:ptr())
end

function NpcProto:getFaceId()
    return C.npc_proto_get_face(self:ptr())
end

function NpcProto:getBodyTypeId()
    return C.npc_proto_get_body_type(self:ptr())
end

function NpcProto:getTextureId()
    return C.npc_proto_get_texture(self:ptr())
end

function NpcProto:getHelmTextureId()
    return C.npc_proto_get_helm_texture(self:ptr())
end

function NpcProto:getMaxHp()
    return C.npc_proto_get_max_hp(self:ptr())
end

function NpcProto:getMaxMana()
    return C.npc_proto_get_max_mana(self:ptr())
end

function NpcProto:getMaxEndurance()
    return C.npc_proto_get_max_endurance(self:ptr())
end

function NpcProto:getSize()
    return C.npc_proto_get_size(self:ptr())
end

function NpcProto:getRunningSpeed()
    return C.npc_proto_get_running_speed(self:ptr())
end

function NpcProto:getWalkingSpeed()
    return C.npc_proto_get_walking_speed(self:ptr())
end

function NpcProto:getAggroRadius()
    return C.npc_proto_get_aggro_radius(self:ptr())
end

function NpcProto:getMaterialId(slot)
    return C.npc_proto_get_material(self:ptr(), slot)
end

function NpcProto:getTint(slot)
    return C.npc_proto_get_tint(self:ptr(), slot)
end

function NpcProto:getPrimaryModelId()
    return C.npc_proto_get_primary_model_id(self:ptr())
end

function NpcProto:getSecondaryModelId()
    return C.npc_proto_get_secondary_model_id(self:ptr())
end

function NpcProto:getPrimaryMessageSkill()
    return C.npc_proto_get_primary_message_skill(self:ptr())
end

function NpcProto:getSecondaryMessageSkill()
    return C.npc_proto_get_secondary_message_skill(self:ptr())
end

function NpcProto:getSwingsPerRound()
    return C.npc_proto_get_swings_per_round(self:ptr())
end

function NpcProto:getMinMeleeDamage()
    return C.npc_proto_get_min_damage(self:ptr())
end

function NpcProto:getMaxMeleeDamage()
    return C.npc_proto_get_max_damage(self:ptr())
end

function NpcProto:randomize()

end

function NpcProto:calculateDefaults(lvl)
    lvl = lvl or self:getLevel()
end

return NpcProto