#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "textures.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "Leaf.h"
#include "Nodes.h"
#include "cmdlib.h"
#include "log.h"
#include "brush.h"
#include "hooks.h"
#include "worldbsp.h"
#include "bspfile.h"
#include "threads.h"

//normal, xv, yv
static const vec3_t s_baseaxis_nightfire[18] = {
    {0, 0, 1}, {1, 0, 0}, {0, -1, 0}, // floor
    {0, 0, -1}, {1, 0, 0}, {0, -1, 0}, // ceiling
    {0, -1, 0}, {1, 0, 0}, {0, 0, -1}, // north wall
    {0, 1, 0}, {1, 0, 0}, {0, 0, -1}, // south wall
    {-1, 0, 0}, {0, 1, 0}, {0, 0, -1}, // east wall
    {1, 0, 0}, {0, 1, 0}, {0, 0, -1} // west wall
};

static const vec3_t   s_baseaxis[18] = {
    {0, 0, 1}, {1, 0, 0}, {0, -1, 0},                      // floor
    {0, 0, -1}, {1, 0, 0}, {0, -1, 0},                     // ceiling
    {1, 0, 0}, {0, 1, 0}, {0, 0, -1},                      // west wall
    {-1, 0, 0}, {0, 1, 0}, {0, 0, -1},                     // east wall
    {0, 1, 0}, {1, 0, 0}, {0, 0, -1},                      // south wall
    {0, -1, 0}, {1, 0, 0}, {0, 0, -1},                     // north wall
};

// =====================================================================================
//  TextureAxisFromPlane
// =====================================================================================
void TextureAxisFromPlaneHL1(const plane_t* const pln, vec3_t xv, vec3_t yv)
{
    int             bestaxis;
    vec_t           dot, best;
    int             i;

    best = 0;
    bestaxis = 0;

    for (i = 0; i < 6; i++)
    {
        dot = DotProduct(pln->normal, s_baseaxis[i * 3]);
        if (dot > best)
        {
            best = dot;
            bestaxis = i;
        }
    }

    VectorCopy(s_baseaxis[bestaxis * 3 + 1], xv);
    VectorCopy(s_baseaxis[bestaxis * 3 + 2], yv);
}

int TextureAxisFromPlane(const struct plane_s* pln)
{
    int             bestaxis;
    float           dot, best;
    int             i;

    best = 0;
    bestaxis = 6;

    for (i = 0; i < 6; i++)
    {
        vec3_t tmp;
        VectorCopy(s_baseaxis_nightfire[i * 3], tmp);
        dot = DotProduct(pln->normal, s_baseaxis_nightfire[i * 3]);

        if (dot >= best)
        {
            best = dot;
            bestaxis = i;
        }
    }

    return bestaxis;
}

bool TextureAxisFromPlane(const struct plane_s* pln, vec3_t xv, vec3_t yv)
{
    int bestaxis = TextureAxisFromPlane(pln);
    if (bestaxis == 6)
        return false;

    VectorCopy(s_baseaxis_nightfire[bestaxis * 3 + 1], xv);
    VectorCopy(s_baseaxis_nightfire[bestaxis * 3 + 2], yv);
    return true;
}

// Function to create a rotation matrix given a normal vector and an angle
void CalculatePlaneRotationMatrix(matrix3x4_t* matrix, const double normal[3], double angle) {
    double rad = angle * (Q_PI / 180.0);  // Convert angle to radians
    double c = cos(rad);
    double s = sin(rad);
    double t = 1.0 - c;

    double x = normal[0];
    double y = normal[1];
    double z = normal[2];

    matrix->m[0][0] = t * x * x + c;
    matrix->m[0][1] = t * x * y - s * z;
    matrix->m[0][2] = t * x * z + s * y;
    matrix->m[0][3] = 0.0;

    matrix->m[1][0] = t * x * y + s * z;
    matrix->m[1][1] = t * y * y + c;
    matrix->m[1][2] = t * y * z - s * x;
    matrix->m[1][3] = 0.0;

    matrix->m[2][0] = t * x * z - s * y;
    matrix->m[2][1] = t * y * z + s * x;
    matrix->m[2][2] = t * z * z + c;
    matrix->m[2][3] = 0.0;
}

void GetLightmapProjections_Original(double* dest_1_vec3_t, face_t* face, double* dest_2_vec3_t)
{
    DWORD adr = 0x411890;
    DWORD dest1 = (DWORD)dest_1_vec3_t;
    DWORD dest2 = (DWORD)dest_2_vec3_t;
    DWORD f = (DWORD)face;
    __asm
    {
        mov ebx, dest1

        mov edx, dest2
        mov edi, f
        mov eax, adr

        push edx
        push edi
        push 0
        call adr
        add esp, 12
    }
}

void GetLightmapProjections(vec3_t out_s, face_t* face, vec3_t out_t)
{
    vec_t maxs[2];
    vec_t mins[2];
    vec3_t scaled_vecs[2];

    // Initialize vectors to zero
    out_s[0] = 0.0;
    out_s[1] = 0.0;
    out_s[2] = 0.0;
    out_t[0] = 0.0;
    out_t[1] = 0.0;
    out_t[2] = 0.0;

    // Get the plane for this face
    plane_t* plane = &gMappedPlanes[face->planenum];

    // Determine texture axis based on plane
    int index = TextureAxisFromPlane(plane);
    if (index == 6)
        Error("Invalid face orientation\n");

    VectorCopy(s_baseaxis_nightfire[index * 3 + 2], out_s); //yv

    out_t[0] = out_s[2] * plane->normal[1] - plane->normal[2] * out_s[1];
    out_t[1] = plane->normal[2] * out_s[0] - plane->normal[0] * out_s[2];
    out_t[2] = out_s[1] * plane->normal[0] - plane->normal[1] * out_s[0];

    VectorNormalize(out_t);

    out_s[0] = plane->normal[2] * out_t[1] - plane->normal[1] * out_t[2];
    out_s[1] = plane->normal[0] * out_t[2] - plane->normal[2] * out_t[0];
    out_s[2] = plane->normal[1] * out_t[0] - plane->normal[0] * out_t[1];

    VectorNormalize(out_s);

    // Calculate the lightmap rotation matrix for the face
    matrix3x4_t rotation_matrix;
    CalculatePlaneRotationMatrix(&rotation_matrix, plane->normal, face->brushside->td.lightmaprotation);

    // Rotate the s and t vectors by the rotation
    VectorRotate(out_t, &rotation_matrix, out_t);
    VectorRotate(out_s, &rotation_matrix, out_s);

    // Loop iteratively starting from 0:
	// Scale s/t from the size of the face until lightmap width/height is < 128 units
    for (unsigned int incrementIndex = 0; ; ++incrementIndex)
    {
        // Scale the vectors according to lightmap scale and base lightmap scale
        vec_t lightmap_scale = max(face->brushside->td.lightmapscale, (vec_t)g_blscale);
#ifdef MAX_LIGHTMAP_SCALE
        lightmap_scale = min(lightmap_scale, (vec_t)MAX_LIGHTMAP_SCALE);
#endif

        mins[0] = 999999.0;
        mins[1] = 999999.0;
        maxs[0] = -999999.0;
        maxs[1] = -999999.0;

        // note: HL1 is hardcoded to 16 lightmap_scale
        vec_t multiplier = 1.0 / (lightmap_scale + (vec_t)(g_ilscale * incrementIndex));

        VectorScale(out_s, multiplier, scaled_vecs[0]);
        VectorScale(out_t, multiplier, scaled_vecs[1]);

        for (unsigned int i = 0; i < face->winding->m_NumPoints; ++i) 
        {
            vec_t dots[2];

            for (unsigned int j = 0; j < 2; ++j)
            {
                dots[j] = DotProduct(scaled_vecs[j], face->winding->m_Points[i]);
                dots[j] = DotProduct(scaled_vecs[j], face->winding->m_Points[i]);
                mins[j] = min(mins[j], dots[j]);
                maxs[j] = max(maxs[j], dots[j]);
            }
        }
       
        for (unsigned int i = 0; i < 2; ++i)
        {
            mins[i] = floor(mins[i]);
            maxs[i] = ceil(maxs[i]);
        }

        if ((maxs[0] - mins[0] < 128.0) && (maxs[1] - mins[1] < 128.0))
            break;
    }

    VectorCopy(scaled_vecs[0], out_s);
    VectorCopy(scaled_vecs[1], out_t);
}

unsigned int GetTextureIndex(const char* src)
{
    unsigned texture_index;

    for (texture_index = 0; texture_index < g_numDTextures; ++texture_index)
    {
        const char* tex = g_dtextures[texture_index].str;
        if (!_stricmp(tex, src))
            break;
    }

    if (texture_index == g_numDTextures)
    {
        // add new texture
        safe_strncpy(g_dtextures[texture_index].str, src, MAX_TEXTURE_LENGTH);
        ++g_numDTextures;
    }

    // increment num times this texture was used
    ++g_TextureReferenceCount[texture_index];
    return texture_index;
}

unsigned int GetMaterialIndex(const char* src)
{
    unsigned material_index;

    for (material_index = 0; material_index < g_numDMaterials; ++material_index)
    {
        const char* tex = g_dmaterials[material_index].str;
        if (!_stricmp(tex, src))
            break;
    }

    if (material_index == g_numDMaterials)
    {
        // add new material
        safe_strncpy(g_dmaterials[material_index].str, src, MAX_TEXTURE_LENGTH);
        ++g_numDMaterials;
    }
    return material_index;
}

unsigned int GetProjectionIndex(double s[4], double t[4])
{
    dtexmatrix_t output;
    output.s[0] = s[0];
    output.s[1] = s[1];
    output.s[2] = s[2];
    output.s[3] = s[3];

    output.t[0] = t[0];
    output.t[1] = t[1];
    output.t[2] = t[2];
    output.t[3] = t[3];

    for (unsigned int i = 0; i < g_numDTexMatrix; ++i)
    {
        if (output == g_dtexmatrix[i])
            return i;
    }
    dtexmatrix_t* newmatrix = &g_dtexmatrix[g_numDTexMatrix];
    *newmatrix = output;
    g_numDTexMatrix+=1;
    return g_numDTexMatrix - 1;
}

texinfo_t* TexinfoForBrushTexture(side_t* side, const vec3_t origin) 
{
    texinfo_t tx{};
    //memset(&tx, 0, sizeof(tx));
    safe_strncpy(tx.name, side->td.name, MAX_TEXTURE_LENGTH);

    // Ensure scales are not zero
    if (side->td.uscale == 0.0)
        side->td.uscale = 1.0;
    if (side->td.vscale == 0.0)
        side->td.vscale = 1.0;

    vec_t uscale_inverse = 1.0 / side->td.uscale;
    vec_t vscale_inverse = 1.0 / side->td.vscale;

    // Populate tx.vecs
    VectorScale(side->td.uaxis, uscale_inverse, tx.vecs[0]);
    VectorScale(side->td.vaxis, vscale_inverse, tx.vecs[1]);

    // Compute tx.vecs[0][3] and tx.vecs[1][3]  translations
    tx.vecs[0][3] = side->td.ushift + DotProduct(origin, tx.vecs[0]);
    tx.vecs[1][3] = side->td.vshift + DotProduct(origin, tx.vecs[1]);

#ifdef HL2_LUXEL_METHOD
    vec_t lightmapscale_inverse = 1.0 / side->td.lightmapscale;

    VectorScale(side->td.uaxis, lightmapscale_inverse, tx.lightmapVecsLuxelsPerWorldUnits[0]);
    VectorScale(side->td.vaxis, lightmapscale_inverse, tx.lightmapVecsLuxelsPerWorldUnits[1]);

    tx.lightmapVecsLuxelsPerWorldUnits[0][3] = side->td.ushift + DotProduct(origin, tx.lightmapVecsLuxelsPerWorldUnits[0]);
    tx.lightmapVecsLuxelsPerWorldUnits[1][3] = side->td.vshift + DotProduct(origin, tx.lightmapVecsLuxelsPerWorldUnits[1]);
#endif

    // Check if texinfo already exists
    for (unsigned int texinfo_index = 0; texinfo_index < g_numtexinfo; ++texinfo_index) 
    {
        texinfo_t& existing = g_texinfo[texinfo_index];
        if (tx == existing)
            return &existing;
    }

    // Ensure we don't exceed the maximum number of texinfo
    hlassume(g_numtexinfo < MAX_MAP_TEXINFO, assume_MAX_MAP_TEXINFO);

    // Add new texinfo
    unsigned int new_index = g_numtexinfo;
    g_texinfo[g_numtexinfo] = tx;
    ++g_numtexinfo;

    ThreadUnlock();
    return &g_texinfo[new_index];
}