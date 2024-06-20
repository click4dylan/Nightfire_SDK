#include "brad.h"

static dplane_t backplanes[MAX_INTERNAL_MAP_PLANES];

tnode_s* GetTnode(vec3_t point)
{
    tnode_s* node = tnodes;

    while (node->planenum != -1)
    {
        vec_t dist;

        switch (node->plane.closest_axis)
        {
        case planetypes::plane_x:
            dist = point[0] - node->plane.dist;
            break;
        case planetypes::plane_y:
            dist = point[1] - node->plane.dist;
            break;
        default:
            dist = DotProduct(point, node->plane.normal) - node->plane.dist;
            break;
        }

        node = (dist <= 0.0) ? node->children[1] : node->children[0];
    }

    return node;
}

dleaf_t*        PointInLeaf(const vec3_t point)
{
    int             nodenum;
    vec_t           dist;
    dnode_t*        node;
    dplane_t*       plane;

    nodenum = 0;
    while (nodenum >= 0)
    {
        node = &g_dnodes[nodenum];
        plane = &g_dplanes[node->planenum];
        dist = DotProduct(point, plane->normal) - plane->dist;
        if (dist >= 0.0)
        {
            nodenum = node->children[0];
        }
        else
        {
            nodenum = node->children[1];
        }
    }

    return &g_dleafs[-nodenum - 1];
}

/*
 * ==============
 * PatchPlaneDist
 * Fixes up patch planes for brush models with an origin brush
 * ==============
 */
vec_t           PatchPlaneDist(const patch_t* const patch)
{
    const dplane_t* plane = getPlaneFromFaceNumber(patch->faceNumber);

    return plane->dist + DotProduct(g_face_offset[patch->faceNumber], plane->normal);
}

void            MakeBackplanes()
{
    int             i;

    for (i = 0; i < g_numplanes; i++)
    {
        backplanes[i].dist = -g_dplanes[i].dist;
        VectorSubtract(vec3_origin, g_dplanes[i].normal, backplanes[i].normal);
    }
}

const dplane_t* getPlaneFromFace(const dface_t* const face)
{
    if (!face)
    {
        Error("getPlaneFromFace() face was NULL\n");
    }

    if (face->side)
    {
        return &backplanes[face->planenum];
    }
    else
    {
        return &g_dplanes[face->planenum];
    }
}

const dplane_t* getPlaneFromFaceNumber(const unsigned int faceNumber)
{
    dface_t*        face = &g_dFaces[faceNumber];

    return g_dplanes
    /*
    if (face->side)
    {
        return &backplanes[face->planenum];
    }
    else
    {
        return &g_dplanes[face->planenum];
    }*/
}

// Returns plane adjusted for face offset (for origin brushes, primarily used in the opaque code)
void getAdjustedPlaneFromFaceNumber(unsigned int faceNumber, dplane_t* plane)
{
    dface_t*        face = &g_dFaces[faceNumber];
    const vec_t*    face_offset = g_face_offset[faceNumber];

    plane->type = (planetypes)0;
    
    if (face->side)
    {
        vec_t dist;

        VectorCopy(backplanes[face->planenum].normal, plane->normal);
        dist = DotProduct(plane->normal, face_offset);
        plane->dist = backplanes[face->planenum].dist + dist;
    }
    else
    {
        vec_t dist;

        VectorCopy(g_dplanes[face->planenum].normal, plane->normal);
        dist = DotProduct(plane->normal, face_offset);
        plane->dist = g_dplanes[face->planenum].dist + dist;
    }
}

// Will modify the plane with the new dist
void            TranslatePlane(dplane_t* plane, const vec_t* delta)
{
#ifdef HLRAD_FASTMATH
	plane->dist += DotProduct(plane->normal,delta);
#else
    vec3_t          proj;
    vec_t           magnitude;

    ProjectionPoint(delta, plane->normal, proj);
    magnitude = VectorLength(proj);

    if (DotProduct(plane->normal, delta) > 0)              //if zero, magnitude will be zero.
    {
        plane->dist += magnitude;
    }
    else
    {
        plane->dist -= magnitude;
    }
#endif
}

// HuntForWorld will never return CONTENTS_SKY or CONTENTS_SOLID leafs
dleaf_t*        HuntForWorld(vec_t* point, const vec_t* plane_offset, const dplane_t* plane, int hunt_size, vec_t hunt_scale, vec_t hunt_offset)
{
    dleaf_t*        leaf;
    int             x, y, z;
    int             a;

    vec3_t          current_point;
    vec3_t          original_point;

    vec3_t          best_point;
    dleaf_t*        best_leaf = NULL;
    vec_t           best_dist = 100000000.0;

    vec3_t          scales;

    dplane_t        new_plane = *plane;

    if (g_nohunt)
        return NULL;

    if (hunt_scale < 0.1)
    {
        hunt_scale = 0.1;
    }

    scales[0] = 0.0;
    scales[1] = -hunt_scale;
    scales[2] = hunt_scale;

    VectorCopy(point, best_point);
    VectorCopy(point, original_point);

    TranslatePlane(&new_plane, plane_offset);

    if (!hunt_size)
    {
        hunt_size = DEFAULT_HUNT_SIZE;
    }

    for (a = 1; a < hunt_size; a++)
    {
        for (x = 0; x < 3; x++)
        {
            current_point[0] = original_point[0] + (scales[x % 3] * a);
            for (y = 0; y < 3; y++)
            {
                current_point[1] = original_point[1] + (scales[y % 3] * a);
                for (z = 0; z < 3; z++)
                {
                    vec3_t          delta;
                    vec_t           dist;

                    current_point[2] = original_point[2] + (scales[z % 3] * a);
                    SnapToPlane(&new_plane, current_point, hunt_offset);
                    VectorSubtract(current_point, original_point, delta);
                    dist = VectorLength(delta);

                    if (dist < best_dist)
                    {
                        // nightfire
#if 1
                        unsigned int tn = (unsigned int)GetTnode(current_point);
                        if ((tn & 0xFF) != CONTENTS_SOLID)
                        {
                            if (x || y || z)
                            {
                                //dist = best_dist;
                                best_leaf = leaf;
                                VectorCopy(current_point, best_point);
                                continue;
                            }
                            else
                            {
                                VectorCopy(current_point, point);
                                return leaf;
                            }
                        }
#else
                        if ((leaf = PointInLeaf(current_point)) != g_dleafs)
                        {
                            if ((leaf->contents != CONTENTS_SKY) && (leaf->contents != CONTENTS_SOLID))
                            {
                                if (x || y || z)
                                {
                                    //dist = best_dist;
                                    best_leaf = leaf;
                                    VectorCopy(current_point, best_point);
                                    continue;
                                }
                                else
                                {
                                    VectorCopy(current_point, point);
                                    return leaf;
                                }
                            }
                        }
#endif
                    }
                }
            }
        }
        if (best_leaf)
        {
            break;
        }
    }

    VectorCopy(best_point, point);
    return best_leaf;
}
