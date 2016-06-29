
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local classNamesById    = require "enum_class_names"
local raceNamesById     = require "enum_race_names"
local deityNamesById    = require "enum_deity_names"
local bodyTypeNamesById = require "enum_body_type_names"
--------------------------------------------------------------------------------

local Enum = {}

function Enum.getClassNameById(id)
    return classNamesById[id] or "Unknown"
end

function Enum.getRaceNameById(id)
    return raceNamesById[id] or "Unknown"
end

function Enum.getDeityNameById(id)
    return deityNamesById[id] or "Unknown"
end

function Enum.getBodyTypeNameById(id)
    return bodyTypeNamesById[id] or "Unknown"
end

return Enum
