#pragma once
#include "globals.h"

extern void CalcBrushBounds(vec3_t& maxs_dest, vec3_t& mins_dest, const brush_t* brush);
extern void FilterBrushesIntoTree(node_t* node, entity_t* ent, int flags);
extern void WriteDrawBrushSide(side_t* side);
extern void WriteDrawBrushes(brush_t* brush);
extern void MakeHullFaces(brush_t* b);
extern void AddHullPlane(brush_t* brush, vec3_t normal, vec_t dist);;
extern void SortBrushSidesByCanonicalOrder(brush_t* brush);
extern void AddBrushBevels(brush_t* buildBrush);