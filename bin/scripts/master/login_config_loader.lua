
local pairs     = pairs
local table     = table
local tostring  = tostring

local firstLoginServer = {}

local env = {
    LoginServer = function(a)
        for k, v in pairs(a) do
            firstLoginServer[tostring(k):lower()] = v
        end
        
        LoginServer = function() end
    end,
}

local config = loadfile("login_config.lua")
setfenv(config, env)
config()

return firstLoginServer
