
require "scripts/include"
require "master/master_ipc_cdefs"

local ffi = require "ffi"

local C = ffi.C

local function replyAndFinish(thread, ipc, src, msg)
    C.master_ipc_thread_console_reply(thread, ipc, src, msg)
    C.master_ipc_thread_console_finish(thread, ipc, src)
end

local handlers = {
    start = function(thread, M, ipc, src, args, opts)
        replyAndFinish(thread, ipc, src, "eqp-master started successfully")
    end,
    
    shutdown = function(thread, M, ipc, src, args, opts)
        replyAndFinish(thread, ipc, src, "eqp-master is shutting down")
        return true
    end,
}

local function unknown_cmd(thread, ipc, src, cmd)
    replyAndFinish(thread, ipc, src, string.format("Unknown command: %s", cmd))
end

local function no_cmd(thread, ipc, src)
    replyAndFinish(thread, ipc, src, "No command provided")
end

function handle_console(thread, M, ipc, src, cmd, args, opts)
    if cmd then
        local func = handlers[cmd]
        
        if not func then
            return unknown_cmd(thread, ipc, src, cmd)
        end
        
        return func(thread, M, ipc, src, args, opts)
    end
    
    no_cmd(thread, ipc, src)
end
