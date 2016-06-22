
local paths = {
    "scripts/zone_cluster/?.lua",
    "scripts/zone_cluster/sys/?.lua",
    "scripts/zone_cluster/sys/classes/?.lua",
    "scripts/zone_cluster/sys/cdefs/?.lua",
}

package.path = table.concat(paths, ";") ..";".. package.path
