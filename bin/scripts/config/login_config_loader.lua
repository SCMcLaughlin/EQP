
local pairs     = pairs
local table     = table
local tostring  = tostring

local loginservers = {}

local env = {
    LoginServer = function(a)
        local server = {}
        
        for k, v in pairs(a) do
            server[tostring(k):lower()] = v
        end
        
        table.insert(loginservers, server)
    end,
}

local config = loadfile("login_config.lua")
setfenv(config, env)
config()

return loginservers
