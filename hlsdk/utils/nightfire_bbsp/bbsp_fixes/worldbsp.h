#pragma once
#include "bsp_structs.h"

extern void WriteHullFile(const char* fileNamePrefix, entity_t* entity, int brushFlag, char appendMode, char writeFlag);
extern void SetModelBounds(dmodel_t* model, vec3_t& mins, vec3_t& maxs);
extern void BuildLightingBSPTree(entity_t* ent);
extern void PrintLeakInfoIfLeaked(entinfo_t* entinfo, int pass_num);
extern face_t* CombineFacesByPlane(face_t* referenceFaces, face_t* inputFace);
extern void GetFinalBrushFaces(entity_t* entity, int brushflags);
extern void GenerateWaterModel(entity_t* entity);
extern void BuildBSPTree(bool makeNodePortals, node_t* node, face_t* bspFaces);
extern void BuildBspTree_r(int bspdepth, node_t* node, face_t* original_face, bool make_node_portals);
extern void ModelBSP(entity_t* entity, dmodel_t* model, int modelbsp_index);
extern void WorldBSP(entity_t* ent, entinfo_t* info, dmodel_t* dmodel);
extern node_t* FillOutside(entinfo_t* entinfo, node_t* node, int pass_num);
