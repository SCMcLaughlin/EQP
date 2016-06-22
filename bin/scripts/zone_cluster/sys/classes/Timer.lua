
--------------------------------------------------------------------------------
-- Imports
--------------------------------------------------------------------------------
local C         = require "Timer_cdefs"
local ffi       = require "ffi"
local sys       = require "sys"
local class     = require "class"
local LuaObject = require "LuaObject"
local ZC        = require "ZC"
--------------------------------------------------------------------------------

local Timer = class("Timer", LuaObject)

local function gc(ptr)
    local cbIndex = C.zc_lua_timer_get_callback_index(ptr)
    
    sys.callbackGC(cbIndex)
    
    C.zc_lua_timer_destroy(ptr)
end

local function make(milliseconds, callback, start)
    local cbIndex   = sys.pushCallback(callback)
    local ptr       = C.zc_lua_timer_create(ZC:ptr(), milliseconds, cbIndex, start and 1 or 0)
    ffi.gc(ptr, gc)
    return sys.createTimer(ptr, Timer)
end

local function getMilliseconds(args)
    local ms = args.milliseconds
    
    if not ms then
        local sec = args.seconds
        
        if not sec then
            error "Timer must have 'milliseconds' or 'seconds' fields set!"
        end
        
        ms = sec * 1000
    end
    
    return ms
end

local function getCallback(args)
    local cb = args.callback
    
    if not cb then
        error "Timer must have 'callback' field set!"
    end
    
    return cb
end

function Timer.new(args)
    local ms = getMilliseconds(args)
    local cb = getCallback(args)
    
    return make(ms, cb, not args.stopped)
end

function Timer.once(args)
    local ms    = getMilliseconds(args)
    local func  = getCallback(args)
    
    local function callback(timer)
        func(timer)
        timer:stop()
    end
    
    return make(ms, callback, not args.stopped)
end

function Timer:_timer()
    return C.zc_lua_timer_get_timer(self:ptr())
end

function Timer:restart()
    C.timer_restart(self:_timer())
end

Timer.start = Timer.restart

function Timer:stop()
    C.timer_stop(self:_timer())
end

function Timer:delay(bySeconds)
    self:delayMilliseconds(bySeconds * 1000)
end

function Timer:delayMilliseconds(byMilliseconds)
    C.timer_delay(self:_timer(), byMilliseconds)
end

function Timer:trigger()
    C.timer_force_trigger_on_next_cycle(self:_timer())
end

function Timer:execute()
    C.timer_execute_callback(self:_timer())
end

return Timer
