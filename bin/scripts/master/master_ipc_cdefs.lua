
local ffi = require "ffi"

ffi.cdef[[
void    console_reply(void* console, const char* msg, uint32_t len);
void    console_finish(void* console);
]]
