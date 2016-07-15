
local ffi       = require "ffi"
local struct    = require "struct"

local C         = ffi.C
local io        = io
local math      = math
local tonumber  = tonumber

local binutil = {
    Void    = {Ptr = ffi.typeof("void*")},
    Int     = struct.primitive("int"),
    Int16   = struct.primitive("int16_t"),
    Int64   = struct.primitive("int64_t"),
    Uint16  = struct.primitive("uint16_t"),
    Uint32  = struct.primitive("uint32_t"),
    Uint64  = struct.primitive("uint64_t"),
    Float   = struct.primitive("float"),
    Double  = struct.primitive("double"),
    Char    = struct.primitive("char"),
    Byte    = struct.primitive("uint8_t"),
    IntPtr  = struct.primitive("intptr_t"),
    UintPtr = struct.primitive("uintptr_t"),
    -----------------------------------------------
    Long    = struct.primitive("long"),
    ULong   = struct.primitive("unsigned long"),
}

-- Converts a pointer type to its (signed) address as a lua number
function binutil.toAddress(ptr)
    return tonumber(ffi.cast(binutil.IntPtr.Type, ptr))
end
binutil.ptrToInt = binutil.toAddress

-- Converts a numeric address to a void pointer
function binutil.addrToPtr(addr)
    return ffi.cast(binutil.Void.Ptr, addr)
end

-- Turns a 4-character file signature string into a uint32_t
function binutil.toFileSignature(str)
    return ffi.cast(binutil.Uint32.Ptr, str)[0]
end

-- Does (stack-limited) quicksort over a binary array of an arbitrary (supplied) type
function binutil.sortArray(array, numElements, compFunc, cType)
    local temp = cType()
    local size = ffi.sizeof(cType)

    local function swap(a, b)
        if a == b then return end -- Same address
        ffi.copy(temp, array[a], size) -- Can't do copy-assignment because it would overwrite temp variable-wise
        array[a] = array[b]
        array[b] = temp
    end

    local function partition(low, high)
        local pivotIndex = math.floor((low + high) / 2) -- Random may be better than middle

        swap(pivotIndex, high)

        local mem = low
        for i = low, high - 1 do
            if compFunc(array[i], array[high]) then
                swap(mem, i)
                mem = mem + 1
            end
        end

        swap(mem, high)
        return mem
    end

    local function quicksort(low, high)
        if low < high then
            local p = partition(low, high)
            quicksort(low, p - 1)
            quicksort(p + 1, high)
        end
    end

    quicksort(0, numElements - 1)
end

return binutil
