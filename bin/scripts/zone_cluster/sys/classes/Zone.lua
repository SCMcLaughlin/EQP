
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C         = require "Zone_cdefs"
local ffi       = require "ffi"
local class     = require "class"
local LuaObject = require "LuaObject"
local ZC        = require "ZC"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local xpcall        = xpcall
local loadfile      = loadfile
local setfenv       = setfenv
local setmetatable  = setmetatable
local package       = package
local toLuaString   = ffi.string
local traceback     = debug.traceback
--------------------------------------------------------------------------------

local Zone = class("Zone", LuaObject)

local globalScriptEnv = {__index = Zone}
setmetatable(globalScriptEnv, globalScriptEnv)

local function runGlobalScript()
    -- Run global zone init script
    local script = loadfile("scripts/zone_cluster/global/global_zone.lua")
    if script then
        setfenv(script, globalScriptEnv)
        local s, err = xpcall(script, traceback)
        if not s then
            ZC:log(err)
            -- Loading a script is all-or-nothing
            globalScriptEnv = {__index = Zone}
            setmetatable(globalScriptEnv, globalScriptEnv)
        end
    end
    
    runGlobalScript = nil
end

function Zone._wrap(ptr)
    if runGlobalScript then runGlobalScript() end

    local zone  = class.wrap(globalScriptEnv, ffi.cast("Zone*", ptr))
    local env   = zone:getPersonalEnvironment()
    local name  = zone:getShortName()
    
    -- Run zone-specific init script
    local script = loadfile("scripts/zone_cluster/zones/".. name .."/zone.lua")
    if script then
        package.loaded["zone"] = zone
        setfenv(script, env)
        local s, err = xpcall(script, traceback)
        if not s then
            zone:log(err)
            -- Loading a script is all-or-nothing
            for k in pairs(env) do
                env[k] = nil
            end
        end
        package.loaded["zone"] = nil
    end
    
    return zone
end

-- Lookup optimization
Zone.ptr = LuaObject.ptr

function Zone.getClassName()
    return "Zone"
end

function Zone:log(str, ...)
    ZC:logFor(self:getSourceId(), str, ...)
end

function Zone:spawnNpc(proto, x, y, z, heading)
    local npc = C.zone_spawn_npc(ZC:ptr(), self:ptr(), proto:ptr(), x or 0, y or 0, z or 0, heading or 0)
    --fixme: wrap the npc, run scripts, etc...
    return npc
end

function Zone:getSourceId()
    return C.zone_get_source_id(self:ptr())
end

function Zone:getZoneId()
    return C.zone_get_zone_id(self:ptr())
end

function Zone:getInstanceId()
    return C.zone_get_instance_id(self:ptr())
end

function Zone:getShortName()
    return toLuaString(C.zone_get_short_name(self:ptr()))
end

function Zone:getLongName()
    return toLuaString(C.zone_get_long_name(self:ptr()))
end

function Zone:getZoneTypeId()
    return C.zone_get_zone_type_id(self:ptr())
end

function Zone:getSkyId()
    return C.zone_get_sky_id(self:ptr())
end

function Zone:getGravity()
    return C.zone_get_gravity(self:ptr())
end

function Zone:getMinClippingDistance()
    return C.zone_get_min_clipping_distance(self:ptr())
end

function Zone:getMaxClippingDistance()
    return C.zone_get_max_clipping_distance(self:ptr())
end

function Zone:getSafeSpot()
    return C.zone_get_safe_spot_x(self:ptr()), C.zone_get_safe_spot_y(self:ptr()), C.zone_get_safe_spot_z(self:ptr()), C.zone_get_safe_spot_heading(self:ptr())
end

function Zone:setZoneTypeId(id)
    C.zone_set_zone_type_id(self:ptr(), id)
end

function Zone:setSkyId(id)
    C.zone_set_sky_id(self:ptr(), id)
end

function Zone:setGravity(value)
    C.zone_set_gravity(self:ptr(), value)
end

function Zone:setMinClippingDistance(value)
    C.zone_set_min_clipping_distance(self:ptr(), value)
end

function Zone:setMaxClippingDistance(value)
    C.zone_set_max_clipping_distance(self:ptr(), value)
end

function Zone:setSafeSpot(x, y, z, heading)
    C.zone_set_safe_Spot(self:ptr(), x, y, z, heading)
end

function Zone:areLocsInLineOfSight(x1, y1, z1, x2, y2, z2)
    return C.zone_are_points_in_line_of_sight(ZC:ptr(), self:ptr(), x1, y1, z1, x2, y2, z2) ~= 0
end

function Zone:getBestZForLoc(x, y, z)
    return C.zone_get_best_z_for_loc(self:ptr(), x, y, z)
end

return Zone