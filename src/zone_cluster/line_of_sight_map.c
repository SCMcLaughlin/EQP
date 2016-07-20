
#include "line_of_sight_map.h"
#include "zone_cluster.h"
#include "zone.h"

static byte* los_map_open_and_decompress(ZC* zc, Zone* zone, const char* path, uint32_t* length, MapFileHeader* header)
{
    FILE* fp        = fopen(path, "rb");
    byte* buffer    = NULL;
    
    if (!fp)
    {
        zc_zone_log_format(zc, zone, LogWarning, "Line of sight map file did not exist, expected it at '%s'", path);
        return NULL;
    }
    
    fread(header, 1, sizeof(MapFileHeader), fp);
    
    if (header->signature != EQP_MAP_GEN_MAP_FILE_SIGNATURE)
    {
        zc_zone_log_format(zc, zone, LogWarning, "Line of sight map file signature was incorrect, ignoring: '%s'", path);
        goto ret_null;
    }
    
    buffer = file_read_remaining_decompress_no_throw(B(zc), fp, length, header->inflatedLength);

ret_null:
    fclose(fp);
    return buffer;
}

static uint64_t los_map_read(ZC* zc, LineOfSightMap* map, byte* data, uint32_t length, MapFileHeader* header)
{
    uint64_t time               = clock_microseconds();
    uint32_t n                  = header->nodeCount;
    LineOfSightBspNode* nodes   = eqp_alloc_type_array(B(zc), n, LineOfSightBspNode);
    Triangle* triangles         = eqp_alloc_type_array(B(zc), header->triangleCount, Triangle);
    Aligned reader;
    Aligned* a = &reader;
    uint32_t i;
    
    aligned_init(B(zc), a, data, length);
    
    // Triangles
    aligned_read_buffer(a, triangles, sizeof(Triangle) * header->triangleCount);
    
    // Nodes
    for (i = 0; i < n; i++)
    {
        LineOfSightBspNode* node = &nodes[i];
        uint32_t m;
        
        // triangleCount
        m                   = aligned_read_uint32(a);
        node->triangleCount = m;
        // leftIndex
        node->leftIndex     = aligned_read_uint32(a);
        // rightIndex
        node->rightIndex    = aligned_read_uint32(a);
        // bounds
        aligned_read_buffer(a, &node->bounds, sizeof(AABB));
        // trianglesIndex
        node->triangles     = triangles + aligned_read_uint32(a);
    }
    
    map->recursionStack = array_create_type_with_capacity(B(zc), 32, LineOfSightBspNode*);
    map->nodes          = nodes;
    map->triangles      = triangles;
    
    // ZSlices
    map->minX       = header->minX;
    map->minY       = header->minY;
    map->incrementX = header->incrementX;
    map->incrementY = header->incrementY;
    
    map->zSlices            = eqp_alloc_type_array(B(zc), EQP_ZSLICE_COUNT, MapFileZSlice);
    map->zSliceTriangles    = eqp_alloc_type_array(B(zc), header->zSliceTriangleCount, Triangle);
    
    aligned_read_buffer(a, map->zSlices, sizeof(MapFileZSlice) * EQP_ZSLICE_COUNT);
    
    // ZSlice triangles
    aligned_read_buffer(a, map->zSliceTriangles, sizeof(Triangle) * header->zSliceTriangleCount);
    
    return clock_microseconds() - time;
}

void los_map_open(ZC* zc, Zone* zone, LineOfSightMap* map)
{
    MapFileHeader header;
    byte* buffer;
    uint32_t length;
    char path[1024];
    const char* binPath = getenv(EQP_PATH_ENV_VARIABLE);
    
    if (!binPath)
        binPath = ".";
    
    snprintf(path, sizeof(path), "%s/maps/%s.eqpmap", binPath, zone_get_short_name(zone));
    
    buffer = los_map_open_and_decompress(zc, zone, path, &length, &header);
    
    if (buffer)
    {
        uint64_t time;
        
        zone_set_min_z(zone, header.minZ - 10.0f);
        time = los_map_read(zc, map, buffer, length, &header);
        
        zc_zone_log_format(zc, zone, LogInfo, "Loaded line of sight map file in %lu microseconds", time);
        
        free(buffer);
    }
    else
    {
        map->recursionStack     = NULL;
        map->nodes              = NULL;
        map->triangles          = NULL;
        map->zSlices            = NULL;
        map->zSliceTriangles    = NULL;
        map->incrementX         = 1.0f; // Avoid division by zero
        map->incrementY         = 1.0f;
    }
}

void los_map_close(LineOfSightMap* map)
{
    if (map->recursionStack)
    {
        array_destroy(map->recursionStack);
        map->recursionStack = NULL;
    }
    
    if (map->nodes)
    {
        free(map->nodes);
        map->nodes = NULL;
    }
    
    if (map->triangles)
    {
        free(map->triangles);
        map->triangles = NULL;
    }
    
    if (map->zSlices)
    {
        free(map->zSlices);
        map->zSlices = NULL;
    }
    
    if (map->zSliceTriangles)
    {
        free(map->zSliceTriangles);
        map->zSliceTriangles = NULL;
    }
}

static int los_map_line_intersects_triangle(Vector* start, Vector* direction, Triangle* tri, float* t)
{
    Vector e1, e2, h, s, q;
    float a, f, u, v;

    vector_difference(&e1, &tri->points[1], &tri->points[0]);
    vector_difference(&e2, &tri->points[2], &tri->points[0]);
    vector_cross_product(&h, direction, &e2);
    a = vector_dot_product(&e1, &h);
    
    if (a > -FLOAT_EPSILON && a < FLOAT_EPSILON)
        goto ret_false;
    
    f = 1.0f / a;
    vector_difference(&s, start, &tri->points[0]);
    u = f * vector_dot_product(&s, &h);
    
    if (u < 0.0f || u > 1.0f)
        goto ret_false;
    
    vector_cross_product(&q, &s, &e1);
    v = f * vector_dot_product(direction, &q);
    
    if (v < 0.0f || (u + v) > 1.0f)
        goto ret_false;
    
    *t = f * vector_dot_product(&e2, &q);
    
    if (*t <= 0.0f)
        goto ret_false;
    
    return true;
    
ret_false:
    return false;
}

static int los_map_line_find_nearest_intersection_with_triangle_set(Vector* start, Vector* direction, Triangle* triangles, uint32_t n, float* distance)
{
    uint32_t i;
    float t;
    float d = 999999.0f;
    int ret = false;
    
    for (i = 0; i < n; i++)
    {
        if (los_map_line_intersects_triangle(start, direction, &triangles[i], &t))
        {
            if (t < d)
                d = t;
            
            ret = true;
        }
    }
    
    if (ret)
        *distance = d;
    
    return ret;
}

int los_map_points_are_in_line_of_sight(ZC* zc, LineOfSightMap* map, float ax, float ay, float az, float bx, float by, float bz)
{
    uint64_t time = clock_microseconds();
    Array* stack                = map->recursionStack;
    LineOfSightBspNode* nodes   = map->nodes;
    LineOfSightBspNode* ptr;
    Vector start;
    Vector direction;
    float distance;
    float temp;
    
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
    
    if (!nodes)
        goto ret_true;
    
    vector_set(&start, ax, ay, az);
    vector_set(&direction, bx, by, bz);
    
    vector_difference(&direction, &direction, &start);
    distance = vector_length(&direction);
    
    if (distance < FLOAT_EPSILON)
        goto ret_true;
    
    temp = 1.0f / distance;
    vector_multiply_scalar(&direction, temp);
    
    array_push_back(B(zc), &stack, (void*)&nodes);
    
    while (!array_empty(stack))
    {
        uint32_t n;
        LineOfSightBspNode* node = *array_back_type(stack, LineOfSightBspNode*);
        array_pop_back(stack);
        
        if (!aabb_intersected_by_line_segment(&node->bounds, &start, &direction, distance, NULL))
            continue;
        
        n = node->triangleCount;
        
        x++;
        y += n;
        
        if (n > 0)
        {
            uint32_t i;
            float t;
            
            for (i = 0; i < n; i++)
            {
                if (los_map_line_intersects_triangle(&start, &direction, &node->triangles[i], &t))
                    goto ret_false;
            }
        }
        
        if (node->rightIndex)
        {
            ptr = &nodes[node->rightIndex];
            array_push_back(B(zc), &stack, (void*)&ptr);
        }
        
        if (node->leftIndex)
        {
            ptr = &nodes[node->leftIndex];
            array_push_back(B(zc), &stack, (void*)&ptr);
        }
    }
    
    map->recursionStack = stack;
    
ret_true:
    printf("time: %lu, x: %u, y: %u, z: %u\n", clock_microseconds() - time, x, y, z);
    return true;
    
ret_false:
    array_clear(stack);
    map->recursionStack = stack;
    printf("time: %lu, x: %u, y: %u\n", clock_microseconds() - time, x, y);
    return false;
}

float los_map_get_best_z(LineOfSightMap* map, float x, float y, float z)
{
    uint32_t indexX;
    uint32_t indexY;
    uint32_t index;
    Vector start;
    Vector direction;
    float distance;
    MapFileZSlice* slice;
    Triangle* triangles;
    uint32_t n;
    
    uint64_t time = clock_microseconds();
    
    indexX  = (uint32_t)((x - map->minX) / map->incrementX);
    indexY  = (uint32_t)((y - map->minY) / map->incrementY);
    index   = indexX * 256 + indexY;
    
    if (index >= EQP_ZSLICE_COUNT || !map->zSlices)
        goto ret_current;
    
    slice   = &map->zSlices[index];
    n       = slice->triangleCount;
    
    if (n == 0)
        goto ret_current;
    
    triangles = map->zSliceTriangles + slice->triangleIndex;
    
    vector_set(&start, x, y, z);
    vector_set(&direction, 0.0f, 0.0f, -1.0f); // Negative Z unit vector
    
    if (los_map_line_find_nearest_intersection_with_triangle_set(&start, &direction, triangles, n, &distance))
    {
        z += distance * -1.0f;
    }
    else
    {
        // Nothing below us, check above
        direction.z = 1.0f; // Positive Z unit vector
        
        if (los_map_line_find_nearest_intersection_with_triangle_set(&start, &direction, triangles, n, &distance))
        {
            z += distance;
        }
    }
    
    printf("time: %lu, z: %f, tris: %u\n", clock_microseconds() - time, z, n);
ret_current:
    return z;
}
