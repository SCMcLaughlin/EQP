
#include "aabb.h"

void aabb_init(AABB* box)
{
    box->minCorner.x =  999999.0f;
    box->minCorner.y =  999999.0f;
    box->minCorner.z =  999999.0f;
    box->maxCorner.x = -999999.0f;
    box->maxCorner.y = -999999.0f;
    box->maxCorner.z = -999999.0f;
}

static float aabb_min(float a, float b)
{
    return (a < b) ? a : b;
}

static float aabb_max(float a, float b)
{
    return (a > b) ? a : b;
}

void aabb_init_from_corners(AABB* box, Vector* a, Vector* b)
{
    box->minCorner.x = aabb_min(a->x, b->x);
    box->minCorner.y = aabb_min(a->y, b->y);
    box->minCorner.z = aabb_min(a->z, b->z);
    
    box->maxCorner.x = aabb_max(a->x, b->x);
    box->maxCorner.y = aabb_max(a->y, b->y);
    box->maxCorner.z = aabb_max(a->z, b->z);
}

void aabb_add_internal_point(AABB* box, float x, float y, float z)
{
    if (x < box->minCorner.x) box->minCorner.x = x;
    if (y < box->minCorner.y) box->minCorner.y = y;
    if (z < box->minCorner.z) box->minCorner.z = z;
    
    if (x > box->maxCorner.x) box->maxCorner.x = x;
    if (y > box->maxCorner.y) box->maxCorner.y = y;
    if (z > box->maxCorner.z) box->maxCorner.z = z;
}

void aabb_add_internal_point_vector(AABB* box, Vector* vec)
{
    aabb_add_internal_point(box, vec->x, vec->y, vec->z);
}

Vector aabb_get_center(AABB* box)
{
    Vector sphere;
    
    sphere.x = (box->minCorner.x + box->maxCorner.x) * 0.5f;
    sphere.y = (box->minCorner.y + box->maxCorner.y) * 0.5f;
    sphere.z = (box->minCorner.z + box->maxCorner.z) * 0.5f;
    
    return sphere;
}

Vector aabb_get_extent(AABB* box)
{
    Vector e;
    
    e.x = box->maxCorner.x - box->minCorner.x;
    e.y = box->maxCorner.y - box->minCorner.y;
    e.z = box->maxCorner.z - box->minCorner.z;
    
    return e;
}

Vector aabb_get_half_extent(AABB* box)
{
    Vector e;
    
    e.x = (box->maxCorner.x - box->minCorner.x) * 0.5f;
    e.y = (box->maxCorner.y - box->minCorner.y) * 0.5f;
    e.z = (box->maxCorner.z - box->minCorner.z) * 0.5f;
    
    return e;
}

static void aabb_set_vector(Vector* v, float x, float y, float z)
{
    v->x = x;
    v->y = y;
    v->z = z;
}

int aabb_contains_point(AABB* box, Vector* v)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;
    
    return
        x >= box->minCorner.x && y >= box->minCorner.y && z >= box->minCorner.z &&
        x <= box->maxCorner.x && y <= box->maxCorner.y && z <= box->maxCorner.z;
}

uint32_t aabb_clip_vector(AABB* box, Vector* v)
{
    uint32_t ret    = 0;
    float x         = v->x;
    float y         = v->y;
    float z         = v->z;
    
    if (x < box->minCorner.x)
        ret |= AABB_Clip_MinX;
    else if (x > box->maxCorner.x)
        ret |= AABB_Clip_MaxX;
    
    if (y < box->minCorner.y)
        ret |= AABB_Clip_MinY;
    else if (y > box->maxCorner.y)
        ret |= AABB_Clip_MaxY;
    
    if (z < box->minCorner.z)
        ret |= AABB_Clip_MinZ;
    else if (z > box->maxCorner.z)
        ret |= AABB_Clip_MaxZ;
    
    return ret;
}

uint32_t aabb_clip_triangle(AABB* box, Triangle* tri, uint32_t* andCode)
{
    uint32_t c1 = aabb_clip_vector(box, &tri->points[0]);
    uint32_t c2 = aabb_clip_vector(box, &tri->points[1]);
    uint32_t c3 = aabb_clip_vector(box, &tri->points[2]);
    *andCode = 0xffffffff & c1 & c2 & c3;
    return c1 | c2 | c3;
}

int aabb_contains_triangle_precise(AABB* box, Triangle* tri, uint32_t* orCode)
{
    int ret = false;
    uint32_t andCode;
    
    *orCode = aabb_clip_triangle(box, tri, &andCode);
    
    if (andCode == 0)
        ret = true;
    
    return ret;
}

#define asInt(v) (*(uint32_t*)&(v))
#define asFloatArrayFlat(v) ((float*)&(v))
#define asFloatArray(v) ((float*)(v))

int aabb_intersected_by_ray(AABB* box, Vector* origin, Vector* direction, Vector* intersect)
{
    int inside = true;
    float maxT[3];
    uint32_t i;
    uint32_t which;
    
    maxT[0] = maxT[1] = maxT[2] = -1.0f;
    
    for (i = 0; i < 3; i++)
    {
        if (asFloatArray(origin)[i] < asFloatArrayFlat(box->minCorner)[i])
        {
            asFloatArray(intersect)[i]  = asFloatArrayFlat(box->minCorner)[i];
            inside                      = false;
            
            if (asInt(asFloatArray(direction)[i]))
                maxT[i] = (asFloatArrayFlat(box->minCorner)[i] - asFloatArray(origin)[i]) / asFloatArray(direction)[i];
        }
        else if (asFloatArray(origin)[i] > asFloatArrayFlat(box->maxCorner)[i])
        {
            asFloatArray(intersect)[i]  = asFloatArrayFlat(box->maxCorner)[i];
            inside                      = false;
            
            if (asInt(asFloatArray(direction)[i]))
                maxT[i] = (asFloatArrayFlat(box->maxCorner)[i] - asFloatArray(origin)[i]) / asFloatArray(direction)[i];
        }
    }
    
    if (inside)
    {
        memcpy(intersect, origin, sizeof(Vector));
        return true;
    }
    
    which = 0;
    
    if (maxT[1] > maxT[0])
        which = 1;
    if (maxT[2] > maxT[which])
        which = 2;
    
    if (asInt(maxT[which]) & 0x80000000)
        return false;

    for (i = 0; i < 3; i++)
    {
        if (i == which)
            continue;
        
        asFloatArray(intersect)[i] = asFloatArray(origin)[i] + maxT[which] * asFloatArray(direction)[i];
        
        if (asFloatArray(intersect)[i] < (asFloatArrayFlat(box->minCorner)[i] - FLOAT_EPSILON) ||
            asFloatArray(intersect)[i] > (asFloatArrayFlat(box->maxCorner)[i] + FLOAT_EPSILON))
            return false;
    }
    
    return true;
}

#undef asInt
#undef asFloatArray

int aabb_intersected_by_line_segment(AABB* box, Vector* origin, Vector* direction, float distance, float* outDistance)
{
    int ret = false;
    
    if (distance > FLOAT_EPSILON)
    {
        Vector intersect;
        
        ret = aabb_intersected_by_ray(box, origin, direction, &intersect);
        
        if (ret)
        {
            float dsq;
            
            vector_difference(&intersect, origin, &intersect);
            
            dsq = vector_length_squared(&intersect);
            
            if (dsq < (distance * distance))
            {
                if (outDistance)
                    *outDistance = sqrtf(dsq);
            }
            else
            {
                ret = false;
            }
        }
    }
    
    return ret;
}

void aabb_clamp(AABB* box, AABB* to)
{
    if (box->minCorner.x < to->minCorner.x) box->minCorner.x = to->minCorner.x;
    if (box->minCorner.y < to->minCorner.y) box->minCorner.y = to->minCorner.y;
    if (box->minCorner.z < to->minCorner.z) box->minCorner.z = to->minCorner.z;
    if (box->maxCorner.x < to->maxCorner.x) box->maxCorner.x = to->maxCorner.x;
    if (box->maxCorner.y < to->maxCorner.y) box->maxCorner.y = to->maxCorner.y;
    if (box->maxCorner.z < to->maxCorner.z) box->maxCorner.z = to->maxCorner.z;
}

void aabb_split(AABB* box, int axisId, AABB* b1, AABB* b2)
{
    Vector c = aabb_get_center(box);
    Vector temp;
    
    switch (axisId)
    {
    case AABB_AxisX:
        vector_set(&temp, c.x, box->maxCorner.y, box->maxCorner.z);
        aabb_init_from_corners(b1, &box->minCorner, &temp);
    
        vector_set(&temp, c.x, box->minCorner.y, box->minCorner.z);
        aabb_init_from_corners(b2, &temp, &box->maxCorner);
        break;
    
    case AABB_AxisY:
        vector_set(&temp, box->maxCorner.x, c.y, box->maxCorner.z);
        aabb_init_from_corners(b1, &box->minCorner, &temp);
    
        vector_set(&temp, box->minCorner.x, c.y, box->minCorner.z);
        aabb_init_from_corners(b2, &temp, &box->maxCorner);
        break;
    
    case AABB_AxisZ:
        vector_set(&temp, box->maxCorner.x, box->maxCorner.y, c.z);
        aabb_init_from_corners(b1, &box->minCorner, &temp);
    
        vector_set(&temp, box->minCorner.x, box->minCorner.y, c.z);
        aabb_init_from_corners(b2, &temp, &box->maxCorner);
        break;
    }
}

void aabb_get_all_corners_and_center(AABB* box, Vector* out, Vector* outCenter)
{
    Vector mid = aabb_get_center(box);
    Vector diag;
    
    *outCenter = mid;
    
    diag.x = mid.x - box->maxCorner.x;
    diag.y = mid.y - box->maxCorner.y;
    diag.z = mid.z - box->maxCorner.z;
    
    aabb_set_vector(&out[0], mid.x + diag.x, mid.y + diag.y, mid.z + diag.z);
    aabb_set_vector(&out[1], mid.x + diag.x, mid.y + diag.y, mid.z - diag.z);
    aabb_set_vector(&out[2], mid.x + diag.x, mid.y - diag.y, mid.z + diag.z);
    aabb_set_vector(&out[3], mid.x + diag.x, mid.y - diag.y, mid.z - diag.z);
    aabb_set_vector(&out[4], mid.x - diag.x, mid.y + diag.y, mid.z + diag.z);
    aabb_set_vector(&out[5], mid.x - diag.x, mid.y + diag.y, mid.z - diag.z);
    aabb_set_vector(&out[6], mid.x - diag.x, mid.y - diag.y, mid.z + diag.z);
    aabb_set_vector(&out[7], mid.x - diag.x, mid.y - diag.y, mid.z - diag.z);
}
