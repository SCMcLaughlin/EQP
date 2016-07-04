
#ifndef EQP_LINE_OF_SIGHT_MAP_H
#define EQP_LINE_OF_SIGHT_MAP_H

#include "define.h"
#include "eqp_alloc.h"
#include "geometry.h"
#include "map_file.h"
#include "file.h"
#include <zlib.h>

#define EQP_PATH_ENV_VARIABLE   "EQP_PATH"

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(Zone);

/*
    The "box" extents used for doing the rough octree node elimination calculations
    are kept separate from the triangles belonging to those nodes in order to speed
    up the node elimination checks by making better use of cache (almost twice as
    many nodes in one cache line compared to if they were unified).
    
    The assumption is that for a typical line of sight check, the vast majority of
    octree nodes will be excluded; in practice, this is a good assumption. May
    as well exclude them as fast as possible.
*/

#if 0
STRUCT_DEFINE(LineOfSightBox)
{
    Vertex      center;
    Vertex      halfExtent;
};

STRUCT_DEFINE(LineOfSightTriangles)
{
    uint32_t    count;
    Triangle64* triangles;
};

STRUCT_DEFINE(LineOfSightMap)
{
    uint32_t                boxCount;
    uint32_t                triangleCount;
    LineOfSightBox*         boxes;
    LineOfSightTriangles*   triangleSets;
    Triangle64*             triangles;
};
#endif

STRUCT_DEFINE(LineOfSightBspNode)
{
    uint32_t    triangleCount;
    uint32_t    leftIndex;
    uint32_t    rightIndex;
    AABB        bounds;
    Triangle    triangles[EQP_BSP_MAX_TRIANGLES_PER_NODE];
    Triangle*   extraTriangles;
};

STRUCT_DEFINE(LineOfSightMap)
{
    Array*          recursionStack;
    MapFileBspNode* nodes;
    Triangle*       triangles;
};

void    los_map_open(ZC* zc, Zone* zone, LineOfSightMap* map);
void    los_map_close(LineOfSightMap* map);

int     los_map_points_are_in_line_of_sight(LineOfSightMap* map, float ax, float ay, float az, float bx, float by, float bz);
float   los_map_get_best_z(LineOfSightMap* map, float x, float y, float z);

#endif//EQP_LINE_OF_SIGHT_MAP_H
