
local ffi = require "ffi"

local lua = {
    "scripts/?.lua",
    "scripts/common/?.lua",
    "scripts/common/enums/?.lua",
    "scripts/common/cdefs/?.lua",
    "scripts/common/classes/?.lua",
}

local dll = {
    "scripts/dll/?",
}

local dllSuffix
if ffi.os == "Windows" then
    dllSuffix = ".dll"
else
    dllSuffix = ".so"
end

for k, v in pairs(dll) do
    dll[k] = v .. dllSuffix
end

package.path    = table.concat(lua, ";") ..";".. package.path
package.cpath   = table.concat(dll, ";") ..";".. package.cpath
