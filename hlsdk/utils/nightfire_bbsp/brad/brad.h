#ifndef HLRAD_H__
#define HLRAD_H__

#if _MSC_VER >= 1000
#pragma once
#endif
#include <Windows.h>
#include "cmdlib.h"
#include "messages.h"
//#include "win32fix.h"
#include "log.h"
#include "hlassert.h"
//#include "mathlib.h"
#include "bspfile.h"
#include "winding.h"
#include "scriplib.h"
#include "threads.h"
#include "blockmem.h"
#include "filelib.h"
#include "winding.h"

#ifdef SYSTEM_WIN32
#pragma warning(disable: 4142 4028)
#include <io.h>
#pragma warning(default: 4142 4028)
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef STDC_HEADERS
#include <ctype.h>
#endif

#ifdef SYSTEM_WIN32
#include <direct.h>
#endif

#define DEFAULT_LERP_ENABLED        true
#define DEFAULT_FADE                1.0
#define DEFAULT_FALLOFF             2
#define DEFAULT_BOUNCE              1
#define DEFAULT_BOUNCE_DYNAMIC		true
#define DEFAULT_DUMPPATCHES         false
#define DEFAULT_AMBIENT_RED         0.0
#define DEFAULT_AMBIENT_GREEN       0.0
#define DEFAULT_AMBIENT_BLUE        0.0
#define DEFAULT_MAXLIGHT            256.0
#define DEFAULT_TEXSCALE            true
#define DEFAULT_CHOP                64.0
#define DEFAULT_TEXCHOP             32.0
#define DEFAULT_LIGHTSCALE          1.0
#define DEFAULT_DLIGHT_THRESHOLD    25.0
#define DEFAULT_DLIGHT_SCALE        2.0
#define DEFAULT_SMOOTHING_VALUE     50.0
#define DEFAULT_INCREMENTAL         false

#ifdef ZHLT_PROGRESSFILE // AJM
#define DEFAULT_PROGRESSFILE NULL // progress file is only used if g_progressfile is non-null
#endif

// ------------------------------------------------------------------------
// Changes by Adam Foster - afoster@compsoc.man.ac.uk

// superseded by DEFAULT_COLOUR_LIGHTSCALE_*
#ifndef HLRAD_WHOME
   #define DEFAULT_LIGHTSCALE          1.0
#endif

// superseded by DEFAULT_COLOUR_GAMMA_*
#ifndef HLRAD_WHOME
   #define DEFAULT_GAMMA               0.5
#endif
// ------------------------------------------------------------------------

#define DEFAULT_INDIRECT_SUN        1.0
#define DEFAULT_EXTRA               false
#define DEFAULT_SKY_LIGHTING_FIX    true
#define DEFAULT_CIRCUS              false
#define DEFAULT_CORING              1.0
#define DEFAULT_SUBDIVIDE           true
#define DEFAULT_CHART               false
#define DEFAULT_SKYCLIP             true
#define DEFAULT_INFO                true
#define DEFAULT_ALLOW_OPAQUES       true
#define DEFAULT_NOHUNT              false

// ------------------------------------------------------------------------
// Changes by Adam Foster - afoster@compsoc.man.ac.uk
#ifdef HLRAD_WHOME

#define DEFAULT_COLOUR_GAMMA_RED		0.5
#define DEFAULT_COLOUR_GAMMA_GREEN		0.5
#define DEFAULT_COLOUR_GAMMA_BLUE		0.5

#define DEFAULT_COLOUR_LIGHTSCALE_RED		1.0
#define DEFAULT_COLOUR_LIGHTSCALE_GREEN		1.0
#define DEFAULT_COLOUR_LIGHTSCALE_BLUE		1.0

#define DEFAULT_COLOUR_JITTER_HACK_RED		0.0
#define DEFAULT_COLOUR_JITTER_HACK_GREEN	0.0
#define DEFAULT_COLOUR_JITTER_HACK_BLUE		0.0

#define DEFAULT_JITTER_HACK_RED			0.0
#define DEFAULT_JITTER_HACK_GREEN		0.0
#define DEFAULT_JITTER_HACK_BLUE		0.0

#define DEFAULT_DIFFUSE_HACK			true
#define DEFAULT_SPOTLIGHT_HACK			true

#define DEFAULT_SOFTLIGHT_HACK_RED		0.0
#define DEFAULT_SOFTLIGHT_HACK_GREEN		0.0
#define DEFAULT_SOFTLIGHT_HACK_BLUE		0.0
#define DEFAULT_SOFTLIGHT_HACK_DISTANCE 	0.0

#endif
// ------------------------------------------------------------------------

// O_o ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Changes by Jussi Kivilinna <hullu@unitedadmins.com> [http://hullu.xtragaming.com/]
#ifdef HLRAD_HULLU
	// Transparency light support for bounced light(transfers) is extreamly slow 
	// for 'vismatrix' and 'sparse' atm. 
	// Only recommended to be used with 'nomatrix' mode
	#define DEFAULT_CUSTOMSHADOW_WITH_BOUNCELIGHT false

	// RGB Transfers support for HLRAD .. to be used with -customshadowwithbounce
	#define DEFAULT_RGB_TRANSFERS false
#endif
// o_O ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#ifdef SYSTEM_WIN32
#define DEFAULT_ESTIMATE    false
#endif
#ifdef SYSTEM_POSIX
#define DEFAULT_ESTIMATE    true
#endif

// Ideally matches what is in the FGD :)
#define SPAWNFLAG_NOBLEEDADJUST    (1 << 0)

// DEFAULT_HUNT_OFFSET is how many units in front of the plane to place the samples
// Unit of '1' causes the 1 unit crate trick to cause extra shadows
#define DEFAULT_HUNT_OFFSET 0.5
// DEFAULT_HUNT_SIZE number of iterations (one based) of radial search in HuntForWorld
#define DEFAULT_HUNT_SIZE   11
// DEFAULT_HUNT_SCALE amount to grow from origin point per iteration of DEFAULT_HUNT_SIZE in HuntForWorld
#define DEFAULT_HUNT_SCALE 0.1

// If patches are allowed to be closer, the light gets amplified (which looks really damn weird)
#define MINIMUM_PATCH_DISTANCE 1.01

//
// LIGHTMAP.C STUFF
//

typedef enum
{
    emit_surface,
    emit_point,
    emit_spotlight,
    emit_skylight
}
emittype_t;

typedef struct directlight_s
{
    struct directlight_s* next;
    struct dleaf_s* leaf;
    emittype_t      type;
    int             style;
    vec3_t          origin;
    vec3_t          intensity;
    vec3_t          normal;                                // for surfaces and spotlights
    float           stopdot;                               // for spotlights
    float           stopdot2;                              // for spotlights

    // 'Arghrad'-like features
    vec_t           fade;                                  // falloff scaling for linear and inverse square falloff 1.0 = normal, 0.5 = farther, 2.0 = shorter etc
    unsigned char   falloff;                               // falloff style 0 = default (inverse square), 1 = inverse falloff, 2 = inverse square (arghrad compat)

    vec_t mindist; //nightfire
    vec_t maxdist; //nightfire

	// -----------------------------------------------------------------------------------
	// Changes by Adam Foster - afoster@compsoc.man.ac.uk
	// Diffuse light_environment light colour
	// Really horrible hack which probably won't work!
#ifdef HLRAD_WHOME
	vec3_t			diffuse_intensity;
#endif
	// -----------------------------------------------------------------------------------

} directlight_t;

#define TRANSFER_SCALE_VAL    (USHRT_MAX/4)

#define	TRANSFER_SCALE          (1.0 / TRANSFER_SCALE_VAL)
#define	INVERSE_TRANSFER_SCALE	(TRANSFER_SCALE_VAL)
#define TRANSFER_SCALE_MAX	(TRANSFER_SCALE_VAL * 4)

typedef struct
{
    unsigned size  : 12;
    unsigned index : 20;
} transfer_index_t;

typedef unsigned transfer_raw_index_t;
typedef float transfer_data_t;

//Special RGB mode for transfers
#ifdef HLRAD_HULLU
	#if defined(HLRAD_HULLU_48BIT_RGB_TRANSFERS) && defined(HLRAD_HULLU_96BIT_RGB_TRANSFERS)
		#error Conflict: Both HLRAD_HULLU_48BIT_RGB_TRANSFERS and HLRAD_HULLU_96BIT_RGB_TRANSFERS defined!
	#elif defined(HLRAD_HULLU_96BIT_RGB_TRANSFERS)
		//96bit (no noticeable difference to 48bit)
		typedef float rgb_transfer_t[3];
	#else
		//default.. 48bit
		typedef unsigned short rgb_transfer_t[3];
	#endif
	
	typedef rgb_transfer_t rgb_transfer_data_t;
#endif

#define MAX_COMPRESSED_TRANSFER_INDEX_SIZE ((1 << 12) - 1)

#define	MAX_PATCHES	(65535*4)
#define MAX_VISMATRIX_PATCHES 65535
#define MAX_SPARSE_VISMATRIX_PATCHES MAX_PATCHES

typedef enum
{
    ePatchFlagNull = 0,
    ePatchFlagOutside = 1
} ePatchFlags;

typedef struct patch_s
{
    struct patch_s* next;                                  // next in face
    vec3_t          origin;                                // Center centroid of winding (cached info calculated from winding)
    vec_t           area;                                  // Surface area of this patch (cached info calculated from winding)
    Winding*        winding;                               // Winding (patches are triangles, so its easy)
    vec_t           scale;                                 // Texture scale for this face (blend of S and T scale)
    vec_t           chop;                                  // Texture chop for this face factoring in S and T scale

    unsigned        iIndex;
    unsigned        iData;

    transfer_index_t* tIndex;
    transfer_data_t*  tData;
#ifdef HLRAD_HULLU
    rgb_transfer_data_t*	tRGBData;
#endif

    int             faceNumber;
    ePatchFlags     flags;

#ifdef ZHLT_TEXLIGHT
	int				totalstyle[MAXLIGHTMAPS];				//LRC - gives the styles for use by the new switchable totallight values
	vec3_t          totallight[MAXLIGHTMAPS];				// accumulated by radiosity does NOT include light accounted for by direct lighting
	vec3_t			directlight[MAXLIGHTMAPS];				// direct light only
	int				emitstyle;							   //LRC - for switchable texlights
    vec3_t          baselight;                             // emissivity only, uses emitstyle
    vec3_t          samplelight[MAXLIGHTMAPS];
    int             samples[MAXLIGHTMAPS];                 // for averaging direct light
#else
    vec3_t          totallight;                            // accumulated by radiosity does NOT include light accounted for by direct lighting
    vec3_t          baselight;                             // emissivity only
    vec3_t          directlight;                           // direct light value

    vec3_t          samplelight;
    int             samples;                               // for averaging direct light
#endif
} patch_t; //size: 0x68, 108 bytes in nightfire

#ifdef ZHLT_TEXLIGHT
//LRC
vec3_t* GetTotalLight(patch_t* patch, int style);
#endif

typedef struct
{
    dface_t*        faces[2];
    vec3_t          interface_normal;
    vec_t           cos_normals_angle;
    bool            coplanar;
} edgeshare_t;

extern edgeshare_t g_edgeshare[MAX_MAP_FACES];

//
// lerp.c stuff
//

typedef struct lerprect_s
{
    dplane_t        plane; // all walls will be perpindicular to face normal in some direction
    vec3_t          vertex[4];
}
lerpWall_t;

typedef struct lerpdist_s
{
    vec_t           dist;
    unsigned        patch;
} lerpDist_t;

// Valve's default was 2048 originally.
// MAX_LERP_POINTS causes lerpTriangulation_t to consume :
// 2048 : roughly 17.5Mb
// 3072 : roughly 35Mb
// 4096 : roughly 70Mb
#define	DEFAULT_MAX_LERP_POINTS		     512
#define DEFAULT_MAX_LERP_WALLS           128

typedef struct
{
    unsigned        maxpoints;
    unsigned        numpoints;

    //unsigned        maxwalls;
    //unsigned        numwalls;
    patch_t**       points;    // maxpoints
    lerpDist_t*     dists;     // numpoints after points is populated
    //lerpWall_t*     walls;     // maxwalls

    bool* empty_trace;

    unsigned        facenum;
    const dface_t*  face;
    const dplane_t* plane;
}
lerpTriangulation_t;

// These are bitflags for lighting adjustments for special cases
typedef enum
{
    eModelLightmodeNull     = 0,
    eModelLightmodeEmbedded = 0x01,
    eModelLightmodeOpaque   = 0x02,
    eModelLightmodeConcave  = 0x04
}
eModelLightmodes;

typedef struct
{
    Winding* winding;
	vec3_t   mins;
	vec3_t   maxs;
    dplane_t plane;
    unsigned facenum;

#ifdef HLRAD_HULLU
    vec3_t transparency_scale;
    bool transparency;
#endif

} opaqueList_t;

typedef struct tnode_s
{
    unsigned int planenum;
    tnode_s* children[2];
    dplane_t plane;
    unsigned int index_to_children[2];
} tnode_t;

extern tnode_t* tnodes;

#define OPAQUE_ARRAY_GROWTH_SIZE 1024

//
// qrad globals
//

extern patch_t* g_face_patches[MAX_MAP_FACES];
extern entity_t* g_face_entity[MAX_MAP_FACES];
extern vec3_t   g_face_offset[MAX_MAP_FACES];              // for models with origins
extern eModelLightmodes g_face_lightmode[MAX_MAP_FACES];
extern vec3_t   g_face_centroids[MAX_MAP_FACES];//[MAX_MAP_EDGES];
extern patch_t  g_patches[MAX_PATCHES];
extern unsigned g_num_patches;

extern float    g_lightscale;
extern float    g_dlight_threshold;
extern float    g_coring;
extern int      g_lerp_enabled;

extern void     MakeShadowSplits();

//==============================================

extern bool     g_extra;
extern vec3_t   g_ambient;
extern vec_t    g_direct_scale;
extern float    g_maxlight;
extern unsigned g_numbounce;
extern float    g_qgamma;
extern float    g_indirect_sun;
extern float    g_smoothing_threshold;
extern float    g_smoothing_value;
extern bool     g_estimate;
extern char     g_source[_MAX_PATH];
extern vec_t    g_fade;
extern int      g_falloff;
extern bool     g_incremental;
extern bool     g_circus;
//extern bool     g_sky_lighting_fix;
extern vec_t    g_chop;    // Chop value for normal textures
extern vec_t    g_texchop; // Chop value for texture lights
extern opaqueList_t* g_opaque_face_list;
extern unsigned      g_opaque_face_count;
extern unsigned      g_max_opaque_face_count;    // Current array maximum (used for reallocs)
extern bool     g_nohunt;

#ifdef ZHLT_PROGRESSFILE // AJM
extern char*           g_progressfile ;
#endif

// ------------------------------------------------------------------------
// Changes by Adam Foster - afoster@compsoc.man.ac.uk
#ifdef HLRAD_WHOME

extern vec3_t	g_colour_qgamma;
extern vec3_t	g_colour_lightscale;

extern vec3_t	g_colour_jitter_hack;
extern vec3_t	g_jitter_hack;
extern bool	g_diffuse_hack;
extern bool	g_spotlight_hack;
extern vec3_t	g_softlight_hack;
extern float	g_softlight_hack_distance;

#endif
// ------------------------------------------------------------------------


#ifdef HLRAD_HULLU
	extern bool	g_customshadow_with_bouncelight;
	extern bool	g_rgb_transfers;
	extern const vec3_t vec3_one;
#endif

extern void     MakeTnodes();
extern void     PairEdges();
extern void     BuildFacelights(int facenum);
extern void     PrecompLightmapOffsets();
extern void     FinalLightFace(int facenum);
extern struct tnode_s* TestLine(const vec3_t start, const vec3_t stop);
extern struct tnode_s* TestLine_r(const struct tnode_s* node, const vec3_t start, const vec3_t stop);
extern void FreeTnodes();
extern struct lightinfo_s* AllocLightInfo(unsigned int facenum);
extern void FreeAllLightInfo();
extern void     CreateDirectLights(mapinfo_t* map);
extern void     DeleteDirectLights();
extern void     GetPhongNormal(int facenum, vec3_t spot, vec3_t phongnormal);

#ifdef HLRAD_HULLU
typedef bool (*funcCheckVisBit) (unsigned, unsigned, vec3_t&, unsigned int&);
#else
typedef bool (*funcCheckVisBit) (unsigned, unsigned);
#endif
extern funcCheckVisBit g_CheckVisBit;

// qradutil.c
extern tnode_s* GetTnode(vec3_t point);
extern vec_t    PatchPlaneDist(const patch_t* const patch);
extern dleaf_t* PointInLeaf(const vec3_t point);
extern void     MakeBackplanes();
extern const dplane_t* getPlaneFromFace(const dface_t* const face);
extern const dplane_t* getPlaneFromFaceNumber(unsigned int facenum);
extern void     getAdjustedPlaneFromFaceNumber(unsigned int facenum, dplane_t* plane);
extern dleaf_t* HuntForWorld(vec_t* point, const vec_t* plane_offset, const dplane_t* plane, int hunt_size, vec_t hunt_scale, vec_t hunt_offset);

// makescales.c
extern void     MakeScalesVismatrix();
extern void     MakeScalesSparseVismatrix();
extern void     MakeScalesNoVismatrix();

// transfers.c
extern unsigned g_total_transfer;
extern bool     readtransfers(const char* const transferfile, long numpatches);
extern void     writetransfers(const char* const transferfile, long total_patches);

// vismatrixutil.c (shared between vismatrix.c and sparse.c)
extern void     SwapTransfers(int patchnum);
extern void     MakeScales(int threadnum);
extern void     DumpTransfersMemoryUsage();
#ifdef HLRAD_HULLU
extern void     SwapRGBTransfers(int patchnum);
extern void     MakeRGBScales(int threadnum);

// transparency.c (transparency array functions - shared between vismatrix.c and sparse.c)
extern void	GetTransparency(const unsigned p1, const unsigned p2, vec3_t &trans, unsigned int &next_index);
extern void	AddTransparencyToRawArray(const unsigned p1, const unsigned p2, const vec3_t trans);
extern void	CreateFinalTransparencyArrays(const char *print_name);
extern void	FreeTransparencyArrays();
#endif

// lerp.c
#ifdef ZHLT_TEXLIGHT
extern void     SampleTriangulation(const lerpTriangulation_t* const trian, vec3_t point, vec3_t result, int style); //LRC
#else
extern void     SampleTriangulation(const lerpTriangulation_t* const trian, vec3_t point, vec3_t result);
#endif
extern void     DestroyTriangulation(lerpTriangulation_t* trian);
extern lerpTriangulation_t* CreateTriangulation(unsigned int facenum);
extern void     FreeTriangulation(lerpTriangulation_t* trian);
extern unsigned int CalculateIntersectIndex(const lerpTriangulation_t* trian, unsigned int patch1, unsigned int patch2);
extern bool     TestLineSegmentIntersectWall(const lerpTriangulation_t* const trian, const lerpDist_t* dist1, const lerpDist_t* dist2);
extern bool     TestLineSegmentIntersectWall(const lerpTriangulation_t* const trian, unsigned int dist_index1, unsigned int dist_index2);

// mathutil.c
#ifdef HLRAD_HULLU
extern int     TestSegmentAgainstOpaqueList(const vec_t* p1, const vec_t* p2, vec3_t &scaleout);
#else
extern int     TestSegmentAgainstOpaqueList(const vec_t* p1, const vec_t* p2);
#endif

#ifdef HLRAD_FASTMATH
extern void PlaneFromPoints(const vec_t* const p1, const vec_t* const p2, const vec_t* const p3, dplane_t* plane);
extern bool LineSegmentIntersectsPlane(const dplane_t& plane, const vec_t* const p1, const vec_t* const p2, vec3_t& point);
extern bool PointInWinding(const Winding* const W, const dplane_t* const plane, const vec_t* const point);
extern bool PointInWall(const lerpWall_t* const wall, const vec_t* const point);
extern bool PointInTri(const vec_t* const point, const dplane_t* const plane, const vec_t* const p1, const vec_t* const p2, const vec_t* const p3);
extern void SnapToPlane(const dplane_t* const plane, vec_t* const point, const vec_t offset);
#else
extern bool     intersect_line_plane(const dplane_t* const plane, const vec_t* const p1, const vec_t* const p2, vec3_t point);
extern bool     intersect_linesegment_plane(const dplane_t* const plane, const vec_t* const p1, const vec_t* const p2,vec3_t point);
extern void     plane_from_points(const vec3_t p1, const vec3_t p2, const vec3_t p3, dplane_t* plane);
extern bool     point_in_winding(const Winding& w, const dplane_t& plane, const vec_t* point);
extern bool     point_in_wall(const lerpWall_t* wall, vec3_t point);
extern bool     point_in_tri(const vec3_t point, const dplane_t* const plane, const vec3_t p1, const vec3_t p2, const vec3_t p3);
extern void     ProjectionPoint(const vec_t* const v, const vec_t* const p, vec_t* rval);
extern void     SnapToPlane(const dplane_t* const plane, vec_t* const point, vec_t offset);
#endif

extern bool     IsSpecial(const dface_t* const f);
extern bool     IsSpecial(unsigned int flags);


#endif //HLRAD_H__
