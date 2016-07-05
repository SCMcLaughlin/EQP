
#include "z_slices.h"
#include "eqp_basic.h"

void zslices_init(Basic* basic, ZSlices* slices, AABB* box, Triangle* tris, uint32_t triCount)
{
    Vector e            = aabb_get_extent(box);
    ZSlice* array       = eqp_alloc_type_array(basic, EQP_ZSLICE_COUNT, ZSlice);
    Array* triangles    = array_create_type(basic, Triangle);
    float incX          = (e.x / 256.0f) + 0.01f; // Try to account for rounding error
    float incY          = (e.y / 256.0f) + 0.01f;
    float curX          = box->minCorner.x;
    float curY          = box->minCorner.y;
    float resetY        = curY;
    int index           = 0;
    uint32_t x;
    AABB bound;
    
    slices->minX        = curX;
    slices->minY        = curY;
    slices->incrementX  = incX;
    slices->incrementY  = incY;
    slices->slices      = array;
    
    bound.minCorner.z = -999999.0f;
    bound.maxCorner.z =  999999.0f;
    
    memset(array, 0, sizeof(ZSlice) * EQP_ZSLICE_COUNT);
    
    for (x = 0; x < 256; x++)
    {
        uint32_t y;
        
        bound.minCorner.x = curX;
        bound.maxCorner.x = curX + incX;
        
        for (y = 0; y < 256; y++)
        {
            uint32_t t;
            ZSlice* zs = &array[index++];
            
            zs->triangleIndex = array_count(triangles);
            
            bound.minCorner.y = curY;
            bound.maxCorner.y = curY + incY;
            
            for (t = 0; t < triCount; t++)
            {
                Triangle* tri = &tris[t];
                
                if (!aabb_excludes_triangle(&bound, tri))
                {
                    array_push_back(basic, &triangles, tri);
                    zs->triangleCount++;
                }
            }
            
            curY += incY;
        }
        
        curX += incX;
        curY = resetY;
    }
    
    slices->triangles = triangles;
}

void zslices_deinit(ZSlices* slices)
{
    if (slices->slices)
    {
        free(slices->slices);
        slices->slices = NULL;
    }
    
    if (slices->triangles)
    {
        array_destroy(slices->triangles);
        slices->triangles = NULL;
    }
}
