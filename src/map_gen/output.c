
#include "output.h"

static void output_write_header(Octree* octree, Aligned* w, float minZ)
{
    MapFileHeader header;
    
    header.signature    = EQP_MAP_GEN_MAP_FILE_SIGNATURE;
    header.version      = 1;
    header.boxCount     = array_count(octree->nodes);
    header.minZ         = minZ;
    
    aligned_write_buffer(w, &header, sizeof(header));
}

static void output_write_octree_nodes(Octree* octree, Aligned* w)
{
    OctreeNode* nodes   = array_data_type(octree->nodes, OctreeNode);
    uint32_t n          = array_count(octree->nodes);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        OctreeNode* node = &nodes[i];
        MapFileBox boxHeader;
        Triangle* tris  = array_data_type(node->triangles, Triangle);
        uint32_t m      = array_count(node->triangles);
        uint32_t j;
        
        boxHeader.center        = aabb_get_center(&node->box);
        boxHeader.halfExtent    = aabb_get_half_extent(&node->box);
        boxHeader.triangleCount = m;
        
        aligned_write_buffer(w, &boxHeader, sizeof(boxHeader));
        
        for (j = 0; j < m; j++)
        {
            aligned_write_buffer(w, &tris[j], sizeof(Triangle));
        }
    }
}

static void output_write_to_disk(Basic* basic, byte* buffer, uint32_t length, const char* zoneShortName)
{
    unsigned long compLen   = compressBound(length);
    byte* compressed        = eqp_alloc_type_bytes(basic, compLen, byte);
    const char* binPath     = getenv(EQP_MAP_GEN_PATH_ENV_VARIABLE);
    char path[1024];
    int len;
    FILE* fp;
    
    if (!binPath)
        binPath = ".";
    
    len = snprintf(path, sizeof(path), "%s/maps/%s", binPath, zoneShortName);
    snprintf(path + len - 4, sizeof(path) - len + 4, ".eqpmap");
    
    fp = fopen(path, "wb+");
    
    if (!fp)
        exception_throw_format(basic, ErrorDoesNotExist, "Could not open file '%s' for writing", path);
    
    if (compress2(compressed, &compLen, buffer, length, Z_BEST_COMPRESSION))
    {
        fclose(fp);
        exception_throw_format(basic, ErrorCompression, "Compression failed for '%s'", path);
    }
    
    fwrite(compressed, 1, compLen, fp);
    fclose(fp);
    
    free(compressed);
}

static uint32_t output_calc_length(Octree* octree)
{
    OctreeNode* nodes   = array_data_type(octree->nodes, OctreeNode);
    uint32_t n          = array_count(octree->nodes);
    uint32_t len        = sizeof(MapFileHeader);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        len += sizeof(MapFileBox) + (sizeof(Triangle) * array_count(nodes[i].triangles));
    }
    
    return len;
}

void output_to_file(Octree* octree, const char* fileName, float minZ)
{
    uint32_t length = output_calc_length(octree);
    byte* buffer    = eqp_alloc_type_bytes(octree->basic, length, byte);
    Aligned write;
    Aligned* w = &write;
    
    aligned_init(octree->basic, w, buffer, length);

    output_write_header(octree, w, minZ);
    output_write_octree_nodes(octree, w);
    output_write_to_disk(octree->basic, buffer, length, fileName);
    
    free(buffer);
}
