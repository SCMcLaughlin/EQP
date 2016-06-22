
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C     = require "ZoneCluster_cdefs"
local class = require "class"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local string    = string
--------------------------------------------------------------------------------

local ZC = class("ZoneCluster")

function ZC:log(str, ...)
    C.zc_log(self:ptr(), string.format(str, ...))
end

function ZC:logFor(sourceId, str, ...)
    C.zc_log_for(self:ptr(), sourceId, string.format(str, ...))
end

return ZC
