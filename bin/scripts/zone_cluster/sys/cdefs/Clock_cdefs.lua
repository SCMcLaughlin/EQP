
local ffi = require "ffi"

ffi.cdef[[
uint64_t    clock_milliseconds();
uint64_t    clock_microseconds();
uint64_t    clock_unix_seconds();
]]

return ffi.C
