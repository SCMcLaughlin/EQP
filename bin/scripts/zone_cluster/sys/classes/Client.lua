
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C             = require "Client_cdefs"
local ffi           = require "ffi"
local script        = require "script"
local class         = require "class"
local Mob           = require "Mob"
local ZC            = require "ZC"
local expansionId   = require "enum_expansion_ids"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local toLuaString   = ffi.string
local format        = string.format
--------------------------------------------------------------------------------

local Client = class("Client", Mob)

local globalScriptEnv           = script.initEnv(Client)
local sharedScriptEnvsByZone    = {}

local function runGlobalScript()
    script.runGlobal(globalScriptEnv, "scripts/zone_cluster/global/global_client.lua")
    runGlobalScript = nil
end

function Client._wrap(zone, ptr)
    if runGlobalScript then runGlobalScript() end
    
    local sharedEnv = sharedScriptEnvsByZone[zone]
    
    if not sharedEnv then
        sharedEnv = script.initEnv(globalScriptEnv)
        script.runZoneSpecific(zone, sharedEnv, "scripts/zone_cluster/zones/".. zone:getShortName() .."/client.lua")
        sharedScriptEnvsByZone[zone] = sharedEnv
    end
    
    return class.wrap(sharedEnv, ffi.cast("Client*", ptr))
end

-- Lookup optimization
Client.ptr = Mob.ptr

function Client.getClassName()
    return "Client"
end

function Client:getExpansionId()
    return C.client_expansion(self:ptr())
end

function Client:isTrilogy()
    return self:getExpansionId() == expansionId.Trilogy
end

function Client:isPvP()
    return C.client_is_pvp(self:ptr()) ~= 0
end

function Client:isGM()
    return C.client_is_gm(self:ptr()) ~= 0
end

function Client:isAfk()
    return C.client_is_afk(self:ptr()) ~= 0
end

function Client:isLinkead()
    return C.client_is_linkdead(self:ptr()) ~= 0
end

function Client:getGuildRankId()
    return C.client_guild_rank(self:ptr())
end

function Client:getSurname()
    return toLuaString(C.client_surname_cstr(self:ptr()))
end

function Client:sendMessage(channel, str, ...)
    str = format(str, ...)
    C.client_send_custom_message(self:ptr(), channel, str, #str)
end

return Client
