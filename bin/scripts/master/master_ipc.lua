
require "scripts/include"
require "master/master_ipc_cdefs"

local ffi = require "ffi"

local C = ffi.C

local function replyAndFinish(console, msg)
    C.console_reply(console, msg, #msg)
    C.console_finish(console)
end

local handlers = {
    start = function(console, args, opts)
        replyAndFinish(console, "eqp-master started successfully")
    end,
    
    shutdown = function(console, args, opts)
        replyAndFinish(console, "eqp-master is shutting down")
        return true
    end,
}

local function unknown_cmd(console, cmd)
    replyAndFinish(console, string.format("Unknown command: %s", cmd))
end

local function no_cmd(console)
    replyAndFinish(console, "No command provided")
end

function handle_console(console, cmd, args, opts)
    if cmd then
        local func = handlers[cmd]
        
        if not func then
            return unknown_cmd(console, cmd)
        end
        
        return func(console, args, opts)
    end
    
    no_cmd(console)
end
