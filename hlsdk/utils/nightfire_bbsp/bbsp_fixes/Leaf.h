#pragma once

extern void WriteDrawLeaf(node_t* node);
extern bool  PlaceOccupant(const vec3_t point, node_t* headnode, unsigned int entindex);
extern node_t* PointInLeaf(node_t* node, const vec3_t point);
extern void MarkLeafOccupancyAndCheckLeaks(node_t* currentNode, unsigned int occupancyValue);
extern void CountLeaves(int level, node_t* node);
extern void PrintLeafMetrics(node_t* node, const char* name);