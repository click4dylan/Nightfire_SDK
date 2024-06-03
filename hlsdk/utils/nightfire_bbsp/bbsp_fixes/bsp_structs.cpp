#include "globals.h"
#include "bsp_structs.h"
#include "portals.h"

portal_s::portal_s()
{
	winding = nullptr;
	g_NumPortals += 1;
}

portal_s::~portal_s()
{
	g_NumPortals -= 1;
	if (winding)
		delete winding;
}

node_s::node_s()
{
	g_numNodesAllocated += 1;
}

node_s::~node_s()
{
	bool v2 = false;
	if (children[0])
		delete children[0];
	if (children[1])
		delete children[1];

	portal_t* portal = portals;
	while (portal) 
	{
		portal_t* nextPortal = portal->next[portal->nodes[1] == this];
		RemovePortalFromNode(portal, portal->nodes[portal->nodes[1] != this]);
		if (portal)
			delete portal;
		portal = nextPortal;
	}


	// Free markfaces associated with the node
	if (markfaces) //TODO: FIXME: SHOULDNT BE HERE
	{
		markfaces->clear();
		delete markfaces;
	}

	// Free markbrushes associated with the node
	if (markbrushes) //TODO: FIXME: SHOULDNT BE HERE
	{
		markbrushes->clear();
		delete markbrushes;
	}

	--g_numNodesAllocated;
}

entinfo_t::entinfo_t()
{
	g_numMapsAlloced += 1;
}

entinfo_t::~entinfo_t()
{
	if (entities)
	{
		for (unsigned i = 0; i < numentities; ++i)
		{
			if (entities[i])
				delete entities[i];
		}
		delete entities;
	}

	g_numMapsAlloced -= 1;
}

entity_s::entity_s()
{
	g_numEnts += 1;
}

entity_s::~entity_s()
{
	//FIXME:
	if (epairs)
		delete epairs;
#if 0
	epair_t* ep = epairs;
	while (ep)
	{
		epair_t* next = ep->next;
		delete ep;
		ep = next;
	}
#endif

	if (firstbrush)
	{
		for (unsigned int i = 0; i < numbrushes; ++i)
		{
			if (firstbrush[i])
				delete firstbrush[i];
		}
		free(firstbrush);
	}
	g_numEnts -= 1;
}

brush_s* entity_s::CreateNewBrush()
{
	if (numbrushes == max_alloced_brushes) 
	{
		// Increase the capacity of the brush array
		const unsigned int newCapacity = max_alloced_brushes + 64;
		brush_t** newBrushArray = (brush_t**)calloc(1, sizeof(brush_s**) * newCapacity);
		if (firstbrush) 
		{
			memcpy(newBrushArray, firstbrush, sizeof(brush_s**) * numbrushes);
			free(firstbrush);
		}
		firstbrush = newBrushArray;
		max_alloced_brushes = newCapacity;
	}

	// Allocate memory for the new brush
	brush_t* newBrush = new brush_t;
	newBrush->entitynum = index;
	newBrush->brushnum = numbrushes;
	firstbrush[numbrushes++] = newBrush;

	return newBrush;
}

epair_s::epair_s()
{
	g_numEPairs += 1;
}

epair_s::~epair_s()
{
	if (next)
		delete next;
	if (key)
		free(key);
	if (value)
		free(value);
	g_numEPairs -= 1;
}