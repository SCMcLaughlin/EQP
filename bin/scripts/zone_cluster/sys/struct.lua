
local ffi = require "ffi"

local string        = string
local setmetatable  = setmetatable

local struct = {}
struct.__index = struct
struct.__call = function(t, ...) return t.Type(...) end

local function create(type, class)
    local s = {
        Type    = type,
        Ptr     = ffi.typeof("$*", type),
        Array   = ffi.typeof("$[?]", type),
        Arg     = ffi.typeof("$[1]", type),
    }

    if class then ffi.metatype(type, class) end

    return setmetatable(s, struct)
end

function struct.new(contents, class)
    -- anonymous struct, does not pollute the (immutable) global C namespace;
    -- can be replaced at runtime, but cannot be a member of another struct
    local str = string.format("struct { %s }", contents)
    return create(ffi.typeof(str), class)
end

function struct.packed(contents, class)
    ffi.cdef("#pragma pack(1)")
    local ret = struct(contents, class)
    ffi.cdef("#pragma pack()")
    return ret
end

function struct.named(name, contents, class)
    local str = string.format("typedef struct %s { %s } %s;", name, contents, name)
    ffi.cdef(str)
    return create(ffi.typeof(name), class)
end

function struct.namedPacked(name, contents, class)
    ffi.cdef("#pragma pack(1)")
    local ret = struct.named(name, contents, class)
    ffi.cdef("#pragma pack()")
    return ret
end

function struct.primitive(typename)
    return create(ffi.typeof(typename))
end

function struct:hasField(name)
    return ffi.offsetof(self.Type, name) ~= nil
end

function struct:sizeof()
    return ffi.sizeof(self.Type)
end

function struct:cast(ptr)
    return ffi.cast(self.Ptr, ptr)
end

setmetatable(struct, {__call = function(t, ...) return struct.new(...) end})

return struct
