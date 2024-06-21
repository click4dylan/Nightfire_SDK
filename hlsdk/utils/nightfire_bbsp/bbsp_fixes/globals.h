#pragma once

#include "bsp_structs.h"

#if Q3MAP2_EXPERIMENTAL_SNAP_NORMAL_FIX
#define NIGHTFIRE_NORMAL_FIX
// Increasing the normalEpsilon to compensate for new logic in SnapNormal(), where
// this epsilon is now used to compare against 0 components instead of the 1 or -1
// components.  Unfortunately, normalEpsilon is also used in PlaneEqual().  So changing
// this will affect anything that calls PlaneEqual() as well (which are, at the time
// of this writing, FindFloatPlane() and AddBrushBevels()).
#define normalEpsilon 0.00005
#else
#define normalEpsilon 0.00001
#endif

#define	Q_PI	3.14159265358979323846
#define NORMAL_EPSILON   0.00001
#define ON_EPSILON       0.01
#define EQUAL_EPSILON    0.001

#define	PLANENUM_LEAF		-1

#define ANGLE_UP    -1
#define ANGLE_DOWN  -2
#define MAXLIGHTMAPS    4

#define MAX_WORLD_BOUNDS 16384
#define	MAX_SWITCHED_LIGHTS	    32 
#define MAX_LIGHTTARGETS_NAME   64
#define MAX_MAP_PORTALS 50000
#define MAX_HLASSUME_ERRORS 46
#define MAX_MESSAGE 2048
#define MAX_MAP_PLANES 262144
#define MAX_MAP_NODES 262144
#define MAX_MAP_FACES 262144
//#define MAX_MAP_EDGES       256000
//#define MAX_MAP_SURFEDGES   512000
#define MAX_MAP_VERTS 262144
#define MAX_MAP_NORMALS 262144
#define MAX_MAP_INDICES 262142
#define MAX_MAP_BRUSHES 32767
#define MAX_MAP_BRUSHSIDES 196602
#define MAX_MAP_TEXINFO 262144
#define MAX_MAP_TEXMATRIX 65536
#define MAX_MAP_MARKSURFACES 262144
#define MAX_MAP_MARKBRUSHES 262144
#define MAX_MAP_TEXTURES 1024
#define MAX_MAP_MATERIALS 1024
#define MAX_MAP_LEAVES 32000
#define MAX_MAP_LEAFS MAX_MAP_LEAVES
#define MAX_MAP_MODELS 1024

//rad
#define MAX_MAP_LIGHTINFO 64

#define MAX_MAP_ENGINE_ENTITIES 1200

#define MAX_MAP_LIGHTDATA 0x1000000
#define MAX_MAP_VISIBILITY 0x2000000

#define MAX_MAP_ENTSTRING   (1024*1024)

#define BSPVERSION 42

//#define CompileLog *(FILE**)0x8BE3360
//#define CompileErrorLog *(FILE*)0x4290C0
//#define g_NumWaterModels *(int*)0x8BE33F4
inline unsigned int g_NumWaterModels = 0;
//#define g_SetAllocNodeSize *(int*)0x8BE33FC
//#define g_numthreads *(int*)0x429DE0
//#define NUM_SPECIAL_TEXTURES 15
//#define g_SpecialTextures ((special_texture_t*)0x429DE8)
//#define g_Program *(const char**)0x429ED8
//#define g_Mapname ((char*)0x429EE0)
//#define g_log *(bool*)0x429FE4
//#define g_info *(bool*)0x429FE5
#define DEFAULT_CHART       false
#define DEFAULT_INFO        true
inline bool g_info = DEFAULT_INFO;
inline bool g_chart = DEFAULT_CHART;
inline int g_MaxNodeSize = 1024;
#ifdef VARIABLE_LIGHTING_MAX_NODE_SIZE
inline int g_LightingMaxNodeSize = 512;
#endif
inline int g_blscale = 16;
inline int g_ilscale = 16;
inline bool g_lighting = true;
inline bool g_water = true;
inline dbrush_t g_dbrushes[MAX_MAP_BRUSHES];
inline bool g_onlyents = false;
inline unsigned int g_TimesCalledSplitFaces = 0;
#ifdef MERGE
inline unsigned int g_NumPlaneFacesMerged = 0;
inline bool g_nomerge = false;
#endif
#ifdef SUBDIVIDE
inline unsigned int g_NumFacesSubdivided = 0;
#endif
inline unsigned g_numtexinfo = 0;
inline texinfo_t g_texinfo[MAX_MAP_TEXINFO];

//#define g_MaxNodeSize *(int*)0x429FE8
//#define g_blscale *(int*)0x429FEC
//#define g_ilscale *(int*)0x429FF0
//#define g_lighting *(bool*)0x429FF4
//#define g_water *(bool*)0x429FF5
//#define g_dbrushes ((dbrush_t*)0x43F01C8)
//#define g_onlyents *(bool*)0x8BE3380

//#define g_TimesCalledSplitFaces *(int*)0x45B148

//#define g_numtexinfo *(unsigned int*)0x67E1FCC
//#define g_texinfo ((texinfo_t*)0x67E1FD0)

//#define threadlock *(CriticalSection*)0x8BE1FD0

#define MAX_KEY                 32
#define ZHLT3_MAX_VALUE             4096

//100
//#define threadtimes ((double*))0x8BE1FE8
//#define workfunction *(int*)0x8BE2308
//#define q_entry *(int*)0x8BE230C
//#define enter *(int*)0x8BE2310
//1024
#define TEXTURE_REFERENCE_MAX_SIZE (sizeof(int) * 1024)
inline unsigned g_TextureReferenceCount[MAX_MAP_TEXTURES];
inline unsigned g_numWindings = 0;

// yeah...
inline unsigned g_numOccupiedLeafs = 0;
inline unsigned g_numSolidLeafs2 = 0;
inline unsigned g_numLeafs2 = 0;
inline unsigned g_numFloodedLeafs = 0;
inline unsigned g_numFalseNodes = 0;

// yeah.. 
inline unsigned g_numEmptyLeafs = 0;
inline unsigned g_numSolidLeafs = 0;
inline unsigned g_numEmptyNodes = 0;
inline unsigned g_numSolidNodes = 0;
inline int g_MapVersion = 0;
//#define g_TextureReferenceCount  ((int*)0x8BE2318)
//#define g_numWindings *(int*)0x8BE3318
//#define g_threadpriority *(int*)0x8BE331C
//#define dispatch *(int*)0x8BE3320
//#define workcount *(int*)0x8BE3324
//#define oldf *(int*)0x8BE3328
//#define pacifier *(bool*)0x8BE332C
//#define threaded *(bool*)0x8BE332D
//#define threadstart *(double*)0x8BE3330

//#define g_numOccupiedLeafs *(int*)0x8BE3338
//#define g_numSolidLeafs2 *(int*)0x8BE333C
//#define g_numLeafs2 *(int*)0x8BE3340
//#define g_numFloodedLeafs *(int*)0x8BE3344
//#define g_numFalseNodes *(int*)0x8BE3348
//#define g_MapVersion *(int*)0x8BE334C
//#define g_developer *(int*)0x8BE3350
//#define g_verbose *(bool*)0x8BE3354
//#define g_clientid *(int*)0x8BE3358
// 
inline unsigned g_numVisLeafs = 0;
inline unsigned g_numMapsAlloced = 0;
inline unsigned g_numEnts = 0;
inline unsigned g_numEPairs = 0;
#ifdef SUBDIVIDE
inline int g_subdivide_size = 240;
inline bool g_nosubdiv = false;
#endif
inline bool g_nofill = false;
inline bool g_notjunc = false;
inline bool g_noclip = false;
inline bool g_estimate = false;
inline bool g_bLeakOnly = false;
inline bool g_bLeaked = false;
inline bool g_showbevels = false;
inline unsigned g_numBrushes = 0;
inline unsigned g_numBrushSides = 0;
inline unsigned g_numNodesWithMarkFaces = 0;
inline unsigned g_numNodesWithMarkBrushes = 0;
inline unsigned g_NumPortals = 0;
inline unsigned g_numLeafs = 0;
inline unsigned g_numDLeafs = 0;
inline unsigned g_numFaces = 0;
inline unsigned g_numDFaces = 0;
inline unsigned g_numNodesAllocated = 0;
inline unsigned g_numNodes = 0;
//#define g_numMapsAlloced *(int*)0x8BE3374
//#define g_numEnts *(int*)0x8BE3378
//#define g_numEPairs *(int*)0x8BE337C
//#define g_nofill *(bool*)0x8BE3381
//#define g_notjunc *(bool*)0x8BE3382
//#define g_noclip *(bool*)0x8BE3383
//#define g_chart *(bool*)0x8BE3384
//#define g_estimate *(bool*)0x8BE3385
//#define g_bLeakOnly *(bool*)0x8BE3386
//#define g_bLeaked *(bool*)0x8BE3387
//#define g_showbevels *(bool*)0x8BE3388
//#define g_numBrushes *(int*)0x8BE339C
//#define g_numBrushSides *(int*)0x8BE33A0
//#define g_numNodesWithMarkFaces *(int*)0x8BE33A4
//#define g_numNodesWithMarkBrushes *(int*)0x8BE33A8
//#define g_NumPortals *(int*)0x8BE3390
//#define g_numLeafs *(int*)0x42A3E0
//#define g_numDLeafs *(int*)0x1BE2190
//#define g_numFaces *(int*)0x8BE3394
//#define g_numDFaces *(int*)0x1BE2194
//#define g_numNodesAllocated *(int*)0x8BE3398
//#define g_numNodes *(int*)0x45B144
//50000 ints of frontplane, backplane 
//#define g_PortalLog ((int*)0x42A3E8)
//#define g_NumLeafFrontFaces *(int*)0x45B128
//#define g_numEmptyLeafs *(int*)0x45B130
//#define g_numSolidLeafs *(int*)0x45B134
//#define g_numEmptyNodes *(int*)0x45B13C
//#define g_numSolidNodes *(int*)0x45B140

inline char g_dentdata[MAX_MAP_ENTSTRING];

inline unsigned g_dmarksurfaces[MAX_MAP_MARKSURFACES];
inline unsigned g_dmarkbrushes[MAX_MAP_MARKBRUSHES];

//#define g_dentdata ((char*)0x1BE2198)
//#define g_dmarksurfaces ((int*)0x44501C8)
//#define g_dmarkbrushes ((int*)0x5B601E0)

inline texturename_t g_dmaterials[MAX_MAP_MATERIALS];
//#define g_dmaterials ((char*)0x45B150)
//#define g_dmaterials  ((texturename_t*)0x45B150)
//char g_dmaterials[1024][64]

inline int g_dentdata_checksum = 0;
inline int g_dlightdata_checksum = 0;
inline int g_dmodels_checksum = 0;
inline int g_dverts_checksum = 0;
inline int g_dindicies_checksum = 0;
inline int g_dplanes_checksum = 0;
inline int g_dbrushsides_checksum = 0;
inline int g_dtexmatrix_checksum = 0;
inline int g_dvisdata_checksum = 0;
inline int g_dbrushes_checksum = 0;
inline int g_dnodes_checksum = 0;
inline int g_dfaces_checksum = 0;
inline int g_dleafs_checksum = 0;
inline int g_dmaterials_checksum = 0;
inline int g_dtextures_checksum = 0;
inline int g_dnormals_checksum = 0;
inline int g_dmarksurfaces_checksum = 0;
inline int g_dmarkbrushes_checksum = 0;

//#define g_dentdata_checksum *(int*)0x23F01B8
//#define g_dlightdata_checksum *(int*)0x46B150
//#define g_dmodels_checksum *(int*)0x46B154
//#define g_dverts_checksum *(int*)0x23F01B4
//#define g_dindicies_checksum *(int*)0x44501C0
//#define g_dplanes_checksum *(int*)0x1CE2198
//262144
inline dnode_t g_dnodes[MAX_MAP_NODES];
//#define g_dnodes ((dnode_t*)0x46B158)
//#define g_dbrushsides_checksum *(int*)0xD6B158
//int[262144][3]
inline dplane_t g_dplanes[MAX_MAP_PLANES];
//#define g_dplanes ((dplane_t*)0x1EE21A8)
inline unsigned g_dindices[MAX_MAP_INDICES];
inline unsigned g_visdatasize = 0;
inline unsigned g_entdatasize = 0;
//#define g_dindices ((int*)0xD6B160)
//#define g_visdatasize *(unsigned int*)0x44501BC
//#define g_entdatasize *(unsigned int*)0xE6B160


//#define g_dtexmatrix_checksum *(int*)0x45501C8
//#define g_dvisdata_checksum *(int*)0x43F01C0

inline dface_t g_dFaces[MAX_MAP_FACES];
inline unsigned g_numDVerts = 0;
inline unsigned g_numDMarkBrushes = 0;
inline unsigned g_numDBrushSides = 0;
inline unsigned g_numDModels = 0;
inline unsigned g_numDMarkSurfaces = 0;
inline unsigned g_lightdatasize = 0;
#ifdef FIX_NORMALS_LUMP
inline unsigned g_numDNormals = 0;
#endif

inline dleaf_t g_dleafs[MAX_MAP_LEAVES];

//#define g_dFaces ((dface_t*)0xE6B168)
//#define g_numDVerts *(int*)0x1A6B168
//#define g_dbrushes_checksum *(int*)0x1A6B16C
//#define g_numDMarkBrushes *(int*)0x1A6B170
//#define g_dnodes_checksum *(int*)0x1A6B174
//#define g_dfaces_checksum *(int*)0x1A6B178
//#define g_dleafs_checksum *(int*)0x1A6B17C
//#define g_numDBrushSides *(int*)0x1A6B180
//#define g_numDModels *(int*)0x1A6B184
//#define g_numDMarkSurfaces *(int*)0x1A6B188
//#define g_lightdatasize *(int*)0x1A6B18C
//32000
//#define g_dleafs ((dleaf_t*)0x1A6B190)

//#define g_dmaterials_checksum *(int*)0x23E21A8
//#define g_dtextures_checksum *(int*)0x48501D0
// 
inline unsigned g_numDBrushes = 0;
inline dvertex_t g_dverts[MAX_MAP_VERTS];
inline dmodel_t g_dmodels[MAX_MAP_MODELS];
//#define g_numDBrushes *(int*)0x48501D4
//#define g_dverts ((dvertex_t*)0x48501D8)
//#define g_dmodels ((dmodel_t*)0x23E21B0)

inline unsigned g_numDMaterials = 0;
inline unsigned g_numDTextures = 0;

//#define g_numDMaterials *(int*)0x1EE21A0

//#define g_numDTextures *(int*)0x5B501D8
inline texturename_t g_dtextures[MAX_MAP_TEXTURES];
//char[1024][64]
//#define g_dtextures ((char*)0x5B501E0)
inline dtexmatrix_t g_dtexmatrix[MAX_MAP_TEXMATRIX];
inline unsigned g_numDTexMatrix = 0;
inline dnormal_t g_dnormals[MAX_MAP_NORMALS];

inline unsigned g_numDNodes = 0;
inline unsigned g_numDIndices = 0;

inline dbrushside_t g_dbrushsides[MAX_MAP_BRUSHSIDES];
inline char g_dvisdata[MAX_MAP_VISIBILITY];
inline unsigned char g_dlightdata[MAX_MAP_LIGHTDATA];

#ifdef ENFORCE_MAX_LIGHTMAP_SCALE
#define MAX_LIGHTMAP_SCALE 16
#endif
//#define g_dtexmatrix ((dtexmatrix_t*)0x1CE21A0)
//#define g_numDTexMatrix *(int*)0x5C601E0
//#define g_dnormals ((dnormal_t*)0x45501D0)
//#define g_dnormals_checksum *(int*)0x5C601E4
//#define g_dmarksurfaces_checksum *(int*)0x23F01B0
//#define g_dmarkbrushes_checksum *(int*)0x5C601E8
//#define g_numDNodes *(int*)0x5C601EC
//#define g_numDIndices *(int*)0x5C601F0
//#define g_dbrushsides  ((dbrushside_t*)0x5C601F8)
//#define g_dvisdata ((char*)0x23F01C0)
//#define g_dlightdata ((char*)0x4B501D8)


//MAX_PATH
//#define g_portfilename ((char*)0x5DE01C8)
//#define g_pointfilename  ((char*)0x5DE02D0)
//#define g_bspfilename  ((char*)0x5DE03D8)
//#define g_linefilename  ((char*)0x5DE04E0)
//#define g_MapFileName  ((char*)0x5DE05E8)
inline char g_portfilename[MAX_PATH];
inline char g_pointfilename[MAX_PATH];
inline char g_bspfilename[MAX_PATH];
inline char g_linefilename[MAX_PATH];
inline char g_MapFileName[MAX_PATH];

inline FILE* pointfile = nullptr;
//#define pointfile *(FILE**)0x5DE06EC
inline mapinfo_t* g_EntInfo = nullptr;
//#define g_EntInfo *(mapinfo_t**)0x5DE06F0
inline unsigned g_LeakEntity = 0;
//#define g_LeakEntity *(int*)0x5DE06F4
inline FILE* linefile = nullptr;
inline FILE* portalfile = nullptr;
inline FILE* lightingfile = nullptr;
//#define linefile *(FILE**)0x5DE06F8
//#define prevleaknode *(portal_t**)0x5DE06FC
inline portal_t* prevleaknode = nullptr;

//#define gMappedPlanes ((struct plane_s*)0x5DE0700)

inline unsigned g_numDPlanes = 0;
inline plane_t gMappedPlanes[MAX_MAP_PLANES];
inline unsigned gNumMappedPlanes = 0;
inline unsigned num_visleafs = 0;
inline node_t* g_OutsideNode = nullptr;
inline unsigned num_visportals = 0;
inline unsigned g_NumNonInvertedFaces = 0;

//#define g_numDPlanes *(int*)0x1EE21A4
//#define gNumMappedPlanes *(int*)0x67E0700
//#define num_visleafs *(int*)0x67E0704
//#define g_OutsideNode *(node_t**)0x67E0708
//#define Stream *(FILE**)0x67E070C
//#define num_visportals *(int*)0x67E0710

//#define g_NumNonInvertedFaces *(int*)0x45B128

#define MAX_NODE_SIZE g_MaxNodeSize
#define	MAXEDGES			48                 // 32
#define	MAXPOINTS			28                 // don't let a base face get past this

// the exact bounding box of the brushes is expanded some for the headnode
// volume.  is this still needed?
#define	SIDESPACE	24

#if 0
enum BBSP_ERROR_CODES {
    ERROR_INVALID_ASSUME_MESSAGE = 0,
    ERROR_MEMORY_ALLOCATION_FAILURE = 1,
    ERROR_NULL_POINTER = 2,
    ERROR_BAD_THREAD_WORKCOUNT = 3,
    ERROR_MISSING_BRACKET_IN_TEXTUREDEF = 4,
    ERROR_PLANE_WITH_NO_NORMAL = 5,
    ERROR_BRUSH_WITH_COPLANAR_FACES = 6,
    ERROR_BRUSH_OUTSIDE_WORLD = 7,
    ERROR_MIXED_FACE_CONTENTS = 8,
    ERROR_BRUSH_TYPE_NOT_ALLOWED_IN_WORLD = 9,
    ERROR_BRUSH_TYPE_NOT_ALLOWED_IN_ENTITY = 10,
    ERROR_NO_VISIBLE_BRUSHES = 11,
    ERROR_ENTITY_WITH_ONLY_ORIGIN_BRUSH = 12,
    ERROR_EXCEEDED_MAX_SWITCHED_LIGHTS = 13,
    ERROR_LEAK_IN_MAP = 14,
    ERROR_ODD_PLANE_IN_NODE = 15,
    ERROR_EMPTY_SOLID_ENTITY = 16,
    ERROR_LEAF_PORTAL_SAW_INTO_LEAF = 17,
    ERROR_EXCEEDED_MAX_PORTALS_ON_LEAF = 18,
    ERROR_INVALID_CLIENT_SERVER_STATE = 19,
    ERROR_EXCEEDED_MAX_TEXLIGHTS = 20,
    ERROR_EXCEEDED_MAX_PATCHES = 21,
    ERROR_TRANSFER_LESS_THAN_ZERO = 22,
    ERROR_BAD_SURFACE_EXTENTS = 23,
    ERROR_MALFORMED_FACE_NORMAL = 24,
    ERROR_NO_LIGHTS = 25,
    ERROR_BAD_LIGHT_TYPE = 26,
    ERROR_EXCEEDED_MAX_SINGLEMAP = 27,
    ERROR_UNABLE_TO_CREATE_THREAD = 28,
    ERROR_EXCEEDED_MAX_MAP_PLANES = 29,
    ERROR_EXCEEDED_MAX_MAP_TEXTURES = 30,
    ERROR_EXCEEDED_MAX_MAP_TEXINFO = 31,
    ERROR_EXCEEDED_MAX_MAP_BRUSHES = 32,
    ERROR_EXCEEDED_MAX_MAP_BRUSHSIDES = 33,
    ERROR_EXCEEDED_MAX_MAP_ENTITIES = 34,
    ERROR_EXCEEDED_MAX_ENGINE_ENTITIES = 35,
    ERROR_EXCEEDED_MAX_MAP_MODELS = 36,
    ERROR_EXCEEDED_MAX_MAP_VERTS = 37,
    ERROR_EXCEEDED_MAX_MAP_INDICES = 38,
    ERROR_EXCEEDED_MAX_MAP_MARKSURFACES = 39,
    ERROR_EXCEEDED_MAX_MAP_MARKBRUSHES = 40,
    ERROR_EXCEEDED_MAX_MAP_FACES = 41,
    ERROR_EXCEEDED_MAX_MAP_NODES = 42,
    ERROR_COMPRESS_VIS_OVERFLOW = 43,
    ERROR_DECOMPRESS_VIS_OVERFLOW = 44,
    ERROR_BBSP_INTERNAL_ERROR = 45
};
#endif