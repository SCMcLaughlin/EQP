
local ffi = require "ffi"

ffi.cdef[[
void    master_ipc_thread_console_reply(void* thread, void* ipc, const void* src, const char* msg);
void    master_ipc_thread_console_finish(void* thread, void* ipc, const void* src);
]]
