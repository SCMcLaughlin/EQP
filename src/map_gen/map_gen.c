
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

static void map_gen_read_s3d_zone(MapGen* map, const char* path, Pfs* pfs)
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
    wld_read_zone_vertices(map, data, len);
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

void map_gen_read_vertices(MapGen* map, const char* path)
{
    Pfs pfs;
    uint64_t time = clock_microseconds();
    
    pfs_open(B(map), &pfs, path);
    
    if (pfs_is_s3d_zone(&pfs))
        map_gen_read_s3d_zone(map, path, &pfs);
    
    pfs_close(&pfs);
    
    octree_generate(&map->octree, map->vertices, 512);
    
    printf("time: %lu microseconds\n", clock_microseconds() - time);
    printf("vert count: %u\n", array_count(map->vertices));
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
