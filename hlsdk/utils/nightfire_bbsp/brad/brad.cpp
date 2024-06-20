/*
 
    R A D I O S I T Y    -aka-    R A D 

    Code based on original code from Valve Software, 
    Modified by Sean "Zoner" Cavanaugh (seanc@gearboxsoftware.com) with permission.
    Modified by Tony "Merl" Moore (merlinis@bigpond.net.au) [AJM]
    
*/

#ifdef SYSTEM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <vector>
#include <string>

#include "brad.h"
#include <cmdlib.h>
#include <globals.h>
#include <mapfile.h>
#include "helpers.h"
/*
 * NOTES
 * -----
 * every surface must be divided into at least two g_patches each axis
 */

typedef enum
{
    eMethodVismatrix,
    eMethodSparseVismatrix,
    eMethodNoVismatrix
}
eVisMethods;

eVisMethods     g_method = eMethodVismatrix;

vec_t           g_fade = DEFAULT_FADE;
int             g_falloff = DEFAULT_FALLOFF;

patch_t*        g_face_patches[MAX_MAP_FACES];
entity_t*       g_face_entity[MAX_MAP_FACES];
eModelLightmodes g_face_lightmode[MAX_MAP_FACES];
patch_t         g_patches[MAX_PATCHES];
unsigned        g_num_patches;

bool g_warned_direct = false;

#ifdef ZHLT_TEXLIGHT
static vec3_t   emitlight[MAX_PATCHES][MAXLIGHTMAPS]; //LRC
static vec3_t   addlight[MAX_PATCHES][MAXLIGHTMAPS]; //LRC
#else
static vec3_t   emitlight[MAX_PATCHES];
static vec3_t   addlight[MAX_PATCHES];
#endif

vec3_t          g_face_offset[MAX_MAP_FACES];              // for rotating bmodels

vec_t           g_direct_scale = DEFAULT_DLIGHT_SCALE;

unsigned        g_numbounce = DEFAULT_BOUNCE;              // 3; /* Originally this was 8 */
bool			g_bounce_dynamic = DEFAULT_BOUNCE_DYNAMIC; //false

static bool     g_dumppatches = DEFAULT_DUMPPATCHES;

vec3_t          g_ambient = { DEFAULT_AMBIENT_RED, DEFAULT_AMBIENT_GREEN, DEFAULT_AMBIENT_BLUE };
float           g_maxlight = DEFAULT_MAXLIGHT;             // 196  /* Originally this was 196 */

float           g_lightscale = DEFAULT_LIGHTSCALE;
float           g_dlight_threshold = DEFAULT_DLIGHT_THRESHOLD;  // was DIRECT_LIGHT constant

char            g_source[_MAX_PATH] = "";

char            g_vismatfile[_MAX_PATH] = "";
bool            g_incremental = DEFAULT_INCREMENTAL;
#ifndef HLRAD_WHOME
float           g_qgamma = DEFAULT_GAMMA;
#endif
float           g_indirect_sun = DEFAULT_INDIRECT_SUN;
bool            g_extra = DEFAULT_EXTRA;
bool            g_texscale = DEFAULT_TEXSCALE;

float           g_smoothing_threshold;
float           g_smoothing_value = DEFAULT_SMOOTHING_VALUE;

bool            g_circus = DEFAULT_CIRCUS;
bool            g_allow_opaques = DEFAULT_ALLOW_OPAQUES;

// --------------------------------------------------------------------------
// Changes by Adam Foster - afoster@compsoc.man.ac.uk
#ifdef HLRAD_WHOME
vec3_t		g_colour_qgamma = { DEFAULT_COLOUR_GAMMA_RED, DEFAULT_COLOUR_GAMMA_GREEN, DEFAULT_COLOUR_GAMMA_BLUE };
vec3_t		g_colour_lightscale = { DEFAULT_COLOUR_LIGHTSCALE_RED, DEFAULT_COLOUR_LIGHTSCALE_GREEN, DEFAULT_COLOUR_LIGHTSCALE_BLUE };
vec3_t		g_colour_jitter_hack = { DEFAULT_COLOUR_JITTER_HACK_RED, DEFAULT_COLOUR_JITTER_HACK_GREEN, DEFAULT_COLOUR_JITTER_HACK_BLUE };
vec3_t		g_jitter_hack = { DEFAULT_JITTER_HACK_RED, DEFAULT_JITTER_HACK_GREEN, DEFAULT_JITTER_HACK_BLUE };
bool		g_diffuse_hack = DEFAULT_DIFFUSE_HACK;
bool		g_spotlight_hack = DEFAULT_SPOTLIGHT_HACK;
vec3_t		g_softlight_hack = { DEFAULT_SOFTLIGHT_HACK_RED, DEFAULT_SOFTLIGHT_HACK_GREEN, DEFAULT_SOFTLIGHT_HACK_BLUE };
float		g_softlight_hack_distance = DEFAULT_SOFTLIGHT_HACK_DISTANCE;
#endif
// --------------------------------------------------------------------------

#ifdef HLRAD_HULLU
bool		g_customshadow_with_bouncelight = DEFAULT_CUSTOMSHADOW_WITH_BOUNCELIGHT;
bool		g_rgb_transfers = DEFAULT_RGB_TRANSFERS;
#endif

// Cosine of smoothing angle(in radians)
float           g_coring = DEFAULT_CORING;                 // Light threshold to force to blackness(minimizes lightmaps)
bool            g_chart = DEFAULT_CHART;
bool            g_estimate = DEFAULT_ESTIMATE;
bool            g_info = DEFAULT_INFO;

#ifdef ZHLT_PROGRESSFILE // AJM
char*           g_progressfile = DEFAULT_PROGRESSFILE; // "-progressfile path"
#endif

// Patch creation and subdivision criteria
bool            g_subdivide = DEFAULT_SUBDIVIDE;
vec_t           g_chop = DEFAULT_CHOP;
vec_t           g_texchop = DEFAULT_TEXCHOP;
bool            g_nohunt = DEFAULT_NOHUNT;

// Opaque faces
opaqueList_t*   g_opaque_face_list = NULL;
unsigned        g_opaque_face_count = 0;
unsigned        g_max_opaque_face_count = 0;               // Current array maximum (used for reallocs)

// Misc
int             leafparents[MAX_MAP_LEAFS];
int             nodeparents[MAX_MAP_NODES];

#ifdef ZHLT_INFO_COMPILE_PARAMETERS
// =====================================================================================
//  GetParamsFromEnt
//      this function is called from parseentity when it encounters the 
//      info_compile_parameters entity. each tool should have its own version of this
//      to handle its own specific settings.
// =====================================================================================
void            GetParamsFromEnt(entity_t* mapent)
{
    int     iTmp;
    float   flTmp;
    char    szTmp[256];
    const char* pszTmp;

    Log("\nCompile Settings detected from info_compile_parameters entity\n");

    // lightdata(string) : "Lighting Data Memory" : "8192"
    iTmp = IntForKey(mapent, "lightdata") * 1024;
    if (iTmp > g_max_map_miptex)
    {
        g_max_map_lightdata = iTmp;
    }
    sprintf_s(szTmp, "%i", g_max_map_lightdata);
    Log("%30s [ %-9s ]\n", "Lighting Data Memory", szTmp);

    // verbose(choices) : "Verbose compile messages" : 0 = [ 0 : "Off" 1 : "On" ]
    iTmp = IntForKey(mapent, "verbose");
    if (iTmp == 1)
    {
        g_verbose = true;
    }
    else if (iTmp == 0)
    {
        g_verbose = false;
    }
    Log("%30s [ %-9s ]\n", "Compile Option", "setting");
    Log("%30s [ %-9s ]\n", "Verbose Compile Messages", g_verbose ? "on" : "off");

    // estimate(choices) :"Estimate Compile Times?" : 0 = [ 0: "Yes" 1: "No" ]
    if (IntForKey(mapent, "estimate")) 
    {
        g_estimate = true;
    }
    else
    {
        g_estimate = false;
    }
    Log("%30s [ %-9s ]\n", "Estimate Compile Times", g_estimate ? "on" : "off");

	// priority(choices) : "Priority Level" : 0 = [	0 : "Normal" 1 : "High"	-1 : "Low" ]
	if (!strcmp(ValueForKey(mapent, "priority"), "1"))
    {
        g_threadpriority = eThreadPriorityHigh;
        Log("%30s [ %-9s ]\n", "Thread Priority", "high");
    }
    else if (!strcmp(ValueForKey(mapent, "priority"), "-1"))
    {
        g_threadpriority = eThreadPriorityLow;
        Log("%30s [ %-9s ]\n", "Thread Priority", "low");
    }

    // bounce(integer) : "Number of radiosity bounces" : 0 
    iTmp = IntForKey(mapent, "bounce");
    if (iTmp)
    {
        g_numbounce = abs(iTmp);
        Log("%30s [ %-9s ]\n", "Number of radiosity bounces", ValueForKey(mapent, "bounce"));
    }

	if(IntForKey(mapent,"nodynbounce"))
	{
		g_bounce_dynamic = false;
	}
	else
	{
		g_bounce_dynamic = true;
	}
	Log("%30s [ %-9s ]\n", "Bounce dynamic lights", g_bounce_dynamic ? "on" : "off");

    
#ifdef HLRAD_HULLU
    iTmp = IntForKey(mapent, "customshadowwithbounce");
    if (iTmp)
    {  
    	g_customshadow_with_bouncelight = true;
    	Log("%30s [ %-9s ]\n", "Custom Shadow with Bounce Light", ValueForKey(mapent, "customshadowwithbounce"));
    }
    iTmp = IntForKey(mapent, "rgbtransfers");
    if (iTmp)
    {  
    	g_rgb_transfers = true;
    	Log("%30s [ %-9s ]\n", "RGB Transfers", ValueForKey(mapent, "rgbtransfers"));
    }
#endif

    // ambient(string) : "Ambient world light (0.0 to 1.0, R G B)" : "0 0 0" 
    //vec3_t          g_ambient = { DEFAULT_AMBIENT_RED, DEFAULT_AMBIENT_GREEN, DEFAULT_AMBIENT_BLUE };
    pszTmp = ValueForKey(mapent, "ambient");
    if (pszTmp)
    {
        float red = 0, green = 0, blue = 0;
        if (sscanf_s(pszTmp, "%f %f %f", &red, &green, &blue))
        {
            if (red < 0 || red > 1 || green < 0 || green > 1 || blue < 0 || blue > 1)
            {
                Error("info_compile_parameters: Ambient World Light (ambient) all 3 values must be within the range of 0.0 to 1.0\n"
                      "Parsed values:\n"
                      "    red [ %1.3f ] %s\n"
                      "  green [ %1.3f ] %s\n"
                      "   blue [ %1.3f ] %s\n"
                      , red,    (red   < 0 || red   > 1) ? "OUT OF RANGE" : ""
                      , green,  (green < 0 || green > 1) ? "OUT OF RANGE" : ""
                      , blue,   (blue  < 0 || blue  > 1) ? "OUT OF RANGE" : "" );
            }

            if (red == 0 && green == 0 && blue == 0)
            {} // dont bother setting null values
            else
            {
                g_ambient[0] = red * 128;
                g_ambient[1] = green * 128;
                g_ambient[2] = blue * 128;
                Log("%30s [ %1.3f %1.3f %1.3f ]\n", "Ambient world light (R G B)", red, green, blue);
            }
        }
        else
        {
            Error("info_compile_parameters: Ambient World Light (ambient) has unrecognised value\n"
                  "This keyvalue accepts 3 numeric values from 0.000 to 1.000, use \"0 0 0\" if in doubt");
        }
    }

    // smooth(integer) : "Smoothing threshold (in degrees)" : 0 
    flTmp = FloatForKey(mapent, "smooth");
    if (flTmp)
    {
        g_smoothing_threshold = flTmp;
        Log("%30s [ %-9s ]\n", "Smoothing threshold", ValueForKey(mapent, "smooth"));
    }

    // dscale(integer) : "Direct Lighting Scale" : 1 
    flTmp = FloatForKey(mapent, "dscale");
    if (flTmp)
    {
        g_direct_scale = flTmp;
        Log("%30s [ %-9s ]\n", "Direct Lighting Scale", ValueForKey(mapent, "dscale"));
    }

    // chop(integer) : "Chop Size" : 64 
    iTmp = IntForKey(mapent, "chop");
    if (iTmp)
    {
        g_chop = iTmp;
        Log("%30s [ %-9s ]\n", "Chop Size", ValueForKey(mapent, "chop"));
    }

    // texchop(integer) : "Texture Light Chop Size" : 32 
    flTmp = FloatForKey(mapent, "texchop");
    if (flTmp)
    {
        g_texchop = flTmp;
        Log("%30s [ %-9s ]\n", "Texture Light Chop Size", ValueForKey(mapent, "texchop"));
    }

    /* 
    hlrad(choices) : "HLRAD" : 0 =
    [
        0 : "Off"
        1 : "Normal"
        2 : "Extra"
    ]
    */
    iTmp = IntForKey(mapent, "hlrad");
    if (iTmp == 0)
    {
        Fatal(assume_TOOL_CANCEL, 
            "%s flag was not checked in info_compile_parameters entity, execution of %s cancelled", g_Program, g_Program);
        CheckFatal();  
    }
    else if (iTmp == 1)
    {
        g_extra = false;
    }
    else if (iTmp == 2)
    {
        g_extra = true;
    }
    Log("%30s [ %-9s ]\n", "Extra RAD", g_extra ? "on" : "off");
 
    /*
    sparse(choices) : "Vismatrix Method" : 2 =
    [
        0 : "No Vismatrix"
        1 : "Sparse Vismatrix"
        2 : "Normal"
    ]
    */
    iTmp = IntForKey(mapent, "sparse");
    if (iTmp == 1)
    {
        g_method = eMethodSparseVismatrix;
    }
    else if (iTmp == 0)
    {
        g_method = eMethodNoVismatrix;
    }
    else if (iTmp == 2)
    {
        g_method = eMethodVismatrix;
    }
    Log("%30s [ %-9s ]\n", "Sparse Vismatrix",  g_method == eMethodSparseVismatrix ? "on" : "off");
    Log("%30s [ %-9s ]\n", "NoVismatrix",  g_method == eMethodNoVismatrix ? "on" : "off");

    /*
    circus(choices) : "Circus RAD lighting" : 0 =
    [
        0 : "Off"
        1 : "On"
    ]
    */
    iTmp = IntForKey(mapent, "circus");
    if (iTmp == 0)
    {
        g_circus = false;
    }
    else if (iTmp == 1)
    {
        g_circus = true;
    }

    Log("%30s [ %-9s ]\n", "Circus Lighting Mode", g_circus ? "on" : "off");

    ////////////////////
    Log("\n");
}
#endif

// =====================================================================================
//  Extract File stuff (ExtractFile | ExtractFilePath | ExtractFileBase)
//
// With VS 2005 - and the 64 bit build, i had to pull 3 classes over from
// cmdlib.cpp even with the proper includes to get rid of the lnk2001 error
//
// amckern - amckern@yahoo.com
// =====================================================================================

#define PATHSEPARATOR(c) ((c) == '\\' || (c) == '/')

void            ExtractFileBase(const char* const path, char* dest)
{
    hlassert (path != dest);

    const char*           src;

    src = path + strlen(path) - 1;

    //
    // back up until a \ or the start
    //
    while (src != path && !PATHSEPARATOR(*(src - 1)))
        src--;

    while (*src && *src != '.')
    {
        *dest++ = *src++;
    }
    *dest = 0;
}

void            ExtractFilePath(const char* const path, char* dest)
{
    hlassert (path != dest);

    const char*           src;

    src = path + strlen(path) - 1;

    //
    // back up until a \ or the start
    //
    while (src != path && !PATHSEPARATOR(*(src - 1)))
        src--;

    memcpy(dest, path, src - path);
    dest[src - path] = 0;
}

void            ExtractFile(const char* const path, char* dest)
{
    hlassert (path != dest);

    const char*           src;

    src = path + strlen(path) - 1;

    while (src != path && !PATHSEPARATOR(*(src - 1)))
        src--;

    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = 0;
}

// =====================================================================================
//  MakeParents
//      blah
// =====================================================================================
static void     MakeParents(const int nodenum, const int parent)
{
    int             i;
    int             j;
    dnode_t*        node;

    nodeparents[nodenum] = parent;
    node = g_dnodes + nodenum;

    for (i = 0; i < 2; i++)
    {
        j = node->children[i];
        if (j < 0)
        {
            leafparents[-j - 1] = nodenum;
        }
        else
        {
            MakeParents(j, nodenum);
        }
    }
}

// =====================================================================================
//
//    TEXTURE LIGHT VALUES
//
// =====================================================================================

// misc
typedef struct
{
    std::string     name;
    vec3_t          value;
    const char*     filename;
}
texlight_t;

static std::vector< texlight_t > s_texlights;
typedef std::vector< texlight_t >::iterator texlight_i;

// =====================================================================================
//  ReadLightFile
// =====================================================================================
static void     ReadLightFile(const char* const filename)
{
    FILE*           f;
    char            scan[MAXTOKEN];
    short           argCnt;
    unsigned int    file_texlights = 0;

    f = fopen(filename, "r");
    if (!f)
    {
        Warning("Could not open texlight file %s", filename);
        return;
    }
    else
    {
        Log("[Reading texlights from '%s']\n", filename);
    }

    while (fgets(scan, sizeof(scan), f))
    {
        char*           comment;
        char            szTexlight[_MAX_PATH];
        vec_t           r, g, b, i = 1;

        comment = strstr(scan, "//");
        if (comment)
        {
            // Newline and Null terminate the string early if there is a c++ style single line comment
            comment[0] = '\n';
            comment[1] = 0;
        }

        argCnt = sscanf/*_s*/(scan, "%s %f %f %f %f", szTexlight, &r, &g, &b, &i);

        if (argCnt == 2)
        {
            // With 1+1 args, the R,G,B values are all equal to the first value
            g = b = r;
        }
        else if (argCnt == 5)
        {
            // With 1+4 args, the R,G,B values are "scaled" by the fourth numeric value i;
            r *= i / 255.0;
            g *= i / 255.0;
            b *= i / 255.0;
        }
        else if (argCnt != 4)
        {
            if (strlen(scan) > 4)
            {
                Warning("ignoring bad texlight '%s' in %s", scan, filename);
            }
            continue;
        }

        texlight_i it;
        for (it = s_texlights.begin(); it != s_texlights.end(); it++)
        {
            if (strcmp(it->name.c_str(), szTexlight) == 0)
            {
                if (strcmp(it->filename, filename) == 0)
                {
                    Warning("Duplication of texlight '%s' in file '%s'!", it->name.c_str(), it->filename);
                }
                else if (it->value[0] != r || it->value[1] != g || it->value[2] != b)
                {
                    Warning("Overriding '%s' from '%s' with '%s'!", it->name.c_str(), it->filename, filename);
                }
                else
                {
                    Warning("Redundant '%s' def in '%s' AND '%s'!", it->name.c_str(), it->filename, filename);
                }
                s_texlights.erase(it);
                break;
            }
        }

        texlight_t      texlight;
        texlight.name = szTexlight;
        texlight.value[0] = r;
        texlight.value[1] = g;
        texlight.value[2] = b;
        texlight.filename = filename;
        file_texlights++;
        s_texlights.push_back(texlight);
    }
    Log("[%u texlights parsed from '%s']\n\n", file_texlights, filename);
}

// =====================================================================================
//  LightForTexture
// =====================================================================================
static void     LightForTexture(const char* const name, vec3_t result)
{
    texlight_i it;
    for (it = s_texlights.begin(); it != s_texlights.end(); it++)
    {
        if (!_stricmp(name, it->name.c_str()))
        {
            VectorCopy(it->value, result);
            return;
        }
    }
    VectorClear(result);
}


// =====================================================================================
//
//    MAKE FACES
//
// =====================================================================================

// =====================================================================================
//  BaseLightForFace
// =====================================================================================
static void     BaseLightForFace(const dface_t* const f, vec3_t light)
{
    LightForTexture(g_dtextures[f->texture_index].str, light);
}

// =====================================================================================
//  IsSpecial
// =====================================================================================
static bool     IsSpecial(const dface_t* const f)
{
    //return g_texinfo[f->texinfo].flags & TEX_SPECIAL;
    return IsSpecial(f->flags);
}

// =====================================================================================
//  IsSpecial
// =====================================================================================
static bool     IsSpecial(unsigned int flags)
{
    //return g_texinfo[f->texinfo].flags & TEX_SPECIAL;
    return (flags & (CONTENTS_BSP | CONTENTS_UNKNOWN2 | SURFACEFLAG_NODRAW | CONTENTS_PORTAL | CONTENTS_SKY | CONTENTS_SOLID | CONTENTS_EMPTY)) != 0;
}

// =====================================================================================
//  PlacePatchInside
// =====================================================================================
static bool     PlacePatchInside(patch_t* patch)
{
    const dplane_t* plane;
    const vec_t*    face_offset = g_face_offset[patch->faceNumber];

    plane = getPlaneFromFaceNumber(patch->faceNumber);

    if (!HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, 0.01) &&
        !HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, 0.1) &&
        !HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, 0.5) &&
        !HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, -0.01) &&
        !HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, -0.1))
    {
        // Try offsetting it by the plane normal (1 unit away) and try again

        VectorAdd(plane->normal, patch->origin, patch->origin); // Original offset-into-world method
        //if (PointInLeaf(patch->origin) == g_dleafs)
        if ((PointInLeaf(patch->origin)->contents & 0xFF) != CONTENTS_SOLID)
        {
            if (!HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, 0.01) &&
                !HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, 0.1) &&
                !HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, 0.5) &&
                !HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, -0.01) &&
                !HuntForWorld(patch->origin, face_offset, plane, 11, 0.1, -0.1))
            {
                patch->flags = (ePatchFlags)(patch->flags | ePatchFlagOutside);
                Developer(DEVELOPER_LEVEL_MESSAGE, "Patch @ (%4.3f %4.3f %4.3f) outside world\n",
                          patch->origin[0], patch->origin[1], patch->origin[2]);
                return false;
            }
        }
    }

    return true;
}


// =====================================================================================
//
//    SUBDIVIDE PATCHES
//
// =====================================================================================

// misc
#define MAX_SUBDIVIDE 16384
static Winding* windingArray[MAX_SUBDIVIDE];
static unsigned g_numwindings = 0;

// =====================================================================================
//  AddWindingToArray
// =====================================================================================
static void     AddWindingToArray(Winding* winding)
{
    unsigned        x;

    Winding**       wA = windingArray;

    for (x = 0; x < g_numwindings; x++, wA++)
    {
        if (*wA == winding)
        {
            return;
        }
    }

    windingArray[g_numwindings++] = winding;
}

static void     CreateStrips_r(Winding* winding, const vec3_t plane_normal, const vec_t plane_dist, vec_t step)
{
    Winding*        A;
    Winding*        B;
    vec_t           areaA;
    vec_t           areaB;

    winding->Clip(plane_normal, plane_dist + step, &A, &B);

    if (A && B)
    {
        areaA = A->getArea();
        areaB = B->getArea();
        if ((areaA > 1.0) && (areaB > 1.0))
        {
            delete winding;
            CreateStrips_r(A, plane_normal, plane_dist + step, step);
            CreateStrips_r(B, plane_normal, plane_dist + step, step);
            return;
        }
    }
    else
    {                                                      // Try the other direction
        if (A)
        {
            delete A;
        }
        if (B)
        {
            delete B;
        }

        winding->Clip(plane_normal, plane_dist - step, &A, &B);

        if (A && B)
        {
            areaA = A->getArea();
            areaB = B->getArea();
            if ((areaA > 1.0) && (areaB > 1.0))
            {
                delete winding;
                CreateStrips_r(A, plane_normal, plane_dist - step, step);
                CreateStrips_r(B, plane_normal, plane_dist - step, step);
                return;
            }
        }
    }

    // Last recursion, save it into the list
    if (A)
    {
        delete A;
    }
    if (B)
    {
        delete B;
    }

    AddWindingToArray(winding);
    hlassume(g_numwindings < MAX_SUBDIVIDE, assume_GENERIC);
}

// =====================================================================================
//  CreateStrips
// =====================================================================================
static bool     CreateStrips(Winding* winding, const dplane_t* plane, vec_t step)
{
    Winding*        A;
    Winding*        B;
    vec_t           areaA;
    vec_t           areaB;

    winding->Clip(plane->normal, plane->dist, &A, &B);

    if (A && B)
    {
        areaA = A->getArea();
        areaB = B->getArea();
        if ((areaA > 1.0) && (areaB > 1.0))
        {
            CreateStrips_r(A, (vec_t*)plane->normal, plane->dist, step);
            CreateStrips_r(B, (vec_t*)plane->normal, plane->dist, step);
            return true;
        }
    }

    if (A)
    {
        delete A;
    }
    if (B)
    {
        delete B;
    }

    AddWindingToArray(winding);
    hlassume(g_numwindings < MAX_SUBDIVIDE, assume_GENERIC);
    return false;
}

// =====================================================================================
//  cutWindingWithGrid
//      Caller must free this returned value at some point
// =====================================================================================
static void     cutWindingWithGrid(patch_t* patch, const dplane_t* const plA, const dplane_t* const plB)
{
    Winding**       winding;
    unsigned int    count;
    unsigned int    x;

    g_numwindings = 0;
    if (CreateStrips(patch->winding, plA, patch->chop))
    {
        delete patch->winding;
        patch->winding = NULL;                             // Invalidated by CreateStrips routine
    }
    count = g_numwindings;

    for (x = 0, winding = windingArray; x < count; x++, winding++)
    {
        if (CreateStrips(*winding, plB, patch->chop))
        {
            delete *winding;
            *winding = NULL;
        }
    }
}

// =====================================================================================
//  getGridPlanes
//      From patch, determine perpindicular grid planes to subdivide with (returned in planeA and planeB)
//      assume S and T is perpindicular (they SHOULD be in worldcraft 3.3 but aren't always . . .)
// =====================================================================================
static void     getGridPlanes(const patch_t* const p, dplane_t* const pl)
{
    const patch_t*  patch = p;
    dplane_t*       planes = pl;
    const dface_t*  f = g_dFaces + patch->faceNumber;

    dtexmatrix_t* texmatrix = &g_dtexmatrix[f->lightmap_info_projection_index];

    //texinfo_t*      tx = &g_texinfo[f->texinfo];
    dplane_t*       plane = planes;
    const dplane_t* faceplane = getPlaneFromFaceNumber(patch->faceNumber);
    int             x;

    for (x = 0; x < 2; x++, plane++)
    {
        vec3_t          a, b, c;
        vec3_t          delta1, delta2;

        vec_t* vecs = x == 0 ? texmatrix->s : texmatrix->t;

        VectorCopy(patch->origin, a);
        VectorAdd(patch->origin, faceplane->normal, b);
        VectorAdd(patch->origin, vecs, c);

        VectorSubtract(b, a, delta1);
        VectorSubtract(c, a, delta2);

        CrossProduct(delta1, delta2, plane->normal);
        VectorNormalize(plane->normal);
        plane->dist = DotProduct(plane->normal, patch->origin);
    }
}

// =====================================================================================
//  SubdividePatch
// =====================================================================================
static void     SubdividePatch(patch_t* patch)
{
    dplane_t        planes[2];
    dplane_t*       plA = &planes[0];
    dplane_t*       plB = &planes[1];
    Winding**       winding;
    unsigned        x;
    patch_t*        new_patch;

    memset(windingArray, 0, sizeof(windingArray));
    g_numwindings = 0;

    getGridPlanes(patch, planes);
    cutWindingWithGrid(patch, plA, plB);

    x = 0;
    patch->next = NULL;
    winding = windingArray;
    while (*winding == NULL)
    {
        winding++;
        x++;
    }
    patch->winding = *winding;
    winding++;
    x++;
    patch->area = patch->winding->getArea();
    patch->winding->getCenter(patch->origin);
    PlacePatchInside(patch);

    new_patch = g_patches + g_num_patches;
    for (; x < g_numwindings; x++, winding++)
    {
        if (*winding)
        {
            memcpy(new_patch, patch, sizeof(patch_t));

            new_patch->winding = *winding;
            new_patch->area = new_patch->winding->getArea();
            new_patch->winding->getCenter(new_patch->origin);

            if (PlacePatchInside(new_patch))
            {
                new_patch++;
                g_num_patches++;
            }
            else
            {
                if (new_patch->winding)
                    delete new_patch->winding;
                new_patch->winding = NULL;
            }
            hlassume(g_num_patches < MAX_PATCHES, assume_MAX_PATCHES);
        }
    }

    // ATTENTION: We let SortPatches relink all the ->next correctly! instead of doing it here too which is somewhat complicated
}

// =====================================================================================
//  MakePatchForFace
static float    totalarea = 0;
// =====================================================================================
static vec_t    getScale(const patch_t* const patch)
{
    dface_t*        f = g_dFaces + patch->faceNumber;
    dtexmatrix_t* texmatrix = &g_dtexmatrix[f->lightmap_info_projection_index];
   // texinfo_t*      tx = &g_texinfo[f->texinfo];

    if (g_texscale)
    {
        vec_t           scale[2];

        vec_t* s = texmatrix->s;
        vec_t* t = texmatrix->t;

        scale[0] = 0.0;
        scale[1] = 0.0;

        scale[0] += s[0] * s[0];
        scale[0] += s[1] * s[1];
        scale[0] += s[2] * s[2];

        scale[1] += t[0] * t[0];
        scale[1] += t[1] * t[1];
        scale[1] += t[2] * t[2];

        scale[0] = sqrt(scale[0]);
        scale[1] = sqrt(scale[1]);

        return 2.0 / ((scale[0] + scale[1]));
    }
    else
    {
        return 1.0;
    }
}

// =====================================================================================
//  getChop
// =====================================================================================
static vec_t    getChop(const patch_t* const patch)
{
    vec_t           rval;

    if (VectorCompare(patch->baselight, vec3_origin))
    {
        rval = g_chop * getScale(patch);
    }
    else
    {
        rval = g_texchop * getScale(patch);
        if (g_extra)
        {
            rval *= 0.5;
        }
    }

    return rval;
}

// =====================================================================================
//  MakePatchForFace
// =====================================================================================
#ifdef ZHLT_TEXLIGHT
static void     MakePatchForFace(const int fn, Winding* w, int style) //LRC
#else
static void     MakePatchForFace(const int fn, Winding* w)
#endif
{
    const dface_t*  f = g_dFaces + fn;

    // No g_patches at all for the sky!
    if (!IsSpecial(f))
    {
        patch_t*        patch;
        vec3_t          light;
        vec3_t          centroid = { 0, 0, 0 };

        int             numpoints = w->m_NumPoints;

        if (numpoints < 3)                                 // WTF! (Actually happens in real-world maps too)
        {
            Developer(DEVELOPER_LEVEL_WARNING, "Face %d only has %d points on winding\n", fn, numpoints);
            return;
        }
        if (numpoints > MAX_POINTS_ON_WINDING)
        {
            Error("numpoints %d > MAX_POINTS_ON_WINDING", numpoints);
            return;
        }

        patch = &g_patches[g_num_patches];
        hlassume(g_num_patches < MAX_PATCHES, assume_MAX_PATCHES);
        memset(patch, 0, sizeof(patch_t));

        patch->winding = w;

        patch->area = patch->winding->getArea();
        patch->winding->getCenter(patch->origin);
        patch->faceNumber = fn;

        totalarea += patch->area;

        PlacePatchInside(patch);

        BaseLightForFace(f, light);
#ifdef ZHLT_TEXLIGHT
        //LRC        VectorCopy(light, patch->totallight);
#else
        VectorCopy(light, patch->totallight);
#endif
        VectorCopy(light, patch->baselight);

#ifdef ZHLT_TEXLIGHT
        //LRC
		int i;
		patch->totalstyle[0] = 0;
		for (i = 1; i < MAXLIGHTMAPS; i++)
		{
			patch->totalstyle[i] = 255;
		}
		if (style)
		{
			patch->emitstyle = patch->totalstyle[1] = style;
		}
        //LRC (ends)
#endif

        patch->scale = getScale(patch);
        patch->chop = getChop(patch);

        g_face_patches[fn] = patch;
        g_num_patches++;

        // Per-face data
        {
            int             j;

            // Centroid of face for nudging samples in direct lighting pass
            for (j = 0; j < f->num_vertices; j++)
            {
                VectorAdd(g_dverts[f->first_vertex_index + j].point, centroid, centroid);
            }

            // Fixup centroid for anything with an altered origin (rotating models/turrets mostly)
            // Save them for moving direct lighting points towards the face center
            VectorScale(centroid, 1.0 / f->num_vertices, centroid);
            VectorAdd(centroid, g_face_offset[fn], g_face_centroids[fn]);
        }

        {
            vec3_t          mins;
            vec3_t          maxs;

            patch->winding->getBounds(mins, maxs);

            if (g_subdivide)
            {
                vec_t           amt;
                vec_t           length;
                vec3_t          delta;

                VectorSubtract(maxs, mins, delta);
                length = VectorLength(delta);
                if (VectorCompare(patch->baselight, vec3_origin))
                {
                    amt = g_chop;
                }
                else
                {
                    amt = g_texchop;
                }

                if (length > amt)
                {
                    if (patch->area < 1.0)
                    {
                        Developer(DEVELOPER_LEVEL_WARNING,
                                  "Patch at (%4.3f %4.3f %4.3f) (face %d) tiny area (%4.3f) not subdividing \n",
                                  patch->origin[0], patch->origin[1], patch->origin[2], patch->faceNumber, patch->area);
                    }
                    else
                    {
                        SubdividePatch(patch);
                    }
                }
            }
        }
    }
}

// =====================================================================================
//  AddFaceToOpaqueList
// =====================================================================================
#ifdef HLRAD_HULLU
static void     AddFaceToOpaqueList(const unsigned facenum, const Winding* const winding, const vec3_t &transparency_scale, const bool transparency)
#else
static void     AddFaceToOpaqueList(const unsigned facenum, const Winding* const winding)
#endif
{
    if (g_opaque_face_count == g_max_opaque_face_count)
    {
        g_max_opaque_face_count += OPAQUE_ARRAY_GROWTH_SIZE;
        g_opaque_face_list = (opaqueList_t*)realloc(g_opaque_face_list, sizeof(opaqueList_t) * g_max_opaque_face_count);
    }

    {
        opaqueList_t*   opaque = &g_opaque_face_list[g_opaque_face_count];

        g_opaque_face_count++;

#ifdef HLRAD_HULLU
        VectorCopy(transparency_scale, opaque->transparency_scale);
        opaque->transparency = transparency;
#endif
        opaque->facenum = facenum;
        getAdjustedPlaneFromFaceNumber(facenum, &opaque->plane);
        opaque->winding = new Winding(*winding);

		//build bounding box
		opaque->winding->getBounds(opaque->mins,opaque->maxs);
    }
}

// =====================================================================================
//  FreeOpaqueFaceList
// =====================================================================================
static void     FreeOpaqueFaceList()
{
    unsigned        x;
    opaqueList_t*   opaque = g_opaque_face_list;

    for (x = 0; x < g_opaque_face_count; x++, opaque++)
    {
        delete opaque->winding;
        opaque->winding = NULL;
    }
    free(g_opaque_face_list);

    g_opaque_face_list = NULL;
    g_opaque_face_count = 0;
    g_max_opaque_face_count = 0;
}

// =====================================================================================
//  MakePatches
// =====================================================================================
static void     MakePatches(mapinfo_t* map)
{
    int             i;
    int             j;
    unsigned int    k;
    dface_t*        f;
    int             fn;
    Winding*        w;
    dmodel_t*       mod;
    vec3_t          origin;
    entity_t*       ent;
    const char*     s;
    vec3_t          light_origin;
    vec3_t          model_center;
    bool            b_light_origin;
    bool            b_model_center;
    eModelLightmodes lightmode;

#ifdef ZHLT_TEXLIGHT
    int				style; //LRC
#endif

#ifdef HLRAD_HULLU
    vec3_t		d_transparency;
    bool		b_transparency;
#endif

    Log("%i faces\n", g_numDFaces);

    Log("Create Patches : ");

    for (i = 0; i < g_numDModels; i++)
    {
        b_light_origin = false;
        b_model_center = false;
        lightmode = eModelLightmodeNull;

#ifdef HLRAD_OPACITY // AJM
        float         l_opacity = 0.0f; // decimal percentage 
#endif

        mod = g_dmodels + i;
        ent = EntityForModel(i, map);
        VectorCopy(vec3_origin, origin);

        if (*(s = ValueForKey(ent, "zhlt_lightflags")))
        {
            lightmode = (eModelLightmodes)atoi(s);
        }

        // models with origin brushes need to be offset into their in-use position
        if (*(s = ValueForKey(ent, "origin")))
        {
            double          v1, v2, v3;

            if (sscanf_s(s, "%lf %lf %lf", &v1, &v2, &v3) == 3)
            {
                origin[0] = v1;
                origin[1] = v2;
                origin[2] = v3;
            }

        }

        // Allow models to be lit in an alternate location (pt1)
        if (*(s = ValueForKey(ent, "light_origin")))
        {
            entity_t*       e = FindTargetEntity(map, s);

            if (e)
            {
                if (*(s = ValueForKey(e, "origin")))
                {
                    double          v1, v2, v3;

                    if (sscanf_s(s, "%lf %lf %lf", &v1, &v2, &v3) == 3)
                    {
                        light_origin[0] = v1;
                        light_origin[1] = v2;
                        light_origin[2] = v3;

                        b_light_origin = true;
                    }
                }
            }
        }

        // Allow models to be lit in an alternate location (pt2)
        if (*(s = ValueForKey(ent, "model_center")))
        {
            double          v1, v2, v3;

            if (sscanf_s(s, "%lf %lf %lf", &v1, &v2, &v3) == 3)
            {
                model_center[0] = v1;
                model_center[1] = v2;
                model_center[2] = v3;

                b_model_center = true;
            }
        }

#ifdef HLRAD_HULLU
	// Check for colored transparency/custom shadows
        VectorFill(d_transparency, 1.0);
        b_transparency = false;
        
        if (*(s = ValueForKey(ent, "zhlt_customshadow")))
        {
        	double r1 = 1.0, g1 = 1.0, b1 = 1.0, tmp = 1.0;
        	if (sscanf_s(s, "%lf %lf %lf", &r1, &g1, &b1) == 3) //RGB version
        	{
        		if(r1<0.0) r1 = 0.0;
        		if(g1<0.0) g1 = 0.0;
        		if(b1<0.0) b1 = 0.0;
        		
        		d_transparency[0] = r1;
        		d_transparency[1] = g1;
        		d_transparency[2] = b1;
        		b_transparency = true;
        	}
        	else if (sscanf_s(s, "%lf", &tmp) == 1) //Greyscale version
        	{
        		if(tmp<0.0) tmp = 0.0;
        		
        		VectorFill(d_transparency, tmp);
        		b_transparency = true;
        	}
        }
#endif
        // Allow models to be lit in an alternate location (pt3)
        if (b_light_origin && b_model_center)
        {
            VectorSubtract(light_origin, model_center, origin);
        }

#ifdef ZHLT_TEXLIGHT        
		//LRC:
		/* ummmmm..... something is very, very wrong here. */
		if (*(s = ValueForKey(ent, "style")))
		{
			style = atoi(s);
			if (style < 0)
				style = -style;
		}
		else
		{
			style = 0;
		}
        //LRC (ends)

#endif

        for (j = 0; j < mod->numfaces; j++)
        {
            fn = mod->firstface + j;
            g_face_entity[fn] = ent;
            VectorCopy(origin, g_face_offset[fn]);
            g_face_lightmode[fn] = lightmode;
            f = g_dFaces + fn;
            w = new Winding(*f);
            for (k = 0; k < w->m_NumPoints; k++)
            {
                VectorAdd(w->m_Points[k], origin, w->m_Points[k]);
            }
            if (g_allow_opaques)
            {
                if (lightmode & eModelLightmodeOpaque)
                {
#ifdef HLRAD_HULLU
                    AddFaceToOpaqueList(fn, w, d_transparency, b_transparency); 
#else
                    AddFaceToOpaqueList(fn, w); 
#endif
                }
            }
#ifdef ZHLT_TEXLIGHT
            MakePatchForFace(fn, w, style); //LRC
#else
            MakePatchForFace(fn, w);
#endif
        }
    }

    Log("%i base patches\n", g_num_patches);
    Log("%i opaque faces\n", g_opaque_face_count);
    Log("%i square feet [%.2f square inches]\n", (int)(totalarea / 144), totalarea);
}

// =====================================================================================
//  patch_sorter
// =====================================================================================
static int CDECL patch_sorter(const void* p1, const void* p2)
{
    patch_t*        patch1 = (patch_t*)p1;
    patch_t*        patch2 = (patch_t*)p2;

    if (patch1->faceNumber < patch2->faceNumber)
    {
        return -1;
    }
    else if (patch1->faceNumber > patch2->faceNumber)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// =====================================================================================
//  patch_sorter
//      This sorts the patches by facenumber, which makes their runs compress even better
// =====================================================================================
static void     SortPatches()
{
    qsort((void*)g_patches, (size_t) g_num_patches, sizeof(patch_t), patch_sorter);

    // Fixup g_face_patches & Fixup patch->next
    memset(g_face_patches, 0, sizeof(g_face_patches));
    {
        unsigned        x;
        patch_t*        patch = g_patches + 1;
        patch_t*        prev = g_patches;

        g_face_patches[0] = g_patches;

        for (x = 1; x < g_num_patches; x++, patch++)
        {
            if (patch->faceNumber != prev->faceNumber)
            {
                prev->next = NULL;
                g_face_patches[patch->faceNumber] = patch;
            }
            else
            {
                prev->next = patch;
            }
            prev = patch;
        }
    }
}

// =====================================================================================
//  FreePatches
// =====================================================================================
static void     FreePatches()
{
    unsigned        x;
    patch_t*        patch = g_patches;

    // AJM EX
    //Log("patches: %i of %i (%2.2lf percent)\n", g_num_patches, MAX_PATCHES, (double)((double)g_num_patches / (double)MAX_PATCHES));

    for (x = 0; x < g_num_patches; x++, patch++)
    {
        delete patch->winding;
    }
    memset(g_patches, 0, sizeof(patch_t) * g_num_patches);
}

//=====================================================================

// =====================================================================================
//  WriteWorld
// =====================================================================================
static void     WriteWorld(const char* const name)
{
    unsigned        i;
    unsigned        j;
    FILE*           out;
    patch_t*        patch;
    Winding*        w;

    out = fopen(name, "w");

    if (!out)
        Error("Couldn't open %s", name);

    for (j = 0, patch = g_patches; j < g_num_patches; j++, patch++)
    {
        w = patch->winding;
        Log("%i\n", w->m_NumPoints);
        for (i = 0; i < w->m_NumPoints; i++)
        {
#ifdef ZHLT_TEXLIGHT
            Log("%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
                w->m_Points[i][0],
                w->m_Points[i][1],
                w->m_Points[i][2], patch->totallight[0][0] / 256, patch->totallight[0][1] / 256, patch->totallight[0][2] / 256); //LRC
#else
            Log("%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
                w->m_Points[i][0],
                w->m_Points[i][1],
                w->m_Points[i][2], patch->totallight[0] / 256, patch->totallight[1] / 256, patch->totallight[2] / 256);
#endif
        }
        Log("\n");
    }

    fclose(out);
}

// =====================================================================================
//  CollectLight
// =====================================================================================
static void     CollectLight()
{
#ifdef ZHLT_TEXLIGHT
    unsigned        j; //LRC
#endif
    unsigned        i;
    patch_t*        patch;

    for (i = 0, patch = g_patches; i < g_num_patches; i++, patch++)
    {
#ifdef ZHLT_TEXLIGHT
         //LRC
		for (j = 0; j < MAXLIGHTMAPS && patch->totalstyle[j] != 255; j++)
		{
		    VectorAdd(patch->totallight[j], addlight[i][j], patch->totallight[j]);
	        VectorScale(addlight[i][j], TRANSFER_SCALE, emitlight[i][j]);
			VectorClear(addlight[i][j]);
		}
#else
        VectorAdd(patch->totallight, addlight[i], patch->totallight);
        VectorScale(addlight[i], TRANSFER_SCALE, emitlight[i]);
        VectorClear(addlight[i]);
#endif
    }
}

// =====================================================================================
//  GatherLight
//      Get light from other g_patches
//      Run multi-threaded
// =====================================================================================
#ifdef SYSTEM_WIN32
#pragma warning(push)
#pragma warning(disable: 4100)                             // unreferenced formal parameter
#endif
static void     GatherLight(int threadnum)
{
    int             j;
    patch_t*        patch;

#ifdef ZHLT_TEXLIGHT
    unsigned        k,m; //LRC
//LRC    vec3_t          sum;
#else
    unsigned        k;
    vec3_t          sum;
#endif

    unsigned        iIndex;
    transfer_data_t* tData;
    transfer_index_t* tIndex;

    while (1)
    {
        j = GetThreadWork();
        if (j == -1)
        {
            break;
        }

        patch = &g_patches[j];

        tData = patch->tData;
        tIndex = patch->tIndex;
        iIndex = patch->iIndex;

#ifdef ZHLT_TEXLIGHT
  		//LRC
        for (m = 0; m < MAXLIGHTMAPS && patch->totalstyle[m] != 255; m++)
		{
			VectorClear(addlight[j][m]);
		}
#else
        VectorClear(sum);
#endif

        for (k = 0; k < iIndex; k++, tIndex++)
        {
            unsigned        l;
            unsigned        size = (tIndex->size + 1);
            unsigned        patchnum = tIndex->index;

            for (l = 0; l < size; l++, tData++, patchnum++)
            {
                vec3_t          v;
#ifdef ZHLT_TEXLIGHT
                 //LRC:
				patch_t*		emitpatch = &g_patches[patchnum];
				unsigned		emitstyle;

				// for each style on the emitting patch
				for (emitstyle = 0; emitstyle < MAXLIGHTMAPS && emitpatch->totalstyle[emitstyle] != 255; emitstyle++)
				{
					//if dynamic bounce has been turned off, we ignore dynamic lightstyles
					if(!g_bounce_dynamic && emitstyle != 0)
					{ break; }

					// find the matching style on this (destination) patch
					for (m = 0; m < MAXLIGHTMAPS && patch->totalstyle[m] != 255; m++)
					{
						if (patch->totalstyle[m] == emitpatch->totalstyle[emitstyle])
						{
							break;
						}
					}

					if (m == MAXLIGHTMAPS)
					{
						if(!g_warned_direct || g_verbose)
						{ 
							Warning("Too many light styles on a face(%f,%f,%f)",patch->origin[0],patch->origin[1],patch->origin[2]); 
							g_warned_direct = true;
						}
					}
					else
					{
						if (patch->totalstyle[m] == 255)
						{
							patch->totalstyle[m] = emitpatch->totalstyle[emitstyle];
						}
						VectorScale(emitlight[patchnum][emitstyle], (*tData), v);
						if (isPointFinite(v))
						{
							VectorAdd(addlight[j][m], v, addlight[j][m]);
						}
						else
						{
							Verbose("GatherLight, v (%4.3f %4.3f %4.3f)@(%4.3f %4.3f %4.3f)\n",
								v[0], v[1], v[2], patch->origin[0], patch->origin[1], patch->origin[2]);
						}
					}
				}
#else
                VectorScale(emitlight[patchnum], (*tData), v);
                if (isPointFinite(v))
                {
                    VectorAdd(sum, v, sum);
                }
                else
                {
                    Verbose("GatherLight, v (%4.3f %4.3f %4.3f)@(%4.3f %4.3f %4.3f)\n",
                            v[0], v[1], v[2], patch->origin[0], patch->origin[1], patch->origin[2]);
                }
#endif
            }
        }

#ifdef ZHLT_TEXLIGHT
        //LRC        VectorCopy(sum, addlight[j]);
#else
        VectorCopy(sum, addlight[j]);
#endif
    }
}

// RGB Transfer version
#ifdef HLRAD_HULLU
static void     GatherRGBLight(int threadnum)
{
    int             j;
    patch_t*        patch;

#ifdef ZHLT_TEXLIGHT
    unsigned        k,m; //LRC
//LRC    vec3_t          sum;
#else
    unsigned        k;
    vec3_t          sum;
#endif

    unsigned        iIndex;
    rgb_transfer_data_t* tRGBData;
    transfer_index_t* tIndex;

    while (1)
    {
        j = GetThreadWork();
        if (j == -1)
        {
            break;
        }

        patch = &g_patches[j];

        tRGBData = patch->tRGBData;
        tIndex = patch->tIndex;
        iIndex = patch->iIndex;

#ifdef ZHLT_TEXLIGHT
  		//LRC
        for (m = 0; m < MAXLIGHTMAPS && patch->totalstyle[m] != 255; m++)
		{
			VectorClear(addlight[j][m]);
		}
#else
        VectorClear(sum);
#endif

        for (k = 0; k < iIndex; k++, tIndex++)
        {
            unsigned        l;
            unsigned        size = (tIndex->size + 1);
            unsigned        patchnum = tIndex->index;

            for (l = 0; l < size; l++, tRGBData++, patchnum++)
            {
                vec3_t          v;
#ifdef ZHLT_TEXLIGHT
                 //LRC:
				patch_t*		emitpatch = &g_patches[patchnum];
				unsigned		emitstyle;

				// for each style on the emitting patch
				for (emitstyle = 0; emitstyle < MAXLIGHTMAPS && emitpatch->totalstyle[emitstyle] != 255; emitstyle++)
				{
					//if dynamic bounce has been turned off, we ignore nonzero lightstyles
					if(!g_bounce_dynamic && emitstyle != 0)
					{ break; }

					// find the matching style on this (destination) patch
					for (m = 0; m < MAXLIGHTMAPS && patch->totalstyle[m] != 255; m++)
					{
						if (patch->totalstyle[m] == emitpatch->totalstyle[emitstyle])
						{
							break;
						}
					}

					if (m == MAXLIGHTMAPS)
					{
						if(!g_warned_direct || g_verbose)
						{ 
							Warning("Too many light styles on a face(%f,%f,%f)",patch->origin[0],patch->origin[1],patch->origin[2]); 
							g_warned_direct = true;
						}
					}
					else
					{
						if (patch->totalstyle[m] == 255)
						{
							patch->totalstyle[m] = emitpatch->totalstyle[emitstyle];
//							Log("Granting new style %d to patch at idx %d\n", patch->totalstyle[m], m);
						}
						VectorMultiply(emitlight[patchnum][emitstyle], (*tRGBData), v);
						if (isPointFinite(v))
						{
							VectorAdd(addlight[j][m], v, addlight[j][m]);
						}
						else
						{
							Verbose("GatherLight, v (%4.3f %4.3f %4.3f)@(%4.3f %4.3f %4.3f)\n",
								v[0], v[1], v[2], patch->origin[0], patch->origin[1], patch->origin[2]);
						}
					}
				}
                //LRC (ends)
#else
                VectorMultiply(emitlight[patchnum], (*tRGBData), v);
                if (isPointFinite(v))
                {
                    VectorAdd(sum, v, sum);
                }
                else
                {
                    Verbose("GatherLight, v (%4.3f %4.3f %4.3f)@(%4.3f %4.3f %4.3f)\n",
                            v[0], v[1], v[2], patch->origin[0], patch->origin[1], patch->origin[2]);
                }
#endif
            }
        }

#ifdef ZHLT_TEXLIGHT
        //LRC        VectorCopy(sum, addlight[j]);
#else
        VectorCopy(sum, addlight[j]);
#endif
    }
}
#endif

#ifdef SYSTEM_WIN32
#pragma warning(pop)
#endif

// =====================================================================================
//  BounceLight
// =====================================================================================
static void     BounceLight()
{
    unsigned        i;
    char            name[64];

#ifdef ZHLT_TEXLIGHT
    unsigned        j; //LRC
#endif

    for (i = 0; i < g_num_patches; i++)
    {
#ifdef ZHLT_TEXLIGHT
        //LRC
		for (j = 0; j < MAXLIGHTMAPS && g_patches[i].totalstyle[j] != 255; j++)
		{
	        VectorScale(g_patches[i].totallight[j], TRANSFER_SCALE, emitlight[i][j]);
		}
#else
        VectorScale(g_patches[i].totallight, TRANSFER_SCALE, emitlight[i]);
#endif
    }

    for (i = 0; i < g_numbounce; i++)
    {
        printf("Bounce %u ", i + 1);
#ifdef HLRAD_HULLU
	if(g_rgb_transfers)
	{
		NamedRunThreadsOn(g_num_patches, g_estimate, GatherRGBLight);
	}
	else
	{
		NamedRunThreadsOn(g_num_patches, g_estimate, GatherLight);
	}
#else
		NamedRunThreadsOn(g_num_patches, g_estimate, GatherLight);
#endif
        CollectLight();

        if (g_dumppatches)
        {
            sprintf_s(name, "bounce%u.txt", i);
            WriteWorld(name);
        }
    }
}

// =====================================================================================
//  CheckMaxPatches
// =====================================================================================
static void     CheckMaxPatches()
{
    switch (g_method)
    {
    case eMethodVismatrix:
        hlassume(g_num_patches < MAX_VISMATRIX_PATCHES, assume_MAX_PATCHES);
        break;
    case eMethodSparseVismatrix:
        hlassume(g_num_patches < MAX_SPARSE_VISMATRIX_PATCHES, assume_MAX_PATCHES);
        break;
    case eMethodNoVismatrix:
        hlassume(g_num_patches < MAX_PATCHES, assume_MAX_PATCHES);
        break;
    }
}

// =====================================================================================
//  MakeScalesStub
// =====================================================================================
static void     MakeScalesStub()
{
    switch (g_method)
    {
    case eMethodVismatrix:
        MakeScalesVismatrix();
        break;
    case eMethodSparseVismatrix:
        MakeScalesSparseVismatrix();
        break;
    case eMethodNoVismatrix:
        MakeScalesNoVismatrix();
        break;
    }
}

// =====================================================================================
//  FreeTransfers
// =====================================================================================
static void     FreeTransfers()
{
    unsigned        x;
    patch_t*        patch = g_patches;

    for (x = 0; x < g_num_patches; x++, patch++)
    {
        if (patch->tData)
        {
            FreeBlock(patch->tData);
            patch->tData = NULL;
        }
#ifdef HLRAD_HULLU
        if (patch->tRGBData)
        {
            FreeBlock(patch->tRGBData);
            patch->tRGBData = NULL;
        }
#endif
        if (patch->tIndex)
        {
            FreeBlock(patch->tIndex);
            patch->tIndex = NULL;
        }
    }
}

// =====================================================================================
//  RadWorld
// =====================================================================================
static void     RadWorld(mapinfo_t* map)
{
    unsigned        i;
#ifdef ZHLT_TEXLIGHT
    unsigned        j;
#endif

    MakeBackplanes();
    MakeParents(0, -1);
    MakeTnodes(/*&g_dmodels[0]*/);

    // turn each face into a single patch
    MakePatches(map);
    CheckMaxPatches();                                     // Check here for exceeding max patches, to prevent a lot of work from occuring before an error occurs
    SortPatches();                                         // Makes the runs in the Transfer Compression really good
    PairEdges();

    // create directlights out of g_patches and lights
    CreateDirectLights(map);

    Log("\n");

    // build initial facelights
    NamedRunThreadsOnIndividual(g_numDFaces, g_estimate, BuildFacelights);

    // free up the direct lights now that we have facelights
    DeleteDirectLights();

    if (g_numbounce > 0)
    {
        // build transfer lists
        MakeScalesStub();

        // spread light around
        BounceLight();

        for (i = 0; i < g_num_patches; i++)
        {
#ifdef ZHLT_TEXLIGHT// AJM
            for (j = 0; j < MAXLIGHTMAPS && g_patches[i].totalstyle[j] != 255; j++)
			{
	            VectorSubtract(g_patches[i].totallight[j], g_patches[i].directlight[j], g_patches[i].totallight[j]);
			}
#else
            VectorSubtract(g_patches[i].totallight, g_patches[i].directlight, g_patches[i].totallight);
#endif
        }
    }

    FreeTransfers();

    // blend bounced light into direct light and save
    PrecompLightmapOffsets();

    NamedRunThreadsOnIndividual(g_numDFaces, g_estimate, FinalLightFace);
}

// =====================================================================================
//  Usage
// =====================================================================================
static void     Usage()
{
    Banner();

    Log("\n-= %s Options =-\n\n", g_Program);
    Log("    -sparse         : Enable low memory vismatrix algorithm\n");
    Log("    -nomatrix       : Disable usage of vismatrix entirely\n\n");
    Log("    -extra          : Improve lighting quality by doing 9 point oversampling\n");
    Log("    -bounce #       : Set number of radiosity bounces\n");
    Log("    -ambient r g b  : Set ambient world light (0.0 to 1.0, r g b)\n");
    Log("    -maxlight #     : Set maximum light intensity value\n");
    Log("    -circus         : Enable 'circus' mode for locating unlit lightmaps\n");
    Log("    -nopaque        : Disable the opaque zhlt_lightflags for this compile\n\n");
    Log("    -smooth #       : Set smoothing threshold for blending (in degrees)\n");
    Log("    -chop #         : Set radiosity patch size for normal textures\n");
    Log("    -texchop #      : Set radiosity patch size for texture light faces\n\n");
    Log("    -notexscale #   : Do not scale radiosity patches with texture scale\n");
    Log("    -coring #       : Set lighting threshold before blackness\n");
    Log("    -dlight #       : Set direct lighting threshold\n");
    Log("    -nohunt         : Skip lightmap texel hunt for world (for fast compiles)\n");
    Log("    -nolerp         : Disable radiosity interpolation, nearest point instead\n\n");
    Log("    -fade #         : Set global fade (larger values = shorter lights)\n");
    Log("    -falloff #      : Set global falloff mode (1 = inv linear, 2 = inv square)\n");
    Log("    -scale #        : Set global light scaling value\n");
    Log("    -gamma #        : Set global gamma value\n\n");
    Log("    -sky #          : Set ambient sunlight contribution in the shade outside\n");
    Log("    -lights file    : Manually specify a lights.rad file to use\n");
    Log("    -noskyfix       : Disable light_environment being global\n");
    Log("    -incremental    : Use or create an incremental transfer list file\n\n");
    Log("    -dump           : Dumps light patches to a file for hlrad debugging info\n\n");
    Log("    -texdata #      : Alter maximum texture memory limit (in kb)\n");
    Log("    -lightdata #    : Alter maximum lighting memory limit (in kb)\n");
    Log("    -chart          : display bsp statitics\n");
    Log("    -low | -high    : run program an altered priority level\n");
    Log("    -nolog          : Do not generate the compile logfiles\n");
    Log("    -threads #      : manually specify the number of threads to run\n");
#ifdef SYSTEM_WIN32
    Log("    -estimate       : display estimated time during compile\n");
#endif
#ifdef ZHLT_PROGRESSFILE // AJM
    Log("    -progressfile path  : specify the path to a file for progress estimate output\n");
#endif
#ifdef SYSTEM_POSIX
    Log("    -noestimate     : Do not display continuous compile time estimates\n");
#endif
    Log("    -verbose        : compile with verbose messages\n");
    Log("    -noinfo         : Do not show tool configuration information\n");
    Log("    -dev #          : compile with developer message\n\n");

    // ------------------------------------------------------------------------
    // Changes by Adam Foster - afoster@compsoc.man.ac.uk
#ifdef HLRAD_WHOME

    // AJM: we dont need this extra crap
    //Log("-= Unofficial features added by Adam Foster (afoster@compsoc.man.ac.uk) =-\n\n");
    Log("   -colourgamma r g b  : Sets different gamma values for r, g, b\n" );
    Log("   -colourscale r g b  : Sets different lightscale values for r, g ,b\n" );
    Log("   -colourjitter r g b : Adds noise, independent colours, for dithering\n");
    Log("   -jitter r g b       : Adds noise, monochromatic, for dithering\n");
    Log("   -nodiffuse          : Disables light_environment diffuse hack\n");
    Log("   -nospotpoints       : Disables light_spot spherical point sources\n");
    Log("   -softlight r g b d  : Scaling values for backwards-light hack\n\n");
    //Log("-= End of unofficial features! =-\n\n" );

#endif
    // ------------------------------------------------------------------------  
    
#ifdef HLRAD_HULLU
    Log("   -customshadowwithbounce : Enables custom shadows with bounce light\n");
    Log("   -rgbtransfers           : Enables RGB Transfers (for custom shadows)\n\n");
#endif

    Log("    mapfile         : The mapfile to compile\n\n");

    exit(1);
}

// =====================================================================================
//  Settings
// =====================================================================================
static void     Settings()
{
    const char*           tmp;
    char            buf1[1024];
    char            buf2[1024];

    if (!g_info)
    {
        return;
    }

    Log("\n-= Current %s Settings =-\n", g_Program);
    Log("Name                | Setting             | Default\n"
        "--------------------|---------------------|-------------------------\n");

    // ZHLT Common Settings
    if (DEFAULT_NUMTHREADS == -1)
    {
        Log("threads              [ %17d ] [            Varies ]\n", g_numthreads);
    }
    else
    {
        Log("threads              [ %17d ] [ %17d ]\n", g_numthreads, DEFAULT_NUMTHREADS);
    }

    Log("verbose              [ %17s ] [ %17s ]\n", g_verbose ? "on" : "off", DEFAULT_VERBOSE ? "on" : "off");
    Log("log                  [ %17s ] [ %17s ]\n", g_log ? "on" : "off", DEFAULT_LOG ? "on" : "off");
    Log("developer            [ %17d ] [ %17d ]\n", g_developer, DEFAULT_DEVELOPER);
    Log("chart                [ %17s ] [ %17s ]\n", g_chart ? "on" : "off", DEFAULT_CHART ? "on" : "off");
    Log("estimate             [ %17s ] [ %17s ]\n", g_estimate ? "on" : "off", DEFAULT_ESTIMATE ? "on" : "off");
    //Log("max texture memory   [ %17d ] [ %17d ]\n", g_max_map_miptex, DEFAULT_MAX_MAP_MIPTEX);
	//Log("max lighting memory  [ %17d ] [ %17d ]\n", g_max_map_lightdata, DEFAULT_MAX_MAP_LIGHTDATA);

    switch (g_threadpriority)
    {
    case eThreadPriorityNormal:
    default:
        tmp = "Normal";
        break;
    case eThreadPriorityLow:
        tmp = "Low";
        break;
    case eThreadPriorityHigh:
        tmp = "High";
        break;
    }
    Log("priority             [ %17s ] [ %17s ]\n", tmp, "Normal");
    Log("\n");

    // HLRAD Specific Settings
    switch (g_method)
    {
    default:
        tmp = "Unknown";
        break;
    case eMethodVismatrix:
        tmp = "Original";
        break;
    case eMethodSparseVismatrix:
        tmp = "Sparse";
        break;
    case eMethodNoVismatrix:
        tmp = "NoMatrix";
        break;
    }

    Log("vismatrix algorithm  [ %17s ] [ %17s ]\n", tmp, "Original");
    Log("oversampling (-extra)[ %17s ] [ %17s ]\n", g_extra ? "on" : "off", DEFAULT_EXTRA ? "on" : "off");
    Log("bounces              [ %17d ] [ %17d ]\n", g_numbounce, DEFAULT_BOUNCE);
	Log("bounce dynamic light [ %17s ] [ %17s ]\n", g_bounce_dynamic  ? "on" : "off", DEFAULT_BOUNCE_DYNAMIC ? "on" : "off");

    safe_snprintf(buf1, sizeof(buf1), "%1.3f %1.3f %1.3f", g_ambient[0], g_ambient[1], g_ambient[2]);
    safe_snprintf(buf2, sizeof(buf2), "%1.3f %1.3f %1.3f", DEFAULT_AMBIENT_RED, DEFAULT_AMBIENT_GREEN, DEFAULT_AMBIENT_BLUE);
    Log("ambient light        [ %17s ] [ %17s ]\n", buf1, buf2);
    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_maxlight);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_MAXLIGHT);
    Log("maximum light        [ %17s ] [ %17s ]\n", buf1, buf2);
    Log("circus mode          [ %17s ] [ %17s ]\n", g_circus ? "on" : "off", DEFAULT_CIRCUS ? "on" : "off");

    Log("\n");

    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_smoothing_value);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_SMOOTHING_VALUE);
    Log("smoothing threshold  [ %17s ] [ %17s ]\n", buf1, buf2);
    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_dlight_threshold);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_DLIGHT_THRESHOLD);
    Log("direct threshold     [ %17s ] [ %17s ]\n", buf1, buf2);
    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_direct_scale);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_DLIGHT_SCALE);
    Log("direct light scale   [ %17s ] [ %17s ]\n", buf1, buf2);
    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_coring);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_CORING);
    Log("coring threshold     [ %17s ] [ %17s ]\n", buf1, buf2);
    Log("patch interpolation  [ %17s ] [ %17s ]\n", g_lerp_enabled ? "on" : "off", DEFAULT_LERP_ENABLED ? "on" : "off");

    Log("\n");

    Log("texscale             [ %17s ] [ %17s ]\n", g_texscale ? "on" : "off", DEFAULT_TEXSCALE ? "on" : "off");
    Log("patch subdividing    [ %17s ] [ %17s ]\n", g_subdivide ? "on" : "off", DEFAULT_SUBDIVIDE ? "on" : "off");
    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_chop);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_CHOP);
    Log("chop value           [ %17s ] [ %17s ]\n", buf1, buf2);
    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_texchop);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_TEXCHOP);
    Log("texchop value        [ %17s ] [ %17s ]\n", buf1, buf2);
    Log("\n");

    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_fade);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_FADE);
    Log("global fade          [ %17s ] [ %17s ]\n", buf1, buf2);
    Log("global falloff       [ %17d ] [ %17d ]\n", g_falloff, DEFAULT_FALLOFF);
    
    // ------------------------------------------------------------------------
    // Changes by Adam Foster - afoster@compsoc.man.ac.uk
    // replaces the old stuff for displaying current values for gamma and lightscale
#ifdef HLRAD_WHOME
    safe_snprintf(buf1, sizeof(buf1), "%1.3f %1.3f %1.3f", g_colour_lightscale[0], g_colour_lightscale[1], g_colour_lightscale[2]);
    safe_snprintf(buf2, sizeof(buf2), "%1.3f %1.3f %1.3f", DEFAULT_COLOUR_LIGHTSCALE_RED, DEFAULT_COLOUR_LIGHTSCALE_GREEN, DEFAULT_COLOUR_LIGHTSCALE_BLUE);
    Log("global light scale   [ %17s ] [ %17s ]\n", buf1, buf2);

    safe_snprintf(buf1, sizeof(buf1), "%1.3f %1.3f %1.3f", g_colour_qgamma[0], g_colour_qgamma[1], g_colour_qgamma[2]);
    safe_snprintf(buf2, sizeof(buf2), "%1.3f %1.3f %1.3f", DEFAULT_COLOUR_GAMMA_RED, DEFAULT_COLOUR_GAMMA_GREEN, DEFAULT_COLOUR_GAMMA_BLUE);
    Log("global gamma         [ %17s ] [ %17s ]\n", buf1, buf2);
#endif
    // ------------------------------------------------------------------------

    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_lightscale);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_LIGHTSCALE);
    Log("global light scale   [ %17s ] [ %17s ]\n", buf1, buf2);

#ifndef HLRAD_WHOME
    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_qgamma);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_GAMMA);
    Log("global gamma amount  [ %17s ] [ %17s ]\n", buf1, buf2);
#endif

    safe_snprintf(buf1, sizeof(buf1), "%3.3f", g_indirect_sun);
    safe_snprintf(buf2, sizeof(buf2), "%3.3f", DEFAULT_INDIRECT_SUN);
    Log("global sky diffusion [ %17s ] [ %17s ]\n", buf1, buf2);

    Log("\n");
    Log("opaque entities      [ %17s ] [ %17s ]\n", g_allow_opaques ? "on" : "off", DEFAULT_ALLOW_OPAQUES ? "on" : "off");
    //Log("sky lighting fix     [ %17s ] [ %17s ]\n", g_sky_lighting_fix ? "on" : "off", DEFAULT_SKY_LIGHTING_FIX ? "on" : "off");
    Log("incremental          [ %17s ] [ %17s ]\n", g_incremental ? "on" : "off", DEFAULT_INCREMENTAL ? "on" : "off");
    Log("dump                 [ %17s ] [ %17s ]\n", g_dumppatches ? "on" : "off", DEFAULT_DUMPPATCHES ? "on" : "off");

    // ------------------------------------------------------------------------
    // Changes by Adam Foster - afoster@compsoc.man.ac.uk
    // displays information on all the brand-new features :)
#ifdef HLRAD_WHOME

    Log("\n");
    safe_snprintf(buf1, sizeof(buf1), "%3.1f %3.1f %3.1f", g_colour_jitter_hack[0], g_colour_jitter_hack[1], g_colour_jitter_hack[2]);
    safe_snprintf(buf2, sizeof(buf2), "%3.1f %3.1f %3.1f", DEFAULT_COLOUR_JITTER_HACK_RED, DEFAULT_COLOUR_JITTER_HACK_GREEN, DEFAULT_COLOUR_JITTER_HACK_BLUE);
    Log("colour jitter        [ %17s ] [ %17s ]\n", buf1, buf2);
    safe_snprintf(buf1, sizeof(buf1), "%3.1f %3.1f %3.1f", g_jitter_hack[0], g_jitter_hack[1], g_jitter_hack[2]);
    safe_snprintf(buf2, sizeof(buf2), "%3.1f %3.1f %3.1f", DEFAULT_JITTER_HACK_RED, DEFAULT_JITTER_HACK_GREEN, DEFAULT_JITTER_HACK_BLUE);
    Log("monochromatic jitter [ %17s ] [ %17s ]\n", buf1, buf2);

    safe_snprintf(buf1, sizeof(buf1), "%2.1f %2.1f %2.1f %2.1f", g_softlight_hack[0], g_softlight_hack[1], g_softlight_hack[2], g_softlight_hack_distance);
    safe_snprintf(buf2, sizeof(buf2), "%2.1f %2.1f %2.1f %2.1f", DEFAULT_SOFTLIGHT_HACK_RED, DEFAULT_SOFTLIGHT_HACK_GREEN, DEFAULT_SOFTLIGHT_HACK_BLUE, DEFAULT_SOFTLIGHT_HACK_DISTANCE);
    Log("softlight hack       [ %17s ] [ %17s ]\n", buf1, buf2);

    Log("diffuse hack         [ %17s ] [ %17s ]\n", g_diffuse_hack ? "on" : "off", DEFAULT_DIFFUSE_HACK ? "on" : "off");
    Log("spotlight points     [ %17s ] [ %17s ]\n", g_spotlight_hack ? "on" : "off", DEFAULT_SPOTLIGHT_HACK ? "on" : "off");

#endif
    // ------------------------------------------------------------------------

#ifdef HLRAD_HULLU
    Log("\n");
    Log("custom shadows with bounce light\n"
        "                     [ %17s ] [ %17s ]\n", g_customshadow_with_bouncelight ? "on" : "off", DEFAULT_CUSTOMSHADOW_WITH_BOUNCELIGHT ? "on" : "off");
    Log("rgb transfers        [ %17s ] [ %17s ]\n", g_rgb_transfers ? "on" : "off", DEFAULT_RGB_TRANSFERS ? "on" : "off"); 
#endif
    Log("\n\n");
}

#ifdef HLRAD_INFO_TEXLIGHTS
// AJM: added in
// =====================================================================================
//  ReadInfoTexlights
//      try and parse texlight info from the info_texlights entity 
// =====================================================================================
void            ReadInfoTexlights()
{
    int         k;
    int         values;
    int         numtexlights = 0;
    float       r, g, b, i;
    entity_t*   mapent;
    epair_t*    ep;
    texlight_t  texlight;

    for (k = 0; k < g_numentities; k++)
    {
        mapent = &g_entities[k];
        
        if (strcmp(ValueForKey(mapent, "classname"), "info_texlights"))
            continue;

        Log("[Reading texlights from info_texlights map entity]\n");

        for (ep = mapent->epairs; ep; ep = ep->next)
        {
            if (    !strcmp(ep->key, "classname") 
                 || !strcmp(ep->key, "origin")
               )
                continue; // we dont care about these keyvalues

            values = sscanf_s(ep->value, "%f %f %f %f", &r, &g, &b, &i);
            
            if (values == 1)
            {  
                g = b = r;
            }
            else if (values == 4) // use brightness value.
            {
                r *= i / 255.0;
                g *= i / 255.0;
                b *= i / 255.0;
            }
            else if (values != 3)
            {
                Warning("ignoring bad texlight '%s' in info_texlights entity", ep->key);
                continue;
            }

            texlight.name = ep->key;
            texlight.value[0] = r;
            texlight.value[1] = g;
            texlight.value[2] = b;
            texlight.filename = "info_texlights";
            s_texlights.push_back(texlight);
            numtexlights++;
        }

        Log("[%i texlights parsed from info_texlights map entity]\n\n", numtexlights);
    }
}
#endif

const char* lights_rad = "lights.rad";
const char* ext_rad = ".rad";

// =====================================================================================
//  LoadRadFiles
// =====================================================================================
void            LoadRadFiles(const char* const mapname, const char* const user_rad, const char* argv0)
{
    char global_lights[_MAX_PATH];
    char mapname_lights[_MAX_PATH];

    char mapfile[_MAX_PATH];
    char mapdir[_MAX_PATH];
    char appdir[_MAX_PATH];

    // Get application directory (only an approximation on posix systems)
    // try looking in the directory we were run from
    {
        char tmp[_MAX_PATH];
        memset(tmp, 0, sizeof(tmp));
#ifdef SYSTEM_WIN32
        GetModuleFileName(NULL, tmp, _MAX_PATH);
#else
        safe_strncpy(tmp, argv0, _MAX_PATH);
#endif
		ExtractFilePath(tmp, appdir);
    }

    // Get map directory
    ExtractFilePath(mapname, mapdir);
    ExtractFileBase(mapname, mapfile);

    // Look for lights.rad in mapdir
    safe_strncpy(global_lights, mapdir, _MAX_PATH);
    safe_strncat(global_lights, lights_rad, _MAX_PATH);
    if (q_exists(global_lights))
    {
        ReadLightFile(global_lights);
    }
    else
    {
        // Look for lights.rad in appdir
        safe_strncpy(global_lights, appdir, _MAX_PATH);
        safe_strncat(global_lights, lights_rad, _MAX_PATH);
        if (q_exists(global_lights))
        {
            ReadLightFile(global_lights);
        }
        else
        {
            // Look for lights.rad in current working directory
            safe_strncpy(global_lights, lights_rad, _MAX_PATH);
            if (q_exists(global_lights))
            {
                ReadLightFile(global_lights);
            }
        }
    }
   
    // Look for mapname.rad in mapdir
    safe_strncpy(mapname_lights, mapdir, _MAX_PATH);
    safe_strncat(mapname_lights, mapfile, _MAX_PATH);
    DefaultExtension(mapname_lights, ext_rad);
    if (q_exists(mapname_lights))
    {
        ReadLightFile(mapname_lights);
    }


    if (user_rad)
    {
        char user_lights[_MAX_PATH];
        char userfile[_MAX_PATH];

        ExtractFile(user_rad, userfile);

        // Look for user.rad from command line (raw)
        safe_strncpy(user_lights, user_rad, _MAX_PATH);
        if (q_exists(user_lights))
        {
            ReadLightFile(user_lights);
        }
        else
        {
            // Try again with .rad enforced as extension
            DefaultExtension(user_lights, ext_rad);
            if (q_exists(user_lights))
            {
                ReadLightFile(user_lights);
            }
            else
            {
                // Look for user.rad in mapdir
                safe_strncpy(user_lights, mapdir, _MAX_PATH);
                safe_strncat(user_lights, userfile, _MAX_PATH);
                DefaultExtension(user_lights, ext_rad);
                if (q_exists(user_lights))
                {
                    ReadLightFile(user_lights);
                }
                else
                {
                    // Look for user.rad in appdir
                    safe_strncpy(user_lights, appdir, _MAX_PATH);
                    safe_strncat(user_lights, userfile, _MAX_PATH);
                    DefaultExtension(user_lights, ext_rad);
                    if (q_exists(user_lights))
                    {
                        ReadLightFile(user_lights);
                    }
                    else
                    {
                        // Look for user.rad in current working directory
                        safe_strncpy(user_lights, userfile, _MAX_PATH);
                        DefaultExtension(user_lights, ext_rad);
                        if (q_exists(user_lights))
                        {
                            ReadLightFile(user_lights);
                        }
                    }
                }
            }
        }
    }

#ifdef HLRAD_INFO_TEXLIGHTS
    ReadInfoTexlights(); // AJM
#endif
}

// =====================================================================================
//  main
// =====================================================================================
int             main(const int argc, char** argv)
{
    int             i;
    double          start, end;
    const char*     mapname_from_arg = NULL;
    const char*     user_lights = NULL;

    g_Program = "hlrad";

    if (argc == 1)
        Usage();

    for (i = 1; i < argc; i++)
    {
        if (!_stricmp(argv[i], "-dump"))
        {
            g_dumppatches = true;
        }
        else if (!_stricmp(argv[i], "-bounce"))
        {
            if (i < argc)
            {
                g_numbounce = atoi(argv[++i]);
                if (g_numbounce > 1000)
                {
                    Log("Unexpectedly large value (>1000) for '-bounce'\n");
                    Usage();
                }
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-dev"))
        {
            if (i < argc)
            {
                g_developer = (developer_level_t)atoi(argv[++i]);
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-verbose"))
        {
            g_verbose = true;
        }
        else if (!_stricmp(argv[i], "-noinfo"))
        {
            g_info = false;
        }
        else if (!_stricmp(argv[i], "-threads"))
        {
            if (i < argc)
            {
                g_numthreads = atoi(argv[++i]);
                if (g_numthreads < 1)
                {
                    Log("Expected value of at least 1 for '-threads'\n");
                    Usage();
                }
            }
            else
            {
                Usage();
            }
        }
#ifdef SYSTEM_WIN32
        else if (!_stricmp(argv[i], "-estimate"))
        {
            g_estimate = true;
        }
#endif
#ifdef SYSTEM_POSIX
        else if (!_stricmp(argv[i], "-noestimate"))
        {
            g_estimate = false;
        }
#endif
#ifdef ZHLT_NETVIS
        else if (!_stricmp(argv[i], "-client"))
        {
            if (i < argc)
            {
                g_clientid = atoi(argv[++i]);
            }
            else
            {
                Usage();
            }
        }
#endif
        else if (!_stricmp(argv[i], "-nolerp"))
        {
             g_lerp_enabled  = false;
        }
        else if (!_stricmp(argv[i], "-chop"))
        {
            if (i < argc)
            {
                g_chop = atof(argv[++i]);
                if (g_chop < 1)
                {
                    Log("expected value greater than 1 for '-chop'\n");
                    Usage();
                }
                if (g_chop < 32)
                {
                    Log("Warning: Chop values below 32 are not recommended.");
                }
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-texchop"))
        {
            if (i < argc)
            {
                g_texchop = atof(argv[++i]);
                if (g_texchop < 1)
                {
                    Log("expected value greater than 1 for '-texchop'\n");
                    Usage();
                }
                if (g_texchop < 32)
                {
                    Log("Warning: texchop values below 16 are not recommended.");
                }
            }
            else
            {
                Usage();
            }
        }
		else if (!_stricmp(argv[i], "-nodynbounce"))
		{
			g_bounce_dynamic = false;
		}
        else if (!_stricmp(argv[i], "-notexscale"))
        {
            g_texscale = false;
        }
        else if (!_stricmp(argv[i], "-nosubdivide"))
        {
            if (i < argc)
            {
                g_subdivide = false;
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-nohunt"))
        {
            if (i < argc)
            {
                g_nohunt = true;
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-scale"))
        {
            if (i < argc)
            {
             	// ------------------------------------------------------------------------
		        // Changes by Adam Foster - afoster@compsoc.man.ac.uk
		        // Munge monochrome lightscale into colour one
#ifdef HLRAD_WHOME
	    	    i++;
                g_colour_lightscale[0] = (float)atof(argv[i]);
		        g_colour_lightscale[1] = (float)atof(argv[i]);
		        g_colour_lightscale[2] = (float)atof(argv[i]);
#else
                g_lightscale = (float)atof(argv[++i]);
#endif
		        // ------------------------------------------------------------------------
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-falloff"))
        {
            if (i < argc)
            {
                g_falloff = (int)atoi(argv[++i]);
                if ((g_falloff != 1) && (g_falloff != 2))
                {
                    Log("-falloff must be 1 or 2\n");
                    Usage();
                }
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-fade"))
        {
            if (i < argc)
            {
                g_fade = (float)atof(argv[++i]);
                if (g_fade < 0.0)
                {
                    Log("-fade must be a positive number\n");
                    Usage();
                }
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-ambient"))
        {
            if (i + 3 < argc)
            {
                g_ambient[0] = (float)atof(argv[++i]) * 128;
                g_ambient[1] = (float)atof(argv[++i]) * 128;
                g_ambient[2] = (float)atof(argv[++i]) * 128;
            }
            else
            {
                Error("expected three color values after '-ambient'\n");
            }
        }
        else if (!_stricmp(argv[i], "-maxlight"))
        {
            if (i < argc)
            {
                g_maxlight = (float)atof(argv[++i]) * 128;
                if (g_maxlight <= 0)
                {
                    Log("expected positive value after '-maxlight'\n");
                    Usage();
                }
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-lights"))
        {
            if (i < argc)
            {
                user_lights = argv[++i];
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-circus"))
        {
            g_circus = true;
        }
#if 0
        else if (!_stricmp(argv[i], "-noskyfix"))
        {
            g_sky_lighting_fix = false;
        }
#endif
        else if (!_stricmp(argv[i], "-incremental"))
        {
            g_incremental = true;
        }
        else if (!_stricmp(argv[i], "-chart"))
        {
            g_chart = true;
        }
        else if (!_stricmp(argv[i], "-low"))
        {
            g_threadpriority = eThreadPriorityLow;
        }
        else if (!_stricmp(argv[i], "-high"))
        {
            g_threadpriority = eThreadPriorityHigh;
        }
        else if (!_stricmp(argv[i], "-nolog"))
        {
            g_log = false;
        }
        else if (!_stricmp(argv[i], "-gamma"))
        {
            if (i < argc)
            {
            	// ------------------------------------------------------------------------
		        // Changes by Adam Foster - afoster@compsoc.man.ac.uk
		        // Munge values from original, monochrome gamma into colour gamma
#ifdef HLRAD_WHOME
	    	    i++;
                g_colour_qgamma[0] = (float)atof(argv[i]);
		        g_colour_qgamma[1] = (float)atof(argv[i]);
		        g_colour_qgamma[2] = (float)atof(argv[i]);
#else
                g_qgamma = (float)atof(argv[++i]);
#endif
		        // ------------------------------------------------------------------------
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-dlight"))
        {
            if (i < argc)
            {
                g_dlight_threshold = (float)atof(argv[++i]);
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-extra"))
        {
            g_extra = true;
        }
        else if (!_stricmp(argv[i], "-sky"))
        {
            if (i < argc)
            {
                g_indirect_sun = (float)atof(argv[++i]);
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-smooth"))
        {
            if (i < argc)
            {
                g_smoothing_value = atof(argv[++i]);
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-coring"))
        {
            if (i < argc)
            {
                g_coring = (float)atof(argv[++i]);
            }
            else
            {
                Usage();
            }
        }
        /*
        else if (!_stricmp(argv[i], "-texdata"))
        {
            if (i < argc)
            {
                int             x = atoi(argv[++i]) * 1024;

                if (x > g_max_map_miptex)
                {
                    g_max_map_miptex = x;
                }
            }
            else
            {
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-lightdata"))
        {
            if (i < argc)
            {
                int             x = atoi(argv[++i]) * 1024;

                if (x > g_max_map_lightdata)
                {
                    g_max_map_lightdata = x;
                }
            }
            else
            {
                Usage();
            }
        }
        */
        else if (!_stricmp(argv[i], "-sparse"))
        {
            g_method = eMethodSparseVismatrix;
        }
        else if (!_stricmp(argv[i], "-nomatrix"))
        {
            g_method = eMethodNoVismatrix;
        }
        else if (!_stricmp(argv[i], "-nopaque"))
        {
            g_allow_opaques = false;
        }
        else if (!_stricmp(argv[i], "-dscale"))
        {
            if (i < argc)
            {
                g_direct_scale = (float)atof(argv[++i]);
            }
            else
            {
                Usage();
            }
        }

        // ------------------------------------------------------------------------
	    // Changes by Adam Foster - afoster@compsoc.man.ac.uk
#ifdef HLRAD_WHOME
        else if (!_stricmp(argv[i], "-colourgamma"))
        {
        	if (i + 3 < argc)
			{
				g_colour_qgamma[0] = (float)atof(argv[++i]);
				g_colour_qgamma[1] = (float)atof(argv[++i]);
				g_colour_qgamma[2] = (float)atof(argv[++i]);
			}
			else
			{
				Error("expected three color values after '-colourgamma'\n");
			}
        }
        else if (!_stricmp(argv[i], "-colourscale"))
        {
        	if (i + 3 < argc)
			{
				g_colour_lightscale[0] = (float)atof(argv[++i]);
				g_colour_lightscale[1] = (float)atof(argv[++i]);
				g_colour_lightscale[2] = (float)atof(argv[++i]);
			}
			else
			{
				Error("expected three color values after '-colourscale'\n");
			}
        }

        else if (!_stricmp(argv[i], "-colourjitter"))
        {
        	if (i + 3 < argc)
			{
				g_colour_jitter_hack[0] = (float)atof(argv[++i]);
				g_colour_jitter_hack[1] = (float)atof(argv[++i]);
				g_colour_jitter_hack[2] = (float)atof(argv[++i]);
			}
			else
			{
				Error("expected three color values after '-colourjitter'\n");
			}
        }
		else if (!_stricmp(argv[i], "-jitter"))
        {
        	if (i + 3 < argc)
			{
				g_jitter_hack[0] = (float)atof(argv[++i]);
				g_jitter_hack[1] = (float)atof(argv[++i]);
				g_jitter_hack[2] = (float)atof(argv[++i]);
			}
			else
			{
				Error("expected three color values after '-jitter'\n");
			}
        }

        else if (!_stricmp(argv[i], "-nodiffuse"))
        {
        	g_diffuse_hack = false;
        }
        else if (!_stricmp(argv[i], "-nospotpoints"))
        {
        	g_spotlight_hack = false;
        }
        else if (!_stricmp(argv[i], "-softlight"))
        {
        	if (i + 4 < argc)
			{
				g_softlight_hack[0] = (float)atof(argv[++i]);
				g_softlight_hack[1] = (float)atof(argv[++i]);
				g_softlight_hack[2] = (float)atof(argv[++i]);
				g_softlight_hack_distance = (float)atof(argv[++i]);
			}
			else
			{
				Error("expected three color scalers and a distance after '-softlight'\n");
			}
        }
#endif
        // ------------------------------------------------------------------------

#ifdef HLRAD_HULLU
        else if (!_stricmp(argv[i], "-customshadowwithbounce"))
        {
        	g_customshadow_with_bouncelight = true;
        }
        else if (!_stricmp(argv[i], "-rgbtransfers"))
        {
        	g_rgb_transfers = true;
        }
#endif

#ifdef ZHLT_PROGRESSFILE // AJM
        else if (!_stricmp(argv[i], "-progressfile"))
        {
            if (i < argc)
            {
                g_progressfile = argv[++i];
            }
            else
            {
            	Log("Error: -progressfile: expected path to progress file following parameter\n");
                Usage();
            }
        }
#endif

#ifdef HLRAD_FASTMATH
		else if (!_stricmp(argv[i], "-oldmath"))
		{
			Warning("-oldmath was introduced as a temporary workaround to a bug in HLRAD and is no longer supported.\n  Please remove it from your command line.\n");
		}
#endif
        else if (argv[i][0] == '-')
        {
            Log("Unknown option \"%s\"\n", argv[i]);
            Usage();
        }
        else if (!mapname_from_arg)
        {
            mapname_from_arg = argv[i];
        }
        else
        {
            Log("Unknown option \"%s\"\n", argv[i]);
            Usage();
        }
    }

    if (!mapname_from_arg)
    {
        Log("No mapname specified\n");
        Usage();
    }

    g_smoothing_threshold = (float)cos(g_smoothing_value * (Q_PI / 180.0));

    safe_strncpy(g_Mapname, mapname_from_arg, _MAX_PATH);
    FlipSlashes(g_Mapname);
    StripExtension(g_Mapname);
    OpenLog(g_clientid);
    atexit(CloseLog);
    ThreadSetDefault();
    ThreadSetPriority(g_threadpriority);
    LogStart(argc, argv);

    CheckForErrorLog();
    Settings();

    // normalise maxlight
    if (g_maxlight > 255)
        g_maxlight = 255;

   // dtexdata_init();
   // atexit(dtexdata_free);
    // END INIT

    // BEGIN RAD
    start = I_FloatTime();

    strcpy_s(g_source, mapname_from_arg);
    StripExtension(g_source);
    DefaultExtension(g_source, ".bsp");
    LoadBSPFile(g_source);

    mapinfo_t* map = ParseEntities();
 
    LoadRadFiles(g_Mapname, user_lights, argv[0]);
    
    if (!g_visdatasize)
    {
        Warning("No vis information, this might take a while to compile.\nOverriding vismatrix method to nomatrix");
        g_method = eVisMethods::eMethodNoVismatrix;
        //g_numbounce = 0;
        //g_ambient[0] = g_ambient[1] = g_ambient[2] = 0.1f;
    }

    RadWorld(map);

    FreeOpaqueFaceList();
    FreePatches();

    if (g_chart)
        PrintBSPFileSizes();

    WriteBSPFile(g_source);

    end = I_FloatTime();
    LogTimeElapsed(end - start);
    // END RAD

    return 0;
}
