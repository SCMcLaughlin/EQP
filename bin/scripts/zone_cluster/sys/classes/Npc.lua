
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C         = require "Npc_cdefs"
local ffi       = require "ffi"
local sys       = require "sys"
local script    = require "script"
local class     = require "class"
local Mob       = require "Mob"
local ZC        = require "ZC"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local toLuaString   = ffi.string
local format        = string.format
local tostring      = tostring
--------------------------------------------------------------------------------

local Npc = class("Npc", Mob)

local globalScriptEnv                   = script.initEnv(Npc)
local sharedScriptEnvsByZoneShortName   = {}

local function runGlobalScript()
    script.runGlobal(globalScriptEnv, "scripts/zone_cluster/global/global_npc.lua")
    runGlobalScript = nil
end

local function lookUpAndRunScript(envSet, zone, env, zoneShortName, id, name)
    local had
    local str
    local fmt = "scripts/zone_cluster/zones/".. zoneShortName .."/%s.lua"
    
    -- First, search for the specific ad-hoc id
    str = tostring(id)
    had = script.runZoneSpecific(zone, env, format(fmt, str))
    if had then
        envSet[id] = env
        return true
    end
    
    -- Otherwise, search for name + id, e.g. "a bear10023.lua"
    str = name .. id
    had = script.runZoneSpecific(zone, env, format(fmt, str))
    if had then
        envSet[str] = env
        return true
    end
    
    -- Else, search by the name alone
    had = script.runZoneSpecific(zone, env, format(fmt, name))
    if had then
        envSet[name] = env
        return true
    end
    
    -- Fallback so that npcs with no scripts will share an empty script environment
    if not env["__noscript"] then
        envSet["__noscript"] = env
    end
end

function Npc._wrap(zone, ptr)
    if runGlobalScript then runGlobalScript() end
    
    local shortName = zone:getShortName()
    local id        = C.npc_adhoc_id(ptr)
    local name      = toLuaString(C.mob_name_cstr(ptr))
    local envSet    = sharedScriptEnvsByZoneShortName[shortName]
    local sharedEnv
    
    --fixme: should npcs have an extra level of script inheritance? global > zone-wide > name/id within zone?
    
    if envSet then
        sharedEnv = envSet[id] or envSet[name .. id] or envSet[name]
    else
        envSet = {}
        sharedScriptEnvsByZoneShortName[shortName] = envSet
    end
    
    if not sharedEnv then
        sharedEnv = script.initEnv(globalScriptEnv)
        if not lookUpAndRunScript(envSet, zone, sharedEnv, shortName, id, name) then
            sharedEnv = envSet["__noscript"]
        end
    end
    
    return class.wrap(sharedEnv, ffi.cast("Npc*", ptr))
end

-- Lookup optimization
Npc.ptr = Mob.ptr

return Npc
