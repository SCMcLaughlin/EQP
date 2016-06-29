
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C     = require "ZoneCluster_cdefs"
local class = require "class"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local format = string.format
--------------------------------------------------------------------------------

local ZC = class("ZoneCluster")

function ZC:ptr()
    return self._ptr
end

function ZC:log(str, ...)
    C.zc_log(self:ptr(), format(str, ...))
end

function ZC:logFor(sourceId, str, ...)
    C.zc_log_for(self:ptr(), sourceId, format(str, ...))
end

return ZC
