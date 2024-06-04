#pragma once

#include "bsp_structs.h"

// free face without freeing children
inline void FreeFace(face_t* face)
{
#if 1
    if (!face)
        return;
    face->next = nullptr;
    delete face;
#else
    DWORD adr = 0x41CA70;
    __asm
    {
        mov esi, face
        call adr
    }
#endif
}

//replaced by C++
#if 0
inline face_t* DuplicateFaceSharedWinding(face_t* src, Winding* winding)
{
    face_t* result;
    DWORD adr = 0x41CEE0;
    Winding* winding_val = winding;
    __asm
    {
        mov esi, src
        mov eax, winding_val
        push eax
        call adr
        add esp, 4
        mov result, eax
    }
    return result;
}

inline face_t* DuplicateFaceUniqueWinding(face_t* src)
{
    face_t* result;
    DWORD adr = 0x41CEB0;
    __asm
    {
        mov eax, src
        call adr
        mov result, eax
    }
    return result;
}
#endif

extern bool SplitFace(face_t* face, unsigned int plane, face_t** front, face_t** back);
extern void SplitFace(face_t* in, const vec3_t normal, const vec_t dist, face_t** front, face_t** back);
extern void SplitFaces(face_t* original_face, unsigned int node_planenum, int depth, face_t** dest_front_face, face_t** dest_back_face );
extern bool SplitFacesByNodePlane(face_t* face, unsigned int planenum, face_t** front, face_t** back);
extern void StripOutsideFaces(node_t* node, entity_t* ent, unsigned int filterFlag);
extern void MarkEmptyBrushFaces(unsigned int flag, entity_t* entity);
extern void FreeBrushFaces(entity_t* entity, unsigned int flag);
//extern void FreeFaceList(face_t* list);
extern void FilterFacesIntoTree(face_t* list, node_t* node, bool face_windings_are_inverted, bool is_lighting_tree);
extern node_t* ClearOutFaces_r(int depth, node_t* node);
extern node_t* ClearOutFaces(node_t* node);
extern void FilterFaceIntoTree_r(int depth, node_t* node, side_t* brushside, face_t* list, bool face_windings_are_inverted, bool is_in_lighting_stage);
extern face_t* CopyFaceList(entity_t* ent, unsigned int brushflags);
extern face_t* CopyFaceList_Inverted(entity_t* ent, unsigned int brushflags);
extern const char* GetSurfaceTypeStr(__int16 flags);
extern void AddFaceToBounds(const face_t* const f, vec3_t mins, vec3_t maxs);
extern void WriteFace_AkaBuildDrawIndicesForFace(face_t* pFace);
extern void MarkFinalFaceFragments(node_t* node, entity_t* ent);
extern void MarkFaceFragments(node_t* node, entity_t* ent);
extern void MarkFace(node_t* leaf_node, face_t* face);
extern void MarkBrush(node_t* leaf_node, brush_t* brush);
extern void SubdivideFace(face_t* f, face_t** prevptr);