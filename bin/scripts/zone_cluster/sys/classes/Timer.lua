
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
    local tIndex    = sys.nextTimerIndex()
    local ptr       = C.zc_lua_timer_create(ZC:ptr(), milliseconds, cbIndex, tIndex, start and 1 or 0)
    local obj       = class.wrap(Timer, ptr)
    ffi.gc(ptr, gc)
    sys.createTimer(obj, tIndex)
    return obj
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

local function onceEnd(timer)
    timer:stop()
    local ptr = timer:ptr()
    sys.callbackGC(C.zc_lua_timer_get_callback_index(ptr))
    C.zc_lua_timer_set_callback_index(ptr, 0)
end

function Timer.once(args)
    local ms    = getMilliseconds(args)
    local func  = getCallback(args)
    
    -- Single-use timers can be kept alive by their own callbacks
    local timer
    
    local function callback()
        func(timer)
        onceEnd(timer)
    end
    
    timer = make(ms, callback, not args.stopped)
    return timer
end

function Timer.times(args)
    local ms    = getMilliseconds(args)
    local func  = getCallback(args)
    local n     = args.count or args.times or args.n or 1
    
    if n < 1 then
        n = 1
    end
    
    -- Timers with fixed numbers of uses can also by kept alive by their callbacks
    local i = 0
    local timer
    
    local function callback()
        func(timer)
        i = i + 1
        if i >= n then
            onceEnd(timer)
        end
    end
    
    timer = make(ms, callback, not args.stopped)
    return timer
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
