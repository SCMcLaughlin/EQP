
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C     = require "LuaObject_cdefs"
local class = require "class"
--------------------------------------------------------------------------------

local LuaObject = class("LuaObject")

function LuaObject:updateIndex(index)
    C.zc_lua_object_update_index(self:ptr(), index)
end

return LuaObject
