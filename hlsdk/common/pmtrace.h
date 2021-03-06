/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#if !defined( PMTRACEH )
#define PMTRACEH
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif

typedef struct
{
	vec3_t	normal;
	float	dist;
} pmplane_t;

typedef struct pmtrace_s pmtrace_t;

struct pmtrace_s
{
	qboolean allsolid; // if true, plane is not valid
	qboolean startsolid; // if true, the initial point was in a solid area
	qboolean inopen; // End point is in empty space or in water
	qboolean inwater;
	int contents;
	float fraction;  // time completed, 1.0 = didn't hit anything
	vec3_t endpos;  // final position
	msurface_t* surface;
	pmplane_t plane; // surface normal at impact
	const char* sztexturename;
	int surfaceflags;
	int ent;  // entity at impact
	int hitgroup; // Only run on server.
	vec3_t deltavelocity; // Change in player's velocity caused by impact.
};

#endif
