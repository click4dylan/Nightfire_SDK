#pragma once

extern void WriteDrawLeaf(node_t* node);
extern bool  PlaceOccupant(const vec3_t point, node_t* headnode, unsigned int entindex);
extern node_t* PointInLeaf(node_t* node, const vec3_t point);
extern void MarkLeafOccupancyAndCheckLeaks(node_t* headnode, unsigned int occupancyValue);
extern void CountLeaves(int level, node_t* node);
extern void NumberLeafs_r(int depth, node_t* node);
extern void PrintLeafMetrics(node_t* node, const char* name);
extern void AssignVisLeafNumbers_r(node_t* node);