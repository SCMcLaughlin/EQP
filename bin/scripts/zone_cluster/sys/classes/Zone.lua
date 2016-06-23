
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
local loadfile      = loadfile
local setfenv       = setfenv
local setmetatable  = setmetatable
local package       = package
--------------------------------------------------------------------------------

local Zone = class("Zone", LuaObject)

local globalScriptEnv

local function runGlobalScript()
    globalScriptEnv = {
        __index = Zone,
    }
    
    setmetatable(globalScriptEnv, globalScriptEnv)
    
    -- Run global zone init script
    local script = loadfile("scripts/zone_cluster/init/global_zone_init.lua")
    if script then
        setfenv(script, globalScriptEnv)
        script()
    end
    
    runGlobalScript = nil
end

function Zone._wrap(ptr)
    if not globalScriptEnv then runGlobalScript() end
    
    local zone  = class.wrap(globalScriptEnv, ffi.cast("Zone*", ptr))
    local env   = zone:getPersonalEnvironment()
    local name  = zone:getShortName()
    
    -- Run zone-specific init script
    local script = loadfile("scripts/zone_cluster/zones/".. name .."/zone_init.lua")
    if script then
        package.loaded["zone"] = zone
        setfenv(script, env)
        script()
        package.loaded["zone"] = nil
    end
    
    return zone
end

function Zone:log(str, ...)
    ZC:logFor(self:getSourceId(), str, ...)
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
    return ffi.string(C.zone_get_short_name(self:ptr()))
end

function Zone:getLongName()
    return ffi.string(C.zone_get_long_name(self:ptr()))
end

return Zone