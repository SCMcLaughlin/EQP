
local ffi           = require "ffi"
local lfs           = require "lfs"
local ItemPrototype = require "ItemPrototype"
local handlers      = require "item_gen/field_handlers"

ffi.cdef[[
void item_gen_add(void* gen, ItemPrototype* proto, const char* scriptPath, uint32_t len, uint64_t timestamp);
]]

local C         = ffi.C
local pcall     = pcall
local pairs     = pairs
local loadfile  = loadfile
local itemGen   = _G.itemGen
local proto     = ItemPrototype._createItemGen(itemGen)
local scriptPath
local timestamp

ffi.gc(proto, ItemPrototype._destroyItemGen)

local function readItem(a)
    proto:setDefaults()
    
    for k, v in pairs(a) do
        local func = handlers[k:lower()]
        
        if func then
            func(proto, v)
        end
    end
    
    C.item_gen_add(itemGen, proto, scriptPath, #scriptPath, timestamp)
end

package.loaded.ItemPrototype = readItem

local function recurse(dir)
    for filename in lfs.dir(dir) do
        if filename == "." or filename == ".." then goto skip end
        
        local path = dir .. filename
        local mode = lfs.attributes(path, "mode")
        
        if mode == "directory" then
            recurse(path .. "/")
        elseif mode == "file" then
            local script = loadfile(path)
            if script then
                scriptPath      = path:match("scripts/items/(.-)%.lua$")
                timestamp       = lfs.attributes(path, "modification")
                local s, err    = pcall(script)
                
                if s then
                    io.write(scriptPath, "\n")
                else
                    io.write(err, "\n")
                end
            end
        end
        
        ::skip::
    end
end

recurse("scripts/items/")
