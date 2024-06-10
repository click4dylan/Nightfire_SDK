#pragma once

extern void WriteDrawLeaf(node_t* node);
extern void CountLeaves(int level, node_t* node);
extern void NumberLeafs_r(int depth, node_t* node);
extern void PrintLeafMetrics(node_t* node, const char* name);
extern void AssignVisLeafNumbers_r(node_t* node);