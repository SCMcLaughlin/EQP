
local Timer = require "Timer"

function event_spawn(zone, client)
    Timer.once{
        seconds = 5,
        callback = function()
            client:sendMessage(0, "Testing, 1, 2, 3...")
        end,
    }
end
