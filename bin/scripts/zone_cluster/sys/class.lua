
local util = require "util"

local setmetatable = setmetatable

local function call(t, ...)
    return t.new(...)
end

local class = {__call = call}
setmetatable(class, class)

--local mt = {__call = call} -- Needed to avoid class -> class index lookup loop
--mt.__index = mt
--local class = setmetatable({__call = call}, mt)
--class.__index = class

function class.new(name, super)
    local is    = "is" .. name
    class[is]   = util.falseFunc

    local newclass = {
        __call  = call,
        [is]    = util.trueFunc,
    }

    newclass.__index    = super or class
    class[name]         = newclass
    
    return setmetatable(newclass, newclass)
end

-- Simple, pure-Lua classes use this to turn a table into an instance of a class
function class.instance(class, tbl)
    tbl = tbl or {}
    tbl.__index = class
    return setmetatable(tbl, tbl)
end

-- Complex, C-based classes use this instead
-- Inheritance goes like this: [obj] -> personal environment -> shared environment -> specific class -> class (as defined in this script)
-- Any writes made to fields on the object are stored in the personal environment
-- The shared environment is mainly for shared scripts, e.g. all NPCs of a particular type use the same events
function class.wrap(sharedEnv, ptr)
    -- For sanity's sake remember:
    -- each object/env is its own metatable, and its __index points to the next env in the hierarchy
    
    local personalEnv   = {}
    personalEnv.__index = sharedEnv
    
    setmetatable(personalEnv, personalEnv)
    
    local obj = {
        _ptr        = ptr,
        __index     = personalEnv,
        __newindex  = personalEnv,
    }
    
    return setmetatable(obj, obj)
end

function class:ptr()
    local ptr = self._ptr
    if ptr == nil then
        error "Attempt to use an object that has been destroyed on the C-side"
    end
    return ptr
end

function class:isValid()
    local ptr = self._ptr
    return ptr ~= nil
end

class.exists = class.isValid

return class
