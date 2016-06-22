
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C = require "Clock_cdefs"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local tonumber = tonumber
--------------------------------------------------------------------------------

local Clock = {}

Clock.seconds = os.clock

function Clock.milliseconds()
    return tonumber(C.clock_milliseconds())
end

function Clock.milliseconds64()
    return C.clock_milliseconds()
end

function Clock.microseconds()
    return tonumber(C.clock_microseconds())
end

function Clock.microseconds64()
    return C.clock_microseconds()
end

function Clock.unixSeconds()
    return tonumber(C.clock_unix_seconds())
end

function Clock.unixSeconds64()
    return C.clock_unix_seconds()
end

return Clock
