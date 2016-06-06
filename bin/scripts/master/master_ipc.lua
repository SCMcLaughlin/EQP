
local handlers = {
    shutdown = function()
        return true
    end,
}

function handle_console(thread, M, ipc, src, cmd, args, opts)
    local f = io.open("test.txt", "w+")
    f:write(string.format("cmd: %s, args: %i, opts: %i\n", cmd, #args, #opts))
    f:close()
    return true
end
