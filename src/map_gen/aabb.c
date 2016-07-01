
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

void aabb_init_from_corners(AABB* box, Vertex* a, Vertex* b)
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

Vertex aabb_get_center(AABB* box)
{
    Vertex sphere;
    
    sphere.x = (box->minCorner.x + box->maxCorner.x) * 0.5f;
    sphere.y = (box->minCorner.y + box->maxCorner.y) * 0.5f;
    sphere.z = (box->minCorner.z + box->maxCorner.z) * 0.5f;
    
    return sphere;
}

Vertex aabb_get_half_extent(AABB* box)
{
    Vertex e;
    
    e.x = (box->maxCorner.x - box->minCorner.x) * 0.5f;
    e.y = (box->maxCorner.y - box->minCorner.y) * 0.5f;
    e.z = (box->maxCorner.z - box->minCorner.z) * 0.5f;
    
    return e;
}

static void aabb_set_vertex(Vertex* v, float x, float y, float z)
{
    v->x = x;
    v->y = y;
    v->z = z;
}

int aabb_contains_point(AABB* box, Vertex* v)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;
    
    return
        x >= box->minCorner.x && y >= box->minCorner.y && z >= box->minCorner.z &&
        x <= box->maxCorner.x && y <= box->maxCorner.y && z <= box->maxCorner.z;
}

void aabb_get_all_corners_and_center(AABB* box, Vertex* out, Vertex* outCenter)
{
    Vertex mid = aabb_get_center(box);
    Vertex diag;
    
    *outCenter = mid;
    
    diag.x = mid.x - box->maxCorner.x;
    diag.y = mid.y - box->maxCorner.y;
    diag.z = mid.z - box->maxCorner.z;
    
    aabb_set_vertex(&out[0], mid.x + diag.x, mid.y + diag.y, mid.z + diag.z);
    aabb_set_vertex(&out[1], mid.x + diag.x, mid.y + diag.y, mid.z - diag.z);
    aabb_set_vertex(&out[2], mid.x + diag.x, mid.y - diag.y, mid.z + diag.z);
    aabb_set_vertex(&out[3], mid.x + diag.x, mid.y - diag.y, mid.z - diag.z);
    aabb_set_vertex(&out[4], mid.x - diag.x, mid.y + diag.y, mid.z + diag.z);
    aabb_set_vertex(&out[5], mid.x - diag.x, mid.y + diag.y, mid.z - diag.z);
    aabb_set_vertex(&out[6], mid.x - diag.x, mid.y - diag.y, mid.z + diag.z);
    aabb_set_vertex(&out[7], mid.x - diag.x, mid.y - diag.y, mid.z - diag.z);
}
