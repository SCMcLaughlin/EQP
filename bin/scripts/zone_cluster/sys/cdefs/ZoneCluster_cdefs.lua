
local ffi = require "ffi"

ffi.cdef[[
typedef struct ZC ZC;

void    zc_log(ZC*, const char* msg);
void    zc_log_for(ZC*, int sourceId, const char* msg);
]]

return ffi.C
