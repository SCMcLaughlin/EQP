
#include "output.h"

static void output_write_header(Octree* octree, FILE* fp, uint32_t length, float minZ)
{
    MapFileHeader header;
    
    header.signature        = EQP_MAP_GEN_MAP_FILE_SIGNATURE;
    header.version          = 1;
    header.inflatedLength   = length;
    header.minZ             = minZ;
    header.triangleCount    = octree->triangleCount;
    header.boxCount         = array_count(octree->nodes);
    
    fwrite(&header, 1, sizeof(header), fp);
}

static void output_write_octree_nodes(Octree* octree, Aligned* w)
{
    OctreeNode* nodes   = array_data_type(octree->nodes, OctreeNode);
    uint32_t n          = array_count(octree->nodes);
    uint32_t i;
    
    // Triangles
    for (i = 0; i < n; i++)
    {
        OctreeNode* node    = &nodes[i];
        Triangle* tris      = array_data_type(node->triangles, Triangle);
        uint32_t m          = array_count(node->triangles);
        
        aligned_write_buffer(w, tris, sizeof(Triangle) * m);
    }
    
    // Boxes
    for (i = 0; i < n; i++)
    {
        OctreeNode* node = &nodes[i];
        MapFileBox boxHeader;
        
        boxHeader.center        = aabb_get_center(&node->box);
        boxHeader.halfExtent    = aabb_get_half_extent(&node->box);
        boxHeader.triangleCount = array_count(node->triangles);
        
        aligned_write_buffer(w, &boxHeader, sizeof(boxHeader));
    }
}

static void output_write_to_disk(Basic* basic, byte* buffer, uint32_t length, FILE* fp, const char* fileName)
{
    unsigned long compLen   = compressBound(length);
    byte* compressed        = eqp_alloc_type_bytes(basic, compLen, byte);
    
    if (compress2(compressed, &compLen, buffer, length, Z_BEST_COMPRESSION))
    {
        fclose(fp);
        exception_throw_format(basic, ErrorCompression, "Compression failed for '%s'", fileName);
    }
    
    fwrite(compressed, 1, compLen, fp);
    
    free(compressed);
}

static uint32_t output_calc_length(Octree* octree)
{
    return (sizeof(Triangle) * octree->triangleCount) + (sizeof(MapFileBox) * array_count(octree->nodes));
}

static FILE* output_open_file(Basic* basic, const char* zoneShortName)
{
    const char* binPath = getenv(EQP_MAP_GEN_PATH_ENV_VARIABLE);
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
    
    return fp;
}

void output_to_file(Octree* octree, const char* fileName, float minZ)
{
    uint32_t length = output_calc_length(octree);
    byte* buffer    = eqp_alloc_type_bytes(octree->basic, length, byte);
    FILE* fp        = output_open_file(octree->basic, fileName);
    Aligned w;
    
    aligned_init(octree->basic, &w, buffer, length);

    output_write_header(octree, fp, length, minZ);
    output_write_octree_nodes(octree, &w);
    output_write_to_disk(octree->basic, buffer, length, fp, fileName);
    
    fclose(fp);
    free(buffer);
}
