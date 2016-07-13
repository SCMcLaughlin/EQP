
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C     = require "LuaObject_cdefs"
local class = require "class"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local getmetatable  = getmetatable
--------------------------------------------------------------------------------

local LuaObject = class("LuaObject")

function LuaObject:ptr()
    local ptr = self._ptr
    if ptr == nil then
        error "Attempt to use an object that has been destroyed on the C-side"
    end
    return ptr
end

function LuaObject:isValid()
    return self._ptr ~= nil
end

LuaObject.exists = LuaObject.isValid

function LuaObject.getClassName()
    return "LuaObject"
end

function LuaObject:setLuaObjectIndex(index)
    C.zc_lua_object_set_index(self:ptr(), index)
end

function LuaObject:getLuaObjectIndex()
    return C.zc_lua_object_get_index(self:ptr())
end

function LuaObject:getPersonalEnvironment()
    return getmetatable(self)
end

function LuaObject:getSharedEnvironment()
    return getmetatable(getmetatable(self))
end

return LuaObject
