
local lfs = require "lfs"

local ipairs    = ipairs
local table     = table

local includePaths  = {"src/common/"}
local excludeSource = {["exception.c"] = true}

for path in lfs.dir("src/common/") do
    if path ~= "." and path ~= ".." then
        path = "src/common/" .. path .. "/"
        if lfs.attributes(path, "mode") == "directory" then
            table.insert(includePaths, path)
        end
    end
end

local included  = {}
local pending   = {}

local function getFileContents(name)
    for _, path in ipairs(includePaths) do
        local file = io.open(path .. name, "r")
        if file then
            local str = file:read("*a")
            file:close()
            return str
        end
    end
end

local function includeFile(name)
    if included[name] then return "" end
    
    included[name] = true
    
    if name:find("%.h$") then
        local src = name:sub(1, -2) .. "c"
        
        if not excludeSource[src] then
            table.insert(pending, src)
        end
    end
    
    local str = getFileContents(name)
    
    if str then
        str = str:gsub('#%s*include%s*"([^"]+)"[^\n]*\n', includeFile)
        return str
    end
end

local function buildFile(name, extraInclude)
    table.insert(includePaths, extraInclude)
    
    local file = assert(io.open("amalg/amalg_".. name .. ".c", "w+"))
    
    for filename in lfs.dir(extraInclude) do
        if filename:find("%.c$") then
            local str = includeFile(filename)
            if str then
                file:write(str)
            end
        end
    end
    
    while #pending > 0 do
        local n         = #pending
        local filename  = pending[n]
        pending[n]      = nil
        
        local str = includeFile(filename)
        if str then
            file:write(str)
        end
    end

    file:close()
end

buildFile(arg[1], arg[2])
