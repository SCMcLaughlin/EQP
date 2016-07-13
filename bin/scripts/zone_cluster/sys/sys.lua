
--------------------------------------------------------------------------------
-- sys.lua
--------------------------------------------------------------------------------
--
-- Functions in this package are responsible for creating and managing Lua-side
-- references to C-side objects. It is one of the primary interfaces to the Lua-
-- side from the C-side.
--
-- This script is always run automatically when eqp-zone-cluster starts up.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
require "zone_cluster/zc_include"
require "LuaObject_cdefs"

local ffi       = require "ffi"
local class     = require "class"
local ZC        = require "ZoneCluster"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local C             = ffi.C
local setmetatable  = setmetatable
local setfenv       = setfenv
local xpcall        = xpcall
local traceback     = debug.traceback
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Master Lists
--------------------------------------------------------------------------------
local callbacks = {}
local objects   = {}
local clients   = {}
local npcs      = {}
local timers    = setmetatable({}, {__mode = "v"})
--------------------------------------------------------------------------------

local sys = {}

function sys.callbackGC(index)
    callbacks[index] = nil
end

function sys.objectGC(index)
    local obj = objects[index]
    
    if obj then
        obj._ptr        = nil
        objects[index]  = nil
    end
end

function sys.pushCallback(func)
    local i         = #callbacks + 1
    callbacks[i]    = func
    return i
end

local function pushObj(obj)
    local i     = #objects + 1
    objects[i]  = obj
    return i
end

function sys.createZoneCluster(ptr)
    -- ZoneCluster doesn't get any personal or shared environment; would be 
    -- equivalent to the global environment.
    -- We also don't associate it with a finalizer, because its lifetime is the 
    -- same as the whole lua system's.
    
    local obj = {
        _ptr    = ffi.cast("ZC*", ptr),
        __index = ZC,
    }
    
    setmetatable(obj, obj)
    
    -- The ZoneCluster object is globally available, through require "ZC"
    package.loaded["ZC"] = obj

    -- No index number for this -- no point since we can get it through require() anywhere we need it
end

function sys.createZone(ptr)
    -- Avoid requiring "ZC" before it has been set
    local Zone = require "Zone"
    return pushObj(Zone._wrap(ptr))
end

function sys.createClient(zone, ptr)
    -- Avoid requiring "ZC" before it has been set
    local Client = require "Client"
    return pushObj(Client._wrap(zone, ptr))
end

function sys.createNpc(zone, ptr)
    -- Avoid requiring "ZC" before it has been set
    local Npc   = require "Npc"
    local obj   = Npc._wrap(zone, ptr)
    local index = pushObj(obj)
    
    -- Npcs are always created and loaded from the Lua side, so we need to set their C-side object index from this side as well
    obj:setLuaObjectIndex(index)
    
    -- Trigger the spawn event
    sys.eventCall("event_spawn", zone, obj)
    
    return obj
end

local function doEventCall(from, eventName, zone, obj, ...)
    local func = from[eventName]
    
    if not func then return end
    
    local env = from:getPersonalEnvironment()
    setfenv(func, env)
    
    local s, errOrRet = xpcall(func, traceback, zone, obj, ...)
    
    if not s then
        zone:log(errOrRet)
    else
        return errOrRet
    end
end

function sys.eventCall(eventName, zone, obj, ...)
    --[[
        The zone gets first crack at any event called in this way; if the zone's version
        of the event returns true, it will prevent the object's version of the event
        from being called.
    --]]
    
    if not doEventCall(zone, eventName, zone, obj, ...) then
        doEventCall(obj, eventName, zone, obj, ...)
    end
end

function sys.nextTimerIndex()
    return #timers + 1
end

function sys.createTimer(obj, index)
    timers[index] = obj
end

function sys.getCallback(index)
    return callbacks[index]
end

function sys.getObject(index)
    return objects[index]
end

function sys.getTimer(index)
    return timers[index]
end

package.loaded["sys"] = sys

return sys
