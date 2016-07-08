
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local ZC = require "ZC"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local rawset        = rawset
local rawget        = rawget
local xpcall        = xpcall
local loadfile      = loadfile
local setfenv       = setfenv
local setmetatable  = setmetatable
local package       = package
local traceback     = debug.traceback
--------------------------------------------------------------------------------

local script = {}

function script.initEnv(inheritFrom)
    local env = {__index = inheritFrom}
    return setmetatable(env, env)
end

local function resetEnv(env)
    local inherit = rawget(env, "__index")
    for k in pairs(env) do
        env[k] = nil
    end
    rawset(env, "__index", inherit)
end

function script.runGlobal(env, path)
    local script = loadfile(path)
    if script then
        setfenv(script, env)
        local s, err = xpcall(script, traceback)
        if not s then
            ZC:log(err)
            -- Loading a script is all-or-nothing
            resetEnv(env)
        end
    end
end

function script.runZoneSpecific(zone, env, path)
    local script = loadfile(path)
    
    if script then
        package.loaded["zone"] = zone
        setfenv(script, env)
        local s, err = xpcall(script, traceback)
        if not s then
            zone:log(err)
            -- Loading a script is all-or-nothing
            resetEnv(env)
        end
        package.loaded["zone"] = nil
    end
    
    return script
end

return script
