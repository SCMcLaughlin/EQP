
#include "output.h"

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

static void output_write_bsp_header(BspTree* bsp, FILE* fp, uint32_t length, float minZ)
{
    MapFileHeader header;
    
    header.signature        = EQP_MAP_GEN_MAP_FILE_SIGNATURE;
    header.version          = 1;
    header.inflatedLength   = length;
    header.minZ             = minZ;
    header.triangleCount    = bsp->triangleCount;
    header.nodeCount        = array_count(bsp->nodes);
    
    fwrite(&header, 1, sizeof(header), fp);
}

static void output_write_bsp_nodes(BspTree* bsp, Aligned* w)
{
    BspNode* nodes      = array_data_type(bsp->nodes, BspNode);
    uint32_t n          = array_count(bsp->nodes);
    uint32_t triIndex   = 0;
    uint32_t i;
    
    // Triangles
    for (i = 0; i < n; i++)
    {
        BspNode* node   = &nodes[i];
        Triangle* tris  = node->triangles;
        
        if (tris)
            aligned_write_buffer(w, tris, sizeof(Triangle) * node->triangleCount);
    }
    
    // Nodes
    for (i = 0; i < n; i++)
    {
        BspNode* node   = &nodes[i];
        uint32_t m      = node->triangleCount;
        
        // triangleCount
        aligned_write_uint32(w, m);
        // leftIndex
        aligned_write_uint32(w, node->leftIndex);
        // rightIndex
        aligned_write_uint32(w, node->rightIndex);
        // bounds
        aligned_write_buffer(w, &node->bounds, sizeof(AABB));
        // triangleIndex
        aligned_write_uint32(w, triIndex);
            
        triIndex += m;
    }
}

static uint32_t output_bsp_calc_length(BspTree* bsp)
{
    return (sizeof(Triangle) * bsp->triangleCount) + (sizeof(MapFileBspNode) * array_count(bsp->nodes));
}

void output_bsp_to_file(BspTree* bsp, const char* zoneShortName, float minZ)
{
    Basic* basic    = bsp->basic;
    uint32_t length = output_bsp_calc_length(bsp);
    byte* buffer    = eqp_alloc_type_bytes(basic, length, byte);
    FILE* fp        = output_open_file(basic, zoneShortName);
    Aligned w;
    
    aligned_init(basic, &w, buffer, length);

    output_write_bsp_header(bsp, fp, length, minZ);
    output_write_bsp_nodes(bsp, &w);
    output_write_to_disk(basic, buffer, length, fp, zoneShortName);
    
    fclose(fp);
    free(buffer);
}

#if 0
static void output_write_octree_header(Octree* octree, FILE* fp, uint32_t length, float minZ)
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

static uint32_t output_octree_calc_length(Octree* octree)
{
    return (sizeof(Triangle) * octree->triangleCount) + (sizeof(MapFileBox) * array_count(octree->nodes));
}

void output_octree_to_file(Octree* octree, const char* fileName, float minZ)
{
    uint32_t length = output_octree_calc_length(octree);
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
#endif
