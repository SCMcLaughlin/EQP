
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

return Zone