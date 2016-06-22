
local ZC    = require "ZC"
local Clock = require "Clock"
local Timer = require "Timer"

ZC:log("Testing, 1, 2, 3! %s, microseconds: %u", tostring(ZC), Clock.microseconds())

t = Timer{seconds = 1, callback = function(timer) io.write("Timed! ", tostring(timer), " -- ", Clock.milliseconds(), "\n") end}
z = Timer.once{seconds = 3.5, callback = function() io.write("Just once~\n") end}
