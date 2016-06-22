
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

local ffi   = require "ffi"
local ZC    = require "ZoneCluster"
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

local function objectGC(ptr)
    -- GCing plain pointers created by ffi.cast() works
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

function sys.createTimer(func, period)

end

function sys.getCallback(index)
    return callbacks[index]
end

function sys.getObject(index)
    return objects[index]
end

package.loaded["sys"] = sys

return sys
