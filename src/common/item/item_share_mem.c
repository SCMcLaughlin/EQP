
#include "item_share_mem.h"

#define INVALID 0xffffffff

void item_share_mem_open(Basic* basic, lua_State* L, ItemShareMem* shm, const char* path, uint32_t length)
{
    ItemPrototype* array;
    uint32_t n;
    uint32_t i;
    
    shm_viewer_open(basic, shm, path, length);
    
    array   = item_share_mem_get_array(shm);
    n       = item_share_mem_get_array_count(shm);
    
    // In Lua, each item prototype object can be accessed using the path of its definition script,
    // e.g. 'local example = require "items/example"'.
    // We do this by mucking with the import system a bit, pre-placing each item pointer into
    // the package.loaded table under its definition script path.
    //
    // Here we compile and push a little function to accomplish this (simpler and more space
    // efficient than doing it purely through the C API):
    luaL_dostring(L,
        "require 'ItemPrototype_cdefs'                          \n"
        "local ffi              = require 'ffi'                 \n"
        "local cast             = ffi.cast                      \n"
        "local ItemPrototype    = ffi.typeof('ItemPrototype*')  \n"
        "local loaded           = package.loaded                \n"
        "return function(path, ptr)                             \n"
        "   loaded['items/' .. path] = cast(ItemPrototype, ptr) \n"
        "end");
    
    for (i = 0; i < n; i++)
    {
        ItemPrototype* proto = &array[i];
        
        // Push an extra reference to the function compiled above (gets popped when the func is called)
        lua_pushvalue(L, -1);
        // Push the arguments (path, ptr)
        lua_pushstring(L, item_proto_get_script_path(proto));
        lua_pushlightuserdata(L, proto);
        
        // Call the function
        lua_sys_call(basic, L, 2, 0);
    }
    
    // Pop the original reference to the function from the stack
    lua_pop(L, 1);
}

static uint32_t item_share_mem_get_offset(ItemShm_Entry* entries, uint32_t capMinusOne, uint32_t index, uint32_t itemId)
{
    ItemShm_Entry* ent = &entries[index];
    uint32_t mainIndex;
    
    if (ent->offset == 0)
        goto ret_zero;
    
    mainIndex = ent->hash & capMinusOne;
    
    if (mainIndex != index)
        goto ret_zero;
    
    for (;;)
    {
        if (ent->key == itemId)
            return ent->offset;
        
        if (ent->next != INVALID)
            ent = &entries[ent->next];
        else
            break;
    }
    
ret_zero:
    return 0;
}

ItemPrototype* item_share_mem_get_prototype(ItemShareMem* shm, uint32_t itemId)
{
    ItemShm* data   = shm_viewer_memory_type(shm, ItemShm);
    uint32_t hash   = item_share_mem_calc_hash(itemId);
    uint32_t index  = hash & data->capacityMinusOne;
    uint32_t offset = item_share_mem_get_offset(data->entries, data->capacityMinusOne, index, itemId);
    
    if (offset == 0 || (offset + sizeof(ItemPrototype)) > shm_viewer_memory_length(shm))
        return NULL;
    
    return (ItemPrototype*)(((byte*)data) + offset);
}

ItemPrototype* item_share_mem_get_array(ItemShareMem* shm)
{
    ItemShm* data   = shm_viewer_memory_type(shm, ItemShm);
    uint32_t offset = sizeof(ItemShm) + (sizeof(ItemShm_Entry) * (data->capacityMinusOne + 1));
    
    return (ItemPrototype*)(((byte*)data) + offset);
}

uint32_t item_share_mem_get_array_count(ItemShareMem* shm)
{
    ItemShm* data = shm_viewer_memory_type(shm, ItemShm);
    return data->itemCount;
}

static uint32_t item_share_mem_hash(uint32_t lo, uint32_t hi)
{
    lo ^= hi;
    hi = bit_rotate(hi, 14);
    lo -= hi;
    hi = bit_rotate(hi, 5);
    hi ^= lo;
    hi -= bit_rotate(lo, 13);
    return hi;
}

uint32_t item_share_mem_calc_hash(uint32_t itemId)
{
    return item_share_mem_hash(0xffccff44, itemId);
}

#undef INVALID
