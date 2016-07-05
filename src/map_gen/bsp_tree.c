
#include "bsp_tree.h"

#define MIN_AXIS_LENGTH 0.01f
#define MAX_DEPTH       16

STRUCT_DEFINE(BspStack)
{
    int     index;
    int     depth;
    Array*  triangles;
};

void bsp_tree_init(Basic* basic, BspTree* bsp)
{
    bsp->nodes          = array_create_type(basic, BspNode);
    bsp->stack          = array_create_type(basic, BspStack);
    bsp->basic          = basic;
    bsp->triangleCount  = 0;
}

void bsp_tree_deinit(BspTree* bsp)
{
    if (bsp->nodes)
    {
        BspNode* array  = array_data_type(bsp->nodes, BspNode);
        uint32_t n      = array_count(bsp->nodes);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            BspNode* node = &array[i];
            
            if (node->triangles)
                free(node->triangles);
        }
        
        array_destroy(bsp->nodes);
        bsp->nodes = NULL;
    }
    
    if (bsp->stack)
    {
        BspStack* array = array_data_type(bsp->stack, BspStack);
        uint32_t n      = array_count(bsp->stack);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            array_destroy(array[i].triangles);
        }
        
        array_destroy(bsp->stack);
        bsp->stack = NULL;
    }
}

static uint32_t bsp_tree_add_node(BspTree* bsp, AABB* box, Array* triangles, int parentDepth)
{
    BspStack stackEntry;
    BspNode node;
    uint32_t index;
    
    // Node
    memset(&node, 0, sizeof(node));
    node.bounds = *box;
    
    index = array_count(bsp->nodes);
    array_push_back(bsp->basic, &bsp->nodes, &node);
    
    // Stack
    stackEntry.index        = index;
    stackEntry.depth        = parentDepth + 1;
    stackEntry.triangles    = triangles;
    
    array_push_back(bsp->basic, &bsp->stack, &stackEntry);
    
    return index;
}

static void bsp_tree_recurse(BspTree* bsp)
{
    Basic* basic = bsp->basic;

    while (!array_empty(bsp->stack))
    {
        BspStack se         = *array_get_type(bsp->stack, 0, BspStack);
        BspNode* node       = array_get_type(bsp->nodes, se.index, BspNode);
        Vector vec          = aabb_get_extent(&node->bounds);
        Triangle* triangles = array_data_type(se.triangles, Triangle);
        uint32_t n          = array_count(se.triangles);
        AABB b1, b2, leftBounds, rightBounds;
        Array* leftTris;
        Array* rightTris;
        float axisLength;
        int axisId;
        uint32_t i;
        
        // Find the longest axis
        axisId      = AABB_AxisX;
        axisLength  = vec.x;
        
        if (vec.y > vec.x)
        {
            axisId      = AABB_AxisY;
            axisLength  = vec.y;
        }
        
        if (vec.z > vec.x && vec.z > vec.y)
        {
            axisId      = AABB_AxisZ;
            axisLength  = vec.z;
        }
        
        if (n <= EQP_BSP_MAX_TRIANGLES_PER_NODE || axisLength <= MIN_AXIS_LENGTH || se.depth >= MAX_DEPTH)
        {
            Triangle* tris      = eqp_alloc_type_array(basic, n, Triangle);
            node->triangles     = tris;
            node->triangleCount = n;
        
            bsp->triangleCount += n;
            
            memcpy(tris, array_data(se.triangles), sizeof(Triangle) * n);

            goto next;
        }
        
        aabb_split(&node->bounds, axisId, &b1, &b2);
        
        leftTris    = array_create_type(basic, Triangle);
        rightTris   = array_create_type(basic, Triangle);
        
        aabb_init(&leftBounds);
        aabb_init(&rightBounds);
        
        for (i = 0; i < n; i++)
        {
            Triangle* tri = &triangles[i];
            uint32_t orCode;
            
            if (aabb_contains_triangle_precise(&b1, tri, &orCode))
            {
                aabb_add_internal_point_vector(&leftBounds, &tri->points[0]);
                aabb_add_internal_point_vector(&leftBounds, &tri->points[1]);
                aabb_add_internal_point_vector(&leftBounds, &tri->points[2]);
                
                array_push_back(basic, &leftTris, tri);
            }
            
            if (orCode && aabb_contains_triangle_precise(&b2, tri, &orCode))
            {
                aabb_add_internal_point_vector(&rightBounds, &tri->points[0]);
                aabb_add_internal_point_vector(&rightBounds, &tri->points[1]);
                aabb_add_internal_point_vector(&rightBounds, &tri->points[2]);
                
                array_push_back(basic, &rightTris, tri);
            }
        }
        
        if (!array_is_empty(leftTris))
        {
            uint32_t index;
            aabb_clamp(&leftBounds, &b1);
            index   = bsp_tree_add_node(bsp, &leftBounds, leftTris, se.depth); // This call may relocate the nodes array
            node    = array_get_type(bsp->nodes, se.index, BspNode);
            node->leftIndex = index;
        }
        else
        {
            array_destroy(leftTris);
        }
        
        if (!array_is_empty(rightTris))
        {
            uint32_t index;
            aabb_clamp(&rightBounds, &b2);
            index   = bsp_tree_add_node(bsp, &rightBounds, rightTris, se.depth); // This call may relocate the nodes array
            node    = array_get_type(bsp->nodes, se.index, BspNode);
            node->rightIndex = index;
        }
        else
        {
            array_destroy(rightTris);
        }
        
    next:
        array_destroy(se.triangles);
        array_shift_left(bsp->stack, 1);
    }
}

void bsp_tree_generate(BspTree* bsp, Array* vertices, ZSlices* zSlices)
{
    Basic* basic        = bsp->basic;
    Array* triangles    = array_create_type(basic, Triangle);
    Triangle* tris      = (Triangle*)array_data_type(vertices, Vertex);
    uint32_t n          = array_count(vertices) / 3;
    uint32_t i;
    AABB box;
    
    aabb_init(&box);
    
    for (i = 0; i < n; i++)
    {
        Triangle* t = &tris[i];
        
        aabb_add_internal_point_vector(&box, &t->points[0]);
        aabb_add_internal_point_vector(&box, &t->points[1]);
        aabb_add_internal_point_vector(&box, &t->points[2]);
        
        array_push_back(basic, &triangles, t);
    }
    
    bsp_tree_add_node(bsp, &box, triangles, -1);
    bsp_tree_recurse(bsp);
    
    zslices_init(basic, zSlices, &box, tris, n);
}

#undef MIN_AXIS_LENGTH
#undef MAX_DEPTH
