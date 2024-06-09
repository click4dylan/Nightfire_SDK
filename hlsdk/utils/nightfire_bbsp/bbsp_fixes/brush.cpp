#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "Leaf.h"
#include "Nodes.h"
#include "cmdlib.h"
#include "log.h"
#include "brush.h"
#include "textures.h"
#include "planes.h"

void CalcBrushBounds(vec3_t& maxs_dest, vec3_t& mins_dest, const brush_t* brush)
{
    if ((brush->brushflags & CONTENTS_ORIGIN) == 0)
    {
        AddPointToBounds(brush->bounds.m_Mins, mins_dest, maxs_dest);
        AddPointToBounds(brush->bounds.m_Maxs, mins_dest, maxs_dest);
    }
}

void FilterBrushesIntoTree(node_t* node, entity_t* ent, int brushflags) 
{
    g_NumNonInvertedFaces = 0;

    for (unsigned int brushIndex = 0; brushIndex < ent->numbrushes; ++brushIndex)
    {
        brush_t* brush = ent->brushes[brushIndex];

        if ((brush->brushflags & brushflags) != 0)
        {
            for (unsigned int sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
            {
                side_t* side = brush->brushsides[sideIndex];
                face_t* original_face = side->original_face;

                if ((original_face->flags & CONTENTS_BSP) == 0) 
                {
                    face_t* newFace = new face_t(*original_face);
#ifdef SUBDIVIDE
                    if (!g_nosubdiv)
                        FilterFacesIntoTree(newFace, node, false, false);
                    else
                        FilterFaceIntoTree_r(0, node, side, newFace, false, false);
#else
                    FilterFaceIntoTree_r(0, node, side, newFace, false, false);
#endif
                }
            }
        }
    }
}

brush_s::brush_s()
{
    ++g_numBrushes;
}

brush_s::~brush_s()
{
    for (unsigned i = 0; i < numsides; ++i)
    {
        if (brushsides[i])
            delete brushsides[i];
    }
    --g_numBrushes;
}

side_t* brush_s::CreateNewBrushSide()
{
    // Check if there's enough capacity to add a new side
    if (numsides == sidecapacity)
    {
        // Increase side capacity
        int newCapacity = sidecapacity + 8;
        side_t** newBrushSides = (side_t**)calloc(1, sizeof(side_t**) * newCapacity);

        // Copy existing sides to the new array and free the old one
        if (brushsides)
        {
            memcpy(newBrushSides, brushsides, sizeof(side_t**) * numsides);
            free(brushsides);
        }
        
#ifdef _DEBUG
        if (!newBrushSides)
            DebugBreak();
#endif

        // Update brush sides with the new array and capacity
        brushsides = newBrushSides;
        sidecapacity = newCapacity;
    }

    // Allocate memory for the new side
    side_t* newSide = new side_t;

    // Initialize the new side and add it to the brush
    newSide->parent_brush = this;
    brushsides[numsides++] = newSide;

    return newSide;
}

void brush_t::FreeSides()
{
    for (unsigned int i = 0; i < numsides; ++i)
    {
        if (brushsides[i])
            delete brushsides[i];
    }
    *brushsides = nullptr;
    numsides = 0;
}


side_s::side_s()
{
    ++g_numBrushSides;
}

side_s::~side_s()
{
#ifdef BBSP_USE_CPP
    if (inverted_face_fragments)
        delete inverted_face_fragments;
    if (face_fragments)
        delete face_fragments;
    if (original_face)
    {
        original_face->next = nullptr;
        delete original_face; //FIXME: don't loop through each face, fixed
    }
    if (final_face)
    {
        final_face->next = nullptr;
        delete final_face; //FIXME: don't loop through each face, fixed
    }
#else
    FreeFaceList(inverted_face_fragments);
    FreeFaceList(face_fragments);
    FreeFace(original_face);
    FreeFace(final_face);
#endif

    --g_numBrushSides;
}

void WriteDrawBrushSide(side_t* side)
{
    if (!side->built_draw_indices_for_side)
    {
        face_t* final_face = side->final_face;
        side->draw_brush_side_index = g_numDBrushSides;
        side->built_draw_indices_for_side = true;
        WriteFace_AkaBuildDrawIndicesForFace(final_face);
        side->original_face->outputnumber = final_face->built_draw_indices_for_face; //?
        side->original_face->outputnumber = final_face->outputnumber;
        hlassume(g_numDBrushSides < MAX_MAP_BRUSHSIDES, assume_MAX_MAP_SIDES);
        dbrushside_t* out_brushside = &g_dbrushsides[g_numDBrushSides++];
        out_brushside->face = final_face->outputnumber;
        out_brushside->plane = final_face->planenum;
    }
}

void WriteDrawBrush(brush_t* brush)
{
    if (!brush->built_draw_brush)
    {
        // Check if the current number of brushes exceeds the maximum limit
        hlassume(g_numDBrushes < MAX_MAP_BRUSHES, assume_MAX_MAP_BRUSHES);

        // Set output number and mark the brush as processed
        brush->output_num = g_numDBrushes;
        brush->built_draw_brush = true;

        // Get the current draw brush from the global array
        dbrush_t* currentDBrush = &g_dbrushes[g_numDBrushes++];

        // Set the attributes of the draw brush
        currentDBrush->flags = brush->brushflags ^ (unsigned __int8)(brush->leaf_type ^ brush->brushflags);
        currentDBrush->firstbrushside = g_numDBrushSides;
        currentDBrush->numbrushsides = brush->numsides;

        // Process each brush side
        for (unsigned int i = 0; i < brush->numsides; ++i)
        {
            WriteDrawBrushSide(brush->brushsides[i]);
        }
    }
}

void MakeHullFaces(brush_t* b)
{
    face_t* f;
    face_t* f2;

//restart:
   b->bounds.reset();
   //NIGHTFIRE FIXME: face doesn't get marked to not be used!! 
   for (unsigned int i = 0; i < b->numsides; ++i)
   {
       side_t* s = b->brushsides[i];
       f = s->original_face;

       Winding* w = new Winding(gMappedPlanes[f->planenum]);

       for (unsigned int j = 0; j < b->numsides; ++j)
       {
           side_t* s2 = b->brushsides[j];
           f2 = s2->original_face;
      
           if (f == f2)
               continue;

            const plane_t* p = &gMappedPlanes[f2->planenum ^ 1];
            if (!w->Chop(p->normal, p->dist))   // Nothing left to chop (getArea will return 0 for us in this case for below)
                break;
       }

       if (w->getArea() < 0.1)
           w->ClearNumPoints();

       f->winding = w;
       for (unsigned int k = 0; k < w->m_NumPoints; k++)
       {
           b->bounds.add(w->m_Points[k]);
       }
   }

    for (unsigned int i = 0; i < 3; i++)
    {
        if (b->bounds.m_Mins[i] < -BOGUS_RANGE / 2 || b->bounds.m_Maxs[i] > BOGUS_RANGE / 2)
        {
            Fatal(assume_BRUSH_OUTSIDE_WORLD, "Entity %i, Brush %i: outside world(+/-%d): (%.0f,%.0f,%.0f)-(%.0f,%.0f,%.0f)",
                b->entitynum, b->brushnum,
                BOGUS_RANGE / 2,
                b->bounds.m_Mins[0], b->bounds.m_Mins[1], b->bounds.m_Mins[2],
                b->bounds.m_Maxs[0], b->bounds.m_Maxs[1], b->bounds.m_Maxs[2]);
        }
    }
}

bool FindDuplicatePlane(const plane_s* plane, const brush_t* brush) 
{
    for (unsigned int i = 0; i < brush->numsides; ++i) 
    {
        if (PlaneEqual(&gMappedPlanes[brush->brushsides[i]->plane_num], plane, ON_EPSILON, NORMAL_EPSILON))
            return true;
    }
    return false;
}

void AddEdgeBevel(brush_t* brush, vec_t* normal, vec_t* points) 
{
    plane_s currentPlane;
    int direction, axis;
    vec_t dist;

    for (axis = 0; axis < 3; ++axis) 
    {
        for (direction = -1; direction <= 1; direction += 2) 
        {
            // Initialize the current plane
            memset(&currentPlane, 0, sizeof(currentPlane));
            currentPlane.normal[axis] = direction;

            // Calculate the normal vector for the plane
            if (VectorNormalize(currentPlane.normal) > 0.0) 
            {
                dist = DotProduct(currentPlane.normal, points);

                unsigned sideIndex;

                for (sideIndex = 0; sideIndex < brush->numsides; ++sideIndex) 
                {
                    face_t* originalFace = brush->brushsides[sideIndex]->original_face;
                    if (originalFace && !(originalFace->flags & CONTENTS_BSP)) 
                    {
                        if (originalFace->winding)
                        {
                            int side = originalFace->winding->WindingOnPlaneSide(currentPlane.normal, dist + 0.1);
                            if (side != SIDE_BACK && side != SIDE_ON)
                                break;
                        }
                    }
                }

                // If the plane is not part of the brush, add it
                if (sideIndex == brush->numsides && !FindDuplicatePlane(&currentPlane, brush)) 
                   AddHullPlane(brush, currentPlane.normal, dist);
            }
        }
    }
}

void SortBrushSidesByCanonicalOrder(brush_t* brush)
{
    const unsigned int numSides = brush->numsides;
    bool swapped = true; // Set to true initially to enter the loop at least once

    while (swapped) 
    {
        swapped = false; // Reset swapped flag at the beginning of each iteration
        for (unsigned int i = 1; i < numSides; ++i) 
        {
            side_t* currentSide = brush->brushsides[i];
            side_t* previousSide = brush->brushsides[i - 1];
            if (gMappedPlanes[previousSide->plane_num].closest_axis > gMappedPlanes[currentSide->plane_num].closest_axis) 
            {
                side_t* temp = brush->brushsides[i];
                brush->brushsides[i] = brush->brushsides[i - 1];
                brush->brushsides[i - 1] = temp;
                swapped = true; // Set to true if a swap occurs
            }
        }
    }
}

void AddBrushBevels(brush_t* buildBrush) 
{
    int axis, dir;
    unsigned int i, j, k, order;
    side_t sidetemp;
    side_t** s;
    Winding* w;
    vec3_t normal;
    vec_t dist;
    vec3_t vec;

    //
    // add the axial planes
    //
    order = 0;
    for (axis = 0; axis < 3; axis++) 
    {
        for (dir = -1; dir <= 1; dir += 2, order++) 
        {
            // see if the plane is allready present
            for (i = 0, s = buildBrush->brushsides; i < buildBrush->numsides; i++, s++)
            {
                // nightfire logic, todo fixme! floating point precision loss here
#ifndef NIGHTFIRE_NORMAL_FIX
                if (gMappedPlanes[(*s)->plane_num].normal[axis] == (double)dir)
                    break;
#else
                // quake 3 logic
                if ((dir > 0 && gMappedPlanes[(*s)->plane_num].normal[axis] == 1.0) || (dir < 0 && gMappedPlanes[(*s)->plane_num].normal[axis] == -1.0)) 
                    break;
#endif
            }

#if 1
            //nightfire logic
            if (i == buildBrush->numsides)
            {
                VectorClear(normal);
                normal[axis] = dir;

                if (dir == 1)
                    dist = buildBrush->bounds.m_Maxs[axis];
                else
                    dist = -buildBrush->bounds.m_Mins[axis];

                AddHullPlane(buildBrush, normal, dist);
            }
#else
            //q3 logic
            if (i == buildBrush->numsides) 
            {
                // add a new side
                memset(s, 0, sizeof(*s));
                buildBrush->numsides++;
                VectorClear(normal);
                normal[axis] = dir;

                if (dir == 1) 
                   dist = buildBrush->bounds.m_Maxs[axis];
                else
                   dist = -buildBrush->bounds.m_Mins[axis];

                s->planenum = FindFloatPlane(normal, dist, 0, NULL);
                s->contentFlags = buildBrush->sides[0].contentFlags;
                s->bevel = qtrue;
                c_boxbevels++;
            }

            // if the plane is not in it canonical order, swap it
            if (i != order) {
                sidetemp = buildBrush->sides[order];
                buildBrush->sides[order] = buildBrush->sides[i];
                buildBrush->sides[i] = sidetemp;
            }
#endif
        }
    }

    //
    // add the edge bevels
    //
    if (buildBrush->numsides == 6)
        return;     // pure axial

    // test the non-axial plane edges
    //FIXME: todo: q3 starts from 6!
    for (i = 0; i < buildBrush->numsides; i++) 
    {
        face_t* face = buildBrush->brushsides[i]->original_face;
        if (!face)
            continue;

        if ((face->flags & CONTENTS_BSP) != 0)
            continue;

        w = face->winding;
        if (!w)
            continue;

        // nightfire check
        if (!w->HasPoints())
            continue;

        for (j = 0; j < w->m_NumPoints; j++) 
        {
            k = (j + 1) % w->m_NumPoints;
            VectorSubtract(w->m_Points[j], w->m_Points[k], vec);

            //todo nightfire check: fixme
#ifndef NIGHTFIRE_NORMAL_FIX
            if (VectorNormalize(vec) <= 0.5f)
                continue;
#else
            if (VectorNormalize(vec) < 0.5f)
                continue;
#endif
         
#ifndef NIGHTFIRE_NORMAL_FIX
            if (SnapNormal(vec))
                continue;

            AddEdgeBevel(buildBrush, vec, w->m_Points[j]);

#else
            SnapNormal(vec);


            for (k = 0; k < 3; k++) {
                if (vec[k] == -1.0f || vec[k] == 1.0f || (vec[k] == 0.0f && vec[(k + 1) % 3] == 0.0f)) {
                    break;  // axial
                }
            }
            if (k != 3) {
                continue;   // only test non-axial edges
            }

            /* debug code */
            //%	Sys_Printf( "-------------\n" );

            // try the six possible slanted axials from this edge
            for (axis = 0; axis < 3; axis++) {
                for (dir = -1; dir <= 1; dir += 2) {
                    // construct a plane
                    VectorClear(vec2);
                    vec2[axis] = dir;
                    CrossProduct(vec, vec2, normal);
                    if (VectorNormalize(normal, normal) < 0.5f) {
                        continue;
                    }
                    dist = DotProduct(w->p[j], normal);

                    // if all the points on all the sides are
                    // behind this plane, it is a proper edge bevel
                    for (k = 0; k < buildBrush->numsides; k++) {

                        // if this plane has allready been used, skip it
                        if (PlaneEqual(&mapplanes[buildBrush->sides[k].planenum], normal, dist)) {
                            break;
                        }

                        w2 = buildBrush->sides[k].winding;
                        if (!w2) {
                            continue;
                        }
                        minBack = 0.0f;
                        for (l = 0; l < w2->numpoints; l++) {
                            d = DotProduct(w2->p[l], normal) - dist;
                            if (d > 0.1f) {
                                break;  // point in front
                            }
                            if (d < minBack) {
                                minBack = d;
                            }
                        }
                        // if some point was at the front
                        if (l != w2->numpoints) {
                            break;
                        }

                        // if no points at the back then the winding is on the bevel plane
                        if (minBack > -0.1f) {
                            //%	Sys_Printf( "On bevel plane\n" );
                            break;
                        }
                    }

                    if (k != buildBrush->numsides) {
                        continue;   // wasn't part of the outer hull
                    }

                    /* debug code */
                    //%	Sys_Printf( "n = %f %f %f\n", normal[ 0 ], normal[ 1 ], normal[ 2 ] );

                    // add this plane
                    if (buildBrush->numsides == MAX_BUILD_SIDES) {
                        xml_Select("MAX_BUILD_SIDES", buildBrush->entityNum, buildBrush->brushNum, qtrue);
                    }
                    s2 = &buildBrush->sides[buildBrush->numsides];
                    buildBrush->numsides++;
                    memset(s2, 0, sizeof(*s2));

                    s2->planenum = FindFloatPlane(normal, dist, 1, &w->p[j]);
                    s2->contentFlags = buildBrush->sides[0].contentFlags;
                    s2->bevel = qtrue;
                    c_edgebevels++;
                }
            }
#endif
        }
    }

    SortBrushSidesByCanonicalOrder(buildBrush);
}

// replaced with C++
#if 0
side_s* CreateNewBrushSide(brush_t* brush) 
{
    // Check if there's enough capacity to add a new side
    if (brush->numsides == brush->sidecapacity) 
    {
        // Increase side capacity
        int newCapacity = brush->sidecapacity + 8;
        side_t** newBrushSides = (side_t**)Alloc(4 * newCapacity);

        // Copy existing sides to the new array and free the old one
        if (brush->brushsides)
        {
            memcpy(newBrushSides, brush->brushsides, 4 * brush->numsides);
            Free(brush->brushsides);
        }

        // Update brush sides with the new array and capacity
        brush->brushsides = newBrushSides;
        brush->sidecapacity = newCapacity;
    }

    // Allocate memory for the new side
    side_s* newSide = AllocBrushSide();

    // Initialize the new side and add it to the brush
    newSide->parent_brush = brush;
    brush->brushsides[brush->numsides++] = newSide;

    return newSide;
}
#endif