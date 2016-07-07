
local Timer     = require "Timer"
local NpcProto  = require "NpcPrototype"

local proto = NpcProto()
proto:setName("test")

function event_spawn(zone, client)
    Timer.once{
        seconds = 5,
        callback = function()
            zone:spawnNpc(proto, 0, 0, 0, 127)
            client:sendMessage(0, "Testing, 1, 2, 3...")
        end,
    }
end

local ignore = {
    [0x20f5] = true,
    [0x20f3] = true,
}

function event_unhandled_packet(zone, client, packet)
    local op = packet.opcode
    
    if ignore[op] then return end
    
    if op == 0x215f then
        local i = 0
        Timer.times{
            n           = 10,
            seconds     = 1,
            callback    = function()
                i = i + 1
                client:sendMessage(0, "%i...", i)
            end,
        }
    end
    
    client:sendMessage(0, "Opcode %04x length %i", op, #packet.data)
end
