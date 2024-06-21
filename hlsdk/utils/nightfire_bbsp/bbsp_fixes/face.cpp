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

DWORD a = 0x41D3C0;

#ifdef BBSP_USE_CPP
face_s::~face_s()
{
    //FreeFaceList
    face_s* n = next;
    while (n)
    {
        face_s* next_in_chain = n->next;
        free(n);
        --g_numFaces;
        n = next_in_chain;
    }

    --g_numFaces;
}
#endif

// Default constructor
face_s::face_s(unsigned int planenum)
    : planenum(planenum) 
{
    ++g_numFaces;
}

face_s::face_s(const face_s& src)
    : planenum(src.planenum), flags(src.flags), texinfo(src.texinfo), brush(src.brush), brushside(src.brushside)
{
    ++g_numFaces;
    memcpy(vecs, src.vecs, sizeof(vecs));
    if (src.winding)
        winding = new Winding(*src.winding);
    else
        winding = nullptr;
#if defined(_DEBUG) && defined(SUBDIVIDE)
    subdivided = src.subdivided;
#endif
}

face_s::face_s(const face_s& src, Winding* src_winding)
    : planenum(src.planenum), flags(src.flags), texinfo(src.texinfo), brush(src.brush), brushside(src.brushside)
{
    ++g_numFaces;
    memcpy(vecs, src.vecs, sizeof(vecs));
    winding = src_winding;
#if defined(_DEBUG) && defined(SUBDIVIDE)
    subdivided = src.subdivided;
#endif
}

bool SplitFaceByNodePlane(face_t* face, unsigned int planenum, face_t** front, face_t** back)
{
#if 1
    *front = nullptr;
    *back = nullptr;
    if (!face->winding->HasPoints())
        return false;
    if (planenum == face->planenum)
    {
        *front = face;
        return false;
    }
    if (planenum == (face->planenum ^ 1))
    {
        *back = face;
        return false;
    }
    const plane_t& plane = gMappedPlanes[planenum];
    Winding* new_front = nullptr;
    Winding* new_back = nullptr;
    if (face->winding->Divide(plane, &new_front, &new_back))
    {
        if (new_front)
            *front = new face_t(*face, new_front);
        if (new_back)
            *back = new face_t(*face, new_back);
        return true;
    }
    else
    {
        //FIXME: TODO: wtf is this?
        if (new_front)
            *front = face;
        if (new_back)
            *back = face;
        return false;
    }
#else
    DWORD adr = 0x41D3C0;
    DWORD f = (DWORD)face;
    int p = planenum;
    face_t** fr = front;
    face_t** b = back;
    bool result;
    __asm
    {
        mov eax, f
        mov edx, p
        push b
        push fr
        call a
        add esp, 8
        mov result, al
    }
    return result;
#endif
}

//gbx
bool SplitFace(face_t* face, unsigned int plane, face_t** front, face_t** back)
{
    *front = nullptr;
    *back = nullptr;
    if (face->planenum != plane && face->planenum != (plane ^ 1))
    {
        if (face->winding->HasPoints())
        {
            Winding* new_front = nullptr;
            Winding* new_back = nullptr;
            auto pPlane = gMappedPlanes[plane];

#if 0
            for (int i = 0; i < originalFace->winding->m_NumPoints; ++i)
            {
                for (int axis = 0; axis < 3; ++axis)
                {
                    auto point = originalFace->winding->m_Points[i][axis];
                    if (fabs(point) < 1024 && fabs(point) != 0)
                    {
                        char tmp[256];
                        sprintf_s(tmp, "Found point at %f %f %f\n", originalFace->winding->m_Points[i][0], originalFace->winding->m_Points[i][1], originalFace->winding->m_Points[i][2]);
                        OutputDebugStringA(tmp);
                    }
                }
            }
#endif

            face->winding->Clip(gMappedPlanes[plane].normal, gMappedPlanes[plane].dist, &new_front, &new_back);
            if (new_front)
                *front = new face_t(*face, new_front);
            if (new_back)
                *back = new face_t(*face, new_back);
            return true;
        }
    }
    return false;
}

//HL1
void SplitFace(face_t* in, const vec3_t normal, const vec_t dist, face_t** front, face_t** back)
{
    *front = nullptr;
    *back = nullptr;
    //if (originalFace->planenum != plane && originalFace->planenum != plane ^ 1)
    {
        if (in->winding->HasPoints())
        {
            //unsigned int plane_test = FindIntPlane(normal, dist);
            //if (in->planenum == plane_test || in->planenum == (plane_test ^ 1))
            //    return;

            Winding* new_front = nullptr;
            Winding* new_back = nullptr;
            in->winding->Divide(normal, dist, &new_front, &new_back);
            if (new_front)
                *front = new face_t(*in, new_front);
            if (new_back)
                *back = new face_t(*in, new_back);
        }
        else
        {
            Error("Invalid winding");
        }
    }

    // free the original originalFace now that is is represented by the fragments
    if (*front && *back)
    {
        memset(in, 0, sizeof(face_t));
        FreeFace(in);
    }
}

// This function splits original_face based on a given plane number.
// It takes in an array of original_face, the plane number of a node, depth of recursion,
// pointers to pointers for the resulting front and back original_face.
void SplitFaces(
    face_t* original_face,            // Array of original_face
    unsigned int node_planenum,        // Plane number of the node
    int depth,                // Depth of recursion
    face_t** dest_front_face, // Pointer to pointer for front original_face
    face_t** dest_back_face   // Pointer to pointer for back original_face
) {
    face_t* face;       // Current list being processed
    unsigned int planenum;       // Planenum of the current list
    face_t* next;       // Next list in the array
    int back;           // Counter for back original_face
    int front;          // Counter for front original_face
    int source;         // Counter for total original_face processed
    face_t* front_face{}; // Temporary pointer for front list
    face_t* back_face{};  // Temporary pointer for back list

    // Initializing counters and pointers
    face = original_face;
    *dest_front_face = nullptr;
    *dest_back_face = nullptr;
    source = 0;
    front = 0;
    back = 0;


    // Processing each list in the array
    if (original_face) 
    {
        do {
            planenum = face->planenum; // Getting the plane number of the list
            next = face->next;         // Getting the next list in the array
            ++source;                  // Incrementing total list counter

            // If the list is coplanar or back-facing, free the list
            // fixme: q3 doesn't free opposite planes
            if (planenum == node_planenum  || planenum == (node_planenum ^ 1))
            {
                FreeFace(face);
            }
            else {
                // Otherwise, split the list and update front/back counters
                SplitFace(face, node_planenum, &front_face, &back_face);
                FreeFace(face); // Freeing the original list

                if (front_face) 
                {
                    ++front; // Incrementing front list counter
                    // Adding the front list to the front list list
                    front_face->next = *dest_front_face;
                    *dest_front_face = front_face;
                }

                if (back_face) 
                {
                    ++back; // Incrementing back list counter
                    // Adding the back list to the back list list
                    back_face->next = *dest_back_face;
                    *dest_back_face = back_face;
                }
            }

            face = next; // Moving to the next list
        } while (next);
    }

    // Printing debug information
    Developer(DEVELOPER_LEVEL_SPAM, "SplitFaces (#%-5i) (depth %3i) (source %5i) (front %5i) (back %5i)\n", g_TimesCalledSplitFaces, depth, source, front, back);

    // Returning the total times this function has been called
    ++g_TimesCalledSplitFaces;
}

void StripOutsideFaceFragments(node_t* node, entity_t* ent, unsigned int brushflags)
{
    // dylan added for debug clarity..
    Verbose("%s...\n", __FUNCTION__);

    unsigned int keptFragments = 0;
    unsigned int discardedFragments = 0;

    for (unsigned int i = 0; i < ent->numbrushes; ++i)
    {
        brush_t* brush = ent->brushes[i];

        if ((brush->brushflags & brushflags) != 0)
        {
            for (unsigned int j = 0; j < brush->numsides; ++j)
            {
                side_t* brushside = brush->brushsides[j];
                face_t* prevfragment = nullptr;
                face_t* nextfragment;

                // Loop through all original_face of the side
                for (face_t* fragment = brushside->face_fragments; fragment; fragment = nextfragment)
                {
                    nextfragment = fragment->next;

                    if (fragment->leaf_node->contents == CONTENTS_SOLID)
                    {
                        // Free the face if it's associated with a solid leaf node
                        FreeFace(fragment);
                        ++discardedFragments;

                        if (prevfragment)
                            prevfragment->next = nextfragment;
                        else
                            brushside->face_fragments = nextfragment;
                    }
                    else
                    {
                        // Keep the face if it's not associated with a solid leaf node
                        prevfragment = fragment;
                        ++keptFragments;
                    }
                }
            }
        }
    }

    Verbose("%5i face fragments kept\n", keptFragments);
    Verbose("%5i face fragments discarded\n", discardedFragments);
}

void MarkEmptyBrushFaces(entity_t* entity, unsigned int brushflags)
{
    // dylan added for debug clarity..
    Verbose("%s...\n", __FUNCTION__);

    unsigned int totalFaces = 0;
    unsigned int removedFaces = 0;

    for (unsigned int brushIndex = 0; brushIndex < entity->numbrushes; ++brushIndex)
    {
        brush_t* brush = entity->brushes[brushIndex];
        if ((brush->brushflags & brushflags) != 0)
        {
            totalFaces += brush->numsides;

            for (unsigned int sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
            {
                side_t* side = brush->brushsides[sideIndex];
                
                for (face_t* face = side->original_face; face; face = face->next)
                {
                    if (!side->face_fragments && (face->flags & (CONTENTS_DETAIL | CONTENTS_BSP | SURFACEFLAG_NODRAW | CONTENTS_SOLID)) == 0)
                    {
#ifdef MERGE
                        if (side->merged_side_into_another_brush)
                            continue;
#endif
                        ++removedFaces;
                        face->flags |= SURFACEFLAG_NODRAW;
                        if ((brushflags & CONTENTS_BSP) != 0)
                            face->flags |= CONTENTS_DETAIL;
                    }
                }
            }
        }
    }

    const char* surfaceTypeStr = GetSurfaceTypeStr(brushflags);
    Verbose("%5i %s faces removed\n", removedFaces, surfaceTypeStr);
    Verbose("%5i %s faces remaining\n", totalFaces - removedFaces, surfaceTypeStr);
}

void FreeBrushFaces(entity_t* entity, unsigned int brushflags)
{
    for (unsigned int i = 0; i < entity->numbrushes; ++i)
    {
        brush_t* brush = entity->brushes[i];
        if ((brush->brushflags & brushflags) != 0)
        {
            for (unsigned int s = 0; s < brush->numsides; ++s)
            {
                side_t* side = brush->brushsides[s];
#ifdef BBSP_USE_CPP
                if (side->face_fragments)
                    delete side->face_fragments;
                if (side->inverted_face_fragments)
                    delete side->inverted_face_fragments;
#else
                FreeFaceList(side->face_fragments);
                FreeFaceList(side->inverted_face_fragments);
#endif
                side->face_fragments = nullptr;
                side->inverted_face_fragments = nullptr;
            }
        }
    }
}

bool DoFacesHaveOppositePlanes(const face_t* face1, const face_t* face2) 
{
    // Calculate the dot product
    double dot = DotProduct(gMappedPlanes[face1->planenum].normal, gMappedPlanes[face2->planenum].normal);

    // Check if the dot product is less than zero, meaning the normals are opposite
    return dot < 0.0;
}

// Function to project a polygon onto an axis and return min/max projection values
void ProjectPolygon(const Winding& polygon, const vec3_t& axis, vec_t& minProj, vec_t& maxProj)
{
    minProj = std::numeric_limits<double>::infinity();
    maxProj = -std::numeric_limits<double>::infinity();

    for (unsigned int i = 0; i < polygon.m_NumPoints; ++i)
    {
        vec3_t& vertex = polygon.m_Points[i];
        vec_t projection = DotProduct(vertex, axis);
        if (projection < minProj) minProj = projection;
        if (projection > maxProj) maxProj = projection;
    }
}

// Function to check if two faces overlap using Separating Axis Theorem (SAT)
bool DoFacesOverlap(const face_t& face1, const face_t& face2) 
{
    // Check overlap along face1's normals
    for (unsigned int i = 0; i < face1.winding->m_NumPoints; ++i) 
    {
        unsigned int nextIndex = (i + 1) % face1.winding->m_NumPoints;
        vec3_t edge;
        VectorSubtract(face1.winding->m_Points[nextIndex], face1.winding->m_Points[i], edge);
        vec3_t axis;
        CrossProduct(edge, gMappedPlanes[face1.planenum].normal, axis);

        vec_t minProj1, maxProj1, minProj2, maxProj2;
        ProjectPolygon(*face1.winding, axis, minProj1, maxProj1);
        ProjectPolygon(*face2.winding, axis, minProj2, maxProj2);

        if (maxProj1 < minProj2 || maxProj2 < minProj1) 
        {
            return false; // Separating axis found
        }
    }

    // Check overlap along face2's normals
    for (unsigned int i = 0; i < face2.winding->m_NumPoints; ++i)
    {
        unsigned int nextIndex = (i + 1) % face2.winding->m_NumPoints;
        vec3_t edge;
        VectorSubtract(face2.winding->m_Points[nextIndex], face2.winding->m_Points[i], edge);
        vec3_t axis;
        CrossProduct(edge, gMappedPlanes[face2.planenum].normal, axis);

        vec_t minProj1, maxProj1, minProj2, maxProj2;
        ProjectPolygon(*face1.winding, axis, minProj1, maxProj1);
        ProjectPolygon(*face2.winding, axis, minProj2, maxProj2);

        if (maxProj1 < minProj2 || maxProj2 < minProj1) 
        {
            return false; // Separating axis found
        }
    }

    return true; // No separating axis found, faces overlap
}

bool IsFaceEnclosed(const face_t& innerFace, const face_t& outerFace) 
{
    // First, check for overlap
    if (!DoFacesOverlap(innerFace, outerFace)) 
    {
        return false;
    }

    // Then, ensure all points of the inner face are within the outer face
    for (unsigned int i = 0; i < innerFace.winding->m_NumPoints; ++i)
    {
        const vec3_t& point = innerFace.winding->m_Points[i];

        vec_t innerProjection = DotProduct(point, gMappedPlanes[outerFace.planenum].normal);
        vec_t minOuterProj, maxOuterProj;
        ProjectPolygon(*outerFace.winding, gMappedPlanes[outerFace.planenum].normal, minOuterProj, maxOuterProj);

        if (innerProjection < minOuterProj || innerProjection > maxOuterProj) 
        {
            return false;
        }
    }

    return true;
}

const face_t* FindFaceThatEnclosesFace(face_t* f, const face_t* list)
{
    if (!f->winding->HasPoints())
        return nullptr;

    for (const face_t* b = list; b; b = b->next)
    {
        if (b == f)
            continue;
        if (!b->winding->HasPoints())
            continue;
        //if (b->planenum != f->planenum && b->planenum != (f->planenum ^ 1))
        //    continue;
        //if (b->planenum == f->planenum)
        //    continue;
        //if (!DoFacesHaveOppositePlanes(f, b))
        //    continue;
        if (f->flags != b->flags)
            continue;
        if ((f->brush->brushflags & (CONTENTS_BSP | CONTENTS_UNKNOWN2 | SURFACEFLAG_NODRAW | CONTENTS_PORTAL | CONTENTS_SKY | CONTENTS_SOLID | CONTENTS_EMPTY)) != 0)
            continue;
        if (b->planenum == f->planenum)
        {
            if (*f->texinfo != *b->texinfo)
                continue;
        }

        //if (f->brushside == b->brushside)
        //    continue;
        //if (f->brush == b->brush)
        //    continue;

        //if ((f->flags & (CONTENTS_BSP | CONTENTS_UNKNOWN2 | SURFACEFLAG_NODRAW | CONTENTS_PORTAL | CONTENTS_SKY | CONTENTS_SOLID | CONTENTS_EMPTY)) != 0)
        //    continue;

#if 0
        if (strstr(b->texinfo->name, "LAGZONE") && strstr(f->texinfo->name, "STORESIGN"))
        {
            char temp[128];
            sprintf(temp, "Smaller face normal: %f %f %f\n", gMappedPlanes[f->planenum].normal[0], gMappedPlanes[f->planenum].normal[1], gMappedPlanes[f->planenum].normal[2]);
            OutputDebugStringA(temp);
            OutputDebugStringA("Smaller face winding points:\n");
            for (unsigned int i = 0; i < f->winding->m_NumPoints; ++i)
            {
                const vec3_t& point = f->winding->m_Points[i];

                sprintf(temp, "%f %f %f\n", point[0], point[1], point[2]);
                OutputDebugStringA(temp);
            }

            sprintf(temp, "Larger face normal: %f %f %f\n", gMappedPlanes[b->planenum].normal[0], gMappedPlanes[b->planenum].normal[1], gMappedPlanes[b->planenum].normal[2]);
            OutputDebugStringA(temp);
            OutputDebugStringA("Larger face winding points:\n");
            for (unsigned int i = 0; i < b->winding->m_NumPoints; ++i)
            {
                const vec3_t& point = b->winding->m_Points[i];

                sprintf(temp, "%f %f %f\n", point[0], point[1], point[2]);
                OutputDebugStringA(temp);
            }

            if (IsFaceEnclosed(*f, *b))
                return b;
        }
#endif

        if (IsFaceEnclosed(*f, *b))
        {
            return b;
        }
    }

    return nullptr;
}

#ifdef STRIP
void StripEnclosedFaces(face_t** list)
{
    if (g_nostrip)
        return;

    Verbose("%s...\n", __FUNCTION__);

    double startTime = I_FloatTime();

    unsigned int num_stripped = 0;

    face_t* next;
    face_t* prev = nullptr;
    for (face_t* f = *list; f;)
    {
        next = f->next;

        if (FindFaceThatEnclosesFace(f, *list))
        {
            FreeFace(f);

            ++num_stripped;

            if (prev)
                prev->next = next; // bypass the removed face
            else
                *list = next; // update list head if the first face is removed
        }
        else
        {
            // move prev pointer forward only if f is not removed
            prev = f;
        }

        f = next; // move to the next face
    }

    Verbose("%s : ", __FUNCTION__);
    LogTimeElapsed(I_FloatTime() - startTime);

    Verbose("%5u stripped enclosed faces\n", num_stripped);
}
#endif

void FilterFacesIntoTree(face_t* list, node_t* node, bool face_windings_are_inverted, bool is_lighting_tree)
{
    // dylan added this for debug printing clarity
    Verbose("%s...\n", __FUNCTION__);

    if (node->planenum != PLANENUM_LEAF && !face_windings_are_inverted)
    {

        // merge as much as possible
#ifdef MERGE
        if (!g_nomerge)
        {
            g_NumPlaneFacesMerged = 0;
            MergePlaneFaces(&list);
            Verbose("%5u plane faces merged\n", g_NumPlaneFacesMerged);
        }
#endif

#ifdef STRIP
        // strip enclosed faces
        if (!g_nostrip)
            StripEnclosedFaces(&list);
#endif

        // subdivide large faces
#ifdef SUBDIVIDE
        if (!g_nosubdiv)
        {
            g_NumFacesSubdivided = 0;
            SubdivideFaces(&list);
            Verbose("%5u faces subdivided\n", g_NumFacesSubdivided);
        }
#endif
    }

    face_t* next;
    for (face_t* f = list; f; f = next)
    {
        next = f->next;
        FilterFaceIntoTree_r(0, node, f->brushside, f, face_windings_are_inverted, is_lighting_tree);
    }
}

void FilterFaceIntoTree_r(int depth, node_t* node, side_t* brushside, face_t* list, bool face_windings_are_inverted, bool is_in_lighting_stage)
{
    if (node->planenum == PLANENUM_LEAF)
    {
        if (face_windings_are_inverted)
        {
            list->next = brushside->inverted_face_fragments;
            brushside->inverted_face_fragments = list;
            list->leaf_node = node;

            unsigned int brush_contents = 0;

            if (is_in_lighting_stage)
                brush_contents = list->brush->brushflags & 0xFFFFFF00;

            unsigned int brush_leaf_type = list->brush->contents;

            if (!node->contents)
                node->contents = CONTENTS_EMPTY;

            unsigned int combined_type = (brush_contents | node->contents) & 0xFFFFFF00;

            // Update node leaf type if brush leaf type is greater
            if ((node->contents & 0xFF) < (brush_leaf_type & 0xFF))
                node->contents = brush_leaf_type;

            node->contents |= combined_type;
        }
        else
        {
            ++g_NumNonInvertedFaces;
            list->next = brushside->face_fragments;
            brushside->face_fragments = list;
            list->leaf_node = node;
        }
    }
    else
    {
        face_t* front_face;
        face_t* back_face;

        if (SplitFaceByNodePlane(list, node->planenum, &front_face, &back_face))
            FreeFace(list);
        if (front_face)
            FilterFaceIntoTree_r(++depth, node->children[0], brushside, front_face, face_windings_are_inverted, is_in_lighting_stage);
        if (back_face)
            FilterFaceIntoTree_r(++depth, node->children[1], brushside, back_face, face_windings_are_inverted, is_in_lighting_stage);
    }
}

node_t* ClearOutFaces(node_t* a1)
{
    g_numFalseNodes = 0;
    node_t* result = ClearOutFaces_r(0, a1);
    Verbose("%5i false nodes\n", g_numFalseNodes);
    return result;
}

node_t* ClearOutFaces_r(int depth, node_t* node)
{
    // will be set if any children touch it
    node->valid = false;

    if (!node->children[0])
    {
        // Leaf node reached
        if (node->contents != CONTENTS_SOLID)
        {
            // Propagate valid brushflags through portals
            for (portal_t* portal = node->portals, *pnext; portal; portal = pnext)
            {
                if (portal->onnode)
                    portal->onnode->valid = true;

                if (node == portal->nodes[0])
                    pnext = portal->next[0];
                else
                    pnext = portal->next[1];
            }
        }
        return node;
    }

    // Recursively propagate the valid brushflags
    node->children[0] = ClearOutFaces_r(depth + 1, node->children[0]);
    node->children[1] = ClearOutFaces_r(depth + 1, node->children[1]);

    if (node->valid)
        return node;

    // this node does not touch any interior leafs
    if (node->children[0]->contents == CONTENTS_SOLID && node->children[1]->contents == CONTENTS_SOLID)
    {
        // if both children are solid, just make this node solid
        node->contents = CONTENTS_SOLID;
        node->planenum = PLANENUM_LEAF;

        delete node->children[0];
        delete node->children[1];
        node->children[0] = nullptr;
        node->children[1] = nullptr;
    }
    else
    {
        // Increment the count of false nodes
        ++g_numFalseNodes;
    }

    return node;
}

void SetFacesLeafNode(node_t* node, entity_t* entity)
{
    unsigned int numBrushes = entity->numbrushes;

    for (unsigned int brushIndex = 0; brushIndex < numBrushes; ++brushIndex)
    {
        brush_t* brush = entity->brushes[brushIndex];
        unsigned int numSides = brush->numsides;

        for (unsigned int sideIndex = 0; sideIndex < numSides; ++sideIndex)
        {
            side_t* side = brush->brushsides[sideIndex];

            if (side->original_face)
                side->original_face->leaf_node = node;

            if (side->final_face)
                side->final_face->leaf_node = node;

            for (face_t* fragment = side->face_fragments; fragment; fragment = fragment->next)
            {
                fragment->leaf_node = node;
            }

            for (face_t* inverted_fragment = side->inverted_face_fragments; inverted_fragment; inverted_fragment = inverted_fragment->next)
            {
                inverted_fragment->leaf_node = node;
            }
        }
    }
}

face_t* CopyFaceList(entity_t* ent, unsigned int brushflags, unsigned int skipfaceflags)
{
    face_t* copiedFaces = nullptr; // Pointer to the list of copied original_face
    unsigned int numCopiedFaces = 0; // Counter for the number of copied original_face

    // Iterate over all brushes in the entity
    for (unsigned int brushIndex = 0; brushIndex < ent->numbrushes; ++brushIndex)
    {
        brush_t* brush = ent->brushes[brushIndex];

        // Check if the brush brushflags match the specified brushflags
        if ((brush->brushflags & brushflags) != 0)
        {
            // Iterate over all sides of the brush
            for (unsigned int sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
            {
                face_t* face = brush->brushsides[sideIndex]->original_face;

                // Check if the originalFace brushflags match the criteria
                if ((face->flags & skipfaceflags) == 0)
                {
                    // Copy the originalFace and add it to the list
                    face_t* dupFace = new face_t(*face);
                    dupFace->next = copiedFaces;
                    copiedFaces = dupFace;
                    ++numCopiedFaces;
                }
            }
        }
    }

    // Log the number of copied original_face
    const char* surfaceTypeStr = GetSurfaceTypeStr(brushflags);
    Verbose("%5i %s faces\n", numCopiedFaces, surfaceTypeStr);

    return copiedFaces;
}

face_t* CopyFaceList_Inverted(entity_t* ent, unsigned int brushflags, unsigned int skipfaceflags)
{
    // Copy the originalFace list with the given brushflags from the entity
    face_t* face = CopyFaceList(ent, brushflags, skipfaceflags);

    // Iterate over the copied original_face and invert their windings
    for (face_t* f = face; f != nullptr; f = f->next) 
    {
        // Invert the winding of the originalFace
        if (f->winding)
            f->winding->Invert();
 
        // Toggle the planenum of the originalFace
        f->planenum ^= 1u;
    }

    return face;
}

const char* GetSurfaceTypeStr(__int16 flags)
{
    if ((flags & (CONTENTS_DETAIL | CONTENTS_BSP)) == (CONTENTS_DETAIL | CONTENTS_BSP))
        return "world";
    if ((flags & CONTENTS_BSP) != 0)
        return "bsp";
    if ((flags & CONTENTS_DETAIL) != 0)
        return "detail";
    return "???";
}

void AddFaceToBounds(const face_t* const f, vec3_t mins, vec3_t maxs)
{
    // Get the winding from the originalFace
    Winding* winding = f->winding;

    // Check if the winding has points
    if (winding && winding->m_NumPoints > 0)
    {
        // Iterate through each point in the winding
        for (unsigned int i = 0; i < winding->m_NumPoints; ++i)
        {
            // Update bounds based on the current point
            AddPointToBounds(winding->m_Points[i], mins, maxs);
        }
    }
}

void WriteFace_AkaBuildDrawIndicesForFace(face_t* pFace)
{
    if (pFace->built_draw_indices_for_face)
        return;

    // Initialize face and check limits
    hlassume(g_numDFaces < MAX_MAP_FACES, assume_MAX_MAP_FACES);
    pFace->outputnumber = g_numDFaces;
    pFace->built_draw_indices_for_face = true;
    dface_t* dface = &g_dFaces[g_numDFaces++];

    // Handle special face brushflags
    if ((pFace->flags & (SURFACEFLAG_NODRAW | CONTENTS_PORTAL | CONTENTS_SKY | CONTENTS_SOLID | CONTENTS_EMPTY)) != 0)
        pFace->winding->ClearNumPoints();

    // Set face properties
    dface->plane_index = pFace->planenum;
    dface->texture_index = GetTextureIndex(pFace->texinfo->name);
    dface->material_index = GetMaterialIndex(pFace->brushside->td.material);
    dface->flags = pFace->flags;
    dface->tex_info_projection_index = GetProjectionIndex(pFace->texinfo->vecs[0], pFace->texinfo->vecs[1]);

    // Get lightmap projections
//FIXME: todo: i think brad.exe needs a rebuild for this to work properly
#ifdef HL2_LUXEL_METHOD
    dface->lightmap_info_projection_index = GetProjectionIndex(pFace->texinfo->lightmapVecsLuxelsPerWorldUnits[0], pFace->texinfo->lightmapVecsLuxelsPerWorldUnits[1]);
#else
    double s[4], t[4];
    GetLightmapProjections(s, pFace, t);
    s[3] = 0.0f;
    t[3] = 0.0f;

    dface->lightmap_info_projection_index = GetProjectionIndex(s, t);
#endif

    unsigned int numPoints = pFace->winding->m_NumPoints;
    dface->first_vertex_index = g_numDVerts;
    dface->num_vertices = numPoints;
    dface->first_indices_index = 0;
    dface->num_indicies = 0;

    // Add vertices
    if (numPoints > 0)
    {
        for (unsigned int i = 0; i < numPoints; ++i)
        {
            hlassume(g_numDVerts < MAX_MAP_VERTS, assume_MAX_MAP_VERTS);

#ifdef FIX_NORMALS_LUMP
            dnormal_t* normal = &g_dnormals[g_numDNormals++];
#else
            dnormal_t* normal = &g_dnormals[g_numDVerts];
#endif
            dvertex_t* vertex = &g_dverts[g_numDVerts++];
            plane_t* plane = &gMappedPlanes[pFace->planenum];

            VectorCopy(pFace->winding->m_Points[i], vertex->point);
            VectorCopy(plane->normal, normal->normal);
        }
    }

    // Add indices
    if (numPoints >= 3)
    {
        unsigned numFaces = numPoints - 2;

#ifdef OPTIMIZE_INDICES
        unsigned int* new_indices = new unsigned int[3 * numFaces];
        unsigned int num_indices = 0;
        for (unsigned i = 0; i < numFaces; ++i)
        {
            new_indices[num_indices++] = 0;
            new_indices[num_indices++] = i + 1;
            new_indices[num_indices++] = i + 2;
        }

        bool found_existing_indices = false;
        unsigned int* indices_end = &g_dindices[g_numDIndices];
        for (unsigned int* existing_indices = g_dindices; existing_indices + (3 * numFaces) <= indices_end; existing_indices += 3)
        {
            if (!memcmp(existing_indices, new_indices, sizeof(unsigned int) * (3 * numFaces)))
            {
                // use existing indices
                dface->first_indices_index = existing_indices - g_dindices;
                dface->num_indicies = 3 * numFaces;
                found_existing_indices = true;
                break;
            }
        }

        delete[] new_indices;

        if (!found_existing_indices)
        {
            // Set the face indices
            dface->first_indices_index = g_numDIndices;
            dface->num_indicies = 3 * numFaces;

            // Iterate and set indices
            for (unsigned i = 0; i < numFaces; ++i)
            {
                hlassume(g_numDIndices < MAX_MAP_INDICES, assume_MAX_MAP_INDICES);

                g_dindices[g_numDIndices++] = 0;
                g_dindices[g_numDIndices++] = i + 1;
                g_dindices[g_numDIndices++] = i + 2;
            }
        }
#else

        // Set the face indices
        dface->first_indices_index = g_numDIndices;
        dface->num_indicies = 3 * numFaces;

        // Iterate and set indices
        for (unsigned i = 0; i < numFaces; ++i) 
        {
            hlassume(g_numDIndices < MAX_MAP_INDICES, assume_MAX_MAP_INDICES);

            g_dindices[g_numDIndices++] = 0;
            g_dindices[g_numDIndices++] = i + 1;
            g_dindices[g_numDIndices++] = i + 2;
        }
#endif
    }
}

void MarkFace(node_t* leaf_node, face_t* face)
{
    if (!leaf_node->markfaces)
    {
        leaf_node->markfaces = new std::set<face_t*>;
        ++g_numNodesWithMarkFaces;
    }

    leaf_node->markfaces->insert(face);
}

void MarkBrush(node_t* leaf_node, brush_t* brush)
{
    if (leaf_node)
    {
        if (!leaf_node->markbrushes)
        {
            leaf_node->markbrushes = new std::set<brush_t*>;
            ++g_numNodesWithMarkBrushes;
        }

        leaf_node->markbrushes->insert(brush);
    }
}

void MarkFinalFaceFragments(node_t* node, entity_t* ent)
{
    for (unsigned int i = 0; i < ent->numbrushes; ++i)
    {
        brush_t* brush = ent->brushes[i];
        for (unsigned int j = 0; j < brush->numsides; ++j)
        {
            side_t* side = brush->brushsides[j];

            for (face_t* fragment = side->face_fragments; fragment; fragment = fragment->next)
            {
#ifdef SUBDIVIDE
                if (g_nosubdiv)
                {
                    MarkFace(fragment->leaf_node, side->final_face);
                    MarkBrush(fragment->leaf_node, brush);
                }
                else
                {
                    MarkFace(fragment->leaf_node, fragment);
                    MarkBrush(fragment->leaf_node, brush);
                }
#else
                MarkFace(fragment->leaf_node, side->final_face);
                MarkBrush(fragment->leaf_node, brush);
#endif
            }
            for (face_t* inverted_fragment = side->inverted_face_fragments; inverted_fragment; inverted_fragment = inverted_fragment->next)
            {
                MarkBrush(inverted_fragment->leaf_node, brush);
            }
        }
    }
}

void MarkFaceFragments(node_t* node, entity_t* ent)
{
    for (unsigned int i = 0; i < ent->numbrushes; ++i)
    {
        brush_t* brush = ent->brushes[i];
        for (unsigned int j = 0; j < brush->numsides; ++j)
        {
            side_t* side = brush->brushsides[j];
            for (face_t* fragment = side->face_fragments; fragment; fragment = fragment->next)
            {
                MarkFace(fragment->leaf_node, fragment);
                MarkBrush(fragment->leaf_node, brush);
            }
        }
    }
}

// for debugging
unsigned int GetNumFacesFromList(face_t* list)
{
    unsigned int num_faces = 0;
    for (face_t* f = list; f; f = f->next)
    {
        ++num_faces;
    }
    return num_faces;
}

// for debugging
void PrintNumFacesFromList(face_t* list, const char* function_name, const char* stage)
{
    unsigned int num_faces = GetNumFacesFromList(list);
    Verbose("%s: %5u faces %s\n", function_name, num_faces, stage);
}

// not used in nightfire
#ifdef SUBDIVIDE

void SubdivideFaces(face_t** list)
{
    if (g_nosubdiv)
        return;
    
    face_t** prevptr = list;
    face_t* f;
    while (1)
    {
        f = *prevptr;
        if (!f || !f->winding)
        {
            break;
        }

        if (SubdivideFace(f, prevptr))
        {
            ++g_NumFacesSubdivided;
        }
        f = *prevptr;
        prevptr = &f->next;
    }
}

bool SubdivideFace(face_t* f, face_t** prevptr)
{
    vec_t           mins, maxs;
    vec_t           v;
    vec_t           luxelsPerWorldUnit;
    int             axis;
    unsigned int             i;
    plane_t        plane;
    face_t* front;
    face_t* back;
    face_t* next;
    texinfo_t* tex;
    vec3_t          temp;

    // special (non-surface cached) original_face don't need subdivision
    tex = f->texinfo;

    bool did_subdivide = false;

    if (f->flags & CONTENTS_HINTSKIP)
        return did_subdivide;
    if (f->flags & CONTENTS_EMPTY)
        return did_subdivide;
    if (f->flags & SURFACEFLAG_NODRAW)
        return did_subdivide;
    if (f->flags & CONTENTS_ORIGIN)
        return did_subdivide;
    if (f->flags & CONTENTS_WATER)
        return did_subdivide;
    if (f->flags & CONTENTS_SKY)
        return did_subdivide;

    for (axis = 0; axis < 2; axis++)
    {
        while (1)
        {
            mins = 999999;
            maxs = -999999;

#ifdef HL2_LUXEL_METHOD
            VectorCopy(tex->lightmapVecsLuxelsPerWorldUnits[axis], temp); //hl2 method
#else
            VectorCopy(tex->vecs[axis], temp); //hl1 method
#endif

            for (i = 0; i < f->winding->m_NumPoints; i++)
            {
                v = DotProduct(f->winding->m_Points[i], temp);
                if (v < mins)
                {
                    mins = v;
                }
                if (v > maxs)
                {
                    maxs = v;
                }
            }

            if ((maxs - mins) <= g_subdivide_size)
            {
                break;
            }

            // split it
            //subdivides++;

            luxelsPerWorldUnit = VectorNormalize(temp);

            VectorCopy(temp, plane.normal);
#ifdef HL2_LUXEL_METHOD
            plane.dist = (mins + g_subdivide_size - 1) / luxelsPerWorldUnit; //hl2 method
#else
            plane.dist = (mins + g_subdivide_size - 16) / luxelsPerWorldUnit; //hl1 method
#endif

            next = f->next;
            SplitFace(f, plane.normal, plane.dist, &front, &back);
            if (!front || !back)
            {
                Developer(DEVELOPER_LEVEL_SPAM, "SubdivideFace: didn't split the %d-sided polygon @(%.0f,%.0f,%.0f)",
                    f->winding->m_NumPoints, f->winding->m_Points[0][0], f->winding->m_Points[0][1], f->winding->m_Points[0][2]);
                break;
            }
            *prevptr = back;

            // for debugging
            did_subdivide = true;
#ifdef _DEBUG
            front->subdivided = true;
            back->subdivided = true;
#endif

            back->next = front;
            front->next = next;
            f = back;
        }
    }

    return did_subdivide;
}
#endif

//ClipSideIntoTree_r/AddWindingToConvexHull
face_t* CombineFacesByPlane(face_t* face_fragments, face_t* original_face)
{
    face_t* final_face = new face_t(*original_face, new Winding);
    const plane_t& plane = gMappedPlanes[original_face->planenum];
    bool isFirstValidWinding = true;

    for (const face_t* currentFace = face_fragments; currentFace; currentFace = currentFace->next)
    {
        if (!currentFace->winding->HasPoints())
            continue;

        if (isFirstValidWinding)
        {
            if (final_face->winding)
                delete final_face->winding;
            final_face->winding = new Winding(*currentFace->winding);
            isFirstValidWinding = false;
            continue;
        }

        for (unsigned int pointIndex = 0; pointIndex < currentFace->winding->m_NumPoints; ++pointIndex)
        {
            char sides[MAX_POINTS_ON_WINDING]{ SIDE_FRONT };
            int numPointsInBack = final_face->winding->classifyPointAgainstPlaneEdges(sides, sizeof(sides), plane, currentFace->winding->m_Points[pointIndex]);

            if (numPointsInBack == 1)
            {
                unsigned int first_back_index;
                for (first_back_index = 0; first_back_index < final_face->winding->m_NumPoints; ++first_back_index)
                {
                    if (sides[first_back_index] != SIDE_FRONT)
                        break;
                }
                final_face->winding->insertPoint(currentFace->winding->m_Points[pointIndex], first_back_index + 1);
            }
            else if (numPointsInBack > 1)
            {
                unsigned int first_front_index;
                for (first_front_index = 0; first_front_index < final_face->winding->m_NumPoints; ++first_front_index)
                {
                    if (sides[first_front_index] == SIDE_FRONT)
                        break;
                }

                unsigned int offset = 0;
                for (unsigned int k = 0; k < final_face->winding->m_NumPoints; ++k)
                {
                    offset = (k + first_front_index) % final_face->winding->m_NumPoints;
                    if (sides[offset] != SIDE_FRONT)
                        break;
                }
                final_face->winding->shiftPoints(numPointsInBack - 1, (offset + 1) % final_face->winding->m_NumPoints);
                final_face->winding->insertPoint(currentFace->winding->m_Points[pointIndex], offset + 1);
            }
        }
    }

    if (final_face->winding->HasPoints())
    {
        final_face->winding->RemoveColinearPoints();
        return final_face;
    }
    else
    {
        FreeFace(final_face);
        final_face = new face_t(*original_face, new Winding);
        return final_face;
    }
}

void GetFinalBrushFaces(entity_t* entity, int brushflags)
{
    // dylan added for debug clarity..
    Verbose("%s...\n", __FUNCTION__);

    // Measure elapsed time
    double startTime = I_FloatTime();

    // Loop through each brush in the entity
    for (unsigned int i = 0; i < entity->numbrushes; ++i)
    {
        brush_t* brush = entity->brushes[i];

        // Check if the brush brushflags matches the processing brushflags
        if ((brush->brushflags & brushflags) != 0)
        {
            // Loop through each side of the brush
            for (unsigned int s = 0; s < brush->numsides; ++s)
            {
                side_t* side = brush->brushsides[s];

                if (side->final_face)
                    FreeFace(side->final_face);

                if ((side->original_face->flags & (CONTENTS_DETAIL | CONTENTS_BSP | SURFACEFLAG_NODRAW | CONTENTS_SOLID)) != 0)
                {
                    // clear face points
                    side->final_face = new face_t(*side->original_face, new Winding);
                }
                else
                {
                    // combine fragments into one giant face
#if defined(SUBDIVIDE) && defined(NO_FACE_COMBINE)
                    if (!g_nosubdiv)
                        side->final_face = new face_t(*side->original_face, new Winding);
                    else
                        side->final_face = CombineFacesByPlane(side->face_fragments, side->original_face);
#else
                    side->final_face = CombineFacesByPlane(side->face_fragments, side->original_face);
#endif
                }
            }
        }
    }

    // dylan fixed, gbx forgot to add this
    Verbose("%s : ", __FUNCTION__);
    LogTimeElapsed(I_FloatTime() - startTime);
}