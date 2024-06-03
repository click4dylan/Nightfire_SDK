#pragma once
#include "bsp_structs.h"

extern void SplitFaces(
    face_t* original_face,            // Array of original_face
    unsigned int node_planenum,        // Plane number of the node
    int depth,                // Depth of recursion
    face_t** dest_front_face, // Pointer to pointer for front original_face
    face_t** dest_back_face   // Pointer to pointer for back original_face
);

extern void BuildBspTree_r(int bspdepth, node_t* node, face_t* front_face, bool make_node_portals);
extern void FilterFaceIntoTree_r(int depth, node_t* node, side_t* brushside, face_t* list, bool face_windings_are_inverted, bool is_in_lighting_stage);