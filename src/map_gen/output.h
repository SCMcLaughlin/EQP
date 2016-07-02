
#ifndef EQP_OUTPUT_H
#define EQP_OUTPUT_H

#include "define.h"
#include "octree.h"
#include "geometry.h"
#include "map_file.h"
#include "aligned.h"
#include "aabb.h"
#include "exception.h"
#include "eqp_alloc.h"
#include <zlib.h>

#define EQP_MAP_GEN_PATH_ENV_VARIABLE   "EQP_PATH"

void    output_to_file(Octree* octree, const char* zoneShortName, float minZ);

#endif//EQP_OUTPUT_H
