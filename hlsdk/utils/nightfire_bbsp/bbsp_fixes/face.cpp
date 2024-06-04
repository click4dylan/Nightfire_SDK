#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "textures.h"
#include "log.h"

DWORD a = 0x41D3C0;

face_s::~face_s()
{
    //FreeFaceList
    face_s* n = next;
    while (n)
    {
        face_s* next_in_chain = n->next;
        free(n);
        g_numFaces -= 1;
        n = next_in_chain;
    }

    g_numFaces -= 1;
}

// Default constructor
face_s::face_s(unsigned int planenum)
    : planenum(planenum) {
    g_numFaces += 1;
}

face_s::face_s(const face_s& src)
    : planenum(src.planenum), flags(src.flags), texinfo(src.texinfo), brush(src.brush), brushside(src.brushside)
{
    g_numFaces += 1;
    memcpy(vecs, src.vecs, sizeof(vecs));
    if (src.winding)
        winding = new Winding(*src.winding);
    else
        winding = nullptr;
}

face_s::face_s(const face_s& src, Winding* src_winding)
    : planenum(src.planenum), flags(src.flags), texinfo(src.texinfo), brush(src.brush), brushside(src.brushside)
{
    g_numFaces += 1;
    memcpy(vecs, src.vecs, sizeof(vecs));
    winding = src_winding;
}

bool SplitFacesByNodePlane(face_t* face, unsigned int planenum, face_t** front, face_t** back)
{
#if 1
    *front = nullptr;
    *back = nullptr;
    if (!face->winding->Valid())
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
    if (face->winding->Clip(plane, &new_front, &new_back))
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
        if (face->winding->Valid())
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
        if (in->winding->Valid())
        {
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

void StripOutsideFaces(node_t* node, entity_t* ent, unsigned int filterFlag)
{
    unsigned int brushIndex = 0;
    unsigned int numBrushes = ent->numbrushes;
    unsigned int keptFragments = 0;
    unsigned int discardedFragments = 0;

    // Loop through all brushes in the entity
    for (unsigned int i = 0; i < numBrushes; ++i)
    {
        brush_t* brush = ent->firstbrush[brushIndex];

        // Check if the filter flag matches the brush's brushflags
        if ((filterFlag & brush->brushflags) != 0)
        {
            unsigned int numSides = brush->numsides;

            // Loop through all sides of the brush
            for (unsigned int j = 0; j < numSides; ++j)
            {
                side_t* brushSide = brush->brushsides[j];
                face_t* faceFragments = brushSide->face_fragments;
                face_t* keptFaceFragments = nullptr;

                // Loop through all original_face of the side
                while (faceFragments)
                {
                    face_t* next = faceFragments->next;

                    // Check if the originalFace's associated leaf node is solid
                    if (faceFragments->leaf_node->leaf_type == LEAF_SOLID_AKA_OPAQUE)
                    {
                        // Free the originalFace if it's associated with a solid leaf node
                        FreeFace(faceFragments);
                        ++discardedFragments;
                    }
                    else
                    {
                        // Keep the originalFace if it's not associated with a solid leaf node
                        faceFragments->next = keptFaceFragments;
                        keptFaceFragments = faceFragments;
                        ++keptFragments;
                    }

                    faceFragments = next;
                }

                // Update the brush side's normal original_face with the kept originalFace fragments
                brushSide->face_fragments = keptFaceFragments;
            }
        }

        ++brushIndex;
    }

    // Output verbose information about kept and discarded originalFace fragments
    Verbose("%5i face fragments kept\n", keptFragments);
    Verbose("%5i face fragments discarded\n", discardedFragments);
}

void MarkEmptyBrushFaces(unsigned int flag, entity_t* entity)
{
    unsigned int totalFaces = 0;
    unsigned int removedFaces = 0;

    for (unsigned int brushIndex = 0; brushIndex < entity->numbrushes; ++brushIndex)
    {
        brush_t* brush = entity->firstbrush[brushIndex];
        if ((flag & brush->brushflags) != 0)
        {
            totalFaces += brush->numsides;

            for (unsigned int sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
            {
                side_t* side = brush->brushsides[sideIndex];
                face_t* face = side->original_face;

                while (face != NULL)
                {
                    unsigned int faceFlags = face->flags;
                    if ((faceFlags & (CONTENTS_DETAIL | CONTENTS_BSP | CONTENTS_NODRAW | CONTENTS_UNKNOWN)) == 0 && side->face_fragments == NULL)
                    {
                        ++removedFaces;
                        face->flags |= CONTENTS_NODRAW;
                        if ((flag & CONTENTS_BSP) != 0)
                            face->flags |= CONTENTS_DETAIL;
                    }
                    face = face->next;
                }
            }
        }
    }

    const char* surfaceTypeStr = GetSurfaceTypeStr(flag);
    Verbose("%5i %s faces removed\n", removedFaces, surfaceTypeStr);
    Verbose("%5i %s faces remaining\n", totalFaces - removedFaces, surfaceTypeStr);
}

void FreeBrushFaces(entity_t* entity, unsigned int flag)
{
    unsigned int numBrushes = entity->numbrushes;
    unsigned int brushIndex = 0;

    for (unsigned int i = 0; brushIndex < numBrushes; i = brushIndex)
    {
        brush_t* brush = entity->firstbrush[brushIndex];
        if ((flag & brush->brushflags) != 0)
        {
            unsigned int numSides = brush->numsides;
            for (unsigned int sideIndex = 0; sideIndex < numSides; ++sideIndex)
            {
                side_t* side = brush->brushsides[sideIndex];
                if (side->face_fragments)
                    delete side->face_fragments;
                if (side->inverted_face_fragments)
                    delete side->inverted_face_fragments;
                //FreeFaceList(side->face_fragments);
                //FreeFaceList(side->inverted_face_fragments);
                side->face_fragments = NULL;
                side->inverted_face_fragments = NULL;
            }
        }
        ++brushIndex;
    }
}

// replaced with C++
#if 0
void FreeFaceList(face_t* list)
{
    while (list)
    {
       face_t* next = list->next;
       FreeFace(list);
       list = next;
    }
}
#endif

void FilterFacesIntoTree(face_t* list, node_t* node, bool face_windings_are_inverted, bool is_lighting_tree)
{
    while (list)
    {
        face_t* next = list->next;
        FilterFaceIntoTree_r(0, node, list->brushside, list, face_windings_are_inverted, is_lighting_tree);
        list = next;
    }
}

void FilterFaceIntoTree_r(int depth, node_t* node, side_t* brushside, face_t* list, bool face_windings_are_inverted, bool is_in_lighting_stage)
{
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

            // Set node leaf type as empty and update with brush brushflags
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
    // Initialize the valid flag to false
    node->valid = false;

    if (!node->children[0])
    {
        // Leaf node reached
        if (node->leaf_type != LEAF_SOLID_AKA_OPAQUE)
        {
            // Propagate valid flag through portals
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

    // Recursively propagate the valid flag
    node->children[0] = ClearOutFaces_r(depth + 1, node->children[0]);
    node->children[1] = ClearOutFaces_r(depth + 1, node->children[1]);

    if (!node->valid)
    {
        // this node does not touch any interior leafs
        if (node->children[0]->leaf_type == LEAF_SOLID_AKA_OPAQUE && node->children[1]->leaf_type == LEAF_SOLID_AKA_OPAQUE)
        {
            // if both children are solid, just make this node solid
            node->leaf_type = LEAF_SOLID_AKA_OPAQUE;
            node->planenum = -1;

            delete node->children[0];
            delete node->children[1];
            node->children[0] = nullptr;
            node->children[1] = nullptr;
            return node;
        }

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
        brush_t* brush = entity->firstbrush[brushIndex];
        unsigned int numSides = brush->numsides;

        for (unsigned int sideIndex = 0; sideIndex < numSides; ++sideIndex)
        {
            side_t* side = brush->brushsides[sideIndex];

            face_t* originalFace = side->original_face;
            if (originalFace)
                originalFace->leaf_node = node;

            face_t* extraFace = side->final_face;
            if (extraFace)
                extraFace->leaf_node = node;

            face_t* normalFace = side->face_fragments;
            while (normalFace != NULL)
            {
                normalFace->leaf_node = node;
                normalFace = normalFace->next;
            }

            face_t* invertedFace = side->inverted_face_fragments;
            while (invertedFace != NULL)
            {
                invertedFace->leaf_node = node;
                invertedFace = invertedFace->next;
            }
        }
    }
}

face_t* CopyFaceList(entity_t* ent, unsigned int brushflags)
{
    face_t* copiedFaces = nullptr; // Pointer to the list of copied original_face
    unsigned int numCopiedFaces = 0; // Counter for the number of copied original_face

    // Iterate over all brushes in the entity
    for (unsigned int brushIndex = 0; brushIndex < ent->numbrushes; ++brushIndex)
    {
        brush_t* brush = ent->firstbrush[brushIndex];

        // Check if the brush brushflags match the specified brushflags
        if ((brush->brushflags & brushflags) != 0)
        {
            // Iterate over all sides of the brush
            for (unsigned int sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
            {
                face_t* face = brush->brushsides[sideIndex]->original_face;

                // Check if the originalFace brushflags match the criteria
                if ((face->flags & (CONTENTS_DETAIL | CONTENTS_BSP | CONTENTS_UNKNOWN)) == 0)
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

face_t* CopyFaceList_Inverted(entity_t* ent, unsigned int brushflags)
{
    // Copy the originalFace list with the given brushflags from the entity
    face_t* face = CopyFaceList(ent, brushflags);

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
    dface_t* dface = &g_dFaces[g_numDFaces];
    hlassume(g_numDFaces < MAX_MAP_FACES, assume_MAX_MAP_FACES);

    pFace->outputnumber = g_numDFaces;
    pFace->built_draw_indices_for_face = 1;
    g_numDFaces += 1;

    // Handle special face brushflags
    if ((pFace->flags & (CONTENTS_NODRAW | CONTENTS_PORTAL | SURF_SKY | CONTENTS_UNKNOWN | CONTENTS_SOLID)) != 0)
        pFace->winding->m_NumPoints = 0;

    // Set face properties
    dface->plane_index = pFace->planenum;
    dface->texture_index = GetTextureIndex(pFace->texinfo->name);
    dface->material_index = GetMaterialIndex(pFace->brushside->td.material);
    dface->flags = pFace->flags;
    dface->tex_info_projection_index = GetProjectionIndex(pFace->texinfo->vecs[0], pFace->texinfo->vecs[1]);

    // Get lightmap projections
    double s[4], t[4];
    GetLightmapProjections(s, pFace, t);
    s[3] = 0.0f;
    t[3] = 0.0f;

    dface->lightmap_info_projection_index = GetProjectionIndex(s, t);

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

            plane_t* plane = &gMappedPlanes[pFace->planenum];

            dvertex_t* vertex = &g_dverts[g_numDVerts];
            vertex->point[0] = pFace->winding->m_Points[i][0];
            vertex->point[1] = pFace->winding->m_Points[i][1];
            vertex->point[2] = pFace->winding->m_Points[i][2];

            dnormal_t* normal = &g_dnormals[g_numDVerts];
            normal->normal[0] = plane->normal[0];
            normal->normal[1] = plane->normal[1];
            normal->normal[2] = plane->normal[2];

            ++g_numDVerts;
        }
    }

    // Add indices
    if (numPoints > 2)
    {
        unsigned int numIndices = (numPoints - 2) * 3;
        hlassume(g_numDIndices < MAX_MAP_INDICES, assume_MAX_MAP_INDICES);

        dface->first_indices_index = g_numDIndices;
        dface->num_indicies = numIndices;

        for (unsigned int i = 2; i < numPoints; ++i)
        {
            unsigned int index = g_numDIndices;
            unsigned int* indices = &g_dindices[g_numDIndices];
            g_dindices[g_numDIndices] = 0;
            g_numDIndices += 1;
            g_dindices[g_numDIndices] = i - 1;
            g_numDIndices += 1;
            g_dindices[g_numDIndices] = i;
        }
    }
}

void MarkFace(node_t* leaf_node, face_t* face)
{
    if (!leaf_node->markfaces)
        leaf_node->markfaces = new std::set<face_t*>;

    leaf_node->markfaces->insert(face);
}

void MarkBrush(node_t* leaf_node, brush_t* brush)
{
    if (leaf_node)
    {
        if (!leaf_node->markbrushes)
            leaf_node->markbrushes = new std::set<brush_t*>;

        leaf_node->markbrushes->insert(brush);
    }
}

void MarkFinalFaceFragments(node_t* node, entity_t* ent)
{
    for (unsigned int i = 0; i < ent->numbrushes; ++i)
    {
        brush_t* brush = ent->firstbrush[i];
        for (unsigned int j = 0; j < brush->numsides; ++j)
        {
            side_t* side = brush->brushsides[j];
            face_t* face_fragments = side->face_fragments;
            while (face_fragments)
            {
                face_t* next = face_fragments->next;
                MarkFace(face_fragments->leaf_node, side->final_face);
                MarkBrush(face_fragments->leaf_node, brush);
                face_fragments = next;
            }
            face_t* inverted_face_fragments = side->inverted_face_fragments;
            while (inverted_face_fragments)
            {
                face_t* next = inverted_face_fragments->next;
                MarkBrush(inverted_face_fragments->leaf_node, brush);
                inverted_face_fragments = next;
            }
        }
    }
}

void MarkFaceFragments(node_t* node, entity_t* ent)
{
    for (unsigned int i = 0; i < ent->numbrushes; ++i)
    {
        brush_t* brush = ent->firstbrush[i];
        for (unsigned int j = 0; j < brush->numsides; ++j)
        {
            side_t* side = brush->brushsides[j];
            face_t* face_fragments = side->face_fragments;
            while (face_fragments)
            {
                face_t* next = face_fragments->next;
                MarkFace(face_fragments->leaf_node, face_fragments);
                MarkBrush(face_fragments->leaf_node, brush);
                face_fragments = next;
            }
        }
    }
}

// not used in nightfire
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