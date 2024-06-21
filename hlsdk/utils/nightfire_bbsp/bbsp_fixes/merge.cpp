#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "textures.h"
#include "log.h"
#include "planes.h"
#include "cmdlib.h"
#include "merge.h"

#ifdef MERGE

#define CONTINUOUS_EPSILON	ON_EPSILON

// =====================================================================================
//  TryMerge
//      If two polygons share a common edge and the edges that meet at the
//      common points are both inside the other polygons, merge them
//      Returns NULL if the faces couldn't be merged, or the new face.
//      The originals will NOT be freed.
// =====================================================================================
face_t* TryMerge(face_t* f1, face_t* f2)
{
    vec_t* p1;
    vec_t* p2;
    vec_t* p3;
    vec_t* p4;
    vec_t* back;
    face_t* newf;
    unsigned int             i;
    unsigned int             j;
    unsigned int             k;
    unsigned int             l;
    vec3_t          normal;
    vec3_t          delta;
    vec3_t          planenormal;
    vec_t           dot;
    plane_t* plane;
    bool            keep1;
    bool            keep2;

    if (!f1->winding->m_NumPoints || !f2->winding->m_NumPoints)
    {
        return NULL;
    }
    if (*f1->texinfo != *f2->texinfo)
    {
        return NULL;
    }
    if (f1->flags != f2->flags)
    {
        return NULL;
    }

    if (f1->winding->m_NumPoints < 3 || f2->winding->m_NumPoints < 3)
    {
        return NULL;
    }

    if (f1->planenum != f2->planenum)
    {
        return NULL;
    }

    //
    // find a common edge
    //      
    p1 = p2 = NULL;                                        // shut up the compiler
    j = 0;

    for (i = 0; i < f1->winding->m_NumPoints; i++)
    {
        p1 = f1->winding->m_Points[i];
        p2 = f1->winding->m_Points[(i + 1) % f1->winding->m_NumPoints];
        for (j = 0; j < f2->winding->m_NumPoints; j++)
        {
            p3 = f2->winding->m_Points[j];
            p4 = f2->winding->m_Points[(j + 1) % f2->winding->m_NumPoints];
            for (k = 0; k < 3; k++)
            {
#ifdef HLBSP_TryMerge_PRECISION_FIX
                if (fabs(p1[k] - p4[k]) > ON_EPSILON)
                {
                    break;
                }
                if (fabs(p2[k] - p3[k]) > ON_EPSILON)
                {
                    break;
                }
#else
                if (fabs(p1[k] - p4[k]) > EQUAL_EPSILON)
                {
                    break;
                }
                if (fabs(p2[k] - p3[k]) > EQUAL_EPSILON)
                {
                    break;
                }
#endif
            }
            if (k == 3)
            {
                break;
            }
        }
        if (j < f2->winding->m_NumPoints)
        {
            break;
        }
    }

    if (i == f1->winding->m_NumPoints)
    {
        return NULL;                                       // no matching edges
    }

    //
    // check slope of connected lines
    // if the slopes are colinear, the point can be removed
    //
    plane = &gMappedPlanes[f1->planenum];
    VectorCopy(plane->normal, planenormal);

    back = f1->winding->m_Points[(i + f1->winding->m_NumPoints - 1) % f1->winding->m_NumPoints];
    VectorSubtract(p1, back, delta);
    CrossProduct(planenormal, delta, normal);
    VectorNormalize(normal);

    back = f2->winding->m_Points[(j + 2) % f2->winding->m_NumPoints];
    VectorSubtract(back, p1, delta);
    dot = DotProduct(delta, normal);
    if (dot > CONTINUOUS_EPSILON)
    {
        return NULL;                                       // not a convex polygon
    }
    keep1 = dot < -CONTINUOUS_EPSILON;

    back = f1->winding->m_Points[(i + 2) % f1->winding->m_NumPoints];
    VectorSubtract(back, p2, delta);
    CrossProduct(planenormal, delta, normal);
    VectorNormalize(normal);

    back = f2->winding->m_Points[(j + f2->winding->m_NumPoints - 1) % f2->winding->m_NumPoints];
    VectorSubtract(back, p2, delta);
    dot = DotProduct(delta, normal);
    if (dot > CONTINUOUS_EPSILON)
    {
        return NULL;                                       // not a convex polygon
    }
    keep2 = dot < -CONTINUOUS_EPSILON;

    //
    // build the new polygon
    //
    if (f1->winding->m_NumPoints + f2->winding->m_NumPoints > MAXEDGES)
    {
        Warning("TryMerge: too many edges!");

        return NULL;
    }

    newf = new face_t(*f1, new Winding);// NewFaceFromFace(f1);

    // copy first polygon
    for (k = (i + 1) % f1->winding->m_NumPoints; k != i; k = (k + 1) % f1->winding->m_NumPoints)
    {
        if (k == (i + 1) % f1->winding->m_NumPoints && !keep2)
        {
            continue;
        }

        newf->winding->addPoint(f1->winding->m_Points[k]);
    }

    // copy second polygon
    for (l = (j + 1) % f2->winding->m_NumPoints; l != j; l = (l + 1) % f2->winding->m_NumPoints)
    {
        if (l == (j + 1) % f2->winding->m_NumPoints && !keep1)
        {
            continue;
        }
        newf->winding->addPoint(f2->winding->m_Points[l]);
    }

    return newf;
}

void CheckColinear(face_t* f)
{
    unsigned int i, j;
    vec_t v1, v2;

    for (i = 0; i < f->winding->m_NumPoints; i++) 
    {
        // skip the point if the vector from the previous point is the same
        // as the vector to the next point
        j = (i - 1 < 0) ? f->winding->m_NumPoints - 1 : i - 1;
        vec3_t temp;
        VectorSubtract(f->winding->m_Points[i], f->winding->m_Points[j], temp);
        v1 = VectorNormalize(temp);

        j = (i + 1 == f->winding->m_NumPoints) ? 0 : i + 1;
        VectorSubtract(f->winding->m_Points[j], f->winding->m_Points[i], temp);
        v2 = VectorNormalize(temp);

        if (fabs(v1 - v2) <= EQUAL_EPSILON)
            Error("Colinear edge");
    }
}

face_t* MergeFaceToList(face_t* face, face_t* list)
{
    face_t* newf;
    face_t* f;

    for (f = list; f; f = f->next)
    {
        //CheckColinear(f);            
        newf = TryMerge(face, f);
        if (!newf)
        {
            continue;
        }
        // prevent brushside from getting marked for deletion
        f->brushside->merged_side_into_another_brush = true;
#ifdef _DEBUG
        newf->merged = true;
#endif
        ++g_NumPlaneFacesMerged;
        FreeFace(face);
        f->winding->ClearNumPoints();                                 // merged out
        return MergeFaceToList(newf, list);
    }

    // didn't merge, so add at start
    face->next = list;
    return face;
}

// =====================================================================================
//  FreeMergeListScraps
// =====================================================================================
face_t* FreeMergeListScraps(face_t* merged)
{
    face_t* head;
    face_t* next;

    head = NULL;
    for (; merged; merged = next)
    {
        next = merged->next;
        if (merged->winding->m_NumPoints == 0)
        {
            FreeFace(merged);
        }
        else
        {
            merged->next = head;
            head = merged;
        }
    }

    return head;
}

void MergePlaneFaces(face_t** list)
{
    if (g_nomerge)
        return;

    Verbose("MergePlaneFaces...\n");

    double startTime = I_FloatTime();

    face_t* f1;
    face_t* next;
    face_t* merged;

    merged = NULL;

    for (f1 = *list; f1; f1 = next)
    {
        next = f1->next;
        merged = MergeFaceToList(f1, merged);
    }

    // chain all of the non-empty faces to the plane
    *list = FreeMergeListScraps(merged);

    Verbose("MergePlaneFaces : ");
    LogTimeElapsed(I_FloatTime() - startTime);
}

#endif