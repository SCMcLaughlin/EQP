
#include "map_gen.h"

static void map_gen_init_arrays(MapGen* map)
{
    map->vertices = array_create_type(B(map), Vertex);
    octree_init(B(map), &map->octree);
}

void map_gen_init(MapGen* map)
{
    basic_init(B(map), 0, NULL);
    map_gen_init_arrays(map);
}

void map_gen_deinit(MapGen* map)
{
    if (map->vertices)
    {
        array_destroy(map->vertices);
        map->vertices = NULL;
    }
    
    octree_deinit(&map->octree);
}

void map_gen_reset(MapGen* map)
{
    map_gen_deinit(map);
    map_gen_init_arrays(map);
}

static const char* map_gen_file_name(const char* src, int len)
{
    while (len >= 0)
    {
        char c = src[len];
        
        if (c == '/' || c == '\\')
            break;
        
        len--;
    }
    
    return src + len + 1;
}

static void map_gen_read_s3d_zone(MapGen* map, const char* path, Pfs* pfs, float* minZ)
{
    char buf[2048];
    const char* name;
    Pfs objDefsPfs;
    Wld* objDefs;
    byte* data;
    int blen;
    uint32_t len;
    
    blen = snprintf(buf, sizeof(buf), "%s", path);
    snprintf(buf + blen - 4, sizeof(buf) - blen + 4, ".wld");
    name = map_gen_file_name(buf, blen);

    // Main zone geometry
    data = pfs_get_file_by_name(pfs, name, &len);
    wld_read_zone_vertices(map, data, len, minZ);
    free(data);
    
    // Object definitions, from a separate pfs archive
    snprintf(buf + blen - 4, sizeof(buf) - blen + 4, "_obj.s3d");
    pfs_open(B(map), &objDefsPfs, buf);
    
    snprintf(buf + blen, sizeof(buf) - blen, ".wld");
    name = map_gen_file_name(buf, blen);
    
    data = pfs_get_file_by_name(&objDefsPfs, name, &len);
    objDefs = wld_read_object_definitions(map, data, len);
    
    // Object placements
    data = pfs_get_file_by_name(pfs, "objects.wld", &len);
    wld_read_object_placements(map, objDefs, data, len);
    free(data);
    
    wld_destroy(objDefs);
    pfs_close(&objDefsPfs);
}

void map_gen_read_all_zones(MapGen* map, const char* dirPath)
{
    uint64_t totalTime = clock_milliseconds();
#ifdef EQP_WINDOWS
    
#else
    DIR* dir = opendir(dirPath);
    struct dirent entry;
    struct dirent* result;
    char path[1024];
    int len;
    
    if (!dir)
        exception_throw_format(B(map), ErrorDoesNotExist, "Directory '%s' does not exist", dirPath);
    
    len = snprintf(path, sizeof(path), "%s/", dirPath);
    
    while (!readdir_r(dir, &entry, &result))
    {
        if (result == NULL)
            break;
        
        if (!strstr(entry.d_name, ".s3d"))
            continue;
        
        if (strstr(entry.d_name, "_obj") || strstr(entry.d_name, "_chr") || strstr(entry.d_name, "_lit"))
            continue;
        
        snprintf(path + len, sizeof(path) - len, "%s", entry.d_name);
        
        map_gen_read_vertices(map, path);
        map_gen_reset(map);
    }

    closedir(dir);
#endif
    
    totalTime = clock_milliseconds() - totalTime;
    printf("Total time: %lu.%lu seconds\n", totalTime / 1000, totalTime % 1000);
}

void map_gen_read_single_zone(MapGen* map, const char* dirPath, const char* fileName)
{
    char path[1024];
    
    snprintf(path, sizeof(path), "%s/%s", dirPath, fileName);
    
    map_gen_read_vertices(map, path);
}

void map_gen_read_vertices(MapGen* map, const char* path)
{
    Pfs pfs;
    float minZ      = -32000.0f;
    uint64_t time   = clock_milliseconds();
    int isZone      = false;
    
    pfs_open(B(map), &pfs, path);
    
    if (pfs_is_s3d_zone(&pfs))
    {
        map_gen_read_s3d_zone(map, path, &pfs, &minZ);
        isZone = true;
    }

    pfs_close(&pfs);
    
    if (!isZone)
        return;
    
    octree_generate(&map->octree, map->vertices, EQP_MAP_GEN_DEFAULT_TRIANGLES_PER_OCTREE_NODE);
    output_to_file(&map->octree, map_gen_file_name(path, strlen(path)), minZ);
    
    printf("%-19s | %8u triangles | %5u nodes | %7lu milliseconds\n", map_gen_file_name(path, strlen(path)),
        array_count(map->vertices) / 3, array_count(map->octree.nodes), clock_milliseconds() - time);
}

void map_gen_add_object_placement(MapGen* map, Array* object, Matrix* matrix)
{
    Vertex* src = array_data_type(object, Vertex);
    uint32_t n  = array_count(object);
    uint32_t i;
    Vertex dst;
    
    for (i = 0; i < n; i++)
    {
        matrix_transform_vertex(matrix, &dst, &src[i]);
        array_push_back(B(map), &map->vertices, &dst);
    }
}
