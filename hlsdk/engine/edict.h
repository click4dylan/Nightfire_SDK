//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined EDICT_H
#define EDICT_H
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif
#define	MAX_ENT_LEAFS	64 //48 in half-life, 64 in nightfire

#include "progdefs.h"

struct edict_s
{
	qboolean	free;
	unsigned char extrafreeunusedbytes[3];
	int			serialnumber;
	link_t		area;				// linked to a division node or leaf
	
	int			headnode;			// -1 to use normal leaf check
	int			num_leafs;
	//short		leafnums[MAX_ENT_LEAFS];
	int		leafnums[MAX_ENT_LEAFS]; //int in nightfire

	float		freetime;			// sv.time when the object was freed

	void*		pvPrivateData;		// Alloced and freed by engine, used by DLLs

	entvars_t	v;					// C exported fields from progs

	// other fields from progs come immediately after
};

#endif
