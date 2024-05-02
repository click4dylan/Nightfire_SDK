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
// cl_entity.h
#if !defined( CL_ENTITYH )
#define CL_ENTITYH
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif

typedef struct efrag_s
{
	struct mleaf_s		*leaf;
	struct efrag_s		*leafnext;
	struct cl_entity_s	*entity;
	struct efrag_s		*entnext;
} efrag_t;

typedef struct
{
	byte					mouthopen;		// 0 = mouth closed, 255 = mouth agape
	byte					sndcount;		// counter for running average
	int						sndavg;			// running average
} mouth_t;

typedef struct
{
	float					prevanimtime;  //2924
	float					sequencetime; //2928
	byte					prevblending[2]; //2932
	byte					prevseqblending[2]; //2934
	vec3_t					prevorigin; //2936
	vec3_t					prevangles; //2948
	byte					prevcontroller[4]; //2960
	int						prevsequence; //2964
	float					prevframe; //2968
} latchedvars_t;

typedef struct
{
	// Time stamp for this movement
	float					animtime;

	vec3_t					origin;
	vec3_t					angles;
} position_history_t;

typedef struct cl_entity_s cl_entity_t;

#define HISTORY_MAX		64  // Must be power of 2
#define HISTORY_MASK	( HISTORY_MAX - 1 )


#if !defined( ENTITY_STATEH )
#include "entity_state.h"
#endif

#if !defined( PROGS_H )
#include "progs.h"
#endif

struct cl_entity_s
{
	int						index;      // Index into cl_entities ( should match actual slot, but not necessarily )

	qboolean				player;     // True if this entity is a "player"

	char gearbox_unknown[4]; //8
	vec3_t origin; //12
	vec3_t angles; //24
	float unknown2;
	struct model_s* model; //40
	char gearbox_unknown2[20]; //44


	entity_state_t			baseline;  //64 //28 dec // The original state from which to delta during an uncompressed message
	entity_state_t			prevstate; //416 // The state information from the penultimate message received from the server
	entity_state_t			curstate; //768  // The state information from the last message received from server
	int						current_position; //1120 // Last received history update index

	position_history_t		ph[HISTORY_MAX];   // History of position and angle updates for this player

	mouth_t					mouth;			// For synchronizing mouth movements.

	latchedvars_t			latched;		// Variables used by studio model rendering routines

	// Information based on interplocation, extrapolation, prediction, or just copied from last msg received.
	//
	//float					lastmove;

	// Actual render position and angles
	//vec3_t					angles; //796

	// Attachment points
	vec3_t					attachment[4]; //2972

	// Other entity local information
	int						trivial_accept;

	//struct model_s* model;			// cl.model_precache[ curstate.modelindes ];  all visible entities have a model
	struct efrag_s* efrag;			// linked list of efrags
	struct mnode_s* topnode;		// for bmodels, first world node that splits bmodel, or NULL if not split

	float					syncbase;		// for client-side animations -- used by obsolete alias animation system, remove?
	int						visframe;		// last frame this entity was found in an active leaf
	colorVec				cvFloorColor;
	char pad2[280];
};
//size: 3336

#endif // !CL_ENTITYH
