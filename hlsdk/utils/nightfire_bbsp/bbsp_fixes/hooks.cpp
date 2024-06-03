
#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "log.h"
#include "Nodes.h"

void SubdivideFace(face_t* f, face_t** prevptr)
{
    vec_t           mins, maxs;
    vec_t           v;
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

    if (f->flags & CONTENTS_HINTSKIP)
        return;
    if (f->flags & CONTENTS_SOLID)
        return;
    if (f->flags & CONTENTS_NODRAW)
        return;
    if (f->flags & CONTENTS_ORIGIN)
        return;
    if (f->flags & CONTENTS_WATER)
        return;
    if (f->flags & SURF_SKY)
        return;
#if 0
    tex = &g_texinfo[f->texturenum];

    if (tex->flags & TEX_SPECIAL)
    {
        return;
    }

    if (f->facestyle == face_hint)
    {
        return;
    }
    if (f->facestyle == face_skip)
    {
        return;
    }

#ifdef ZHLT_NULLTEX    // AJM
    if (f->facestyle == face_null)
        return; // ideally these should have their tex_special flag set, so its here jic
#endif
#endif

#if 1
    for (axis = 0; axis < 2; axis++)
    {
        while (1)
        {
            mins = 999999;
            maxs = -999999;

            for (i = 0; i < f->winding->m_NumPoints; i++)
            {
                v = DotProduct(f->winding->m_Points[i], tex->vecs[axis]);
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

            VectorCopy(tex->vecs[axis], temp);

            v = VectorNormalize(temp);

            VectorCopy(temp, plane.normal);
            plane.dist = (mins + g_subdivide_size - 16) / v;
            next = f->next;
            SplitFace(f, plane.normal, plane.dist, &front, &back);
            if (!front || !back)
            {
                Developer(DEVELOPER_LEVEL_SPAM, "SubdivideFace: didn't split the %d-sided polygon @(%.0f,%.0f,%.0f)",
                    f->winding->m_NumPoints, f->winding->m_Points[0][0], f->winding->m_Points[0][1], f->winding->m_Points[0][2]);
                break;
            }
            *prevptr = back;
            back->next = front;
            front->next = next;
            f = back;
        }
    }
#endif
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

#if 0
    // subdivide large original_face
   face_t** prevptr = &list;
   face_t* f;
    while (1)
    {
        f = *prevptr;
        if (!f)
        {
            break;
        }
        if (strstr(f->texinfo->name, "lagzone"))
        {
            int breakpoint = 1;
        }
        SubdivideFace(f, prevptr);
        f = *prevptr;
        prevptr = &f->next;
    }

    //list = original_face;
#endif


    // Processing each list in the array
    if (original_face) {
        do {
            planenum = face->planenum; // Getting the plane number of the list
            next = face->next;         // Getting the next list in the array
            ++source;                  // Incrementing total list counter

            // If the list is coplanar or back-facing, free the list
            // fixme: q3 doesn't free opposite planes
            if (planenum == node_planenum || planenum == (node_planenum ^ 1)) 
            {
                FreeFace(face);
            }
            else {
                // Otherwise, split the list and update front/back counters


#if 0
    // subdivide large original_face
                face_t** prevptr = &list;
                face_t* f;
                while (1)
                {
                    f = *prevptr;
                    if (!f || !f->winding)
                    {
                        break;
                    }

                    SubdivideFace(f, prevptr);
                    f = *prevptr;
                    prevptr = &f->next;
                }

                //list = original_face;
#endif
#if 0
                plane_t newplane = gMappedPlanes[node_planenum];
                newplane.dist = 512;
                SplitFace(list, newplane, &original_face, &back_face);

                if (original_face)
                    list = original_face;
#endif
                //int plane_test = ChoosePlaneFromList(g_Node, list);
                //int old_planenum = node_planenum;

                SplitFace(face, node_planenum, &front_face, &back_face);
                FreeFace(face); // Freeing the original list

                if (front_face) {
                    ++front; // Incrementing front list counter
                    // Adding the front list to the front list list
                    front_face->next = *dest_front_face;
                    *dest_front_face = front_face;
                }

                if (back_face) {
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

void ComputeNodeChildBounds(unsigned int planenum, node_t* node)
{
    for (int child = 0; child < 2; ++child)
    {
        for (int axis = 0; axis < 3; ++axis)
        {
            node->children[child]->mins[axis] = node->mins[axis];
            node->children[child]->maxs[axis] = node->maxs[axis];
        }
    }
    
    const plane_t* plane = &gMappedPlanes[planenum];
    const auto axis = plane->closest_axis;
    if (axis <= plane_z)
    {
        if (plane->normal[axis] < 1)
        {
            node->children[0]->mins[axis] = -plane->dist;
            node->children[1]->maxs[axis] = -plane->dist;
        }
        else
        {
            node->children[0]->mins[axis] = plane->dist;
            node->children[1]->maxs[axis] = plane->dist;
        }
    }
}

static bool     CalcNodeBounds(node_t* node, int& oversized_axis)
{
    int             i;
#if 0
    int             j;
    vec_t           v;
    portal_t* p;
    portal_t* next_portal;
    int             side = 0;

    node->mins[0] = node->mins[1] = node->mins[2] = 9999;
    node->maxs[0] = node->maxs[1] = node->maxs[2] = -9999;

    for (p = node->portals; p; p = next_portal)
    {
        if (p->nodes[0] == node)
        {
            side = 0;
        }
        else if (p->nodes[1] == node)
        {
            side = 1;
        }
        else
        {
            Error2("CalcNodeBounds: mislinked portal");
        }
        next_portal = p->next[side];

        for (i = 0; i < p->winding->m_NumPoints; i++)
        {
            for (j = 0; j < 3; j++)
            {
                v = p->winding->m_Points[i][j];
                if (v < node->mins[j])
                {
                    node->mins[j] = v;
                }
                if (v > node->maxs[j])
                {
                    node->maxs[j] = v;
                }
            }
        }
    }
#endif

    for (i = 0; i < 3; i++)
    {
        if (node->maxs[i] - node->mins[i] > g_MaxNodeSize)
        {
            oversized_axis = i;
            return true;
        }
    }
    return false;
}

static int ChooseMidPlaneFromListNew(node_t* node, face_t* original_face, const vec3_t mins, const vec3_t maxs)
{
    int             j, l;
    face_t* p;
    face_t* bestsurface;
    vec_t           bestvalue;
    vec_t           value;
    vec_t           dist;
    plane_t* plane;

    //
    // pick the plane that splits the least
    //
    bestvalue = 6 * 8192 * 8192;
    bestsurface = NULL;

    for (p = original_face; p; p = p->next)
    {
        bool on_node = p->planenum == node->planenum || p->planenum == (node->planenum ^ 1);
        if (on_node)
        {
            continue;
        }

        plane = &gMappedPlanes[p->planenum];

        // check for axis aligned surfaces
        l = plane->closest_axis;
        if (l > plane_z)
        {
            continue;
        }

        //
        // calculate the split metric along axis l, smaller values are better
        //
        value = 0;

        dist = plane->dist * plane->normal[l];
        for (j = 0; j < 3; j++)
        {
            if (j == l)
            {
                value += (maxs[l] - dist) * (maxs[l] - dist);
                value += (dist - mins[l]) * (dist - mins[l]);
            }
            else
            {
                value += 2 * (maxs[j] - mins[j]) * (maxs[j] - mins[j]);
            }
        }

        if (value > bestvalue)
        {
            continue;
        }

        //
        // currently the best!
        //
        bestvalue = value;
        bestsurface = p;
    }

    if (!bestsurface)
    {
        for (p = original_face; p; p = p->next)
        {
            bool on_node = p->planenum == node->planenum || p->planenum == (node->planenum ^ 1);
            if (!on_node)
            {
                return p->planenum;                                  // first valid surface
            }
        }
        //Error2("ChooseMidPlaneFromList: no valid planes");
        return -1;  
    }

    return bestsurface->planenum;
}

void __cdecl BuildBspTree_r(int bspdepth, node_t* node, face_t* original_face, bool make_node_portals)
{
    //for (;;)
    //{
    //    Sleep(1);
   // }

    g_MakeNodePortals = make_node_portals;

    if (0 && g_MakeNodePortals)
    {
        face_t* f = original_face;
        while (1)
        {
            if (!f)
                break;
            char tmp[1024];
            sprintf_s(tmp, "Tree contains: %s Flags: %i BrushFlags %i\n", f->texinfo->name, f->flags, f->brush->brushflags);
            OutputDebugStringA(tmp);
            f = f->next;
        }
    }
#if 0
    if (g_MakeNodePortals)
    {
        // subdivide large original_face
        face_t** prevptr = &original_face;
        face_t* f;
        while (1)
        {
            f = *prevptr;
            if (!f || !f->winding)
            {
                break;
            }

            SubdivideFace(f, prevptr);
            f = *prevptr;
            prevptr = &f->next;
        }

        //list = original_face;
    }
#endif

    unsigned int planenum;                // Planenum of the current node
    face_t* original_back_face;  // Original back list
    face_t* front_face_new;      // New front list after splitting
    face_t* back_face_new;       // New back list after splitting

    // Checking if the recommended maximum BSP depth is exceeded
    if (bspdepth > 256)
        PrintOnce("BuildBspTree_r MAX_RECOMMENDED_BSP_DEPTH exceeded\n");

    // Calculating the planenum of the current node based on its bounds and size
    // If the node size is within a maximum size, choose a plane from a list, otherwise choose a middle plane.
    planenum = CalculateNodePlane(node, bspdepth, original_face);

    if (planenum == -1) {
        // If planenum is -1, it means the node has no bounds, mark it as a leaf
        node->planenum = -1;
        ++g_numLeafs;
    }
    else {
        // Otherwise, split the original_face based on the selected plane
        node->planenum = planenum & 0xFFFFFFFE;
        ++g_numNodes;

        g_Node = node;

        // Splitting the original_face into front and back
        SplitFaces(original_face, planenum & 0xFFFFFFFE, bspdepth, &front_face_new, &back_face_new);

        // Allocating memory for the child nodes
        node->children[0] = new node_t;
        node->children[1] = new node_t;

        // compute bounds
        ComputeNodeChildBounds(planenum & 0xFFFFFFFE, node);

        if (make_node_portals) {
            // Making the current node a portal node and splitting its portals
            MakeNodePortal(node);
            SplitNodePortals(node);
        }

        // Recursively build the BSP tree for the child nodes
        BuildBspTree_r(bspdepth + 1, node->children[0], front_face_new, make_node_portals);
        BuildBspTree_r(bspdepth + 1, node->children[1], back_face_new, make_node_portals);
    }
}

#include "textures.h"

void __cdecl FilterFaceIntoTree_r(int depth, node_t* node, side_t* brushside, face_t* list, bool face_windings_are_inverted, bool is_in_lighting_stage)
{
#if 1
    if (node->planenum == -1) //PLANENUM_LEAF
    {
        if (face_windings_are_inverted)
        {
            list->next = brushside->inverted_face_fragments;
            brushside->inverted_face_fragments = list;
            list->leaf_node = node;

            unsigned int brush_flags = 0;
            if (is_in_lighting_stage)
                brush_flags = list->brush->brushflags & 0xFFFFFF00;
            int brush_leaf_type = list->brush->leaf_type;

            // Set node leaf type as empty and update with brush flags
            node->leaf_type = LEAF_EMPTY_AKA_NOT_OPAQUE; //1
            node->leaf_type |= (brush_flags | node->leaf_type) & 0xFFFFFF00;

            // Update node leaf type if brush leaf type is greater
            if ((unsigned char)node->leaf_type < (int)(unsigned char)brush_leaf_type)
                node->leaf_type = brush_leaf_type;
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
        face_t* front_of_node_face_list;
        face_t* back_of_node_face_list;

        if (SplitFacesByNodePlane(list, node->planenum, &front_of_node_face_list, &back_of_node_face_list))
            FreeFace(list);

        if (front_of_node_face_list)
            FilterFaceIntoTree_r(++depth, node->children[0], brushside, front_of_node_face_list, face_windings_are_inverted, is_in_lighting_stage);
        if (back_of_node_face_list)
            FilterFaceIntoTree_r(++depth, node->children[1], brushside, back_of_node_face_list, face_windings_are_inverted, is_in_lighting_stage);
    }

#else


#if 1
    side_t* brushside; // ebp
    face_t* f; // esi
    node_t* node_; // edi
    int v9; // ebx
    bool _is_in_lighting_stage; // al
    int v11; // edx
    int brush_leaf_type; // esi
    unsigned int v13; // ecx
    face_t* face_fragments; // [esp+10h] [ebp-4h] BYREF
    bool is_back_faces = false;

    brushside = brushside;
    f = list;
    node_ = node;

#if 0
    //if (face_windings_are_inverted == 0)
    {
        // subdivide large original_face
        face_t** prevptr = &f;
        face_t* f2;
        while (1)
        {
            f2 = *prevptr;
            if (!f2)
            {
                break;
            }
            //if (strstr(f2->texinfo->name, "lagzone"))
           // {
            //    int breakpoint = 1;
            //}
            SubdivideFace(f2, prevptr);
            f2 = *prevptr;
            prevptr = &f2->next;
        }
    }
#endif


    if (node->planenum == -1)
    {
    LABEL_9:
        if (face_windings_are_inverted)
        {
            _is_in_lighting_stage = is_in_lighting_stage;
            f->next = brushside->inverted_face_fragments;
            v11 = 0;
            brushside->inverted_face_fragments = f;
            f->leaf_node = node_;
            if (_is_in_lighting_stage)
                v11 = f->brush->brushflags & 4294967040;
            brush_leaf_type = f->brush->brush_leaf_type;
            if (!node_->brush_leaf_type)
                node_->brush_leaf_type = 1;
            v13 = (v11 | node_->brush_leaf_type) & 0xFFFFFF00;
            if ((unsigned __int8)node_->brush_leaf_type < (int)(unsigned __int8)brush_leaf_type)
                node_->brush_leaf_type = brush_leaf_type;
            node_->brush_leaf_type |= v13;
        }
        else
        {
            ++g_NumNonInvertedFaces;
            f->next = brushside->face_fragments;
            brushside->face_fragments = f;
            f->leaf_node = node_;
        }
    }
    else
    {
        v9 = depth + 1;
        while (1)
        {
#if 0
            //if (face_windings_are_inverted == 0)
            {
                // subdivide large original_face
                face_t** prevptr = &f;
                face_t* f2;
                while (1)
                {
                    f2 = *prevptr;
                    if (!f2)
                    {
                        break;
                    }
                    //if (strstr(f2->texinfo->name, "lagzone"))
                   // {
                    //    int breakpoint = 1;
                    //}
                    SubdivideFace(f2, prevptr);
                    f2 = *prevptr;
                    prevptr = &f2->next;
                }
            }
            //f = f2;
#endif

            if (SplitFacesByNodePlane(f, node_->planenum, (face_t**)&node, &face_fragments))
                FreeFace(f);
            if (node)
                FilterFaceIntoTree_r(v9, node_->children[0], brushside, (face_t*)node, face_windings_are_inverted, is_in_lighting_stage);
            f = face_fragments;
            if (!face_fragments)
            {
                is_back_faces = false;
                break;
            }
            is_back_faces = true;
            node_ = node_->children[1];
            ++v9;
            if (node_->planenum == -1)
            {
                is_back_faces = false;
                goto LABEL_9;
            }
        }
    }
#else
    side_t* brushside = brushside;
    face_t* current_face = list;
    node_t* current_node = node;

    // If the current node has no plane number (terminal leaf node)
    if (current_node->planenum == -1)
    {
        // If face_windings_are_inverted is true
        if (face_windings_are_inverted)
        {
            // Assign the list to the front or back of the brushside based on the lighting stage
            current_face->next = brushside->inverted_face_fragments;
            brushside->inverted_face_fragments = current_face;
            current_face->leaf_node = current_node;

            // If we're in the lighting stage, update the brush_leaf_type and brush flags
            if (is_in_lighting_stage)
            {
                int brush_flags = current_face->brush->brushflags & 4294967040;
                int brush_leaf_type = current_face->brush->brush_leaf_type;

                current_node->brush_leaf_type = 1;
                current_node->brush_leaf_type |= (brush_flags | current_node->brush_leaf_type) & 0xFFFFFF00;

                if ((unsigned char)current_node->brush_leaf_type < (int)(unsigned char)brush_leaf_type)
                    current_node->brush_leaf_type = brush_leaf_type;
            }
        }
        else // If face_windings_are_inverted is false
        {
            ++g_NumNonInvertedFaces;
            // Assign the list to the back of the brushside
            current_face->next = brushside->face_fragments;
            brushside->face_fragments = current_face;
            current_face->leaf_node = current_node;
        }
    }
    else // If the current node has a plane number (internal node)
    {
        int new_depth = depth + 1;
        face_t* original_face;
        face_t* back_face;

        while (1)
        {
            // Split the list along the current node
            if (SplitFacesByNodePlane(current_face, node->planenum, &original_face, &back_face))
                FreeFace(current_face);

            // Recursively build the tree for the front list if it exists
            if (original_face)
                FilterFaceIntoTree_r(new_depth, current_node->children[0], brushside, original_face, face_windings_are_inverted, is_in_lighting_stage);

            // Update the list to the back list
            current_face = back_face;

            // If there's no more back list, break the loop
            if (!back_face)
                break;

            // Move to the next child node
            current_node = current_node->children[1];
            ++new_depth;

            // If the next node is a terminal leaf node, handle it
            if (current_node->planenum == -1)
                break;
        }
    }
#endif
#endif
}