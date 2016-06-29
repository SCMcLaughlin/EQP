
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C             = require "Client_cdefs"
local ffi           = require "ffi"
local class         = require "class"
local Mob           = require "Mob"
local ZC            = require "ZC"
local expansionId   = require "enum_expansion_ids"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local xpcall        = xpcall
local loadfile      = loadfile
local setfenv       = setfenv
local setmetatable  = setmetatable
local package       = package
local traceback     = debug.traceback
local toLuaString   = ffi.string
local format        = string.format
--------------------------------------------------------------------------------

local Client = class("Client", Mob)

local globalScriptEnv = {__index = Client)
setmetatable(globalScriptEnv, globalScriptEnv)
local sharedScriptEnvsByZoneId = {}

local function runGlobalScript()
    -- Run global zone init script
    local script = loadfile("scripts/zone_cluster/global/global_client.lua")
    if script then
        setfenv(script, globalScriptEnv)
        local s, err = xpcall(script, traceback)
        if not s then 
            ZC:log(err)
            -- Loading a script is all-or-nothing
            globalScriptEnv = {__index = Client)
            setmetatable(globalScriptEnv, globalScriptEnv)
        end
    end
    
    runGlobalScript = nil
end

function Client._wrap(zone, ptr)
    if runGlobalScript then runGlobalScript() end
    
    local zoneId    = zone:getZoneId()
    local sharedEnv = sharedScriptEnvsByZoneId[zoneId]
    
    if not sharedEnv then
        sharedEnv = {__index = globalScriptEnv}
        setmetatable(sharedEnv, sharedEnv)
        
        -- Run zone-specific script
        local script = loadfile("scripts/zone_cluster/zones/".. zone:getShortName() .."/client.lua")
        if script then
            package.loaded["zone"] = zone
            setfenv(script, sharedEnv)
            local s, err = xpcall(script, traceback)
            if not s then
                zone:log(err)
                -- Loading a script is all-or-nothing
                sharedEnv = {__index = globalScriptEnv}
                setmetatable(sharedEnv, sharedEnv)
            end
            package.loaded["zone"] = nil
        end
        
        sharedScriptEnvsByZoneId[zoneId] = sharedEnv
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
