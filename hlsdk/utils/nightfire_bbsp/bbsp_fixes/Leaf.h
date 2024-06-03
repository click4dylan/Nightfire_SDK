#pragma once

extern void WriteDrawLeaf(node_t* node);
extern bool  PlaceOccupant(double* a1, node_t* n, unsigned int entindex);
extern node_t* FindLeafNodeAlongVector(node_t* startNode, const double* vector);
extern void MarkLeafOccupancyAndCheckLeaks(node_t* currentNode, unsigned int occupancyValue);
extern void CountLeaves(int level, node_t* node);
extern void PrintLeafMetrics(node_t* node, const char* name);