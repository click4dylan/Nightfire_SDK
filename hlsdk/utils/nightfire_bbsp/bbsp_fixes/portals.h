#pragma once

#include "bsp_structs.h"

extern void MarkLeakTrail(portal_t* portal);
extern void RemovePortalFromNode(portal_t* portal, node_t* l);