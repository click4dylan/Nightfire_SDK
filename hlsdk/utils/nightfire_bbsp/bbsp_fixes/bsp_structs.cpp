#include "globals.h"
#include "bsp_structs.h"
#include "portals.h"

portal_s::portal_s()
{
	winding = nullptr;
	++g_NumPortals;
}

portal_s::~portal_s()
{
	--g_NumPortals;
	if (winding)
		delete winding;
}

node_s::node_s()
{
	++g_numNodesAllocated;
}

node_s::~node_s()
{
	if (children[0])
		delete children[0];
	if (children[1])
		delete children[1];

	// Free portals
	portal_t* p, * nextp;
	for (p = portals; p; p = nextp)
	{
		int s = (p->nodes[1] == this);
		nextp = p->next[s];

		RemovePortalFromNode(p, p->nodes[!s]);
		delete p;
	}


	// Free markfaces associated with the node
	ClearMarkFaces();

	// Free markbrushes associated with the node
	ClearMarkBrushes();

	--g_numNodesAllocated;
}

mapinfo_t::mapinfo_t()
{
	++g_numMapsAlloced;
}

mapinfo_t::~mapinfo_t()
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

	--g_numMapsAlloced;
}

entity_s::entity_s()
{
	++g_numEnts;
}

entity_s::~entity_s()
{
	epair_t* ep = epairs;
	while (ep)
	{
		epair_t* next = ep->next;
		delete ep;
		ep = next;
	}

	if (brushes)
	{
		for (unsigned int i = 0; i < numbrushes; ++i)
		{
			if (brushes[i])
				delete brushes[i];
		}
		free(brushes);
	}
	--g_numEnts;
}

brush_s* entity_s::CreateNewBrush()
{
	if (numbrushes == max_alloced_brushes) 
	{
		// Increase the capacity of the brush array
		const unsigned int newCapacity = max_alloced_brushes + 64;
		brush_t** newBrushArray = (brush_t**)calloc(1, sizeof(brush_s**) * newCapacity);
		if (brushes) 
		{
			memcpy(newBrushArray, brushes, sizeof(brush_s**) * numbrushes);
			free(brushes);
		}
		brushes = newBrushArray;
		max_alloced_brushes = newCapacity;
	}

	// Allocate memory for the new brush
	brush_t* newBrush = new brush_t;
	newBrush->entitynum = index;
	newBrush->brushnum = numbrushes;
	brushes[numbrushes++] = newBrush;

	return newBrush;
}

epair_s::epair_s()
{
	++g_numEPairs;
}

epair_s::~epair_s()
{
	//if (next)
	//	delete next;
	if (key)
		free(key);
	if (value)
		free(value);
	--g_numEPairs;
}