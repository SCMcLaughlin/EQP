
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

local ffi   = require "ffi"
local class = require "class"
local ZC    = require "ZoneCluster"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local C             = ffi.C
local setmetatable  = setmetatable
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Master Lists
--------------------------------------------------------------------------------
local callbacks = {}
local objects   = {}
local clients   = {}
local npcs      = {}
--------------------------------------------------------------------------------

local sys = {}

function sys.callbackGC(index)
    callbacks[index] = nil
end

function sys.objectGC(ptr)
    -- GCing plain pointers created by ffi.cast() works
    local index     = C.zc_lua_object_get_index(ptr)
    objects[index]  = nil
end

function sys.objectGCByIndex(index)
    objects[index] = nil
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
    return 0
end

function sys.createClient(ptr)
    return 0
end

function sys.createNPC(ptr)
    return 0
end

function sys.createTimer(ptr, Timer)
    local obj = class.wrap(Timer, ptr)
    
    C.zc_lua_object_update_index(ptr, pushObj(obj))
    
    return obj
end

function sys.getCallback(index)
    return callbacks[index]
end

function sys.getObject(index)
    return objects[index]
end

package.loaded["sys"] = sys

return sys
