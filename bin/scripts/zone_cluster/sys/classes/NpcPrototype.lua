
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C         = require "NpcPrototype_cdefs"
local ffi       = require "ffi"
local class     = require "class"
local Enum      = require "Enum"
local ZC        = require "ZC"
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Caches
--------------------------------------------------------------------------------
local getClassNameById      = Enum.getClassNameById
local getRaceNameById       = Enum.getRaceNameById
local getDeityNameById      = Enum.getDeityNameById
local getBodyTypeNameById   = Enum.getBodyTypeNameById
local toLuaString           = ffi.string
local tonumber              = tonumber
--------------------------------------------------------------------------------

local NpcProto = class("NpcPrototype")

function NpcProto.new()
    local ptr = C.npc_proto_create(ZC:ptr())
    
    ffi.gc(ptr, C.npc_proto_destroy)
    
    return NpcProto:instance{_ptr = ptr}
end

function NpcProto:ptr()
    return self._ptr
end

function NpcProto:setName(name)
    C.npc_proto_set_name(ZC:ptr(), self:ptr(), name, #name)
end

function NpcProto:setId(id)
    C.npc_proto_set_adhoc_id(self:ptr(), id)
end

return NpcProto