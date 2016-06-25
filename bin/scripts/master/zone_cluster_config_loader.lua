
require "scripts/include"
local C = require "master/zone_cluster_config_cdefs"

local pairs     = pairs
local ipairs    = ipairs
local table     = table
local tostring  = tostring
local type      = type
local rawset    = rawset

local function key(k)
    return tostring(k):lower()
end

local function value(v)
    local t = type(v)
    
    if t == "string" then
        return v:lower()
    end
    
    return v
end

return function(mgr)
    local clusters = {}
    
    local env = {
        Cluster = function(a)
            local cluster = {}
            
            for k, v in pairs(a) do
                cluster[key(k)] = value(v)
            end
            
            table.insert(clusters, cluster)
        end,
        
        Zone = function(a)
            local zone = {}
            
            for k, v in pairs(a) do
                zone[key(k)] = value(v)
            end
            
            return zone
        end,
    }

    function env.__newindex(t, k, v)
        rawset(env, key(k), value(v))
    end

    setmetatable(env, env)

    local config = loadfile("zone_cluster_config.lua")
    setfenv(config, env)
    config()
    
    local per = env.maxzonespercluster
    if per then
        C.zc_mgr_set_max_zones_per_cluster(mgr, per)
    end
    
    for _, cluster in ipairs(clusters) do
        local zc = C.zc_mgr_start_zone_cluster(mgr)
        
        per = cluster.maxzones
        if per then
            C.zone_cluster_set_max_zones(zc, per)
        end
        
        for _, zone in ipairs(cluster) do
            local name  = zone.shortname
            local id    = zone.zoneid or 0
            local inst  = zone.instanceid or 0
            local up    = zone.alwaysup and 1 or 0
            
            C.zc_mgr_add_zone_reservation(mgr, zc, name, id, inst, up)
        end
    end
end
