#pragma once
#include "globals.h"

//extern void FreeEntity(entity_t* ent);
//extern void FreeMap(entinfo_t* ent);
extern entinfo_t* LoadMapFile(const char* filename);
extern unsigned int GetNumEngineEntities(entinfo_t* mapfile);
extern void  CheckBrushForWater(entity_t* mapent);
extern unsigned int GetNumDetailFaces(entity_t* entity);
extern unsigned int GetNumFaces(entity_t* entity);
extern unsigned int GetNumDetailBrushes(entity_t* entity);
extern unsigned int GetNumBrushes(entity_t* entity);
extern epair_t* ParseEpair();
extern entity_t* AllocNewEntities(entinfo_t* mapfile);
extern bool ParseMapEntity(entinfo_t* map);
extern void ParseMapBrush(entity_t* ent);
extern void ParseBrushSide(brush_t* brush, side_s* brush_side);
extern void ParsePlanePoints(brush_t* brush, side_s* brush_side, int pointIndex);
extern void ParseTexture(brush_t* brush, side_s* brush_side);
extern void ParseMaterial(brush_t* brush, side_s* brush_side);
extern void ParseFlags(brush_t* brush, side_s* brush_side);
extern void ParseAxis(brush_t* brush, vec3_t axis, vec_t& shift, const char* axisName);
extern void ParseScale(brush_t* brush, side_s* brush_side);
extern void ParseLightmapInfo(brush_t* brush, side_s* brush_side);
extern void HandleBrushFlags(brush_t* brush);
extern void HandleOriginBrush(brush_t* brush);
extern void ParseBrush(entity_t* mapent);
extern void CreateBrush(brush_t* brush, entity_t* ent);
extern bool MakeBrushPlanes(entity_t* entity, brush_t* brush);
extern void MakeBrushFaces(entity_t* ent, brush_t* brush);
extern void ExportBevels(entinfo_t* entinfo);