
#include "wld.h"
#include "map_gen.h"

#define VERSION1    0x00015500
#define VERSION2    0x1000C800
#define SIGNATURE   0x54503d02

STRUCT_DEFINE(Wld_Header)
{
    uint32_t    signature;
    uint32_t    version;
    uint32_t    fragCount;
    uint32_t    unknownA[2];
    uint32_t    stringsLength;
    uint32_t    unknownB;
};

typedef void (*WldFragReader)(MapGen*, Wld*, Fragment* frag, Array**);

static void wld_decode_string(char* str, uint32_t len)
{
    static const byte hash[8] = {0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A};
    
    for (uint32_t i = 0; i < len; i++)
    {
        str[i] ^= hash[i & 7];
        str[i] = tolower(str[i]);
    }
}

static const char* wld_frag_name_by_ref(Wld* wld, int ref)
{
    if (ref < 0 && ref > -((int)wld->stringsLength))
        return wld->stringBlock - ref;
    
    return NULL;
}

static const char* wld_frag_name(Wld* wld, Fragment* frag)
{
    return wld_frag_name_by_ref(wld, frag->nameRef);
}

static void wld_read_frags(MapGen* map, Wld* wld, byte* data, uint32_t len, WldFragReader callback, Array** array, int decodeStrings)
{
    Wld_Header* h   = (Wld_Header*)data;
    uint32_t p      = sizeof(Wld_Header);
    uint32_t version;
    uint32_t n;
    uint32_t i;
    
    if (p > len)
        exception_throw(B(map), ErrorOutOfBounds);
    
    if (h->signature != SIGNATURE)
        exception_throw(B(map), ErrorMismatch);
    
    version = h->version & 0xfffffffe;
    
    if (version != VERSION1 && version != VERSION2)
        exception_throw(B(map), ErrorMismatch);
    
    wld->version        = version;
    wld->stringBlock    = (char*)(data + p);
    wld->stringsLength  = h->stringsLength;
    n                   = h->fragCount;
    
    p += h->stringsLength;
    
    if (p > len)
        exception_throw(B(map), ErrorOutOfBounds);
    
    if (decodeStrings)
        wld_decode_string(wld->stringBlock, wld->stringsLength);
    
    for (i = 0; i < n; i++)
    {
        Fragment* frag = (Fragment*)(data + p);
        p += frag->length + EQP_WLD_FRAG_HEADER_LENGTH;
        
        if (p > len)
            exception_throw(B(map), ErrorOutOfBounds);
        
        callback(map, wld, frag, array);
    }
}

static void wld_read_zone_vertices_callback(MapGen* map, Wld* wld, Fragment* frag, Array** vertArray)
{
    Frag36* f36;
    float cx, cy, cz;
    float scale;
    uint32_t p;
    uint32_t i;
    uint32_t n;
    uint32_t len;
    Wld_Vertex* verts;
    Wld_Triangle* tris;
    byte* data;
    uint32_t vcount;
    uint32_t tcount;
    
    if (frag->type != 0x36)
        return;
    
    p   = sizeof(Frag36);
    len = frag->length + EQP_WLD_FRAG_HEADER_LENGTH;
    
    if (p > len)
        exception_throw(B(map), ErrorOutOfBounds);
    
    len     = frag->length + EQP_WLD_FRAG_HEADER_LENGTH;
    f36     = (Frag36*)frag;
    data    = (byte*)f36;
    cx      = f36->x;
    cy      = f36->y;
    cz      = f36->z;
    n       = f36->polyTextureCount;
    scale   = 1.0f / (float)(1 << f36->scale);
    vcount  = f36->vertCount;
    tcount  = f36->polyCount;
    
    verts = (Wld_Vertex*)(data + p);
    p += sizeof(Wld_Vertex) * vcount;
    
    if (wld->version == VERSION1)
        p += sizeof(Wld_Uv16) * f36->uvCount;
    else
        p += sizeof(Wld_Uv32) * f36->uvCount;
    
    p += sizeof(Wld_Normal) * vcount + sizeof(Wld_Color) * f36->colorCount;
    
    tris = (Wld_Triangle*)(data + p);
    p += sizeof(Wld_Triangle) * tcount;
    
    p += sizeof(Wld_BoneAssignment) * f36->boneAssignCount;
    
    if (p > len)
        exception_throw(B(map), ErrorOutOfBounds);
    
    for (i = 0; i < n; i++)
    {
        uint32_t j, m;
        Wld_TextureEntry* te = (Wld_TextureEntry*)(data + p);
        p += sizeof(Wld_TextureEntry);
        
        if (p > len)
            exception_throw(B(map), ErrorOutOfBounds);
        
        m = te->count;
        
        if (m > tcount)
            exception_throw(B(map), ErrorOutOfBounds);
        
        for (j = 0; j < m; j++)
        {
            Wld_Triangle tri = tris[j];
            Vertex v;
            int k;
            
            if (tri.flag & EQP_WLD_TRIANGLE_FLAG_PERMEABLE)
                continue;
            
            for (k = 2; k >= 0; k--)
            {
                uint16_t index = tri.index[k];
                Wld_Vertex vert;
                
                if (index >= vcount)
                    exception_throw(B(map), ErrorOutOfBounds);
                
                vert = verts[index];
                
                v.x = cx + (float)vert.x * scale;
                v.z = cy + (float)vert.y * scale;
                v.y = cz + (float)vert.z * scale;
                
                array_push_back(B(map), vertArray, &v);
            }
        }
        
        tris += m;
        tcount -= m;
    }
}

static void wld_read_object_definitions_callback(MapGen* map, Wld* wld, Fragment* frag, Array** unused)
{
    (void)unused;
    Array* array;
    char buf[256];
    const char* name;
    int len;
    
    if (frag->type != 0x36)
        return;
    
    name = wld_frag_name(wld, frag);
    
    if (!name)
        return;

    len = snprintf(buf, sizeof(buf), "%s", name);
    snprintf(buf + len - 12, sizeof(buf) - len + 12, "_actordef");

    array = array_create_type(B(map), Vertex);
    wld_read_zone_vertices_callback(map, wld, frag, &array);
    
    if (array_count(array) > 0)
        hash_table_set_by_cstr(B(map), &wld->objectDefsByName, buf, strlen(buf), (void*)&array);
    else
        array_destroy(array);
}

static void wld_read_object_placements_callback(MapGen* map, Wld* wld, Fragment* frag, Array** unused)
{
    (void)unused;
    Frag15* f15;
    const char* modelName;
    Array** model;
    float rotY;
    float rotZ;
    float scale;
    Matrix matrix;
    
    if (frag->type != 0x15)
        return;
    
    f15         = (Frag15*)frag;
    modelName   = wld_frag_name_by_ref(wld, f15->modelNameRef);
    
    if (!modelName)
        return;
    
    model = hash_table_get_type_by_cstr(wld->objDefs->objectDefsByName, modelName, strlen(modelName), Array*);
    
    if (!model)
        return;
    
    rotY = -(f15->rotX / 512.0f * 360.0f);
    rotZ =  (f15->rotY / 512.0f * 360.0f);
    
    matrix = matrix_angle_yz(rotY, rotZ);
    matrix_set_translation(&matrix, f15->x, f15->z, f15->y);
    
    scale = f15->scaleZ;
    
    if (scale != 0.0f && scale != 1.0f)
    {
        Matrix sc   = matrix_scale(scale);
        matrix      = matrix_mult(&matrix, &sc);
    }
    
    map_gen_add_object_placement(map, *model, &matrix);
}

void wld_read_zone_vertices(MapGen* map, byte* data, uint32_t len)
{
    Wld wld;
    
    wld_read_frags(map, &wld, data, len, wld_read_zone_vertices_callback, &map->vertices, false);
}

Wld* wld_read_object_definitions(MapGen* map, byte* data, uint32_t len)
{
    Wld* wld = eqp_alloc_type(B(map), Wld);
    
    wld->objectDefsByName   = hash_table_create_type(B(map), Array*);
    wld->data               = data;
    wld->length             = len;
    
    wld_read_frags(map, wld, data, len, wld_read_object_definitions_callback, NULL, true);
    
    return wld;
}

void wld_read_object_placements(MapGen* map, Wld* objDefs, byte* data, uint32_t len)
{
    Wld wld;
    
    wld.objDefs = objDefs;
    
    wld_read_frags(map, &wld, data, len, wld_read_object_placements_callback, NULL, true);
}

static void wld_free_object_def(void* def)
{
    Array** object = (Array**)def;
    array_destroy(*object);
}

void wld_destroy(Wld* wld)
{
    if (wld->objectDefsByName)
    {
        hash_table_for_each(wld->objectDefsByName, wld_free_object_def);
        hash_table_destroy(wld->objectDefsByName);
        wld->objectDefsByName = NULL;
    }
    
    if (wld->data)
    {
        free(wld->data);
        wld->data = NULL;
    }
    
    free(wld);
}

#undef VERSION1
#undef VERSION2
#undef SIGNATURE
