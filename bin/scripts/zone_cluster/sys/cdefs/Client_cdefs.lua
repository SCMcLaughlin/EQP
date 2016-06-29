
local ffi = require "ffi"

ffi.cdef[[
typedef struct Client Client;

int         client_is_pvp(Client* client);
int         client_is_gm(Client* client);
int         client_is_afk(Client* client);
int         client_is_linkdead(Client* client);
uint8_t     client_anon_setting(Client* client);
uint8_t     client_guild_rank(Client* client);
const char* client_surname_cstr(Client* client);

void        client_set_bind_point(Client* client, uint32_t bindId, int zoneId, float x, float y, float z, float heading);
]]

return ffi.C
