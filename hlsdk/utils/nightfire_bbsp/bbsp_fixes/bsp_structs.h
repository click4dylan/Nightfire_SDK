#pragma once
#include <Windows.h>
#include "vectors.h"
#include "winding.h"
#include "boundingbox.h"
#include <set>

/*
struct LeafType
{
    unsigned int type : 8 = 0;
    unsigned int flags : 30 = 0;
};
*/

enum surfaceflags_t
{
    CONTENTS_EMPTY    = (1 << 0),                                                                 // 1 (0x00000001)
    CONTENTS_SOLID    = (1 << 1),                                                                 // 2 (0x00000002)
    CONTENTS_SKY      = (1 << 2),                                                                 // 4 (0x00000004)
    SURFACEFLAG_KEEP  = (1 << 3),                                                                 // 8 (0x00000008)
    CONTENTS_PORTAL   = (1 << 4),                                                                 // 16 (0x00000010)
    SURFACEFLAG_NODRAW = (1 << 5),                                                                // 32 (0x00000020)
    CONTENTS_UNKNOWN2 = (1 << 6),                                                                 // 64 (0x00000040)
    CONTENTS_UNKNOWN3 = (1 << 7),                                                                 // 128 (0x00000080)
    CONTENTS_BSP      = (1 << 8),                                                                 // 256 (0x00000100)
    CONTENTS_DETAIL   = (1 << 9),                                                                 // 512 (0x00000200)
    CONTENTS_ORIGIN   = (1 << 10),                                                                // 1024 (0x00000400)
    FLAG_UNKNOWN      = (1 << 11),                                                                // 2048 (0x00000800)
    CONTENTS_HINTSKIP = (1 << 12),                                                                // 4096 (0x00001000)
    CONTENTS_UNKNOWN4 = (1 << 13),                                                                // 8192 (0x00002000)
    CONTENTS_UNKNOWN5 = (1 << 14),                                                                // 16384 (0x00004000)
    CONTENTS_UNKNOWN6 = (1 << 15),                                                                // 32768 (0x00008000)
    CONTENTS_CLIP     = (1 << 16),                                                                // 65536 (0x00010000)
    SURFACEFLAG_NOIMPACTS = (1 << 17),                                                            // 131072 (0x00020000)
    SURFACEFLAG_NODECALS = (1 << 18),                                                             // 262144 (0x00040000)
    CONTENTS_ITEMCLIP  = (1 << 19),                                                               // 524288 (0x00080000)
    CONTENTS_WATER    = (1 << 20),                                                                // 1048576 (0x00100000)
    CONTENTS_TRIGGER  =  (1 << 21)                                                                // 2097152 (0x00200000)
};

// bevel creation
#define FLAG_BEVEL         (SURFACEFLAG_NODRAW | CONTENTS_DETAIL | CONTENTS_BSP)                    // 800 (0x00000320)

// special textures flags
#define SURFACEFLAG_HINT    (CONTENTS_EMPTY | SURFACEFLAG_NODRAW)                                   // 33 (0x00000021)
#define BRUSHFLAG_HINT     (CONTENTS_BSP | CONTENTS_HINTSKIP)                                       // 4352 (0x00001100)
//#define SURFACEFLAG_NODRAW (SURFACEFLAG_NODRAW)                                                   // 32 (0x00000020)
#define BRUSHFLAG_NODRAW 0
#define SURFACEFLAG_BSP     (CONTENTS_EMPTY | SURFACEFLAG_NODRAW)                                   // 33 (0x00000021)
#define BRUSHFLAG_BSP      CONTENTS_BSP                                                             // 256 (0x00000100)
#define SURFACEFLAG_PORTAL  (CONTENTS_EMPTY | CONTENTS_PORTAL | SURFACEFLAG_NODRAW)                 // 49 (0x00000031)
#define BRUSHFLAG_PORTAL   CONTENTS_BSP                                                             // 256 (0x00000100)
#define SURFACEFLAG_SKIP    (SURFACEFLAG_NODRAW | CONTENTS_DETAIL)                                  // 544 (0x00000220)
#define BRUSHFLAG_SKIP     CONTENTS_HINTSKIP                                                        // 4096 (0x00001000)
#define SURFACEFLAG_OPAQUE  SURFACEFLAG_NODRAW                                                      // 32 (0x00000020)
#define BRUSHFLAG_OPAQUE   CONTENTS_DETAIL                                                          // 512 (0x00000200)
#define SURFACEFLAG_CLIP    SURFACEFLAG_NODRAW                                                      // 32 (0x00000020)
#define BRUSHFLAG_CLIP     (CONTENTS_DETAIL | CONTENTS_CLIP | SURFACEFLAG_NOIMPACTS | SURFACEFLAG_NODECALS) // 459264 (0x00070200)
#define SURFACEFLAG_NPCCLIP SURFACEFLAG_NODRAW                                                      // 32 (0x00000020)
#define BRUSHFLAG_NPCCLIP  (CONTENTS_DETAIL | SURFACEFLAG_NODECALS)                                 // 262656 (0x00040200)
#define SURFACEFLAG_PLAYERCLIP SURFACEFLAG_NODRAW                                                   // 32 (0x00000020)
#define BRUSHFLAG_PLAYERCLIP (CONTENTS_DETAIL | CONTENTS_CLIP)                                      // 66048 (0x00010200)
#define SURFACEFLAG_ENEMYCLIP SURFACEFLAG_NODRAW                                                    // 32 (0x00000020)
#define BRUSHFLAG_ENEMYCLIP (CONTENTS_DETAIL | SURFACEFLAG_NOIMPACTS)                               // 131584 (0x00020020)
#define SURFACEFLAG_ITEMCLIP SURFACEFLAG_NODRAW                                                     // 32 (0x00000020)
#define BRUSHFLAG_ITEMCLIP (CONTENTS_DETAIL | CONTENTS_ITEMCLIP)                                    // 524800 (0x00080020)
#define SURFACEFLAG_ORIGIN  SURFACEFLAG_NODRAW                                                      // 32 (0x00000020)
#define BRUSHFLAG_ORIGIN   CONTENTS_ORIGIN                                                          // 1024 (0x00000400)
#define SURFACEFLAG_SKY     (CONTENTS_SKY | SURFACEFLAG_NOIMPACTS | SURFACEFLAG_NODECALS)           // 262148 (0x00040004)
#define BRUSHFLAG_SKY      (CONTENTS_BSP | FLAG_UNKNOWN)                                            // 2304 (0x00000900)
#define SURFACEFLAG_AAATRIGGER SURFACEFLAG_NODRAW                                                   // 32 (0x00000020)
#define BRUSHFLAG_AAATRIGGER (CONTENTS_DETAIL | CONTENTS_TRIGGER)                                   // 2621440 (0x00200200)
#define SURFACEFLAG_TRIGGER	SURFACEFLAG_NODRAW                                                      // 32 (0x00000020)
#define BRUSHFLAG_TRIGGER (CONTENTS_DETAIL | CONTENTS_TRIGGER)                                      // 2621440 (0x00200200)
#define SURFACEFLAG_LIQUIDS (CONTENTS_CLIP | SURFACEFLAG_NODECALS)                                  // 327680 (0x00050000)
#define BRUSHFLAG_LIQUIDS  CONTENTS_WATER                                                           // 1048576 (0x00100000)

//new
#ifdef BBSP_NULL_SUPPORT
#define SURFACEFLAG_NULL (SURFACEFLAG_NODRAW | CONTENTS_DETAIL)
#define BRUSHFLAG_NULL 0
#endif
#ifdef BBSP_BLOCKLIGHT_SUPPORT
#define SURFACEFLAG_BLOCKLIGHT (SURFACEFLAG_NODRAW | SURFACEFLAG_NODECALS | SURFACEFLAG_NOIMPACTS)
#define BRUSHFLAG_BLOCKLIGHT CONTENTS_DETAIL
#endif

#define MAX_TEXTURE_LENGTH 64

struct texturename_t
{
    char str[MAX_TEXTURE_LENGTH];
};

typedef struct dleaf_s
{
    unsigned int contents{};
    int visofs{};
    float mins[3]{};
    float maxs[3]{};
    unsigned int firstmarksurface{};
    unsigned int nummarksurfaces{};
    unsigned int firstdrawmarkbrush_index{};
    unsigned int numdrawmarkbrushes{};
} dleaf_t;

typedef struct dtexmatrix_s
{
    float s[4]{};
    float t[4]{};

    bool operator== (const dtexmatrix_s& other) const = default;
} dtexmatrix_t;

typedef struct epair_s
{
    struct epair_s* next{};
    char* key{};
    char* value{};

    epair_s();
    ~epair_s();
} epair_t;

typedef struct entity_s
{
    unsigned int index{};
    vec3_t origin{};
    epair_t* epairs{};
    struct brush_s** brushes{};
    unsigned int numbrushes{};
    unsigned int max_alloced_brushes{};
    entity_s();
    ~entity_s();
    struct brush_s* CreateNewBrush();
} entity_t;

struct mapinfo_t
{
    entity_t** entities{};
    unsigned int numentities{};
    unsigned int numalloced{};

    mapinfo_t();
    ~mapinfo_t();
};

typedef struct dmodel_s
{
    float mins[3]{};
    float maxs[3]{};
    unsigned int headnode[4]{};
    unsigned int firstleaf{};
    unsigned int numleafs{};
    unsigned int firstface{};
    unsigned int numfaces{};
} dmodel_t;

typedef struct dbrushside_s
{
    unsigned int face{};
    unsigned int plane{};
} dbrushside_t;

/*
typedef struct mtexture_s
{
    char name[MAX_TEXTURE_LENGTH];
    unsigned int index_in_memory;
    int unknown2;
} mtexture_t;

struct texture_projection_t
{
    char texturename[MAX_TEXTURE_LENGTH];
    double vecs[2][4];
};
*/

struct brush_texture_t
{
    char name[MAX_TEXTURE_LENGTH]{};
    vec3_t uaxis{}; // [s/t][xyz offset]
    vec3_t vaxis{};
    vec_t ushift{};
    vec_t vshift{};
    vec_t uscale{};
    vec_t vscale{};
    unsigned int surfaceflags{};
    char material[MAX_TEXTURE_LENGTH]{};
    vec_t lightmaprotation{};
    vec_t lightmapscale{};
};

struct texinfo_t
{
    char name[MAX_TEXTURE_LENGTH]{};
    double vecs[2][4]{};                            // [s/t][xyz offset]  (x, y, z, translation)
    int unused{};
    unsigned int flags{};
    int unused2[2]{};
#ifdef HL2_LUXEL_METHOD
    double lightmapVecsLuxelsPerWorldUnits[2][4]{};
#endif
    bool operator== (const texinfo_t& other) const
    {
        if (!strcmp(name, other.name) && flags == other.flags)
        {
            for (int j = 0; j < 2; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    if (vecs[j][k] != other.vecs[j][k])
                        return false;
#ifdef HL2_LUXEL_METHOD
                    if (lightmapVecsLuxelsPerWorldUnits[j][k] != other.lightmapVecsLuxelsPerWorldUnits[j][k])
                        return false;
#endif
                }
            }
            return true;
        }
        return false;
    }
};

// leftover from reversing
/*
struct markbrush_t
{
    markbrush_t* next;
    struct markface_t* markface_map_root;
    unsigned int num_markfaces;
};

struct markface_t
{
    markface_t* next;
    markface_t* last;
    markface_t* unknownmarkface;
    struct face_* face;
    bool initialized_by_markbrush_constructor;
    bool initialized_by_alloc_markface;
};
*/

typedef struct face_s
{
    face_s* next = nullptr;
    struct brush_s* brush = nullptr;
    struct side_s* brushside = nullptr;
    unsigned int planenum = 0;
    texinfo_t* texinfo = nullptr;
    bool built_draw_indices_for_face = false;
    unsigned int outputnumber = 0;
    class Winding* winding = nullptr;
    unsigned int flags = 0;
    int unknown2 = 0;
    double vecs[2][4]{};
    struct node_s* leaf_node = nullptr;
    int unknown4 = 0;
#if defined(SUBDIVIDE) && defined(_DEBUG)
    bool subdivided = false;
#endif
#if defined(MERGE) && defined(_DEBUG)
    bool merged = false;
#endif

#ifdef BBSP_USE_CPP
    ~face_s();
#else
    ~face_s() = delete;
#endif

    face_s(unsigned int planenum);
    face_s(const face_s& src);
    face_s(const face_s& src, Winding* src_winding);

} face_t;

typedef enum
{
    plane_x = 0,
    plane_y,
    plane_z,
    plane_anyx,
    plane_anyy,
    plane_anyz
}
planetypes;

#define last_axial plane_z

typedef struct plane_s
{
    vec3_t normal{};
    double dist{};
    planetypes closest_axis{};
} plane_t;

typedef struct dplane_s
{
    float normal[3];
    float dist;
    int closest_axis;
} dplane_t;

typedef struct dnode_s
{
    unsigned int planenum{};
    int children[2]{};
    float mins[3]{};
    float maxs[3]{};
} dnode_t;

typedef struct dvertex_s
{
    float point[3]{};
} dvertex_t;

typedef struct dnormal_s
{
    float normal[3]{};
} dnormal_t;

typedef struct dface_s
{
    unsigned int plane_index{};
    unsigned int first_vertex_index{};
    unsigned int num_vertices{};
    unsigned int first_indices_index{};
    unsigned int num_indicies{};
    unsigned int flags{};
    unsigned int texture_index{};
    unsigned int material_index{};
    unsigned int tex_info_projection_index{};
    unsigned int lightmap_info_projection_index{};
    uint8_t light_styles[4]{};
    int lightmap_offset{};
} dface_t;

typedef struct dbrush_s
{
    unsigned int flags{};
    unsigned int firstbrushside{};
    unsigned int numbrushsides{};
} dbrush_t;

typedef struct portal_s
{
    unsigned int planenum{};
    struct node_s* onnode{};
    struct node_s* nodes[2]{};
    struct portal_s* next[2]{};
    class Winding* winding{};

    portal_s();
    ~portal_s();
} portal_t;

typedef struct lump_s
{
    int offset;
    int length;
} lump_t;

typedef struct dheader_s
{
    int version;
    lump_t lumps[18];
} dheader_t;

typedef struct node_s
{
    unsigned int planenum{}; // -1 = leaf node
    void* unused{}; //actually unused, probably was meant to be parent as it's in the same spot as quake3 compiler
    vec3_t mins{};
    vec3_t maxs{};
    portal_t* portals{};
    unsigned int contents{};
    struct node_s* children[2]{}; // only valid for decision nodes, 0 is left, 1 is right
    bool valid{};
    unsigned int visleafnum{}; // -1 = solid
    std::set<struct face_s*>* markfaces{};  // leaf nodes only, point to node original_face
    std::set<struct brush_s*>* markbrushes{};  // leaf nodes only, point to node brushes
    unsigned int occupied{}; // light number in leaf for outside filling
    int unused2{};

    void ClearMarkFaces();
    void ClearMarkBrushes();
    bool IsPortalLeaf();

    node_s();
    ~node_s();
} node_t;

typedef struct side_s
{
    brush_texture_t td{};
    vec_t planepts[3][3];
    unsigned int plane_num{};
    face_t* original_face{};
    face_t* face_fragments{};
    face_t* inverted_face_fragments{};
    face_t* final_face{};
    struct brush_s* parent_brush{};
    bool built_draw_indices_for_side{};
    unsigned int draw_brush_side_index{};
#ifdef MERGE
    bool merged_side_into_another_brush{};
#endif

    side_s();
    ~side_s();
} side_t;

typedef struct brush_s
{
    unsigned int entitynum = 0;
    unsigned int brushnum = 0;
    side_t** brushsides = nullptr;
    unsigned numsides = 0;
    unsigned int sidecapacity = 0;
    unsigned int contents = 0;
    unsigned int brushflags = 0;
    bool built_draw_brush = 0;
    unsigned int output_num = 0;
    BoundingBox bounds{};

    side_t* CreateNewBrushSide();
    brush_s();
    ~brush_s();

    void FreeSides();
} brush_t;


struct special_texture_t
{
    const char* name;
    unsigned int contents;
    unsigned int renderflags;
    unsigned int brushflags;
};