
#ifndef EQP_OUTPUT_H
#define EQP_OUTPUT_H

#include "define.h"
#include "octree.h"
#include "bsp_tree.h"
#include "geometry.h"
#include "map_file.h"
#include "aligned.h"
#include "aabb.h"
#include "exception.h"
#include "eqp_alloc.h"
#include <zlib.h>

#define EQP_MAP_GEN_PATH_ENV_VARIABLE   "EQP_PATH"

#if 0
void    output_octree_to_file(Octree* octree, const char* zoneShortName, float minZ);
#endif
void    output_bsp_to_file(BspTree* bsp, const char* zoneShortName, float minZ);

#endif//EQP_OUTPUT_H
