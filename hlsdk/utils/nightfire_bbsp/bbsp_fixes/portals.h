#pragma once

#include "bsp_structs.h"

extern void MarkLeakTrail(portal_t* portal);
extern void RemovePortalFromNode(portal_t* portal, node_t* l);
extern void WritePortalfile(node_t* node);
extern void WritePortalFile_r(int depth, node_t* headnode);
extern void MakeHeadnodePortals(node_t* node);