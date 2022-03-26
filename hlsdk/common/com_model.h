//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================


//  06/25/2002 MAH
//  This header file has been modified to now include the proper BSP model
//  structure definitions for each of the two Quakeworld client renderers:
//  software mode and GL mode. Originally, Valve only supplied it with
//  the software mode definitions, which caused General Protection Fault's
//  when accessing members of the structures that are different between
//  the two versions.  These are: 'mnode_t', 'mleaf_t', 'msurface_t' and
//  'texture_t'. To select the GL hardware rendering versions of these
//  structures, define 'HARDWARE_MODE' as a preprocessor symbol, otherwise
//  it will default to software mode as supplied.

// Hardcoded HARDWARE_MODE - Max Vollmer, 2018-01-21
#ifndef HARDWARE_MODE
#define HARDWARE_MODE
#endif

#include <cache_user.h>


// com_model.h
#ifndef COM_MODEL_H
#define COM_MODEL_H

#define STUDIO_RENDER 1
#define STUDIO_EVENTS 2

#define MAX_CLIENTS 32
#define MAX_EDICTS  3072

#define SURF_PLANEBACK 2  // plane should be negated // Added from bspfile.h - Max Vollmer, 2018-02-04

#define MAX_MODEL_NAME 64
#define MAX_MAP_HULLS  4
#define MIPLEVELS      4
#define NUM_AMBIENTS   4  // automatic ambient sounds
#define MAXLIGHTMAPS   4
#define PLANE_ANYZ     5

#define ALIAS_Z_CLIP_PLANE 5

// flags in finalvert_t.flags
#define ALIAS_LEFT_CLIP    0x0001
#define ALIAS_TOP_CLIP     0x0002
#define ALIAS_RIGHT_CLIP   0x0004
#define ALIAS_BOTTOM_CLIP  0x0008
#define ALIAS_Z_CLIP       0x0010
#define ALIAS_ONSEAM       0x0020
#define ALIAS_XY_CLIP_MASK 0x000F

#define ZISCALE ((float)0x8000)

#define CACHE_SIZE 32  // used to align key data structures

typedef enum
{
	mod_unknown,
	mod_brush, //1
	mod_unknown2,
	mod_studio //3

	//mod_brush,
	//mod_sprite,
	//mod_alias,
	//mod_studio
} modtype_t;

// must match definition in modelgen.h
#ifndef SYNCTYPE_T
#define SYNCTYPE_T

typedef enum
{
	ST_SYNC = 0,
	ST_RAND
} synctype_t;

#endif

typedef struct dmodel_s
{
	float mins[3] = { 0 }, maxs[3] = { 0 };
	float origin[3] = { 0 };
	int headnode[MAX_MAP_HULLS] = { 0 };
	int visleafs = 0;  // not including the solid leaf 0
	int firstface = 0, numfaces = 0;
} dmodel_t;
//size: 0x40

// nightfire structure in memory
typedef struct mplane_s
{
	vec3_t normal;  // surface normal
	float dist = 0.f;     // closest appoach to origin
	byte type = 0;      // for texture axis selection and fast side tests
	byte signbits = 0;  // signx + signy<<1 + signz<<1
	byte pad[2] = { 0 };
} mplane_t;
//size: 0x14

// nightfire structure in .bsp file
typedef struct dplane_s
{
	vec3_t normal;
	float dist;
	unsigned int closest_axis;
}dplane_t;
//size: 0x14

// nightfire structure in memory
typedef struct mbrushside_s
{
	mplane_t* plane;
	msurface_t* surface;
} mbrushside_t;
//size: 0x8

// nightfire structure in .bsp file
typedef struct dbrushside_s
{
	int plane;
	int face;
} dbrushside_t;
//size: 0x8

typedef struct
{
	vec3_t position;
} mvertex_t;
//size: 0x12

// 06/23/2002 MAH
// This structure is the same in QW source files
//  'model.h' and 'gl_model.h'
typedef struct
{
	unsigned short v[2] = { 0 };
	unsigned int cachededgeoffset = 0;
} medge_t;


// nightfire structure in memory
typedef struct mtexture_s
{
	char name[64];
	int index_in_memory;
	int unknown2;
} mtexture_t;
//size: 0x48

// nightfire structure in memory
typedef struct
{
	float vecs[2][4] = { 0 };  // [s/t] unit vectors in world space.
					   // [i][3] is the s/t offset relative to the origin.
					   // s or t = dot(3Dpoint,vecs[i])+vecs[i][3]
} mtexinfo_t;
//size: 0x20

// 06/23/2002 MAH
// This structure is only need for hardware rendering
#define VERTEXSIZE 7

typedef struct glpoly_s
{
	struct glpoly_s* next = nullptr;
	struct glpoly_s* chain = nullptr;
	int numverts = 0;
	int flags = 0;  // for SURF_UNDERWATER

	// 07/24/2003 R9 - not actually four! AHA! AHAHAHAHAHAHA! hack.
	float verts[4][VERTEXSIZE] = { 0 };  // variable sized (xyz s1t1 s2t2)
} glpoly_t;


//
//  hardware mode - QW 'gl_model.h'
typedef struct mnode_s
{
	// common with leaf
	int planenum = 0;  //-1?
	int visframe = 0;  // node needs to be traversed if current

	float minmaxs[6] = { 0 };  // for bounding box culling

	struct mnode_s* parent = nullptr;

	// node specific
	mplane_t* plane = nullptr;
	struct mnode_s* children[2] = { 0 };
} mnode_t;
//size: 0x30


typedef struct msurface_s msurface_t;
typedef struct decal_s decal_t;

// JAY: Compress this as much as possible
struct decal_s
{
	decal_t* pnext = nullptr;        // linked list for each surface
	msurface_t* psurface = nullptr;  // Surface id for persistence / unlinking
	short dx = 0;              // Offsets into surface texture (in texture coordinates, so we don't need floats)
	short dy = 0;
	short texture = 0;  // Decal texture
	byte scale = 0;     // Pixel scale
	byte flags = 0;     // Decal flags

	short entityIndex = 0;  // Entity this is attached to
};

// nightfire structure in .bsp
typedef struct dbrush_s
{
	int contents;
	int firstside;
	int numsides;
} dbrush_t;
//size: 0xC


// nightfire structure in memory
struct mbrush_t
{
	int contents;
	int numsides;
	mbrushside_t* sides;
	int unknown;
};
//size: 0x10

// nightfire structure in memory
typedef struct mleaf_s
{
	int leafnum = 0;
	int visframe = 0; // unused?     node needs to be traversed if current
	float minmaxs[6] = { 0 };
	struct mnode_s* parent = nullptr;
	int contents = 0;  // wil be a negative contents number

	// leaf specific
	byte* compressed_vis = nullptr;
	struct efrag_s* efrags = nullptr;

	msurface_t** firstmarksurface = nullptr;
	int nummarksurfaces = 0;
	mbrush_t** firstmarkbrush = nullptr;
	int nummarkbrushes = 0;
	byte ambient_sound_level[NUM_AMBIENTS] = { 0 }; //unused?
} mleaf_t;
//size: 0x44

// nightfire structure in .bsp file
typedef struct dleaf_s
{
	int contents;
	int visofs;
	float mins[3];
	float maxs[3];
	unsigned int firstmarksurface;
	unsigned int nummarksurfaces;
	unsigned int firstmarkbrush;
	unsigned int nummarkbrushes;
} dleaf_t;
//size: 0x30

// nightfire structure in .bsp file
typedef struct dnode_s
{
	int				planenum;
	int				children[2];
	float			mins[3];
	float			maxs[3];
} dnode_t;
//size: 0x24

// nightfire
struct mmaterial_t
{
	char name[64];
};
//size: 0x40

// nightfire
struct lightingdata_t
{
	int index;
};
//size: 0x4

// nightfires structure in mmemory
struct texmatrix_t
{
	float vecs[4];
};
//size: 0x10

// nightfire structure in memory
typedef struct mface_s
{
	unsigned char gap0[4];
	int texture;
	int firstVertex;
	int numVertices;
	int firstIndex;
	int numIndices;
	int bmins[2];
	int bmaxs[2];
	int extent;
	int unknown_;
	mtexinfo_t* scale_info;
	texmatrix_t* _texturematrices;
	float unknown_float;
	unsigned char gap38[36];
	unsigned __int8 lightStyles[4];
	unsigned char gap61[20];
	int lightmap;
	int unknown;
} mface_t;
//size: 0x30


// nightfire structure in .bsp file
typedef struct dface_s
{
	int plane;
	int firstVertex;
	int numVertices;
	int firstIndex;
	int numIndices;
	unsigned char flags[4];
	int texture;
	int material;
	int texInfo;
	int lightmapTexInfo;
	int lightStyles;
	int lightmapOffset;
} dface_t;
//size: 0x30

// nightfire structure in memory
typedef struct msurface_s
{
	int flags;					// see SURF_ #defines
	mplane_t* plane;			// pointer to shared plane
	mtexinfo_t* texinfo;
	const char* materialname;	//nightfire material file
	mbrush_t* parent_brush;
	mface_s* parent_face;
	int unknown3;
} msurface_t;
//size: 0x1C

//
//  06/23/2002 MAH
//  Note: this structure is exactly the same in QW software
//      and hardware renderers QW - 'bspfile.h'
typedef struct
{
	int planenum = 0;
	short children[2] = { 0 };  // negative numbers are contents
} dclipnode_t;

//
//  06/23/2002 MAH
//  Note: this structure is exactly the same in QW software
//      and hardware renderers 'model.h' and 'gl_model.h'
typedef struct hull_s
{
	dclipnode_t* clipnodes = nullptr;
	mplane_t* planes = nullptr;
	int firstclipnode = 0;
	int lastclipnode = 0;
	vec3_t clip_mins;
	vec3_t clip_maxs;
} hull_t;
//size: 0x28, unknown

class StudioModelData
{
public:
	char pad[0x9C];
};

// nightfire format in memory
typedef struct model_s
{
	char name[64] = { 0 };
	unsigned int cachesize = 0;
	unsigned int N000002C7 = 0;
	int isloaded = 0;
	int type = 0;
	int numframes = 0;
	int otherflags = 0;
	int flags = 0;
	vec3_t mins = { 0 };
	vec3_t maxs = { 0 };
	vec3_t centroid = { 0 };
	float radius = 0.0f;
	int firstmodelsurface = 0;
	unsigned int nummodelsurfaces = 0;
	unsigned int numsubmodels = 0;
	dmodel_t* submodels = nullptr;
	unsigned int numplanes = 0;
	mplane_t* planes = nullptr;
	unsigned int numleafs = 0;
	mleaf_s* leafs = nullptr;
	unsigned int numverts = 0;
	vec3_t* verts = nullptr;
	unsigned int N000002D5 = 0;
	unsigned int N000002D6 = 0;
	unsigned int numindices = 0;
	unsigned int* indices = nullptr;
	unsigned int numnodes = 0;
	mnode_t* nodes = nullptr;
	unsigned int numsurfaces = 0;
	msurface_t* surfaces = nullptr;
	unsigned int nummarksurfaces = 0;
	unsigned int* marksurfaces = nullptr;
	unsigned int nummarkbrushes = 0;
	unsigned int* markbrushes = nullptr;
	unsigned int numtextures = 0;
	mtexture_t* textures = nullptr;
	unsigned int nummaterials = 0;
	mmaterial_t* materials = nullptr;
	unsigned int numtexturematrices = 0;
	mtexinfo_t* texturematrices = nullptr;
	unsigned int numbrushsides = 0;
	mbrushside_t* brushsides = nullptr;
	unsigned int numbrushes = 0;
	mbrush_t* brushes = nullptr;
	mnode_t* nodes_copy = nullptr;
	mleaf_s* leafs_copy = nullptr;
	mplane_t* planes_copy = nullptr;
	mbrush_t* brushes_copy = nullptr;
	unsigned int numnodes_copy = 0;
	unsigned int numleafs_copy = 0;
	unsigned int numplanes_copy = 0;
	unsigned int numbrushes_copy = 0;
	void* visdata = nullptr;
	void* lighting = nullptr;
	void* entities = nullptr;
	void* studiomodel = nullptr;
	void* cache = nullptr;

#if 0
	char name[MAX_MODEL_NAME] = { 0 };  //
	size_t cachesize; //0x40 : value of 76
	int unknown2; //0x44
	qboolean needload = 0; //0x48 bmodels and sprites don't cache normally
	modtype_t type;       // 0x4C
	int numframes = 0;    //0x50
	synctype_t synctype;  //

	int otherflags = 0;  //0x54
	int flags = 0; //0x58 , 0x800

	//
	// volume occupied by the model
	//
	vec3_t mins; //0x5C
	vec3_t maxs;
	float radius = 0.f; //

	//
	// brush model
	//
	int firstmodelsurface = 0; 
	int nummodelsurfaces = 0;  //

	int numsubmodels = 0;     //0x8C
	dmodel_t* submodels = nullptr;  //0x90

	int numplanes = 0;     //
	mplane_t* planes = nullptr;  //

	int numleafs = 0;           //      number of visible leafs, not counting 0
	struct mleaf_s* leafs = nullptr;  //

	int numvertexes = 0;      //
	mvertex_t* vertexes = nullptr;  //

	int numedges = 0;    //
	medge_t* edges = nullptr;  //

	int numnodes = 0;    //
	mnode_t* nodes = nullptr;  //

	int numtexinfo = 0;       // 
	mtexinfo_t* texinfo = nullptr;  //

	int numsurfaces = 0;       // 
	msurface_t* surfaces = nullptr;  // 

	int numsurfedges = 0;
	int* surfedges = nullptr;

	int numclipnodes = 0;
	dclipnode_t* clipnodes = nullptr;

	int nummarksurfaces = 0;
	msurface_t** marksurfaces = nullptr;

	hull_t hulls[MAX_MAP_HULLS] = { 0 };

	int numtextures = 0;
	texture_t** textures = nullptr;

	byte* visdata = nullptr;

	color24* lightdata = nullptr;

	char* entities = nullptr;

	StudioModelData* studiomodeldata = nullptr;//0x130
	//
	// additional model data
	//
	cache_user_t cache;  //0x134 only access through Mod_Extradata
#endif
} model_t;
//size: 0x138

typedef vec_t vec4_t[4];

typedef struct alight_s
{
	int ambientlight = 0;  // clip at 128
	int shadelight = 0;    // clip at 192 - ambientlight
	vec3_t color;
	float* plightvec = nullptr;
} alight_t;

typedef struct auxvert_s
{
	float fv[3] = { 0 };  // viewspace x, y
} auxvert_t;

//
// ------------------  Player Model Animation Info ----------------
//
#include "custom.h"

typedef struct DM_PlayerState_s
{
	char modelname[260];
	char modelpath[260];
	struct model_s* model;
} DM_PlayerState_t;

#define MAX_INFO_STRING    256
#define MAX_SCOREBOARDNAME 32
typedef struct player_info_s
{
	// User id on server
	int userid = 0;

	// User info string
	char userinfo[MAX_INFO_STRING] = { 0 };

	// Name
	char name[MAX_SCOREBOARDNAME] = { 0 };

	// Spectator or not, unused
	int spectator = 0;

	int ping = 0;
	int packet_loss = 0;

	// skin information
	char model[MAX_QPATH] = { 0 };
	int topcolor = 0;
	int bottomcolor = 0;

	// last frame rendered
	int renderframe = 0;

	// Gait frame estimation
	int gaitsequence = 0;
	float gaitframe = 0.f;
	float gaityaw = 0.f;
	vec3_t prevgaitorigin;

	customization_t customdata;
} player_info_t;

#endif  // #define COM_MODEL_H
