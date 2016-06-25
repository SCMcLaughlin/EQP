
--[=[
local ZC    = require "ZC"
local Clock = require "Clock"
local Timer = require "Timer"

ZC:log("Testing, 1, 2, 3! %s, microseconds: %u", tostring(ZC), Clock.microseconds())

Timer.once{seconds = 3.5, callback = function()
    io.write("Just once~\n")
end}

t = Timer{seconds = 1, callback = function(timer)
    io.write("Timed! ", timer.x, " -- ", Clock.milliseconds(), "\n")
    timer.x = timer.x + 10
    collectgarbage()
end}
t.x = 10
--]=]
