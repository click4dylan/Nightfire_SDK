#include "brad.h"
#include "mapfile.h"

#include <unordered_map>

edgeshare_t     g_edgeshare[MAX_MAP_FACES];
vec3_t          g_face_centroids[MAX_MAP_FACES];
//bool            g_sky_lighting_fix = DEFAULT_SKY_LIGHTING_FIX;
extern bool g_warned_direct;

//#define TEXTURE_STEP   16.0
#define TEXTURE_STEP   1.0

// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
typedef struct
{
    unsigned /*short*/int  v[2];                                  // vertex numbers
}
dedge_t;

static dedge_t g_dedges[MAX_MAP_FACES];
unsigned int g_numdeges = 0;
static unsigned int g_dsurfedges[MAX_MAP_FACES];
static const dface_t* edgefaces[MAX_MAP_FACES][2];
unsigned int g_numdsurfedges = 0;
unsigned int firstmodeledge = 1;
std::unordered_map<const dface_t*, unsigned int> g_firstedges; //face, firstedge (numedges = face->num_vertices)

unsigned int GetEdge(unsigned int v1, unsigned int v2, const dface_t& f)
{
    dedge_t* edge;
    unsigned int i;

    for (i = firstmodeledge; i < g_numdeges; i++)
    {
        edge = &g_dedges[i];
        if (v1 == edge->v[1] && v2 == edge->v[0] && !edgefaces[i][1] && edgefaces[i][0]->flags == f.flags)
        {
            edgefaces[i][1] = &f;
            return -i;
        }
    }

    // emit an edge
    //FIXME
    //hlassume(g_numedges < MAX_MAP_EDGES, assume_MAX_MAP_EDGES);
    edge = &g_dedges[g_numdeges++];
    edge->v[0] = v1;
    edge->v[1] = v2;
    edgefaces[i][0] = &f;

    return i;
}

void GetEdges()
{
    for (unsigned int m = 0; m < g_numDModels; ++m)
    {
        const dmodel_t& model = g_dmodels[m];

        if (model.numfaces > 0)
        {
            firstmodeledge = g_numdeges; //fixme: should this be above the numfaces check?

            for (unsigned int fi = 0; fi < model.numfaces; ++fi)
            {
                const dface_t& face = g_dFaces[model.firstface + fi];

                g_firstedges[&face] = g_numdsurfedges;

                for (unsigned int v = 0; v < face.num_vertices; ++v)
                {
                    unsigned int v1 = face.first_vertex_index + v;
                    unsigned int v2 = (face.first_vertex_index + v + 1) % face.num_vertices;

                    unsigned int e = GetEdge(v1, v2, face);
                    //hlassume(g_numsurfedges < MAX_MAP_SURFEDGES, assume_MAX_MAP_SURFEDGES);
                    //FIXME
                    g_dsurfedges[g_numdsurfedges++] = e;
                }
            }
        }
    }
}

// =====================================================================================
//  PairEdges
// =====================================================================================
void            PairEdges()
{
    unsigned int             i, j, k;
    dface_t* f;
    edgeshare_t* e;

    memset(&g_edgeshare, 0, sizeof(g_edgeshare));

    f = g_dFaces;
    for (i = 0; i < g_numDFaces; i++, f++)
    {
        auto test = g_firstedges.find(f);
        if (test == g_firstedges.end())
            Error("Can't find edge for face %u\n", i);
       
        unsigned int firstedge = test->second;

        for (j = 0; j < f->num_vertices; j++)
        {
            k = g_dsurfedges[firstedge + j];
            if (k < 0)
            {
                e = &g_edgeshare[-k];

                hlassert(e->faces[1] == NULL);
                e->faces[1] = f;
            }
            else
            {
                e = &g_edgeshare[k];

                hlassert(e->faces[0] == NULL);
                e->faces[0] = f;
            }

            if (e->faces[0] && e->faces[1]) {
                // determine if coplanar
                if (e->faces[0]->plane_index == e->faces[1]->plane_index) {
                    e->coplanar = true;
                }
                else {
                    // see if they fall into a "smoothing group" based on angle of the normals
                    vec3_t          normals[2];

                    VectorCopy(getPlaneFromFace(e->faces[0])->normal, normals[0]);
                    VectorCopy(getPlaneFromFace(e->faces[1])->normal, normals[1]);

                    e->cos_normals_angle = DotProduct(normals[0], normals[1]);

                    if (e->cos_normals_angle > (1.0 - NORMAL_EPSILON))
                    {
                        e->coplanar = true;
                    }
                    else if (g_smoothing_threshold > 0.0)
                    {
                        if (e->cos_normals_angle >= g_smoothing_threshold)
                        {
                            VectorAdd(normals[0], normals[1], e->interface_normal);
                            VectorNormalize(e->interface_normal);
                        }
                    }
                }
            }
        }
    }
}

#define MAX_SINGLEMAP 268324
//#define	MAX_SINGLEMAP	(18*18*4)

typedef struct lightinfo_s
{
    vec_t* light;
    vec_t           facedist;
    vec3_t          facenormal;

    vec3_t          texorg;
    vec3_t          worldtotex[2];                         // s = (world - texorg) . worldtotex[0]
    vec3_t          textoworld[2];                         // world = texorg + s * textoworld[0]

    vec_t           exactmins[2], exactmaxs[2];

    int             texmins[2], texsize[2];
    int             lightstyles[256];
    int             surfnum;
    dface_t* face;

    int             numsurfpt;
    vec3_t          surfpt[MAX_SINGLEMAP];

    void Clear()
    {
        light = NULL;
        facedist = 0;
        VectorClear(facenormal);
        VectorClear(texorg);

        for (int i = 0; i < 2; ++i)
        {
            VectorClear(worldtotex[i]);
            VectorClear(textoworld[i]);
            exactmins[i] = 0;
            exactmaxs[i] = 0;
            texmins[i] = 0;
            texsize[i] = 0;
        }

        memset(lightstyles, 0, sizeof(lightstyles));
        surfnum = 0;
        face = NULL;
        numsurfpt = 0;
    }
}
lightinfo_t;

// =====================================================================================
//  TextureNameFromFace
// =====================================================================================
static const char* TextureNameFromFace(const dface_t* const f)
{
    return g_dtextures[f->texture_index].str;
}

// =====================================================================================
//  CalcFaceExtents
//      Fills in s->texmins[] and s->texsize[]
//      also sets exactmins[] and exactmaxs[]
// =====================================================================================
static void     CalcFaceExtents(lightinfo_t* l)
{
    const int       facenum = l->surfnum;
    dface_t*        s;
    vec_t           mins[2], maxs[2], val;
    unsigned int    i, j, e;
    dvertex_t*      v;
    dtexmatrix_t*   texmatrix;

    s = l->face;

    mins[0] = mins[1] = 999999;
    maxs[0] = maxs[1] = -99999;

    texmatrix = &g_dtexmatrix[s->lightmap_info_projection_index];

    for (i = 0; i < s->num_vertices; i++)
    {
        v = &g_dverts[s->first_vertex_index + i];

        for (j = 0; j < 2; j++)
        {
            float* vecs = j == 0 ? texmatrix->s : texmatrix->t;

            val = v->point[0] * vecs[0] +
                v->point[1] * vecs[1] + v->point[2] * vecs[2] + vecs[3];
            if (val < mins[j])
            {
                mins[j] = val;
            }
            if (val > maxs[j])
            {
                maxs[j] = val;
            }
        }
    }

    for (i = 0; i < 2; i++)
    {
        l->exactmins[i] = mins[i];
        l->exactmaxs[i] = maxs[i];

        mins[i] = floor(mins[i] /* / 16.0*/); // nightfire does not divide by 16 here
        maxs[i] = ceil(maxs[i] /* 16.0*/);

		l->texmins[i] = mins[i];
        l->texsize[i] = maxs[i] - mins[i];
	}

	if (!IsSpecial(s))
	{
		if ((l->texsize[0] > 257.0) || (l->texsize[1] > 257.0))
		{
			ThreadLock();
			PrintOnce("\nfor Face %d (texture %s) at ", s - g_dFaces, TextureNameFromFace(s));

            for (i = 0; i < s->num_vertices; ++i)
            {
                v = &g_dverts[s->first_indices_index + i];

                VectorAdd(v->point, g_face_offset[facenum], v->point);
                Log("(%4.3f %4.3f %4.3f) ", v->point[0], v->point[1], v->point[2]);
            }
			Log("\n");
			Error( "Bad surface extents (%d x %d)\nCheck the file ZHLTProblems.html for a detailed explanation of this problem", l->texsize[0], l->texsize[1]);
		}
	}
}

// =====================================================================================
//  CalcFaceVectors
//      Fills in texorg, worldtotex. and textoworld
// =====================================================================================
static void     CalcFaceVectors(lightinfo_t* l)
{
    //texinfo_t*      tex;
    int             i, j;
    vec3_t          texnormal;
    vec_t           distscale;
    vec_t           dist, len;

    //tex = &g_texinfo[l->face->texinfo];
    dtexmatrix_t* texmatrix = &g_dtexmatrix[l->face->lightmap_info_projection_index];

    // convert from float to double
    for (i = 0; i < 2; i++)
    {
        vec_t* vecs = i == 0 ? texmatrix->s : texmatrix->t;
        for (j = 0; j < 3; j++)
        {
            l->worldtotex[i][j] = vecs[j];
        }
    }

    // calculate a normal to the texture axis.  points can be moved along this
    // without changing their S/T
    CrossProduct(texmatrix->t, texmatrix->s, texnormal);
    VectorNormalize(texnormal);

    // flip it towards plane normal
    distscale = DotProduct(texnormal, l->facenormal);

    // nightfire doesn't do this!
    if (distscale == 0.0)
    {
        const unsigned facenum = l->face - g_dFaces;
    
        ThreadLock();
        Log("Malformed face (%d) normal @ \n", facenum);
        Winding* w = new Winding(*l->face);
        {
            const unsigned numpoints = w->m_NumPoints;
            unsigned x;
            for (x=0; x<numpoints; x++)
            {
                VectorAdd(w->m_Points[x], g_face_offset[facenum], w->m_Points[x]);
            }
        }
        //w->Print();
        delete w;
        ThreadUnlock();

        hlassume(false, assume_MalformedTextureFace);
    }

    if (distscale < 0)
    {
        distscale = -distscale;
        VectorSubtract(vec3_origin, texnormal, texnormal);
    }

    // nightfire
    if (distscale > 1.0 - 0.0001)
        distscale = 1.0;

    //NIGHTFIRE FIXME: below looks wrong!

    // distscale is the ratio of the distance along the texture normal to
    // the distance along the plane normal
    distscale = 1.0 / distscale;

    for (i = 0; i < 2; i++)
    {
        len = (float)VectorLength(l->worldtotex[i]);
        dist = DotProduct(l->worldtotex[i], l->facenormal);
        dist *= distscale;
        VectorMA(l->worldtotex[i], -dist, texnormal, l->textoworld[i]);
        VectorScale(l->textoworld[i], (1 / len) * (1 / len), l->textoworld[i]);
    }

    // calculate texorg on the texture plane
    for (i = 0; i < 3; i++)
    {
        l->texorg[i] = -texmatrix->s[3] * l->textoworld[0][i] - texmatrix->t[3] * l->textoworld[1][i];
    }

    // project back to the face plane
    dist = DotProduct(l->texorg, l->facenormal) - l->facedist - DEFAULT_HUNT_OFFSET;
    dist *= distscale;
    VectorMA(l->texorg, -dist, texnormal, l->texorg);

}

// =====================================================================================
//  SetSurfFromST
// =====================================================================================
static void     SetSurfFromST(const lightinfo_t* const l, vec_t* surf, const vec_t s, const vec_t t)
{
    const int       facenum = l->surfnum;
    int             j;

    for (j = 0; j < 3; j++)
    {
        surf[j] = l->texorg[j] + l->textoworld[0][j] * s + l->textoworld[1][j] * t;
    }

    // Adjust for origin-based models
    VectorAdd(surf, g_face_offset[facenum], surf);
}

// =====================================================================================
//  FindSurfaceMidpoint
// =====================================================================================
static dleaf_t* FindSurfaceMidpoint(const lightinfo_t* const l, vec_t* midpoint)
{
    int             s, t;
    int             w, h;
    vec_t           starts, startt;
    vec_t           us, ut;

    vec3_t          broken_midpoint;
    vec3_t          surface_midpoint;
    int             inside_point_count;

    dleaf_t*        last_valid_leaf = NULL;
    dleaf_t*        leaf_mid;

    const int       facenum = l->surfnum;
    const dface_t*  f = g_dFaces + facenum;
    const dplane_t* p = getPlaneFromFace(f);

    const vec_t*    face_delta = g_face_offset[facenum];

    h = l->texsize[1] + 1;
    w = l->texsize[0] + 1;
    starts = (float)l->texmins[0] /* * 16*/;
    startt = (float)l->texmins[1] /* * 16*/; // nightfire fixme

    // General case
    inside_point_count = 0;
    VectorClear(surface_midpoint);
    for (t = 0; t < h; t++)
    {
        for (s = 0; s < w; s++)
        {
            us = starts + s * TEXTURE_STEP;
            ut = startt + t * TEXTURE_STEP;

            SetSurfFromST(l, midpoint, us, ut);
            if ((leaf_mid = PointInLeaf(midpoint)) != g_dleafs)
            {
                if ((leaf_mid->contents != CONTENTS_SKY) && (leaf_mid->contents != CONTENTS_SOLID))
                {
                    last_valid_leaf = leaf_mid;
                    inside_point_count++;
                    VectorAdd(surface_midpoint, midpoint, surface_midpoint);
                }
            }
        }
    }

    if (inside_point_count > 1)
    {
        vec_t           tmp = 1.0 / inside_point_count;

        VectorScale(surface_midpoint, tmp, midpoint);

        //Verbose("Trying general at (%4.3f %4.3f %4.3f) %d\n", surface_midpoint[0], surface_midpoint[1], surface_midpoint[2], inside_point_count);
        if (
            (leaf_mid =
             HuntForWorld(midpoint, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET)))
        {
            //Verbose("general method succeeded at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
            return leaf_mid;
        }
        //Verbose("Tried general , failed at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
    }
    else if (inside_point_count == 1)
    {
        //Verbose("Returning single point from general\n");
        VectorCopy(surface_midpoint, midpoint);
        return last_valid_leaf;
    }
    else
    {
        //Verbose("general failed (no points)\n");
    }

    // Try harder
    inside_point_count = 0;
    VectorClear(surface_midpoint);
    for (t = 0; t < h; t++)
    {
        for (s = 0; s < w; s++)
        {
            us = starts + s * TEXTURE_STEP;
            ut = startt + t * TEXTURE_STEP;

            SetSurfFromST(l, midpoint, us, ut);
            leaf_mid =
                HuntForWorld(midpoint, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET);
            if (leaf_mid != g_dleafs)
            {
                last_valid_leaf = leaf_mid;
                inside_point_count++;
                VectorAdd(surface_midpoint, midpoint, surface_midpoint);
            }
        }
    }

    if (inside_point_count > 1)
    {
        vec_t           tmp = 1.0 / inside_point_count;

        VectorScale(surface_midpoint, tmp, midpoint);

        if (
            (leaf_mid =
             HuntForWorld(midpoint, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET)))
        {
            //Verbose("best method succeeded at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
            return leaf_mid;
        }
        //Verbose("Tried best, failed at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
    }
    else if (inside_point_count == 1)
    {
        //Verbose("Returning single point from best\n");
        VectorCopy(surface_midpoint, midpoint);
        return last_valid_leaf;
    }
    else
    {
        //Verbose("best failed (no points)\n");
    }

    // Original broken code
    {
        vec_t           mids = (l->exactmaxs[0] + l->exactmins[0]) / 2;
        vec_t           midt = (l->exactmaxs[1] + l->exactmins[1]) / 2;

        SetSurfFromST(l, midpoint, mids, midt);

        if ((leaf_mid = PointInLeaf(midpoint)) != g_dleafs)
        {
            if ((leaf_mid->contents != CONTENTS_SKY) && (leaf_mid->contents != CONTENTS_SOLID))
            {
                return leaf_mid;
            }
        }

        VectorCopy(midpoint, broken_midpoint);
        //Verbose("Tried original method, failed at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
    }

    VectorCopy(broken_midpoint, midpoint);
    return HuntForWorld(midpoint, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET);
}

// =====================================================================================
//  SimpleNudge
//      Return vec_t in point only valid when function returns true
//      Use negative scales to push away from center instead
// =====================================================================================
static bool     SimpleNudge(vec_t* const point, const lightinfo_t* const l, vec_t* const s, vec_t* const t, const vec_t delta)
{
    const int       facenum = l->surfnum;
    const dface_t*  f = g_dFaces + facenum;
    const dplane_t* p = getPlaneFromFace(f);
    const vec_t*    face_delta = g_face_offset[facenum];
    const int       h = l->texsize[1] + 1;
    const int       w = l->texsize[0] + 1;
    const vec_t     half_w = (vec_t)(w - 1) / 2.0;
    const vec_t     half_h = (vec_t)(h - 1) / 2.0;
    const vec_t     s_vec = *s;
    const vec_t     t_vec = *t;
    vec_t           s1;
    vec_t           t1;

    if (s_vec > half_w)
    {
        s1 = s_vec - delta;
    }
    else
    {
        s1 = s_vec + delta;
    }

    SetSurfFromST(l, point, s1, t_vec);
    if (HuntForWorld(point, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET))
    {
        *s = s1;
        return true;
    }

    if (t_vec > half_h)
    {
        t1 = t_vec - delta;
    }
    else
    {
        t1 = t_vec + delta;
    }

    SetSurfFromST(l, point, s_vec, t1);
    if (HuntForWorld(point, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET))
    {
        *t = t1;
        return true;
    }

    return false;
}

typedef enum
{
    LightOutside,                                          // Not lit
    LightShifted,                                          // used HuntForWorld on 100% dark face
    LightShiftedInside,                                    // moved to neighbhor on 2nd cleanup pass
    LightNormal,                                           // Normally lit with no movement
    LightPulledInside,                                     // Pulled inside by bleed code adjustments
    LightSimpleNudge,                                      // A simple nudge 1/3 or 2/3 towards center along S or T axist
    LightSimpleNudgeEmbedded                               // A nudge either 1 full unit in each of S and T axis, or 1/3 or 2/3 AWAY from center
}
light_flag_t;

// =====================================================================================
//  CalcPoints
//      For each texture aligned grid point, back project onto the plane
//      to get the world xyz value of the sample point
// =====================================================================================
static void     CalcPoints(lightinfo_t* l)
{
    const int       facenum = l->surfnum;
    const dface_t*  f = g_dFaces + facenum;
    const dplane_t* p = getPlaneFromFace(f);

    const vec_t*    face_delta = g_face_offset[facenum];
    const eModelLightmodes lightmode = g_face_lightmode[facenum];

    const vec_t     mids = (l->exactmaxs[0] + l->exactmins[0]) / 2;
    const vec_t     midt = (l->exactmaxs[1] + l->exactmins[1]) / 2;

    const int       h = l->texsize[1] + 1;
    const int       w = l->texsize[0] + 1;

    const vec_t     starts = (l->texmins[0] /* * 16*/); //nightfire fixme
    const vec_t     startt = (l->texmins[1] /* * 16*/); //nightfire fixme

    light_flag_t    LuxelFlags[MAX_SINGLEMAP];
    light_flag_t*   pLuxelFlags;
    vec_t           us, ut;
    vec_t*          surf;
    vec3_t          surface_midpoint;
    dleaf_t*        leaf_mid;
    dleaf_t*        leaf_surf;
    int             s, t;
    int             i;

    l->numsurfpt = w * h;

    memset(LuxelFlags, 0, sizeof(LuxelFlags));

    leaf_mid = FindSurfaceMidpoint(l, surface_midpoint);
#if 0
    if (!leaf_mid)
    {
        Developer(DEVELOPER_LEVEL_FLUFF, "CalcPoints [face %d] (%4.3f %4.3f %4.3f) midpoint outside world\n",
                  facenum, surface_midpoint[0], surface_midpoint[1], surface_midpoint[2]);
    }
    else
    {
        Developer(DEVELOPER_LEVEL_FLUFF, "FindSurfaceMidpoint [face %d] @ (%4.3f %4.3f %4.3f)\n",
                  facenum, surface_midpoint[0], surface_midpoint[1], surface_midpoint[2]);
    }
#endif

    // First pass, light normally, and pull any faces toward the center for bleed adjustment

    surf = l->surfpt[0];
    pLuxelFlags = LuxelFlags;
    for (t = 0; t < h; t++)
    {
        for (s = 0; s < w; s++, surf += 3, pLuxelFlags++)
        {
            vec_t           original_s = us = starts + s * TEXTURE_STEP;
            vec_t           original_t = ut = startt + t * TEXTURE_STEP;

            SetSurfFromST(l, surf, us, ut);
            leaf_surf = HuntForWorld(surf, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET);

            if (!leaf_surf)
            {
                // At first try a 1/3 and 2/3 distance to nearest in each S and T axis towards the face midpoint
                if (SimpleNudge(surf, l, &us, &ut, TEXTURE_STEP * (1.0 / 3.0)))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
                else if (SimpleNudge(surf, l, &us, &ut, -TEXTURE_STEP * (1.0 / 3.0)))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
                else if (SimpleNudge(surf, l, &us, &ut, TEXTURE_STEP * (2.0 / 3.0)))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
                else if (SimpleNudge(surf, l, &us, &ut, -TEXTURE_STEP * (2.0 / 3.0)))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
                // Next, if this is a model flagged with the 'Embedded' mode, try away from the facemid too
                else if (lightmode & eModelLightmodeEmbedded)
                {
                    SetSurfFromST(l, surf, us, ut);
                    if (SimpleNudge(surf, l, &us, &ut, TEXTURE_STEP))
                    {
                        *pLuxelFlags = LightSimpleNudgeEmbedded;
                        continue;
                    }
                    if (SimpleNudge(surf, l, &us, &ut, -TEXTURE_STEP))
                    {
                        *pLuxelFlags = LightSimpleNudgeEmbedded;
                        continue;
                    }

                    SetSurfFromST(l, surf, original_s, original_t);
                    *pLuxelFlags = LightOutside;
                    continue;
                }
            }

            if (!(lightmode & eModelLightmodeEmbedded))
            {
                // Pull the sample points towards the facemid if visibility is blocked
                // and the facemid is inside the world
                int             nudge_divisor = max(max(w, h), 4);
                int             max_nudge = nudge_divisor + 1;
                bool            nudge_succeeded = false;

                vec_t           nudge_s = (mids - us) / (vec_t)nudge_divisor;
                vec_t           nudge_t = (midt - ut) / (vec_t)nudge_divisor;
				vec3_t			transparency;

                // if a line can be traced from surf to facemid, the point is good
                for (i = 0; i < max_nudge; i++)
                {
                    // Make sure we are "in the world"(Not the zero leaf)
                    if (leaf_mid)
                    {
                        SetSurfFromST(l, surf, us, ut);
                        leaf_surf =
                            HuntForWorld(surf, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE,
                                         DEFAULT_HUNT_OFFSET);

                        if (leaf_surf)
                        {
                            if (((unsigned int)TestLine(surface_midpoint, surf) & 0xFF) == CONTENTS_EMPTY)
                            {
                                if (lightmode & eModelLightmodeConcave)
                                {
#ifdef HLRAD_HULLU
									//removed reset of transparency - TestSegmentAgainstOpaqueList already resets to 1,1,1
									int blocking_facenum = TestSegmentAgainstOpaqueList(surface_midpoint, surf, transparency);
#else
									int blocking_facenum = TestSegmentAgainstOpaqueList(surface_midpoint, surf);
#endif
									if(blocking_facenum > -1 && blocking_facenum != facenum)
                                    {
                                        //Log("SDF::4\n");
                                        us += nudge_s;
                                        ut += nudge_t;
                                        continue;   // Try nudge again, we hit an opaque face
                                    }
                                }
                                if (i)
                                {
                                    *pLuxelFlags = LightPulledInside;
                                }
                                else
                                {
                                    *pLuxelFlags = LightNormal;
                                }
                                nudge_succeeded = true;
                                break;
                            }
                        }
                    }
                    else
                    {
                        leaf_surf = PointInLeaf(surf);
                        if (leaf_surf != g_dleafs)
                        {
                            if ((leaf_surf->contents != CONTENTS_SKY) && (leaf_surf->contents != CONTENTS_SOLID))
                            {
                                *pLuxelFlags = LightNormal;
                                nudge_succeeded = true;
                                break;
                            }
                        }
                    }

                    us += nudge_s;
                    ut += nudge_t;
                }

                if (!nudge_succeeded)
                {
                    SetSurfFromST(l, surf, original_s, original_t);
                    *pLuxelFlags = LightOutside;
                }
            }
        }
    }

    // 2nd Pass, find units that are not lit and try to move them one half or unit worth 
    // in each direction and see if that is lit.
    // This handles 1 x N lightmaps which are all dark everywhere and have no frame of refernece
    // for a good center or directly lit areas
    surf = l->surfpt[0];
    pLuxelFlags = LuxelFlags;
#if 0
    Developer(DEVELOPER_LEVEL_SPAM,
              "w (%d) h (%d) dim (%d) leafmid (%4.3f %4.3f %4.3f) plane normal (%4.3f) (%4.3f) (%4.3f) dist (%f)\n", w,
              h, w * h, surface_midpoint[0], surface_midpoint[1], surface_midpoint[2], p->normal[0], p->normal[1],
              p->normal[2], p->dist);
#endif
    {
        int             total_dark = 0;
        int             total_adjusted = 0;

        for (t = 0; t < h; t++)
        {
            for (s = 0; s < w; s++, surf += 3, pLuxelFlags++)
            {
                if (!*pLuxelFlags)
                {
#if 0
                    Developer(DEVELOPER_LEVEL_FLUFF, "Dark (%d %d) (%4.3f %4.3f %4.3f)\n",
                              s, t, surf[0], surf[1], surf[2]);
#endif
                    total_dark++;
                    if (HuntForWorld(surf, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET))
                    {
#if 0
                        Developer(DEVELOPER_LEVEL_FLUFF, "Shifted %d %d to (%4.3f %4.3f %4.3f)\n", s, t, surf[0],
                                  surf[1], surf[2]);
#endif
                        *pLuxelFlags = LightShifted;
                        total_adjusted++;
                    }
                    else if (HuntForWorld(surf, face_delta, p, 101, 0.5, DEFAULT_HUNT_OFFSET))
                    {
#if 0
                        Developer(DEVELOPER_LEVEL_FLUFF, "Shifted %d %d to (%4.3f %4.3f %4.3f)\n", s, t, surf[0],
                                  surf[1], surf[2]);
#endif
                        *pLuxelFlags = LightShifted;
                        total_adjusted++;
                    }
                }
            }
        }
#if 0
        if (total_dark)
        {
            Developer(DEVELOPER_LEVEL_FLUFF, "Pass 2 : %d dark, %d corrected\n", total_dark, total_adjusted);
        }
#endif
    }

    // 3rd Pass, find units that are not lit and move them towards neighbhors who are
    // Currently finds the first lit neighbhor and uses its data
    surf = l->surfpt[0];
    pLuxelFlags = LuxelFlags;
    {
        int             total_dark = 0;
        int             total_adjusted = 0;

        for (t = 0; t < h; t++)
        {
            for (s = 0; s < w; s++, surf += 3, pLuxelFlags++)
            {
                if (!*pLuxelFlags)
                {
                    int             x_min = max(0, s - 1);
                    int             x_max = min(w, s + 1);
                    int             y_min = max(0, t - 1);
                    int             y_max = min(t, t + 1);

                    int             x, y;

#if 0
                    Developer(DEVELOPER_LEVEL_FLUFF, "Point outside (%d %d) (%4.3f %4.3f %4.3f)\n",
                              s, t, surf[0], surf[1], surf[2]);
#endif

                    total_dark++;

                    for (x = x_min; x < x_max; x++)
                    {
                        for (y = y_min; y < y_max; y++)
                        {
                            if (*pLuxelFlags >= LightNormal)
                            {
                                dleaf_t*        leaf;
                                vec_t*          other_surf = l->surfpt[0];

                                other_surf += ((y * w) + x) * 3;

                                leaf = PointInLeaf(other_surf);
                                if ((leaf->contents != CONTENTS_SKY && leaf->contents != CONTENTS_SOLID))
                                {
                                    *pLuxelFlags = LightShiftedInside;
#if 0
                                    Developer(DEVELOPER_LEVEL_MESSAGE,
                                              "Nudged (%d %d) (%4.3f %4.3f %4.3f) to (%d %d) (%4.3f %4.3f %4.3f) \n",
                                              s, t, surf[0], surf[1], surf[2], x, y, other_surf[0], other_surf[1],
                                              other_surf[2]);
#endif
                                    VectorCopy(other_surf, surf);
                                    total_adjusted++;
                                    goto found_it;
                                }
                            }
                        }
                    }
                }
              found_it:;
            }
        }
#if 0
        if (total_dark)
        {
            Developer(DEVELOPER_LEVEL_FLUFF, "Pass 2 : %d dark, %d corrected\n", total_dark, total_adjusted);
        }
#endif
    }
}

//==============================================================

typedef struct
{
    vec3_t          pos;
    vec3_t          light;
}
sample_t;

typedef struct
{
    int             numsamples;
    sample_t*       samples[MAXLIGHTMAPS];
}
facelight_t;

//static directlight_t* directlights[MAX_MAP_LEAFS];
static facelight_t facelight[MAX_MAP_FACES];
static int      numdlights;
static lightinfo_t* lightinfos[MAX_MAP_LIGHTINFO];

lightinfo_s* AllocLightInfo(unsigned int facenum)
{
    if (!lightinfos[facenum])
        lightinfos[facenum] = (lightinfo_t*)calloc(1, sizeof(lightinfo_t));

    return lightinfos[facenum];
}

void FreeAllLightInfo()
{
    for (unsigned i = 0; i < MAX_MAP_LIGHTINFO; ++i)
    {
        if (lightinfos[i])
        {
            free(lightinfos[i]);
            lightinfos[i] = NULL;
        }
    }
}

#define	DIRECT_SCALE	0.1f

// =====================================================================================
//  CreateDirectLights
// =====================================================================================
void            CreateDirectLights(mapinfo_t* map)
{
    unsigned        i;
    patch_t*        p;
    directlight_t*  dl;
    dleaf_t*        leaf;
    //int             leafnum;
    entity_t*       e;
    entity_t*       e2;
    const char*     name;
    const char*     target;
    float           angle;
    vec3_t          dest;

    // AJM: coplaner lighting
    vec3_t          temp_normal;

    numdlights = 0;

    //
    // surfaces
    //
    for (i = 0, p = g_patches; i < g_num_patches; i++, p++)
    {
#ifdef ZHLT_TEXLIGHT
        if (VectorAvg(p->baselight) >= g_dlight_threshold) //LRC
#else
        if (VectorAvg(p->totallight) >= g_dlight_threshold)
#endif
        {
            numdlights++;
            dl = (directlight_t*)calloc(1, sizeof(directlight_t));

            VectorCopy(p->origin, dl->origin);

            if (g_visdatasize)
                leaf = PointInLeaf(dl->origin);
            else
                leaf = nullptr;
            
            dl->leaf = leaf;

            //leafnum = leaf - g_dleafs;

            tnode_t* tn = GetTnode(dl->origin);

            dl->next = (directlight_t*)tn->children[1];
            tn->children[1] = (tnode_t*)dl;

#ifdef ZHLT_TEXLIGHT
            dl->style = p->emitstyle; //LRC
#endif

            dl->type = emit_surface;
            VectorCopy(getPlaneFromFaceNumber(p->faceNumber)->normal, dl->normal);
#ifdef ZHLT_TEXLIGHT
            VectorCopy(p->baselight, dl->intensity); //LRC
#else
            VectorCopy(p->totallight, dl->intensity);
#endif
            VectorScale(dl->intensity, p->area, dl->intensity);
            VectorScale(dl->intensity, DIRECT_SCALE, dl->intensity);
        
        	// --------------------------------------------------------------
	        // Changes by Adam Foster - afoster@compsoc.man.ac.uk
	        // mazemaster's l33t backwards lighting (I still haven't a clue
	        // what it's supposed to be for) :-)
#ifdef HLRAD_WHOME

	        if (g_softlight_hack[0] || g_softlight_hack[1] || g_softlight_hack[2]) 
            {
		        numdlights++;
		        dl = (directlight_t *) calloc(1, sizeof(directlight_t));

		        VectorCopy(p->origin, dl->origin);

		        leaf = PointInLeaf(dl->origin);
		        leafnum = leaf - g_dleafs;

		        dl->next = directlights[leafnum];
		        directlights[leafnum] = dl;

		        dl->type = emit_surface;
		        VectorCopy(getPlaneFromFaceNumber(p->faceNumber)->normal, dl->normal);
		        VectorScale(dl->normal, g_softlight_hack_distance, temp_normal);
		        VectorAdd(dl->origin, temp_normal, dl->origin);
		        VectorScale(dl->normal, -1, dl->normal);

#ifdef ZHLT_TEXLIGHT
                VectorCopy(p->baselight, dl->intensity); //LRC
#else
		        VectorCopy(p->totallight, dl->intensity);
#endif
		        VectorScale(dl->intensity, p->area, dl->intensity);
		        VectorScale(dl->intensity, DIRECT_SCALE, dl->intensity);

		        dl->intensity[0] *= g_softlight_hack[0];
		        dl->intensity[1] *= g_softlight_hack[1];
		        dl->intensity[2] *= g_softlight_hack[2];
	        }

#endif
	        // --------------------------------------------------------------
        }

#ifdef ZHLT_TEXLIGHT
        //LRC        VectorClear(p->totallight[0]);
#else
        VectorClear(p->totallight);
#endif
    }

    //
    // entities
    //
    for (i = 0; i < map->numentities; i++)
    {
        const char*     pLight;
        double          r, g, b, scaler;
        float           l1;
        int             argCnt;

        e = map->entities[i];
        name = ValueForKey(e, "classname");
        if (strcmp(name, "light") && strcmp(name, "light_spot") && strcmp(name, "light_environment"))
            continue;

        numdlights++;
        dl = (directlight_t*)calloc(1, sizeof(directlight_t));

        GetVectorForKey(e, "origin", dl->origin);

        if (g_visdatasize)
            leaf = PointInLeaf(dl->origin);
        else
            leaf = nullptr;

        dl->leaf = leaf;

        tnode_t* tn = GetTnode(dl->origin);

        dl->next = (directlight_t*)tn->children[1];
        tn->children[1] = (tnode_t*)dl;

        dl->style = IntForKey(e, "style");
#ifdef ZHLT_TEXLIGHT
        if (dl->style < 0) 
            dl->style = -dl->style; //LRC
#endif

        pLight = ValueForKey(e, "_light");
        // scanf into doubles, then assign, so it is vec_t size independent
        r = g = b = scaler = 0;
        argCnt = sscanf_s(pLight, "%lf %lf %lf %lf", &r, &g, &b, &scaler);
        dl->intensity[0] = (float)r;
        if (argCnt == 1)
        {
            // The R,G,B values are all equal.
            dl->intensity[1] = dl->intensity[2] = (float)r;
        }
        else if (argCnt == 3 || argCnt == 4)
        {
            // Save the other two G,B values.
            dl->intensity[1] = (float)g;
            dl->intensity[2] = (float)b;

            // Did we also get an "intensity" scaler value too?
            if (argCnt == 4)
            {
                // Scale the normalized 0-255 R,G,B values by the intensity scaler
                dl->intensity[0] = dl->intensity[0] / 255 * (float)scaler;
                dl->intensity[1] = dl->intensity[1] / 255 * (float)scaler;
                dl->intensity[2] = dl->intensity[2] / 255 * (float)scaler;
            }
        }
        else
        {
            Log("light at (%f,%f,%f) has bad or missing '_light' value : '%s'\n",
                dl->origin[0], dl->origin[1], dl->origin[2], pLight);
            continue;
        }

        dl->fade = FloatForKey(e, "_fade");
        if (dl->fade == 0.0)
        {
            dl->fade = g_fade;
        }

        dl->falloff = IntForKey(e, "_falloff");
        if (dl->falloff == 0)
        {
            dl->falloff = g_falloff;
        }

        dl->mindist = FloatForKey(e, "_mindist");
        dl->maxdist = FloatForKey(e, "_maxdist");

        target = ValueForKey(e, "target");

        if (!strcmp(name, "light_spot") || !strcmp(name, "light_environment") || target[0])
        {
            if (!VectorAvg(dl->intensity))
            {
                VectorFill(dl->intensity, 500);
            }
            dl->type = emit_spotlight;
            dl->stopdot = FloatForKey(e, "_cone");
            if (!dl->stopdot)
            {
                dl->stopdot = 10;
            }
            dl->stopdot2 = FloatForKey(e, "_cone2");
            if (!dl->stopdot2)
            {
                dl->stopdot2 = dl->stopdot;
            }
            if (dl->stopdot2 < dl->stopdot)
            {
                dl->stopdot2 = dl->stopdot;
            }
            dl->stopdot2 = (float)cos(dl->stopdot2 / 180 * Q_PI);
            dl->stopdot = (float)cos(dl->stopdot / 180 * Q_PI);

            if (target[0])
            {                                              // point towards target
                e2 = FindTargetEntity(map, target);
                if (!e2)
                {
                    Warning("light at (%i %i %i) has missing target",
                            (int)dl->origin[0], (int)dl->origin[1], (int)dl->origin[2]);
                }
                else
                {
                    GetVectorForKey(e2, "origin", dest);
                    VectorSubtract(dest, dl->origin, dl->normal);
                    VectorNormalize(dl->normal);
                }
            }
            else
            {                                              // point down angle
                vec3_t          vAngles;

                GetVectorForKey(e, "angles", vAngles);

                angle = (float)FloatForKey(e, "angle");
                if (angle == ANGLE_UP)
                {
                    dl->normal[0] = dl->normal[1] = 0;
                    dl->normal[2] = 1;
                }
                else if (angle == ANGLE_DOWN)
                {
                    dl->normal[0] = dl->normal[1] = 0;
                    dl->normal[2] = -1;
                }
                else
                {
                    // if we don't have a specific "angle" use the "angles" YAW
                    if (!angle)
                    {
                        angle = vAngles[1];
                    }

                    dl->normal[2] = 0;
                    dl->normal[0] = (float)cos(angle / 180 * Q_PI);
                    dl->normal[1] = (float)sin(angle / 180 * Q_PI);
                }

                angle = FloatForKey(e, "pitch");
                if (!angle)
                {
                    // if we don't have a specific "pitch" use the "angles" PITCH
                    angle = vAngles[0];
                }

                dl->normal[2] = (float)sin(angle / 180 * Q_PI);
                dl->normal[0] *= (float)cos(angle / 180 * Q_PI);
                dl->normal[1] *= (float)cos(angle / 180 * Q_PI);
            }

            if (FloatForKey(e, "_sky") || !strcmp(name, "light_environment"))
            {
				// -----------------------------------------------------------------------------------
				// Changes by Adam Foster - afoster@compsoc.man.ac.uk
				// diffuse lighting hack - most of the following code nicked from earlier
				// need to get diffuse intensity from new _diffuse_light key
				//
				// What does _sky do for spotlights, anyway?
				// -----------------------------------------------------------------------------------
#ifdef HLRAD_WHOME
				pLight = ValueForKey(e, "_diffuse_light");
        		r = g = b = scaler = 0;
        		argCnt = sscanf_s(pLight, "%lf %lf %lf %lf", &r, &g, &b, &scaler);
        		dl->diffuse_intensity[0] = (float)r;
        		if (argCnt == 1)
        		{
            		// The R,G,B values are all equal.
            		dl->diffuse_intensity[1] = dl->diffuse_intensity[2] = (float)r;
        		}
        		else if (argCnt == 3 || argCnt == 4)
        		{
            		// Save the other two G,B values.
            		dl->diffuse_intensity[1] = (float)g;
            		dl->diffuse_intensity[2] = (float)b;

            		// Did we also get an "intensity" scaler value too?
      		    	if (argCnt == 4)
     	       		{
                		// Scale the normalized 0-255 R,G,B values by the intensity scaler
                		dl->diffuse_intensity[0] = dl->diffuse_intensity[0] / 255 * (float)scaler;
                		dl->diffuse_intensity[1] = dl->diffuse_intensity[1] / 255 * (float)scaler;
                		dl->diffuse_intensity[2] = dl->diffuse_intensity[2] / 255 * (float)scaler;
            		}
        		}
				else
        		{
					// backwards compatibility with maps without _diffuse_light

					dl->diffuse_intensity[0] = dl->intensity[0];
					dl->diffuse_intensity[1] = dl->intensity[1];
					dl->diffuse_intensity[2] = dl->intensity[2];
        		}
#endif
				// -----------------------------------------------------------------------------------

                dl->type = emit_skylight;
                dl->stopdot2 = FloatForKey(e, "_sky");     // hack stopdot2 to a sky key number
            }
        }
        else
        {
            if (!VectorAvg(dl->intensity))
                VectorFill(dl->intensity, 300);
            dl->type = emit_point;
        }

        if (dl->type != emit_skylight)
        {
            l1 = max(dl->intensity[0], max(dl->intensity[1], dl->intensity[2]));
            l1 = l1 * l1 / 10;

            dl->intensity[0] *= l1;
            dl->intensity[1] *= l1;
            dl->intensity[2] *= l1;
        }
    }

    hlassume(numdlights, assume_NoLights);
    Log("%i direct lights\n", numdlights);
}

// =====================================================================================
//  DeleteDirectLights
// =====================================================================================
void            DeleteDirectLights()
{
    FreeTnodes();
    FreeAllLightInfo();

#if 0
    // nightfire doesn't do this
    int             l;
    directlight_t*  dl;

    for (l = 0; l < g_numleafs; l++)
    {
        dl = directlights[l];
        while (dl)
        {
            directlights[l] = dl->next;
            free(dl);
            dl = directlights[l];
        }
    }

    // AJM: todo: strip light entities out at this point
#endif
}

// =====================================================================================
//  GatherSampleLight
// =====================================================================================
#define NUMVERTEXNORMALS	162
double          r_avertexnormals[NUMVERTEXNORMALS][3] = {
//#include "../common/anorms.h"
};
static void     GatherSampleLight(tnode_t* node, const int input_facenum, const vec3_t pos, const byte* const pvs, const vec3_t normal, vec3_t* sample, byte* styles)
{
    int             i;
    directlight_t*  l;
    vec3_t          add;
    vec3_t          delta;
#ifdef HLRAD_FASTMATH2
	const float		LIGHT_EPSILON = ON_EPSILON;
	float			dot;
#else
    float           dot, dot2;
#endif
    float           dist;
    float           ratio;
#ifdef HLRAD_OPACITY // AJM
    float           l_opacity;
#endif
    int             style_index;
    directlight_t*  sky_used = NULL;
	vec3_t			transparency;

    for (; node->planenum != -1; node = node->children[1])
    {
        // Process the left child recursively
        GatherSampleLight(node->children[0], input_facenum, pos, pvs, normal, sample, styles);
    }

    l = (directlight_t*)node->children[1];

    while (l)
    {
        i = l->leaf - g_dleafs;
        //l = directlights[i];
        //if (l)
        {
            if (((l->type == emit_skylight) /* && (g_sky_lighting_fix)*/) || (pvs[(i - 1) >> 3] & (1 << ((i - 1) & 7))))
            {
                for (; l; l = l->next)
                {
                    // skylights work fundamentally differently than normal lights
                    if (l->type == emit_skylight)
                    {
                        // only allow one of each sky type to hit any given point
                        if (sky_used)
                        {
                            continue;
                        }
                        sky_used = l;

                        // make sure the angle is okay
                        dot = -DotProduct(normal, l->normal);
                        if (dot <= ON_EPSILON / 10)
                        {
                            continue;
                        }

                        // search back to see if we can hit a sky brush
                        VectorScale(l->normal, -10000, delta);
                        VectorAdd(pos, delta, delta);
                        //if (TestLine(pos, delta) != CONTENTS_SKY)
                        if ((((unsigned __int16)TestLine(pos, delta) >> 8) & 8) == 0)
                        {
                            continue;                      // occluded
                        }

#ifdef HLRAD_HULLU
						//removed reset of transparency - TestSegmentAgainstOpaqueList already resets to 1,1,1
						int facenum = TestSegmentAgainstOpaqueList(pos, delta, transparency);
#else
						int facenum = TestSegmentAgainstOpaqueList(pos, delta);
#endif                  
						if(facenum > -1 && facenum != input_facenum)
                        {
                            continue;
                        }

                        VectorScale(l->intensity, dot, add);
#ifdef HLRAD_HULLU
                        VectorMultiply(add, transparency, add);
#endif
                    }
#ifdef HLRAD_FASTMATH2
					else
					{
						VectorSubtract(l->origin,pos,delta);
						dist = min(1,VectorNormalize(delta)); //VectorNormalize returns old length
						dot = DotProduct(delta,normal);

						if(dot <= LIGHT_EPSILON) { continue; } //behind surface

						//calculate numberator factor caused by light type
						switch(l->type)
						{
						case emit_point:
							ratio = 1;
							break;
						case emit_surface:
							ratio = -DotProduct(delta,l->normal);
							if(ratio <= LIGHT_EPSILON) { continue; } //behind light surface
							break;
						case emit_spotlight:
							ratio = -DotProduct(delta,l->normal);
							if(ratio <= l->stopdot2) { continue; } //outside penumbra
							if(ratio <= l->stopdot) //between umbra and penumbra - scale it
							{ ratio *= (ratio - l->stopdot2) / (l->stopdot - l->stopdot2); }
							break;
						default:
							hlassume(false, assume_BadLightType);
							break;
						}

						//primitive ratio (all types)
						ratio *= dot;
						ratio /= dist;

						//calculate denominator factor caused by light type
						switch(l->type)
						{
						case emit_point:
						case emit_spotlight:
							ratio /= l->fade;
							if(l->falloff == 2)
							{ ratio /= dist; }
							break;
						case emit_surface:
							ratio /= g_fade;
							if(g_falloff == 2)
							{ ratio /= dist; }
							break;
						}
						VectorScale(l->intensity, ratio, add);
					}
#else
					else
                    {
                        float           denominator;

                        VectorSubtract(l->origin, pos, delta);
                        dist = VectorNormalize(delta);
                        dot = DotProduct(delta, normal);

                        //                        if (dot <= 0.0)
                        //                            continue;
                        if (dot <= ON_EPSILON / 10)
                        {
                            continue;                      // behind sample surface
                        }

                        // nightfire mindist and maxdist
                        if (l->maxdist > 0.0 && dist > l->maxdist)
                            continue;
                        dist = dist - l->mindist;

                        if (dist < 1.0)
                        {
                            dist = 1.0;
                        }

                        // Variable power falloff (1 = inverse linear, 2 = inverse square
                        denominator = dist * l->fade;
                        if (l->falloff == 2)
                        {
                            denominator *= dist;
                        }

                        switch (l->type)
                        {
                        case emit_point:
                        {
                            // Variable power falloff (1 = inverse linear, 2 = inverse square
                            vec_t           denominator = dist * l->fade;

                            if (l->falloff == 2)
                            {
                                denominator *= dist;
                            }
                            ratio = dot / denominator;
                            VectorScale(l->intensity, ratio, add);
                            break;
                        }

                        case emit_surface:
                        {
                            dot2 = -DotProduct(delta, l->normal);
                            if (dot2 <= ON_EPSILON / 10)
                            {
                                continue;                  // behind light surface
                            }

                            // Variable power falloff (1 = inverse linear, 2 = inverse square
                            vec_t           denominator = dist * g_fade;
                            if (g_falloff == 2)
                            {
                                denominator *= dist;
                            }
                            ratio = dot * dot2 / denominator;

                            VectorScale(l->intensity, ratio, add);
                            break;
                        }

                        case emit_spotlight:
                        {
                            dot2 = -DotProduct(delta, l->normal);
                            if (dot2 <= l->stopdot2)
                            {
                                continue;                  // outside light cone
                            }

                            // Variable power falloff (1 = inverse linear, 2 = inverse square
                            vec_t           denominator = dist * l->fade;
                            if (l->falloff == 2)
                            {
                                denominator *= dist;
                            }
                            ratio = dot * dot2 / denominator;

                            if (dot2 <= l->stopdot)
                            {
                                ratio *= (dot2 - l->stopdot2) / (l->stopdot - l->stopdot2);
                            }
                            VectorScale(l->intensity, ratio, add);
                            break;
                        }

                        default:
                        {
                            hlassume(false, assume_BadLightType);
                            break;
                        }
                        }
                    }
#endif
                    if (VectorMaximum(add) > (l->style ? g_coring : 0))
                    {
#ifdef HLRAD_HULLU
                 	    vec3_t transparency = {1.0,1.0,1.0};
#endif 

                        if (l->type != emit_skylight && ((unsigned)TestLine(pos, l->origin) & 0xFF) != CONTENTS_EMPTY)
                        {
                            continue;                      // occluded
                        }

                        if (l->type != emit_skylight)
                        {                                  // Don't test from light_environment entities to face, the special sky code occludes correctly
#ifdef HLRAD_HULLU
							int facenum = TestSegmentAgainstOpaqueList(pos, delta, transparency);
#else
							int facenum = TestSegmentAgainstOpaqueList(pos, delta);
#endif                  
							if(facenum > -1 && facenum != input_facenum)
                            {
                                continue;
                            }
                        }

#ifdef HLRAD_OPACITY
                        //VectorScale(add, l_opacity, add);
#endif

                        for (style_index = 0; style_index < MAXLIGHTMAPS; style_index++)
                        {
                            if (styles[style_index] == l->style || styles[style_index] == 255)
                            {
                                break;
                            }
                        }

                        if (style_index == MAXLIGHTMAPS)
                        {
                            Warning("Too many direct light styles on a face(%f,%f,%f)", pos[0], pos[1], pos[2]);
                            continue;
                        }

                        if (styles[style_index] == 255)
                        {
                            styles[style_index] = l->style;
                        }

#ifdef HLRAD_HULLU
                        VectorMultiply(add,transparency,add);
#endif
                        VectorAdd(sample[style_index], add, sample[style_index]);
                    }
                }
            }
        }
    }

    if (sky_used && g_indirect_sun != 0.0)
    {
        vec3_t          total;
        int             j;
		vec3_t          sky_intensity;

		// -----------------------------------------------------------------------------------
		// Changes by Adam Foster - afoster@compsoc.man.ac.uk
		// Instead of using intensity from sky_used->intensity, get it from the new sky_used->diffuse_intensity
#ifdef HLRAD_WHOME
		VectorScale(sky_used->diffuse_intensity, g_indirect_sun / (NUMVERTEXNORMALS * 2), sky_intensity);
#else
        VectorScale(sky_used->intensity, g_indirect_sun / (NUMVERTEXNORMALS * 2), sky_intensity);
#endif
		// That should be it. Who knows - it might actually work!
        // AJM: It DOES actually work. Havent you ever heard of beta testing....
		// -----------------------------------------------------------------------------------

        total[0] = total[1] = total[2] = 0.0;
        for (j = 0; j < NUMVERTEXNORMALS; j++)
        {
            // make sure the angle is okay
            dot = -DotProduct(normal, r_avertexnormals[j]);
            if (dot <= ON_EPSILON / 10)
            {
                continue;
            }

            // search back to see if we can hit a sky brush
            VectorScale(r_avertexnormals[j], -10000, delta);
            VectorAdd(pos, delta, delta);
            if (((unsigned)TestLine(pos, delta) & 0xFF) != CONTENTS_SKY)
            {
                continue;                                  // occluded
            }

            VectorScale(sky_intensity, dot, add);
            VectorAdd(total, add, total);
        }
        if (VectorMaximum(total) > 0)
        {
            for (style_index = 0; style_index < MAXLIGHTMAPS; style_index++)
            {
                if (styles[style_index] == sky_used->style || styles[style_index] == 255)
                {
                    break;
                }
            }

            if (style_index == MAXLIGHTMAPS)
            {
                Warning("Too many direct light styles on a face(%f,%f,%f)", pos[0], pos[1], pos[2]);
                return;
            }

            if (styles[style_index] == 255)
            {
                styles[style_index] = sky_used->style;
            }

            VectorAdd(sample[style_index], total, sample[style_index]);
        }
    }
}

// =====================================================================================
//  AddSampleToPatch
//      Take the sample's collected light and add it back into the apropriate patch for the radiosity pass.
// =====================================================================================
#ifdef ZHLT_TEXLIGHT
static void     AddSampleToPatch(const sample_t* const s, const int facenum, int style) //LRC
#else
static void     AddSampleToPatch(const sample_t* const s, const int facenum)
#endif
{
    patch_t*        patch;
    BoundingBox     bounds;
    int             i;

    if (g_numbounce == 0)
    {
        return;
    }

    for (patch = g_face_patches[facenum]; patch; patch = patch->next)
    {
        // see if the point is in this patch (roughly)
        patch->winding->getBounds(bounds);
        for (i = 0; i < 3; i++)
        {
            if (bounds.m_Mins[i] > s->pos[i] + 16)
            {
                goto nextpatch;
            }
            if (bounds.m_Maxs[i] < s->pos[i] - 16)
            {
                goto nextpatch;
            }
        }

        // add the sample to the patch
#ifdef ZHLT_TEXLIGHT
        //LRC:
		for (i = 0; i < MAXLIGHTMAPS && patch->totalstyle[i] != 255; i++)
		{
			if (patch->totalstyle[i] == style)
				break;
		}
		if (i == MAXLIGHTMAPS)
		{
			if(!g_warned_direct || g_verbose)
			{
                Warning("Too many light styles on a face(%f,%f,%f)",patch->origin[0],patch->origin[1],patch->origin[2]);
				g_warned_direct = true;
			}
		}
		else
		{
			if (patch->totalstyle[i] == 255)
			{
				patch->totalstyle[i] = style;
			}

	        patch->samples[i]++;
			VectorAdd(patch->samplelight[i], s->light, patch->samplelight[i]);
		}
        //LRC (ends)
#else
        patch->samples++;
        VectorAdd(patch->samplelight, s->light, patch->samplelight);
#endif
        //return;

      nextpatch:;
    }

    // don't worry if some samples don't find a patch
}

// =====================================================================================
//  GetPhongNormal
// =====================================================================================
void            GetPhongNormal(int facenum, vec3_t spot, vec3_t phongnormal)
{
    unsigned int             j;
    const dface_t* f = g_dFaces + facenum;
    const dplane_t* p = getPlaneFromFace(f);
    vec3_t          facenormal;

    VectorCopy(p->normal, facenormal);
    VectorCopy(facenormal, phongnormal);

    if (g_smoothing_threshold > 0.0)
    {
        // Calculate modified point normal for surface
        // Use the edge normals iff they are defined.  Bend the surface towards the edge normal(s)
        // Crude first attempt: find nearest edge normal and do a simple interpolation with facenormal.
        // Second attempt: find edge points+center that bound the point and do a three-point triangulation(baricentric)
        // Better third attempt: generate the point normals for all vertices and do baricentric triangulation.
        unsigned int firstedge = g_firstedges[f];

        for (j = 0; j < f->num_vertices; j++)
        {
            vec3_t          p1;
            vec3_t          p2;
            vec3_t          v1;
            vec3_t          v2;
            vec3_t          vspot;
            unsigned        prev_edge;
            unsigned        next_edge;
            int             e;
            int             e1;
            int             e2;
            edgeshare_t* es;
            edgeshare_t* es1;
            edgeshare_t* es2;
            float           a1;
            float           a2;
            float           aa;
            float           bb;
            float           ab;

            if (j)
            {
                prev_edge = firstedge + ((j - 1) % f->num_vertices);
            }
            else
            {
                prev_edge = firstedge + f->num_vertices - 1;
            }

            if ((j + 1) != firstedge)
            {
                next_edge = firstedge + ((j + 1) % f->num_vertices);
            }
            else
            {
                next_edge = firstedge;
            }

            e = g_dsurfedges[firstedge + j];
            e1 = g_dsurfedges[prev_edge];
            e2 = g_dsurfedges[next_edge];

            es = &g_edgeshare[abs(e)];
            es1 = &g_edgeshare[abs(e1)];
            es2 = &g_edgeshare[abs(e2)];

            if (
                (es->coplanar && es1->coplanar && es2->coplanar)
                ||
                (VectorCompare(es->interface_normal, vec3_origin) &&
                    VectorCompare(es1->interface_normal, vec3_origin) &&
                    VectorCompare(es2->interface_normal, vec3_origin)))
            {
                continue;
            }

            if (e > 0)
            {
                VectorCopy(g_dverts[g_dedges[e].v[0]].point, p1);
                VectorCopy(g_dverts[g_dedges[e].v[1]].point, p2);
            }
            else
            {
                VectorCopy(g_dverts[g_dedges[-e].v[1]].point, p1);
                VectorCopy(g_dverts[g_dedges[-e].v[0]].point, p2);
            }

            // Adjust for origin-based models
            VectorAdd(p1, g_face_offset[facenum], p1);
            VectorAdd(p2, g_face_offset[facenum], p2);

            // Build vectors from the middle of the face to the edge vertexes and the sample pos.
            VectorSubtract(p1, g_face_centroids[facenum], v1);
            VectorSubtract(p2, g_face_centroids[facenum], v2);
            VectorSubtract(spot, g_face_centroids[facenum], vspot);

            aa = DotProduct(v1, v1);
            bb = DotProduct(v2, v2);
            ab = DotProduct(v1, v2);
            a1 = (bb * DotProduct(v1, vspot) - ab * DotProduct(vspot, v2)) / (aa * bb - ab * ab);
            a2 = (DotProduct(vspot, v2) - a1 * ab) / bb;

            // Test center to sample vector for inclusion between center to vertex vectors (Use dot product of vectors)
            if (a1 >= 0.0 && a2 >= 0.0)
            {
                // calculate distance from edge to pos
                vec3_t          n1, n2;
                vec3_t          temp;

                VectorAdd(es->interface_normal, es1->interface_normal, n1);

                if (VectorCompare(n1, vec3_origin))
                {
                    VectorCopy(facenormal, n1);
                }
                VectorNormalize(n1);

                VectorAdd(es->interface_normal, es2->interface_normal, n2);

                if (VectorCompare(n2, vec3_origin))
                {
                    VectorCopy(facenormal, n2);
                }
                VectorNormalize(n2);

                // Interpolate between the center and edge normals based on sample position
                VectorScale(facenormal, 1.0 - a1 - a2, phongnormal);
                VectorScale(n1, a1, temp);
                VectorAdd(phongnormal, temp, phongnormal);
                VectorScale(n2, a2, temp);
                VectorAdd(phongnormal, temp, phongnormal);
                VectorNormalize(phongnormal);
                break;
            }
        }
    }
}

const vec3_t    s_circuscolors[] = {
    {100000.0,  100000.0,   100000.0},                              // white
    {100000.0,  0.0,        0.0     },                              // red
    {0.0,       100000.0,   0.0     },                              // green
    {0.0,       0.0,        100000.0},                              // blue
    {0.0,       100000.0,   100000.0},                              // cyan
    {100000.0,  0.0,        100000.0},                              // magenta
    {100000.0,  100000.0,   0.0     }                               // yellow
};

// =====================================================================================
//  BuildFacelights
// =====================================================================================
void            BuildFacelights(const int facenum)
{
    dface_t*        f;
    vec3_t          sampled[MAXLIGHTMAPS];
    int             i;
    int             j;
    int             k;
    sample_t*       s;
    vec_t*          spot;
    patch_t*        patch;
    const dplane_t* plane;
    byte            pvs[(MAX_MAP_LEAFS + 7) / 8];
    int             thisoffset = -1, lastoffset = -1;
    int             lightmapwidth;
    int             lightmapheight;
    int             size;

#ifdef HLRAD_HULLU
    bool            b_transparency_loss = false;
    vec_t           light_left_for_facelight = 1.0;
#endif

    f = &g_dFaces[facenum];

    //
    // some surfaces don't need lightmaps
    //
    f->lightmap_offset = -1;
    for (j = 0; j < MAXLIGHTMAPS; j++)
    {
        f->light_styles[j] = 255;
    }

    if (IsSpecial(f))
    {
        return;                                            // non-lit texture
    }

    f->light_styles[0] = 0;                                      // Everyone gets the style zero map.


    lightinfo_t& l = *AllocLightInfo(facenum);
    l.Clear();
    //memset(&l, 0, sizeof(l));

    l.surfnum = facenum;
    l.face = f;

    //
    // get transparency loss (part of light go through transparency faces.. reduce facelight on these)
    //
#ifdef HLRAD_HULLU
    for(unsigned int m = 0; m < g_opaque_face_count; m++)
    {
        opaqueList_t* opaque = &g_opaque_face_list[m];
        if(opaque->facenum == facenum && opaque->transparency)
        {
            vec_t transparency = opaque->transparency;
            
            b_transparency_loss = true;
            
            light_left_for_facelight = 1.0 - transparency;
            if( light_left_for_facelight < 0.0 ) light_left_for_facelight = 0.0;
            if( light_left_for_facelight > 1.0 ) light_left_for_facelight = 1.0;
            
            break;
        }
    }
#endif

    //
    // rotate plane
    //
    plane = getPlaneFromFace(f);
    VectorCopy(plane->normal, l.facenormal);
    l.facedist = plane->dist;

    CalcFaceVectors(&l);
    CalcFaceExtents(&l);
    CalcPoints(&l);

    lightmapwidth = l.texsize[0] + 1;
    lightmapheight = l.texsize[1] + 1;

    size = lightmapwidth * lightmapheight;
    hlassume(size <= MAX_SINGLEMAP, assume_MAX_SINGLEMAP);

    facelight[facenum].numsamples = l.numsurfpt;

    for (k = 0; k < MAXLIGHTMAPS; k++)
    {
        facelight[facenum].samples[k] = (sample_t*)calloc(l.numsurfpt, sizeof(sample_t));
    }

    spot = l.surfpt[0];
    for (i = 0; i < l.numsurfpt; i++, spot += 3)
    {
        vec3_t          pointnormal = { 0, 0, 0 };

        for (k = 0; k < MAXLIGHTMAPS; k++)
        {
            VectorCopy(spot, facelight[facenum].samples[k][i].pos);
        }

        // get the PVS for the pos to limit the number of checks
        if (!g_visdatasize)
        {
            memset(pvs, 255, (g_numDLeafs + 7) / 8);
            lastoffset = -1;
        }
        else
        {
            dleaf_t*        leaf = PointInLeaf(spot);

            thisoffset = leaf->visofs;
            if (i == 0 || thisoffset != lastoffset)
            {
                hlassert(thisoffset != -1);

                memcpy(pvs, &g_dvisdata[leaf->visofs], sizeof(pvs));
                //DecompressVis(&g_dvisdata[leaf->visofs], pvs, sizeof(pvs));
            }
            lastoffset = thisoffset;
        }

        memset(sampled, 0, sizeof(sampled));

        // If we are doing "extra" samples, oversample the direct light around the point.
        if (g_extra)
        {
            int             weighting[3][3] = { {5, 9, 5}, {9, 16, 9}, {5, 9, 5} };
            vec3_t          pos;
            int             s, t, subsamples = 0;

            for (t = -1; t <= 1; t++)
            {
                for (s = -1; s <= 1; s++)
                {
                    int             subsample = i + t * lightmapwidth + s;
                    int             sample_s = i % lightmapwidth;
                    int sample_t = i / lightmapwidth;

                    if ((0 <= s + sample_s) && (s + sample_s < lightmapwidth)
                        && (0 <= t + sample_t)&&(t + sample_t <lightmapheight))
                    {
                        vec3_t          subsampled[MAXLIGHTMAPS];

                        for (j = 0; j < MAXLIGHTMAPS; j++)
                        {
                            VectorFill(subsampled[j], 0);
                        }

                        // Calculate the point one third of the way toward the "subsample point"
                        VectorCopy(l.surfpt[i], pos);
                        VectorAdd(pos, l.surfpt[i], pos);
                        VectorAdd(pos, l.surfpt[subsample], pos);
                        VectorScale(pos, 1.0 / 3.0, pos);

                        GetPhongNormal(facenum, pos, pointnormal);
                        GatherSampleLight(tnodes, facenum, pos, pvs, pointnormal, subsampled, f->light_styles);
                        for (j = 0; j < MAXLIGHTMAPS && (f->light_styles[j] != 255); j++)
                        {
                            VectorScale(subsampled[j], weighting[s + 1][t + 1], subsampled[j]);
                            VectorAdd(sampled[j], subsampled[j], sampled[j]);
                        }
                        subsamples += weighting[s + 1][t + 1];
                    }
                }
            }
            for (j = 0; j < MAXLIGHTMAPS && (f->light_styles[j] != 255); j++)
            {
                VectorScale(sampled[j], 1.0 / subsamples, sampled[j]);
            }
        }
        else
        {
            GetPhongNormal(facenum, spot, pointnormal);
            GatherSampleLight(tnodes, facenum, spot, pvs, pointnormal, sampled, f->light_styles);
        }

        for (j = 0; j < MAXLIGHTMAPS && (f->light_styles[j] != 255); j++)
        {
            VectorCopy(sampled[j], facelight[facenum].samples[j][i].light);

#ifdef HLRAD_HULLU            
            if(b_transparency_loss)
            {
            	 VectorScale(facelight[facenum].samples[j][i].light, light_left_for_facelight, facelight[facenum].samples[j][i].light);
            }
#endif

#ifdef ZHLT_TEXLIGHT
            AddSampleToPatch(&facelight[facenum].samples[j][i], facenum, f->light_styles[j]); //LRC
#else
            if (f->light_styles[j] == 0)
            {
                AddSampleToPatch(&facelight[facenum].samples[j][i], facenum);
            }
#endif
        }
    }

    // average up the direct light on each patch for radiosity
    if (g_numbounce > 0)
    {
        for (patch = g_face_patches[facenum]; patch; patch = patch->next)
        {
#ifdef ZHLT_TEXLIGHT
            //LRC:
			unsigned istyle;
			for (istyle = 0; istyle < MAXLIGHTMAPS && patch->totalstyle[istyle] != 255; istyle++)
			{
				if (patch->samples[istyle])
		        {
		            vec3_t          v;                         // BUGBUG: Use a weighted average instead?

					VectorScale(patch->samplelight[istyle], (1.0f / patch->samples[istyle]), v);
					VectorAdd(patch->totallight[istyle], v, patch->totallight[istyle]);
	                VectorAdd(patch->directlight[istyle], v, patch->directlight[istyle]);
				}
			}
            //LRC (ends)
#else
            if (patch->samples)
            {
                vec3_t          v;                         // BUGBUG: Use a weighted average instead?

                VectorScale(patch->samplelight, (1.0f / patch->samples), v);
                VectorAdd(patch->totallight, v, patch->totallight);
                VectorAdd(patch->directlight, v, patch->directlight);
            }
#endif
        }
    }

    // add an ambient term if desired
    if (g_ambient[0] || g_ambient[1] || g_ambient[2])
    {
        for (j = 0; j < MAXLIGHTMAPS && f->light_styles[j] != 255; j++)
        {
            if (f->light_styles[j] == 0)
            {
                s = facelight[facenum].samples[j];
                for (i = 0; i < l.numsurfpt; i++, s++)
                {
                    VectorAdd(s->light, g_ambient, s->light);
                }
                break;
            }
        }

    }

    // add circus lighting for finding black lightmaps
    if (g_circus)
    {
        for (j = 0; j < MAXLIGHTMAPS && f->light_styles[j] != 255; j++)
        {
            if (f->light_styles[j] == 0)
            {
                int             amt = 7;

                s = facelight[facenum].samples[j];

                while ((l.numsurfpt % amt) == 0)
                {
                    amt--;
                }
                if (amt < 2)
                {
                    amt = 7;
                }

                for (i = 0; i < l.numsurfpt; i++, s++)
                {
                    if ((s->light[0] == 0) && (s->light[1] == 0) && (s->light[2] == 0))
                    {
                        VectorAdd(s->light, s_circuscolors[i % amt], s->light);
                    }
                }
                break;
            }
        }
    }

    // light from dlight_threshold and above is sent out, but the
    // texture itself should still be full bright

    // if( VectorAvg( face_patches[facenum]->baselight ) >= dlight_threshold)       // Now all lighted surfaces glow
    {
#ifdef ZHLT_TEXLIGHT
        //LRC:
		if (g_face_patches[facenum])
		{
			for (j = 0; j < MAXLIGHTMAPS && f->light_styles[j] != 255; j++)
			{
                if (f->light_styles[j] == g_face_patches[facenum]->emitstyle) //LRC
				{
					break;
				}
			}

			if (j == MAXLIGHTMAPS)
			{
				if(!g_warned_direct || g_verbose)
				{ 
					Warning("Too many light styles on a face(%f,%f,%f)",g_face_patches[facenum]->origin[0],g_face_patches[facenum]->origin[1],g_face_patches[facenum]->origin[2]); 
					g_warned_direct = true;
				}
			}
			else
			{
				if (f->light_styles[j] == 255)
				{
					f->light_styles[j] = g_face_patches[facenum]->emitstyle;
				}

				s = facelight[facenum].samples[j];
				for (i = 0; i < l.numsurfpt; i++, s++)
				{
					VectorAdd(s->light, g_face_patches[facenum]->baselight, s->light);
				}
			}
		}
        //LRC (ends)
#else
        for (j = 0; j < MAXLIGHTMAPS && f->light_styles[j] != 255; j++)
        {
            if (f->light_styles[j] == 0)
            {
                if (g_face_patches[facenum])
                {
                    s = facelight[facenum].samples[j];
                    for (i = 0; i < l.numsurfpt; i++, s++)
                    {
                        VectorAdd(s->light, g_face_patches[facenum]->baselight, s->light);
                    }
                    break;
                }
            }
        }
#endif
    }
}

// =====================================================================================
//  PrecompLightmapOffsets
// =====================================================================================
void            PrecompLightmapOffsets()
{
    int             facenum;
    dface_t*        f;
    facelight_t*    fl;
    int             lightstyles;

#ifdef ZHLT_TEXLIGHT
    int             i; //LRC
	patch_t*        patch; //LRC
#endif

    g_lightdatasize = 0;

    for (facenum = 0; facenum < g_numDFaces; facenum++)
    {
        f = &g_dFaces[facenum];
        fl = &facelight[facenum];

        if (IsSpecial(f))
        {
            continue;                                      // non-lit texture
        }

#ifdef ZHLT_TEXLIGHT
        		//LRC - find all the patch lightstyles, and add them to the ones used by this face
		patch = g_face_patches[facenum];
		if (patch)
		{
			for (i = 0; i < MAXLIGHTMAPS && patch->totalstyle[i] != 255; i++)
			{
				for (lightstyles = 0; lightstyles < MAXLIGHTMAPS && f->styles[lightstyles] != 255; lightstyles++)
				{
					if (f->styles[lightstyles] == patch->totalstyle[i])
						break;
				}
				if (lightstyles == MAXLIGHTMAPS)
				{
					if(!g_warned_direct || g_verbose)
					{ 
						Warning("Too many direct light styles on a face(%f,%f,%f)",patch->origin[0],patch->origin[1],patch->origin[2]); 
						g_warned_direct = true;
					}
				}
				else if (f->styles[lightstyles] == 255)
				{
					f->styles[lightstyles] = patch->totalstyle[i];
//					Log("Face acquires new lightstyle %d at offset %d\n", f->styles[lightstyles], lightstyles);
				}
			}
		}
		//LRC (ends)
#endif

        for (lightstyles = 0; lightstyles < MAXLIGHTMAPS; lightstyles++)
        {
            if (f->light_styles[lightstyles] == 255)
            {
                break;
            }
        }

        if (!lightstyles)
        {
            continue;
        }

        f->lightmap_offset = g_lightdatasize;
        g_lightdatasize += fl->numsamples * 3 * lightstyles; //nightfire todo fixme: 2 *?

        Verbose("Face %d : numsamples(%d), lightstyles(%d), g_lightdatasize(%d)\n", facenum, fl->numsamples, lightstyles, g_lightdatasize);
    }
}

// =====================================================================================
//  FinalLightFace
//      Add the indirect lighting on top of the direct lighting and save into final map format
// =====================================================================================
void            FinalLightFace(const int facenum)
{
    int             i, j, k;
    vec3_t          lb, v;
    facelight_t*    fl;
    sample_t*       samp;
    float           minlight;
    int             lightstyles;
    dface_t*        f;
    lerpTriangulation_t* trian = NULL;

    // ------------------------------------------------------------------------
    // Changes by Adam Foster - afoster@compsoc.man.ac.uk
#ifdef HLRAD_WHOME
    float           temp_rand;
#endif
    // ------------------------------------------------------------------------

    f = &g_dFaces[facenum];
    fl = &facelight[facenum];

    if (IsSpecial(f))
    {
        return;                                            // non-lit texture
    }

    for (lightstyles = 0; lightstyles < MAXLIGHTMAPS; lightstyles++)
    {
        if (f->light_styles[lightstyles] == 255)
        {
            break;
        }
    }

    if (!lightstyles)
    {
        return;
    }

    //
    // set up the triangulation
    //
    if (g_numbounce)
    {
        trian = CreateTriangulation(facenum);
    }
    //
    // sample the triangulation
    //
    minlight = FloatForKey(g_face_entity[facenum], "_minlight") * 128;

	//hlassume(f->lightmap_offset + fl->numsamples*lightstyles*3 < g_max_map_lightdata, assume_MAX_MAP_LIGHTING);

    for (k = 0; k < lightstyles; k++)
    {
        samp = fl->samples[k];
        for (j = 0; j < fl->numsamples; j++, samp++)
        {
            // Should be a VectorCopy, but we scale by 2 to compensate for an earlier lighting flaw
            // Specifically, the directlight contribution was included in the bounced light AND the directlight
            // Since many of the levels were built with this assumption, this "fudge factor" compensates for it.

            VectorScale(samp->light, g_direct_scale, lb);

#ifdef ZHLT_TEXLIGHT
            if (g_numbounce)//LRC && (k == 0))
            {
                SampleTriangulation(trian, samp->pos, v, f->styles[k]); //LRC
#else
            if (g_numbounce && (k == 0))
            {
                SampleTriangulation(trian, samp->pos, v);
#endif

                if (isPointFinite(v))
                {
                    VectorAdd(lb, v, lb);
                }
                else
                {
                    Warning("point (%4.3f %4.3f %4.3f) infinite v (%4.3f %4.3f %4.3f)\n",
                            samp->pos[0], samp->pos[1], samp->pos[2], v[0], v[1], v[2]);
                }


            }

            // ------------------------------------------------------------------------
	        // Changes by Adam Foster - afoster@compsoc.man.ac.uk
	        // colour lightscale - code was originally:
#ifdef HLRAD_WHOME
	        lb[0] *= g_colour_lightscale[0];
	        lb[1] *= g_colour_lightscale[1];
	        lb[2] *= g_colour_lightscale[2];
#else
            VectorScale(lb, g_lightscale, lb);
#endif
            // ------------------------------------------------------------------------

            // clip from the bottom first
            for (i = 0; i < 3; i++)
            {
                if (lb[i] < minlight)
                {
                    lb[i] = minlight;
                }
            }

            // clip from the top
            {
                vec_t           max = VectorMaximum(lb);

                if (max > g_maxlight)
                {
                    vec_t           scale = g_maxlight / max;

                    lb[0] *= scale;
                    lb[1] *= scale;
                    lb[2] *= scale;
                }
            }

	        // ------------------------------------------------------------------------
	        // Changes by Adam Foster - afoster@compsoc.man.ac.uk
#ifdef HLRAD_WHOME

            // AJM: your code is formatted really wierd, and i cant understand a damn thing. 
            //      so i reformatted it into a somewhat readable "normal" fashion. :P

	        // colour gamma  - code was originally:
	        //   if (g_qgamma != 1.0) {
	        //       for (i = 0; i < 3; i++) {
	        //           lb[i] = (float) pow(lb[i] / 256.0f, g_qgamma) * 256.0f;
	        //	     }
	        //   }

	        if ( g_colour_qgamma[0] != 1.0 ) 
		        lb[0] = (float) pow(lb[0] / 256.0f, g_colour_qgamma[0]) * 256.0f;

	        if ( g_colour_qgamma[1] != 1.0 ) 
		        lb[1] = (float) pow(lb[1] / 256.0f, g_colour_qgamma[1]) * 256.0f;

	        if ( g_colour_qgamma[2] != 1.0 ) 
		        lb[2] = (float) pow(lb[2] / 256.0f, g_colour_qgamma[2]) * 256.0f;

	        // Two different ways of adding noise to the lightmap - colour jitter
	        // (red, green and blue channels are independent), and mono jitter
	        // (monochromatic noise). For simulating dithering, on the cheap. :)

	        // Tends to create seams between adjacent polygons, so not ideal.

	        // Got really weird results when it was set to limit values to 256.0f - it
	        // was as if r, g or b could wrap, going close to zero.

	        if (g_colour_jitter_hack[0] || g_colour_jitter_hack[1] || g_colour_jitter_hack[2]) 
            {
		        for (i = 0; i < 3; i++) 
                {
		            lb[i] += g_colour_jitter_hack[i] * ((float)rand() / RAND_MAX - 0.5);
		            if (lb[i] < 0.0f)
                    {
			            lb[i] = 0.0f;
                    }
		            else if (lb[i] > 255.0f)
                    {
			            lb[i] = 255.0f;
                    }
		        }
	        }

	        if (g_jitter_hack[0] || g_jitter_hack[1] || g_jitter_hack[2]) 
            {
		        temp_rand = (float)rand() / RAND_MAX - 0.5;
		        for (i = 0; i < 3; i++) 
                {
		            lb[i] += g_jitter_hack[i] * temp_rand;
		            if (lb[i] < 0.0f)
                    {
			            lb[i] = 0.0f;
                    }
		            else if (lb[i] > 255.0f)
                    {
			            lb[i] = 255.0f;
                    }
		        }
	        }
#else
            if (g_qgamma != 1.0) {
	            for (i = 0; i < 3; i++) {
	                lb[i] = (float) pow(lb[i] / 256.0f, g_qgamma) * 256.0f;
	            }
	        }
#endif
	        // ------------------------------------------------------------------------

            {
                unsigned char* colors = &g_dlightdata[f->lightmap_offset + k * fl->numsamples * 3 + j * 3];

                colors[0] = (unsigned char)lb[0];
                colors[1] = (unsigned char)lb[1];
                colors[2] = (unsigned char)lb[2];
            }
        }
    }

    if (g_numbounce)
    {
        FreeTriangulation(trian);
    }
}


#ifdef ZHLT_TEXLIGHT
//LRC
vec3_t    totallight_default = { 0, 0, 0 };

//LRC - utility for getting the right totallight value from a patch
vec3_t* GetTotalLight(patch_t* patch, int style)
{
	int i;
	for (i = 0; i < MAXLIGHTMAPS && patch->totalstyle[i] != 255; i++)
	{
		if (patch->totalstyle[i] == style)
			return &(patch->totallight[i]);
	}
	return &totallight_default;
}

#endif
