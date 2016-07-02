
#include "octree.h"
#include "eqp_basic.h"

STRUCT_DEFINE(OctreeStack)
{
    AABB        box;
    uint32_t    claimId;
    int         terminal;
};

void octree_init(Basic* basic, Octree* octree)
{
    octree->nodes = array_create_type(basic, OctreeNode);
    octree->basic = basic;
}

void octree_deinit(Octree* octree)
{
    if (octree->nodes)
    {
        OctreeNode* array   = array_data_type(octree->nodes, OctreeNode);
        uint32_t n          = array_count(octree->nodes);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            array_destroy(array[i].triangles);
        }
        
        array_destroy(octree->nodes);
        octree->nodes = NULL;
    }
}

static void octree_recurse(Basic* basic, Array** stack, Triangle* triangles, uint32_t trianglesCount, uint32_t* claims, uint32_t maxTrianglesPerNode)
{
    AABB box;
    Vertex corners[8];
    Vertex mid;
    uint32_t curIndex = 0;
    uint32_t topIndex = 0;
    
    for (;;)
    {
        OctreeStack* cur = array_get_type(*stack, curIndex++, OctreeStack);
        uint32_t parentId;
        uint32_t c;
        
        if (cur->terminal)
            goto check_end;
        
        parentId    = cur->claimId;
        box         = cur->box;
        
        aabb_get_all_corners_and_center(&box, corners, &mid);
        
        for (c = 0; c < 8; c++)
        {
            uint32_t triCount   = 0;
            uint32_t claim      = topIndex + 1;
            uint32_t i;
            
            aabb_init_from_corners(&box, &mid, &corners[c]);
            
            for (i = 0; i < trianglesCount; i++)
            {
                Triangle* tri;
                uint32_t j;
                
                if (claims[i] != parentId)
                    continue;
                
                tri = &triangles[i];
                
                for (j = 0; j < 3; j++)
                {
                    if (!aabb_contains_point(&box, &tri->points[j]))
                        goto skip;
                }
                
                claims[i] = claim;
                triCount++;
                
            skip: ;
            }
            
            if (triCount)
            {
                OctreeStack add;
                
                add.box         = box;
                add.claimId     = claim;
                add.terminal    = (triCount <= maxTrianglesPerNode);
                
                array_push_back(basic, stack, &add);
                
                topIndex++;
            }
        }
        
    check_end:
        if (curIndex >= topIndex)
            break;
    }
}

static void octree_create_nodes(Octree* octree, Array* stack, Triangle* triangles, uint32_t triCount, uint32_t* claims)
{
    Basic* basic    = octree->basic;
    uint32_t n      = array_count(stack);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        Array* tris = array_create_type(basic, Triangle);
        uint32_t j;
        AABB box;
        OctreeNode node;
        
        aabb_init(&box);
        
        for (j = 0; j < triCount; j++)
        {
            if (claims[j] == i)
            {
                Triangle* tri = &triangles[j];
                
                aabb_add_internal_point(&box, tri->points[0].x, tri->points[0].y, tri->points[0].z);
                aabb_add_internal_point(&box, tri->points[1].x, tri->points[1].y, tri->points[1].z);
                aabb_add_internal_point(&box, tri->points[2].x, tri->points[2].y, tri->points[2].z);
                
                array_push_back(basic, &tris, tri);
            }
        }
        
        node.triangles  = tris;
        node.box        = box;
        
        array_push_back(basic, &octree->nodes, &node);
    }
}

void octree_generate(Octree* octree, Array* vertices, uint32_t maxTrianglesPerNode)
{
    Basic* basic    = octree->basic;
    Array* stack    = array_create_type(basic, OctreeStack);
    Vertex* verts   = array_data_type(vertices, Vertex);
    uint32_t n      = array_count(vertices);
    uint32_t i;
    OctreeStack top;
    uint32_t* claims;
    AABB box;
    
    aabb_init(&box);
    
    for (i = 0; i < n; i++)
    {
        Vertex* v = &verts[i];
        aabb_add_internal_point(&box, v->x, v->y, v->z);
    }
    
    n /= 3;
    
    octree->triangleCount = n;
    
    top.box         = box;
    top.claimId     = 0;
    top.terminal    = false;
    claims          = eqp_alloc_type_array(basic, n, uint32_t);
    
    memset(claims, 0, sizeof(uint32_t) * n);
    
    array_push_back(basic, &stack, &top);
    octree_recurse(basic, &stack, (Triangle*)verts, n, claims, maxTrianglesPerNode);
    
    octree_create_nodes(octree, stack, (Triangle*)verts, n, claims);
    
    array_destroy(stack);
    free(claims);
}
