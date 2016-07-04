
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
        // triangles
        aligned_read_buffer(a, &node->triangles, sizeof_field(LineOfSightBspNode, triangles));
        
        // extraTriangleIndex
        if (m > EQP_BSP_MAX_TRIANGLES_PER_NODE)
        {
            node->extraTriangles = triangles + aligned_read_uint32(a);
        }
        else
        {
            node->extraTriangles = NULL;
            aligned_advance(a, sizeof(uint32_t));
        }
    }
    
#if 0
    LineOfSightBox* boxes           = eqp_alloc_type_array(B(zc), header->boxCount, LineOfSightBox);
    LineOfSightTriangles* triSets   = eqp_alloc_type_array(B(zc), header->boxCount, LineOfSightTriangles);
    Triangle64* triangles           = eqp_alloc_type_array(B(zc), header->triangleCount, Triangle64);
    uint32_t triOffset              = 0;
    Aligned reader;
    Aligned* a = &reader;
    uint32_t n;
    uint32_t i;
    
    aligned_init(B(zc), a, data, length);
    
    // Triangles
    n = header->triangleCount;
    
    for (i = 0; i < n; i++)
    {
        Triangle64* t = &triangles[i];
        Triangle tri;
        
        aligned_read_buffer(a, &tri, sizeof(tri));
        
        t->points[0].x = tri.points[0].x;
        t->points[0].y = tri.points[0].y;
        t->points[0].z = tri.points[0].z;
        t->points[1].x = tri.points[1].x;
        t->points[1].y = tri.points[1].y;
        t->points[1].z = tri.points[1].z;
        t->points[2].x = tri.points[2].x;
        t->points[2].y = tri.points[2].y;
        t->points[2].z = tri.points[2].z;
        
        //los_map_calc_tri_normal(t);
    }
    
    // Boxes
    n = header->boxCount;
    
    for (i = 0; i < n; i++)
    {
        MapFileBox box;
        
        aligned_read_buffer(a, &box, sizeof(box));
        
        boxes[i].center         = box.center;
        boxes[i].halfExtent     = box.halfExtent;
        triSets[i].count        = box.triangleCount;
        triSets[i].triangles    = triangles + triOffset;
        
        triOffset += box.triangleCount;
    }
    
    map->boxCount       = header->boxCount;
    map->triangleCount  = header->triangleCount;
    map->boxes          = boxes;
    map->triangleSets   = triSets;
    map->triangles      = triangles;
#endif
    
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
        map->recursionStack = NULL;
        map->nodes          = NULL;
        map->triangles      = NULL;
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
}

#if 0
// Pass-by-copy is deliberate
static int los_map_point_is_on_triangle_side(Vector64* p1, Vector64* p2, Vector64 a, Vector64* b)
{
    Vector64 bmin;
    Vector64 cp1;
    Vector64 cp2;
    Vector64 diff;
    Vector64 temp;
    double result;
    
    bmin.x = b->x - a.x;
    bmin.y = b->y - a.y;
    bmin.z = b->z - a.z;
    
    diff.x = p1->x - a.x;
    diff.y = p1->y - a.y;
    diff.z = p1->z - a.z;
    
    temp.x = diff.x;
    temp.y = diff.y;
    temp.z = diff.z;
    
    cross_product(cp1, bmin, temp);
    
    temp.x = p2->x - a.x;
    temp.y = p2->y - a.y;
    temp.z = p2->z - a.z;
    
    cross_product(cp2, bmin, temp);
    
    result = dot_product(cp1, cp2);
    
    if (result >= 0.0)
    {
    ret_true:
        return true;
    }
    else
    {
        // Normalize bmin
        result = bmin.x * bmin.x + bmin.y * bmin.y + bmin.z * bmin.z;
        
        if (result != 0.0)
        {
            result = 1.0 / sqrt(result);
            
            bmin.x *= result;
            bmin.y *= result;
            bmin.z *= result;
        }
        
        // Normalize diff
        result = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
        
        if (result != 0.0)
        {
            result = 1.0 / sqrt(result);
            
            diff.x *= result;
            diff.y *= result;
            diff.z *= result;
        }
        
        // Cross product
        cp1.x = bmin.y * diff.z - bmin.z * diff.y;
        cp1.y = bmin.z * diff.x - bmin.x * diff.z;
        cp1.z = bmin.x * diff.y - bmin.y * diff.x;
        
        if (cp1.x < 0.0)
            cp1.x = -cp1.x;
        
        if (cp1.y < 0.0)
            cp1.y = -cp1.y;
        
        if (cp1.z < 0.0)
            cp1.z = -cp1.z;
        
        if (cp1.x < EPSILON && cp1.y < EPSILON && cp1.z < EPSILON)
            goto ret_true;
    }
    
    return false;
}

// Pass-by-copy is deliberate
static int los_map_line_intersects_triangles(LineOfSightTriangles* triSet, Vector64 lineVector, Vector64 start, Vector64 end, double lengthSquared)
{
    uint32_t n              = triSet->count;
    Triangle64* triangles   = triSet->triangles;
    uint32_t i;
    Triangle64 tri;
    double result;
    double d;
    double t;
    Vector64 temp;
    Vector64 intersect;
    
    //printf("tri count: %u\n", n);
    
    for (i = 0; i < n; i++)
    {
        // Check if the line intersects the triangle's plane
        tri     = triangles[i];
        result  = dot_product(tri.normal, lineVector);
        
        if (result < 0.0)
            result = -result;
        
        if (result < EPSILON)
            continue;
        
        // Find possible intersection point
        d = dot_product(tri.points[0], tri.normal);
        t = -(dot_product(tri.normal, start) - d) / result;
        
        intersect.x = start.x + (lineVector.x * t);
        intersect.y = start.y + (lineVector.y * t);
        intersect.z = start.z + (lineVector.z * t);
        
        // Check if the intersection point is actually on the line
        temp.x = intersect.x - start.x;
        temp.y = intersect.y - start.y;
        temp.z = intersect.z - start.z;
        
        d = temp.x * temp.x + temp.y * temp.y + temp.z * temp.z;
        
        if (d > lengthSquared)
            continue;
        
        temp.x = intersect.x - end.x;
        temp.y = intersect.y - end.y;
        temp.z = intersect.z - end.z;
        
        d = temp.x * temp.x + temp.y * temp.y + temp.z * temp.z;
        
        if (d > lengthSquared)
            continue;
        
        // Check if the point is actually within the triangle
        if (!los_map_point_is_on_triangle_side(&intersect, &tri.points[0], tri.points[1], &tri.points[2]))
            continue;
        
        if (!los_map_point_is_on_triangle_side(&intersect, &tri.points[1], tri.points[0], &tri.points[2]))
            continue;
        
        if (!los_map_point_is_on_triangle_side(&intersect, &tri.points[2], tri.points[0], &tri.points[1]))
            continue;
        
        //printf("intersect: %g, %g, %g\n", intersect.x, intersect.y, intersect.z);
        return true;
    }
    
    return false;
}
#endif

#if 0
static int los_map_line_intersects_triangle(Vector64* start, Vector64* lineVector, Triangle64* tri, double* t)
{
    Vector64 e1, e2, h, s, q;
    double a, f, u, v;
    //uint64_t time = clock_microseconds();
    
    vector_difference(&e1, &tri->points[1], &tri->points[0]);
    vector_difference(&e2, &tri->points[2], &tri->points[0]);
    cross_product(&h, lineVector, &e2);
    a = dot_product(&e1, &h);
    
    if (a > -0.00001 && a < 0.00001)
        goto ret_false;
    
    f = 1.0 / a;
    vector_difference(&s, start, &tri->points[0]);
    u = f * dot_product(&s, &h);
    
    if (u < 0.0 || u > 1.0)
        goto ret_false;
    
    cross_product(&q, &s, &e1);
    v = f * dot_product(lineVector, &q);
    
    if (v < 0.0 || (u + v) > 1.0)
        goto ret_false;
    
    *t = f * dot_product(&e2, &q);
    
    if (*t <= 0.0)
        goto ret_false;
    
    //printf("math time: %lu\n", clock_microseconds() - time);
    return true;
    
ret_false:
    //printf("math time: %lu\n", clock_microseconds() - time);
    return false;
}

// Pass-by-copy is deliberate
static int los_map_line_intersects_triangles(LineOfSightTriangles* triSet, Vector64* lineVector, Vector64* start)
{
    uint32_t n              = triSet->count;
    Triangle64* triangles   = triSet->triangles;
    uint32_t i;
    Vector64 intersect;
    double t = 0.0;
    //uint64_t time = clock_microseconds();
    
    for (i = 0; i < n; i++)
    {
        if (!los_map_line_intersects_triangle(start, lineVector, &triangles[i], &t))
            continue;
        
        /*
        intersect.x = start->x + lineVector->x * t;
        intersect.y = start->y + lineVector->y * t;
        intersect.z = start->z + lineVector->z * t;
        
        printf("intersect: %g, %g, %g\n", intersect.x, intersect.y, intersect.z);
        */
        
        return true;
    }
    
    //printf("triangles %u access time %lu (%g)\n", n, clock_microseconds() - time, t);
    
    return false;
}
#endif

int los_map_points_are_in_line_of_sight(LineOfSightMap* map, float ax, float ay, float az, float bx, float by, float bz)
{
    return true;
#if 0
    LineOfSightTriangles* triangleSets;
    LineOfSightBox* boxes;
    uint32_t n = map->boxCount;
    uint32_t i;
    Vector64 start;
    Vector64 end;
    Vector64 lineMid;
    Vector64 lineVector;
    Vector64 lineVectorAbs;
    Vector64 lineVectorUnnormalized;
    double lineHalfLength;
    double lengthSquared;
    double temp;
    uint64_t time = clock_microseconds();
    
    if (n == 0)
        goto ret_true;
    
    boxes           = map->boxes;
    triangleSets    = map->triangleSets;
    
    start.x = ax;
    start.y = ay;
    start.z = az;
    
    end.x = bx;
    end.y = by;
    end.z = bz;
    
    // Line middle
    lineMid.x = (ax + bx) * 0.5;
    lineMid.y = (ay + by) * 0.5;
    lineMid.z = (az + bz) * 0.5;
    
    // Unnormalized vector
    lineVectorUnnormalized.x = bx - ax;
    lineVectorUnnormalized.y = by - ay;
    lineVectorUnnormalized.z = bz - az;
    
    // Normalized vector
    lineVector.x = lineVectorUnnormalized.x;
    lineVector.y = lineVectorUnnormalized.y;
    lineVector.z = lineVectorUnnormalized.z;
    
    lengthSquared   = lineVector.x * lineVector.x + lineVector.y * lineVector.y + lineVector.z * lineVector.z;
    temp            = sqrt(lengthSquared);
    
    // Detour for line half length
    lineHalfLength = temp * 0.5;
    // End detour
    
    if (temp != 0.0)
    {
        temp = 1.0 / temp;
        
        lineVector.x *= temp;
        lineVector.y *= temp;
        lineVector.z *= temp;
    }
    
    // Line vector absolute values
    lineVectorAbs.x = fabs(lineVector.x);
    lineVectorAbs.y = fabs(lineVector.y);
    lineVectorAbs.z = fabs(lineVector.z);
    
    for (i = 0; i < n; i++)
    {
        LineOfSightBox* box = &boxes[i];
        Vertex t            = box->center;
        Vertex e            = box->halfExtent;
        
        t.x -= lineMid.x;
        t.y -= lineMid.y;
        t.z -= lineMid.z;
        
        if (fabs(t.x) > (e.x + (lineHalfLength * lineVectorAbs.x)))
            continue;
        
        if (fabs(t.y) > (e.y + (lineHalfLength * lineVectorAbs.y)))
            continue;
        
        if (fabs(t.z) > (e.z + (lineHalfLength * lineVectorAbs.z)))
            continue;
        
        temp = (e.y * lineVectorAbs.z) + (e.z * lineVectorAbs.y);
        if (fabs((t.y * lineVector.z) - (t.z * lineVector.y)) > temp)
            continue;
        
        temp = (e.x * lineVectorAbs.z) + (e.z * lineVectorAbs.x);
        if (fabs((t.z * lineVector.x) - (t.x * lineVector.z)) > temp)
            continue;
        
        temp = (e.x * lineVectorAbs.y) + (e.y * lineVectorAbs.x);
        if (fabs((t.x * lineVector.y) - (t.y * lineVector.x)) > temp)
            continue;
        
        //printf("box %u\n", i);
        
        // If we reach here, the line intersects this box... time to check its triangles
        if (los_map_line_intersects_triangles(&triangleSets[i], &lineVector, &start))
        {
            time = clock_microseconds() - time;
            printf("los time: %lu\n", time);
            return false;
        }
    }
    
    printf("los time: %lu\n", clock_microseconds() - time);
    
ret_true:
    return true;
#endif
}

float los_map_get_best_z(LineOfSightMap* map, float x, float y, float z)
{
    (void)map;
    (void)x;
    (void)y;
    (void)z;
    
    return z;
}

#undef EPSILON
#undef dot_product
