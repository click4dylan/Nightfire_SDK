#pragma once
#include "bsp_structs.h"

extern node_t* FillOutside(mapinfo_t* mapfile, node_t* node, int pass_num);
extern void PrintLeakInfoIfLeaked(mapinfo_t* mapfile, int pass_num);
extern bool  PlaceOccupant(const vec3_t point, node_t* node, unsigned int entindex);
extern node_t* PointInLeaf(node_t* node, const vec3_t point);
extern void MarkLeafOccupancyAndCheckLeaks(node_t* node, unsigned int occupancyValue);