
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C     = require "ItemPrototype_cdefs"
local ffi   = require "ffi"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local toLuaString = ffi.string
--------------------------------------------------------------------------------

--[[
    ItemPrototype is a closed class -- no functions can be added or changed after
    this script runs, it has no inheritance structure, and instances cannot have
    "personal" variables.
    
    Any instance of a ItemPrototype* cdata is automatically of this class, no
    wrapping required.
--]]

local ItemPrototype = {}
ItemPrototype.__index = ItemPrototype

--------------------------------------------------------------------------------

function ItemPrototype:setDefaults()
    C.item_proto_set_defaults(self)
end

function ItemPrototype:setName(name)
    C.item_proto_set_name(self, name)
end

function ItemPrototype:setLoreText(lore)
    C.item_proto_set_lore_text(self, lore)
end

function ItemPrototype:setModelId(id)
    C.item_proto_set_model_id(self, id)
end

function ItemPrototype:setIsLore(bool)
    C.item_proto_set_is_lore(self, bool and 1 or 0)
end

function ItemPrototype:setIsDroppable(bool)
    C.item_proto_set_is_droppable(self, bool and 1 or 0)
end

function ItemPrototype:setIsNoDrop(bool)
    C.item_proto_set_is_droppable(self, bool and 0 or 1)
end

ItemPrototype.setIsNoTrade = ItemPrototype.setIsNoDrop

function ItemPrototype:setIsMagic(bool)
    C.item_proto_set_is_magic(self, bool and 1 or 0)
end

function ItemPrototype:setIsPermanent(bool)
    C.item_proto_set_is_permanent(self, bool and 1 or 0)
end

function ItemPrototype:setIsNoRent(bool)
    C.item_proto_set_is_permanent(self, bool and 0 or 1)
end

function ItemPrototype:setIsStackable(bool)
    C.item_proto_set_is_stackable(self, bool and 1 or 0)
end

function ItemPrototype:setSize(val)
    C.item_proto_set_size(self, val)
end

function ItemPrototype:setWeight(val)
    C.item_proto_set_weight(self, val)
end

function ItemPrototype:setItemTypeId(val)
    C.item_proto_set_item_type_id(self, val)
end

function ItemPrototype:setItemSkillId(val)
    C.item_proto_set_item_skill_id(self, val)
end

function ItemPrototype:setMaterial(val)
    C.item_proto_set_material(self, val)
end

function ItemPrototype:setLight(val)
    C.item_proto_set_light(self, val)
end

function ItemPrototype:setEffectTypeId(val)
    C.item_proto_set_effect_type_id(self, val)
end

function ItemPrototype:setMaxCharges(val)
    C.item_proto_set_max_charges(self, val)
end

function ItemPrototype:setStr(val)
    C.item_proto_set_str(self, val)
end

function ItemPrototype:setDex(val)
    C.item_proto_set_dex(self, val)
end

function ItemPrototype:setAgi(val)
    C.item_proto_set_agi(self, val)
end

function ItemPrototype:setSta(val)
    C.item_proto_set_sta(self, val)
end

function ItemPrototype:setInt(val)
    C.item_proto_set_int(self, val)
end

function ItemPrototype:setWis(val)
    C.item_proto_set_wis(self, val)
end

function ItemPrototype:setCha(val)
    C.itme_proto_set_cha(self, val)
end

function ItemPrototype:setSvMagic(val)
    C.item_proto_set_sv_magic(self, val)
end

ItemPrototype.setSvM    = ItemPrototype.setSvMagic
ItemPrototype.setMR     = ItemPrototype.setSvMagic

function ItemPrototype:setSvFire(val)
    C.item_proto_set_sv_fire(self, val)
end

ItemPrototype.setSvF    = ItemPrototype.setSvFire
ItemPrototype.setFR     = ItemPrototype.setSvFire

function ItemPrototype:setSvCold(val)
    C.item_proto_set_sv_cold(self, val)
end

ItemPrototype.setSvC    = ItemPrototype.setSvCold
ItemPrototype.setCR     = ItemPrototype.setSvCold

function ItemPrototype:setSvPoison(val)
    C.item_proto_set_sv_poison(self, val)
end

ItemPrototype.setSvP    = ItemPrototype.setSvPoison
ItemPrototype.setPR     = ItemPrototype.setSvPoison

function ItemPrototype:setSvDisease(val)
    C.item_proto_set_sv_disease(self, val)
end

ItemPrototype.setSvD    = ItemPrototype.setSvDisease
ItemPrototype.setDR     = ItemPrototype.setSvDisease

function ItemPrototype:setHp(val)
    C.item_proto_set_hp(self, val)
end

function ItemPrototype:setMana(val)
    C.item_proto_set_mana(self, val)
end

function ItemPrototype:setAc(val)
    C.item_proto_set_ac(self, val)
end

ItemPrototype.setAC = ItemPrototype.setAc

function ItemPrototype:setDamage(val)
    C.item_proto_set_damage(self, val)
end

function ItemPrototype:setDelay(val)
    C.item_proto_set_delay(self, val)
end

function ItemPrototype:setRange(val)
    C.item_proto_set_range(self, val)
end

function ItemPrototype:setSlotBitfield(val)
    C.item_proto_set_slot_bitfield(self, val)
end

function ItemPrototype:setRaceBitfield(val)
    C.item_proto_set_race_bitfield(self, val)
end

function ItemPrototype:setClassBitfield(val)
    C.item_proto_set_class_bitfield(self, val)
end

function ItemPrototype:setSpellId(val)
    C.item_proto_set_spell_id(self, val)
end

function ItemPrototype:setCastingTime(val)
    C.item_proto_set_casting_time(self, val)
end

function ItemPrototype:setIconId(val)
    C.item_proto_set_icon_id(self, val)
end

function ItemPrototype:setCost(val)
    C.item_proto_set_cost(self, val)
end

function ItemPrototype:setTint(val)
    C.item_proto_set_tint(self, val)
end

--------------------------------------------------------------------------------

function ItemPrototype:getItemId()
    return C.item_proto_get_item_id(self)
end

function ItemPrototype:getScriptPath()
    return toLuaString(C.item_proto_get_script_path(self))
end

function ItemPrototype:getName()
    return toLuaString(C.item_proto_get_name(self))
end

function ItemPrototype:getLoreText()
    return toLuaString(C.item_proto_get_lore_text(self))
end

function ItemPrototype:getModelId()
    return C.item_proto_get_model_id(self)
end

function ItemPrototype:isLore()
    return C.item_proto_is_lore(self) ~= 0
end

function ItemPrototype:isDroppable()
    return C.item_proto_is_droppable(self) ~= 0
end

function ItemPrototype:isNoDrop()
    return not self:isDroppable()
end

function ItemPrototype:isMagic()
    return C.item_proto_is_magic(self) ~= 0
end

function ItemPrototype:isPermanent()
    return C.item_proto_is_permanent(self) ~= 0
end

function ItemPrototype:isNoRent()
    return not self:isPermanent()
end

function ItemPrototype:isStackable()
    return C.item_proto_is_stackable(self) ~= 0
end

function ItemPrototype:getSize()
    return C.item_proto_get_size(self)
end

function ItemPrototype:getWeight()
    return C.item_proto_get_weight(self)
end

function ItemPrototype:getItemTypeId()
    return C.item_proto_get_item_type_id(self)
end

function ItemPrototype:getItemSkillId()
    return C.item_proto_get_item_skill_id(self)
end

function ItemPrototype:getMaterial()
    return C.item_proto_get_material(self)
end

function ItemPrototype:getLight()
    return C.item_proto_get_light(self)
end

function ItemPrototype:getEffectTypeId()
    return C.item_proto_get_effect_type_id(self)
end

function ItemPrototype:getMaxCharges()
    return C.item_proto_get_max_charges(self)
end

function ItemPrototype:getStr()
    return C.item_proto_get_str(self)
end

ItemPrototype.getSTR = ItemPrototype.getStr

function ItemPrototype:getDex()
    return C.item_proto_get_dex(self)
end

ItemPrototype.getDEX = ItemPrototype.getDex

function ItemPrototype:getAgi()
    return C.item_proto_get_agi(self)
end

ItemPrototype.getAGI = ItemPrototype.getAgi

function ItemPrototype:getSta()
    return C.item_proto_get_sta(self)
end

ItemPrototype.getSTA = ItemPrototype.getSta

function ItemPrototype:getInt()
    return C.item_proto_get_int(self)
end

ItemPrototype.getINT = ItemPrototype.getInt

function ItemPrototype:getWis()
    return C.item_proto_get_wis(self)
end

ItemPrototype.getWIS = ItemPrototype.getWis

function ItemPrototype:getCha()
    return C.item_proto_get_cha(self)
end

ItemPrototype.getCHA = ItemPrototype.getCha

function ItemPrototype:getSvMagic()
    return C.item_proto_get_sv_magic(self)
end

ItemPrototype.getSvM    = ItemPrototype.getSvMagic
ItemPrototype.getMR     = ItemPrototype.getSvMagic

function ItemPrototype:getSvFire()
    return C.item_proto_get_sv_fire(self)
end

ItemPrototype.getSvF    = ItemPrototype.getSvFire
ItemPrototype.getFR     = ItemPrototype.getSvFire

function ItemPrototype:getSvCold()
    return C.item_proto_get_sv_cold(self)
end

ItemPrototype.getSvC    = ItemPrototype.getSvCold
ItemPrototype.getCR     = ItemPrototype.getSvCold

function ItemPrototype:getSvPoison()
    return C.item_proto_get_sv_poison(self)
end

ItemPrototype.getSvP    = ItemPrototype.getSvPoison
ItemPrototype.getPR     = ItemPrototype.getSvPoison

function ItemPrototype:getSvDisease()
    return C.item_proto_get_sv_disease(self)
end

ItemPrototype.getSvD    = ItemPrototype.getSvDisease
ItemPrototype.getDR     = ItemPrototype.getSvDisease

function ItemPrototype:getHp()
    return C.item_proto_get_hp(self)
end

ItemPrototype.getHP = ItemPrototype.getHp

function ItemPrototype:getMana()
    return C.item_proto_get_mana(self)
end

function ItemPrototype:getAc()
    return C.item_proto_get_ac(self)
end

ItemPrototype.getAC = ItemPrototype.getAc

function ItemPrototype:getDamage()
    return C.item_proto_get_damage(self)
end

ItemPrototype.getDMG = ItemPrototype.getDamage

function ItemPrototype:getDelay()
    return C.item_proto_get_delay(self)
end

ItemPrototype.getDLY = ItemPrototype.getDelay

function ItemPrototype:getRange()
    return C.item_proto_get_range(self)
end

function ItemPrototype:getSlotBitfield()
    return C.item_proto_get_slot_bitfield(self)
end

function ItemPrototype:getRaceBitfield()
    return C.item_proto_get_race_bitfield(self)
end

function ItemPrototype:getClassBitfield()
    return C.item_proto_get_class_bitfield(self)
end

function ItemPrototype:getSpellId()
    return C.item_proto_get_spell_id(self)
end

function ItemPrototype:getCastingTime()
    return C.item_proto_get_casting_time(self)
end

function ItemPrototype:getIconId()
    return C.item_proto_get_icon_id(self)
end

function ItemPrototype:getCost()
    return C.item_proto_get_cost(self)
end

function ItemPrototype:getTint()
    return C.item_proto_get_tint(self)
end

--------------------------------------------------------------------------------

function ItemPrototype._createItemGen(basic)
    return C.item_proto_create(basic)
end

function ItemPrototype._destroyItemGen(ptr)
    C.item_proto_destroy(ptr)
end

ffi.metatype("ItemPrototype", ItemPrototype)

return ItemPrototype
