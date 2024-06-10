#pragma once
#include "bsp_structs.h"

#define WRITE_FINAL_FACES true
#define WRITE_FACE_FRAGMENTS false
#define TRUNCATE false
#define APPEND true

extern void WriteHullFile(const char* fileNamePrefix, entity_t* entity, int brushflags, bool append, bool write_final_faces);
extern void SetModelBounds(dmodel_t* model, vec3_t& mins, vec3_t& maxs);
extern void BuildLightingBSPTree(entity_t* ent);
extern void GenerateWaterModel(entity_t* entity);
extern void BuildBSPTree(bool makeNodePortals, node_t* node, face_t* bspFaces);
extern void BuildBspTree_r(int bspdepth, node_t* node, face_t* original_face, bool make_node_portals);
extern void ModelBSP(entity_t* entity, dmodel_t* model, int modelbsp_index);
extern void WorldBSP(entity_t* ent, mapinfo_t* info, dmodel_t* dmodel);
