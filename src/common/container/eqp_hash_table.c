
#include "eqp_hash_table.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"

#define DEFAULT_CAPACITY    2 /* Must be a power of 2 */
#define NEXT_INVALID        0xffffffff
#define FREE_INDEX_INVALID  0xffffffff

STRUCT_DEFINE(HashTable_Entry)
{
    union
    {
        String*     key;    // The hash table makes private copies of all keys
        uint64_t    padding;
    };
    uint32_t    hash;
    uint32_t    next;
    byte        data[0];
};

STRUCT_DEFINE(HashTable)
{
    uint32_t    capacity;
    uint32_t    elemSize;
    uint32_t    freeIndex;
    uint32_t    entSize;    // Make sure each HashTable_Entry will be a multiple of 8 bytes
    byte        data[0];    // Aligned on a 16-byte boundary
};

HashTable* hash_table_create(Basic* basic, size_t elementSize)
{
    uint32_t entSize = elementSize + sizeof(HashTable_Entry);
    uint32_t initSize;
    HashTable* tbl;
    uint32_t i;
    
    if (entSize%8 != 0)
        entSize += 8 - entSize%8;
    
    initSize    = sizeof(HashTable) + (entSize * DEFAULT_CAPACITY);
    tbl         = eqp_alloc_type_bytes(basic, initSize, HashTable);
    
    memset(tbl, 0, initSize);
    
    tbl->capacity   = DEFAULT_CAPACITY;
    tbl->elemSize   = elementSize;
    tbl->freeIndex  = 0;
    tbl->entSize    = entSize;
    
    for (i = 0; i < DEFAULT_CAPACITY; i++)
    {
        HashTable_Entry* ent = (HashTable_Entry*)&tbl->data[entSize * i];
        
        ent->next = NEXT_INVALID;
    }
    
    return tbl;
}

void hash_table_destroy(HashTable* tbl)
{
    uint32_t entSize    = tbl->entSize;
    byte* data          = tbl->data;
    uint32_t n          = tbl->capacity;
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        HashTable_Entry* ent = (HashTable_Entry*)data;
        
        if (ent->key)
            string_destroy(ent->key);
        
        data += entSize;
    }
    
    free(tbl);
}

static uint32_t hash_table_calc_hash(const char* key, uint32_t length)
{
    uint32_t h      = length;
    uint32_t step   = (length >> 5) + 1;
    uint32_t i;
    
    for (i = length; i >= step; i -= step)
    {
        h = h ^ ((h << 5) + (h >> 2) + (key[i - 1]));
    }
    
    return h;
}

static void hash_table_advance_free_index(HashTable* tbl)
{
    uint32_t freeIndex  = tbl->freeIndex;
    uint32_t capacity   = tbl->capacity;
    uint32_t size       = tbl->entSize;
    
    while (++freeIndex != capacity)
    {
        HashTable_Entry* ent = (HashTable_Entry*)&tbl->data[freeIndex * size];
        
        if (ent->key == NULL)
        {
            tbl->freeIndex = freeIndex;
            return;
        }
    }
    
    tbl->freeIndex = FREE_INDEX_INVALID;
}

static void hash_table_do_insert(Basic* basic, HashTable_Entry* ent, const char* key, uint32_t length, void* value, uint32_t hash, uint32_t elemSize)
{
    ent->key    = string_create_from_cstr(basic, key, length);
    ent->hash   = hash;
    memcpy(ent->data, value, elemSize);
}

static void hash_table_entry_copy(HashTable_Entry* newEnt, HashTable_Entry* oldEnt, uint32_t elemSize)
{
    newEnt->key     = oldEnt->key;
    newEnt->hash    = oldEnt->hash;
    memcpy(newEnt->data, oldEnt->data, elemSize);
}

static void hash_table_realloc(Basic* basic, HashTable** ptbl, HashTable* oldTbl)
{
    uint32_t n              = oldTbl->capacity;
    uint32_t newCap         = n * 2;
    uint32_t elemSize       = oldTbl->elemSize;
    uint32_t entSize        = oldTbl->entSize;
    uint32_t initSize       = sizeof(HashTable) + (entSize * newCap);
    HashTable* newTbl       = eqp_alloc_type_bytes(basic, initSize, HashTable);
    uint32_t newFreeIndex   = 0;
    uint32_t i;
    
    memset(newTbl, 0, initSize);
    
    newTbl->capacity    = newCap;
    newTbl->elemSize    = elemSize;
    newTbl->entSize     = entSize;
    
    for (i = 0; i < newCap; i++)
    {
        HashTable_Entry* ent = (HashTable_Entry*)&newTbl->data[entSize * i];
        
        ent->next = NEXT_INVALID;
    }
    
    newCap--;
    
    for (i = 0; i < n; i++)
    {
        HashTable_Entry* oldEnt = (HashTable_Entry*)&oldTbl->data[entSize * i];
        uint32_t pos            = oldEnt->hash & newCap;
        HashTable_Entry* newEnt = (HashTable_Entry*)&newTbl->data[entSize * pos];
        
        if (newEnt->key == NULL)
        {
            // Copy key ptr, hash, value
            hash_table_entry_copy(newEnt, oldEnt, elemSize);
            
            if (pos != newFreeIndex)
                continue;
        }
        else
        {
            uint32_t mainPos = newEnt->hash & newCap;
            
            if (mainPos != pos)
            {
                HashTable_Entry* mainEnt;
                memcpy(&newTbl->data[entSize * newFreeIndex], newEnt, entSize);
                mainEnt = (HashTable_Entry*)&newTbl->data[entSize * mainPos];
                
                while (mainEnt->next != pos)
                {
                    mainEnt = (HashTable_Entry*)&newTbl->data[entSize * mainEnt->next];
                }
                
                mainEnt->next = newFreeIndex;
                hash_table_entry_copy(newEnt, oldEnt, elemSize);
            }
            else
            {
                HashTable_Entry* freeEnt;
                
                while (newEnt->next != NEXT_INVALID)
                {
                    newEnt = (HashTable_Entry*)&newTbl->data[entSize * newEnt->next];
                }
                
                newEnt->next    = newFreeIndex;
                freeEnt         = (HashTable_Entry*)&newTbl->data[entSize * newFreeIndex];
                hash_table_entry_copy(freeEnt, oldEnt, elemSize);
            }
        }
        
        // If we reach here, advance the new free index
        for (;;)
        {
            HashTable_Entry* ent = (HashTable_Entry*)&newTbl->data[entSize * (++newFreeIndex)];
            
            if (ent->key == NULL)
                break;
        }
    }
    
    newTbl->freeIndex = newFreeIndex;
    free(oldTbl);
    *ptbl = newTbl;
}

static int hash_table_do_set(Basic* basic, HashTable** ptbl, const char* key, uint32_t length, void* value, uint32_t hash)
{
    HashTable* tbl          = *ptbl;
    uint32_t freeIndex      = tbl->freeIndex;
    uint32_t capMinusOne    = tbl->capacity - 1;
    uint32_t pos            = hash & capMinusOne;
    uint32_t entSize        = tbl->entSize;
    HashTable_Entry* ent    = (HashTable_Entry*)&tbl->data[pos * entSize];
    
    if (ent->key == NULL)
    {
        if (pos == freeIndex)
            hash_table_advance_free_index(tbl);
        
        hash_table_do_insert(basic, ent, key, length, value, hash, tbl->elemSize);
        goto ret_true;
    }
    
    if (freeIndex != FREE_INDEX_INVALID)
    {
        uint32_t mainPos = ent->hash & capMinusOne;
        
        if (mainPos != pos)
        {
            HashTable_Entry* mainEnt;
            
            // Evict this entry to the free index
            memcpy(&tbl->data[freeIndex * entSize], ent, entSize);
            
            // Follow this entry's chain to the end, then point the last entry to the free index
            mainEnt = (HashTable_Entry*)&tbl->data[mainPos * entSize];
            
            while (mainEnt->next != pos)
            {
                mainEnt = (HashTable_Entry*)&tbl->data[mainEnt->next * entSize];
            }
            
            mainEnt->next = freeIndex;
            hash_table_advance_free_index(tbl);
            hash_table_do_insert(basic, ent, key, length, value, hash, tbl->elemSize);
            goto ret_true;
        }
        
        // If we reach here, the entry is in its main position
        // Check if we already have this key in the table, and abort if so;
        // otherwise, follow the chain to the end, link the last entry to the
        // free index, and add our new key value pair at the free index
        for (;;)
        {
            if (ent->hash == hash && string_compare_cstr(ent->key, key) == 0)
                return false;
            
            if (ent->next != NEXT_INVALID)
                ent = (HashTable_Entry*)&tbl->data[ent->next * entSize];
            else
                break;
        }
        
        ent->next = freeIndex;
        hash_table_advance_free_index(tbl);
        hash_table_do_insert(basic, (HashTable_Entry*)&tbl->data[freeIndex * entSize], key, length, value, hash, tbl->elemSize);
        goto ret_true;
    }
    
    // If we reach here, there are no free spaces left in the hash table
    hash_table_realloc(basic, ptbl, tbl);
    return hash_table_do_set(basic, ptbl, key, length, value, hash);
    
ret_true:
    return true;
}

int hash_table_set_by_cstr(Basic* basic, HashTable** ptbl, const char* key, uint32_t length, void* value)
{
    uint32_t hash = hash_table_calc_hash(key, length);
    return hash_table_do_set(basic, ptbl, key, length, value, hash);
}

void* hash_table_get_by_cstr(HashTable* tbl, const char* key, uint32_t length)
{
    uint32_t hash           = hash_table_calc_hash(key, length);
    uint32_t capMinusOne    = tbl->capacity - 1;
    uint32_t size           = tbl->entSize;
    uint32_t pos            = hash & capMinusOne;
    HashTable_Entry* ent    = (HashTable_Entry*)&tbl->data[pos * size];
    uint32_t mainPos;
    
    if (!ent->key)
        goto ret_null;
    
    mainPos = ent->hash & capMinusOne;
    
    if (mainPos != pos)
        goto ret_null;
    
    for (;;)
    {
        if (ent->hash == hash && string_compare_cstr(ent->key, key) == 0)
            return ent->data;
        
        if (ent->next != NEXT_INVALID)
            ent = (HashTable_Entry*)&tbl->data[ent->next * size];
        else
            break;
    }
    
ret_null:
    return NULL;
}

void hash_table_remove_by_cstr(HashTable* tbl, const char* key, uint32_t length)
{
    uint32_t hash           = hash_table_calc_hash(key, length);
    uint32_t capMinusOne    = tbl->capacity - 1;
    uint32_t entSize        = tbl->entSize;
    uint32_t freeIndex      = tbl->freeIndex;
    uint32_t pos            = hash & capMinusOne;
    HashTable_Entry* ent    = (HashTable_Entry*)&tbl->data[entSize * pos];
    HashTable_Entry* prev   = NULL;
    uint32_t mainPos;
    
    if (!ent->key)
        goto ret;
    
    mainPos = ent->hash & capMinusOne;
    
    if (mainPos != pos)
        goto ret;
    
    for (;;)
    {
        if (ent->hash == hash && string_compare_cstr(ent->key, key) == 0)
        {
            // Found the one we want to remove
            string_destroy(ent->key);
            
            // Need to fix links leading to (or following) this, if there were any
            if (prev)
            {
                // prev -> cur -> next ==> prev -> next
                uint32_t cur = prev->next;
                prev->next = ent->next;
                
                if (freeIndex > cur)
                    tbl->freeIndex = cur;
            }
            else
            {
                uint32_t next = ent->next;
                
                if (next != NEXT_INVALID)
                {
                    // [main] -> next ==> [next]
                    prev    = ent;
                    ent     = (HashTable_Entry*)&tbl->data[next * entSize];
                    memcpy(prev, ent, entSize);
                    
                    if (freeIndex > next)
                        tbl->freeIndex = next;
                }
            }
            
            // Bit confusing, but the key we are setting to NULL might not be the key we just destroyed; intended
            ent->key    = NULL;
            ent->next   = NEXT_INVALID;
            goto ret;
        }
        
        // Not the one we were looking for, move down the chain
        if (ent->next != NEXT_INVALID)
        {
            prev    = ent;
            ent     = (HashTable_Entry*)&tbl->data[ent->next * entSize];
        }
        else
        {
            break;
        }
    }
    
ret:
    return;
}

#undef DEFAULT_CAPACITY
#undef NEXT_INVALID
#undef FREE_INDEX_INVALID
