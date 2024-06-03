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

        // Check if the filter flag matches the brush's flags
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

node_t* ClearOutFaces(node_t* a1)
{
    g_numFalseNodes = 0;
    node_t* result = ClearOutFaces_r(0, a1);
    Verbose("%5i false nodes\n", g_numFalseNodes);
    return result;
}

node_t* ClearOutFaces_r(int depth, node_t* node)
{
    node_t* left_child = node->children[0];

    // Initialize the valid flag to false
    node->valid = false;

    if (!left_child)
    {
        // Leaf node reached
        if (node->leaf_type != LEAF_SOLID_AKA_OPAQUE)
        {
            // Propagate valid flag through portals
            portal_t* portals = node->portals;
            while (portals)
            {
                node_t* on_node = portals->onnode;
                if (on_node)
                    on_node->valid = true;
                portals = (portals->nodes[0] == node) ? portals->next[0] : portals->next[1];
            }
        }
        return node;
    }

    // Recursively propagate the valid flag
    node->children[0] = ClearOutFaces_r(depth + 1, left_child);
    node->children[1] = ClearOutFaces_r(depth + 1, node->children[1]);

    // Check if the current node should be pruned
    bool is_valid = node->valid;
    if (!is_valid && node->children[0]->leaf_type == LEAF_SOLID_AKA_OPAQUE && node->children[1]->leaf_type == LEAF_SOLID_AKA_OPAQUE)
    {
        // Prune the current node
        delete node->children[0];
        delete node->children[1];
        node->leaf_type = LEAF_SOLID_AKA_OPAQUE;
        node->children[0] = nullptr;
        node->children[1] = nullptr;
        node->planenum = -1;
    }
    else if (!is_valid)
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

face_t* CopyFaceList(entity_t* ent, unsigned int flags)
{
    face_t* copiedFaces = nullptr; // Pointer to the list of copied original_face
    unsigned int numCopiedFaces = 0; // Counter for the number of copied original_face

    // Iterate over all brushes in the entity
    for (unsigned int brushIndex = 0; brushIndex < ent->numbrushes; ++brushIndex)
    {
        brush_t* brush = ent->firstbrush[brushIndex];

        // Check if the brush flags match the specified flags
        if ((flags & brush->brushflags) != 0)
        {
            // Iterate over all sides of the brush
            for (unsigned int sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
            {
                face_t* face = brush->brushsides[sideIndex]->original_face;

                // Check if the originalFace flags match the criteria
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
    const char* surfaceTypeStr = GetSurfaceTypeStr(flags);
    Verbose("%5i %s faces\n", numCopiedFaces, surfaceTypeStr);

    return copiedFaces;
}

face_t* CopyFaceList_Inverted(entity_t* ent, unsigned int flags)
{
    // Copy the originalFace list with the given flags from the entity
    face_t* face = CopyFaceList(ent, flags);

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

    // Handle special face flags
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