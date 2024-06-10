#pragma once

extern void WriteDrawNodes_r(int depth, const node_t* const node);
extern void EmitDrawNode_r(node_t* n);
extern void CalcNodeChildBounds(unsigned int planenum, node_t* node);
extern void CalcNodeBoundsFromBrushes(node_t* node, entity_t* ent);
extern void CalcNodeBoundsFromFaces(node_t* node, face_t* face);
extern void CountNodesAndLeafsByPlane(int level, node_t* node);
extern void CountNodesAndLeafsByChildren(int level, node_t* node);
extern void PrintNodeMetricsByPlane(node_t* node);
extern void PrintNodeMetricsByChildren(node_t* node);
extern void SetAllFacesLeafNode(node_t* leafNode, entity_t* entity);
extern void CalcInternalNodes_r(node_t* node);
extern void MarkUnoccupiedLeafsAsSolid(node_t* node);
extern void MakeNodePortal(node_t* node);
extern void AddPortalToNodes(portal_t* p, node_t* front, node_t* back);
extern unsigned int CalcNodePlane(node_t* node, int bsp_depth, face_t* face);
extern void SplitNodePortals(node_t* node);
extern void AddPortalToNodes(portal_t* p, node_t* front, node_t* back);
#if 0
inline node_t* AllocNode()
{
	return reinterpret_cast<node_t * (*)()>(0x41CA40)();
}
#endif