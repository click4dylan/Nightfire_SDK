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
#if !defined ( R_EFXH )
#define R_EFXH
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif

// particle_t
#if !defined( PARTICLEDEFH )  
#include "particledef.h"
#endif

// BEAM
#if !defined( BEAMDEFH )
#include "beamdef.h"
#endif

// dlight_t
#if !defined ( DLIGHTH )
#include "dlight.h"
#endif

// cl_entity_t
#if !defined( CL_ENTITYH )
#include "cl_entity.h"
#endif

/*
// FOR REFERENCE, These are the built-in tracer colors.  Note, color 4 is the one
//  that uses the tracerred/tracergreen/tracerblue and traceralpha cvar settings
color24 gTracerColors[] =
{
	{ 255, 255, 255 },		// White
	{ 255, 0, 0 },			// Red
	{ 0, 255, 0 },			// Green
	{ 0, 0, 255 },			// Blue
	{ 0, 0, 0 },			// Tracer default, filled in from cvars, etc.
	{ 255, 167, 17 },		// Yellow-orange sparks
	{ 255, 130, 90 },		// Yellowish streaks (garg)
	{ 55, 60, 144 },		// Blue egon streak
	{ 255, 130, 90 },		// More Yellowish streaks (garg)
	{ 255, 140, 90 },		// More Yellowish streaks (garg)
	{ 200, 130, 90 },		// More red streaks (garg)
	{ 255, 120, 70 },		// Darker red streaks (garg)
};
*/

// Temporary entity array
#define TENTPRIORITY_LOW	0
#define TENTPRIORITY_HIGH	1

// TEMPENTITY flags
#define	FTENT_NONE				0x00000000
#define	FTENT_SINEWAVE			0x00000001
#define	FTENT_GRAVITY			0x00000002
#define FTENT_ROTATE			0x00000004
#define	FTENT_SLOWGRAVITY		0x00000008
#define FTENT_SMOKETRAIL		0x00000010
#define FTENT_COLLIDEWORLD		0x00000020
#define FTENT_FLICKER			0x00000040
#define FTENT_FADEOUT			0x00000080
#define FTENT_SPRANIMATE		0x00000100
#define FTENT_HITSOUND			0x00000200
#define FTENT_SPIRAL			0x00000400
#define FTENT_SPRCYCLE			0x00000800
#define FTENT_COLLIDEALL		0x00001000 // will collide with world and slideboxes
#define FTENT_PERSIST			0x00002000 // tent is not removed when unable to draw 
#define FTENT_COLLIDEKILL		0x00004000 // tent is removed upon collision with anything
#define FTENT_PLYRATTACHMENT	0x00008000 // tent is attached to a player (owner)
#define FTENT_SPRANIMATELOOP	0x00010000 // animating sprite doesn't die when last frame is displayed
#define FTENT_SPARKSHOWER		0x00020000
#define FTENT_NOMODEL			0x00040000 // Doesn't have a model, never try to draw ( it just triggers other things )
#define FTENT_CLIENTCUSTOM		0x00080000 // Must specify callback.  Callback function is responsible for killing tempent and updating fields ( unless other flags specify how to do things )

typedef struct tempent_s
{
	int			flags;
	float		die;
	float		frameMax;
	float		x;
	float		y;
	float		z;
	float		fadeSpeed;
	float		bounceFactor;
	int			hitSound;
	void		( *hitcallback )	( struct tempent_s *ent, struct pmtrace_s *ptr );
	void		( *callback )		( struct tempent_s *ent, float frametime, float currenttime );
	struct tempent_s	*next;
	int			priority;
	short		clientIndex;	// if attached, this is the index of the client to stick to
								// if COLLIDEALL, this is the index of the client to ignore
								// TENTS with FTENT_PLYRATTACHMENT MUST set the clientindex! 

	vec3_t		tentOffset;		// if attached, client origin + tentOffset = tent origin.
	cl_entity_t	entity;

	// baseline.origin		- velocity
	// baseline.renderamt	- starting fadeout intensity
	// baseline.angles		- angle velocity
} TEMPENTITY;

typedef struct efx_api_s efx_api_t;

#define EFFECTS_API_VERSION 1

struct efx_api_s
{
	int version; //version 1
	int size; //0xB4 bytes
	particle_t  *( *R_AllocParticle )			( void ( *callback ) ( struct particle_s *particle, float frametime ) );
	void		( *R_BreakModel )				( float const *pos, float const *size, float const *dir, float random, float life, int count, int modelIndex, char flags );
	void		( *R_Bubbles )					( float const *mins, float const *maxs, float height, int modelIndex, int count, float speed );
	void		( *R_BubbleTrail )				( float const *start, float const *end, float height, int modelIndex, int count, float speed );
	void		( *R_BulletImpactParticles )	( float const *pos );
	void		( *R_EntityParticles )			( struct cl_entity_s *ent );
	void		( *R_FlickerParticles )			( float const *org );
	void		( *R_MuzzleFlash )				( float const *pos1, int type, bool add_sparks_maybe );
	void		( *R_RicochetSound )			( float const *pos );
	void		( *R_RicochetSprite )			( float const *pos, struct model_s *pmodel, float duration, float scale );
	void		( *R_RocketFlare )				( float const *pos );
	void		( *R_RocketTrail )				( float const *start, float const *end, int type );
	void		( *R_RunParticleEffect )		( float const *org, float const *dir, int r, int g, int b, int count );
	void		( *R_ShowLine )					( float const *start, float const *end );
	void		( *R_SparkEffect )				( float const *pos, int count, int velocityMin, int velocityMax );
	void		( *R_SparkShower )				( float const *pos );
	void		( *R_SparkStreaks )				( float const *pos, int count, int velocityMin, int velocityMax );
	void		( *R_Sprite_Explode )			( TEMPENTITY *pTemp, float scale, int flags );
	void		( *R_Sprite_Smoke )				( TEMPENTITY *pTemp, float scale );
	void		( *R_Sprite_Smoke_Color )		( TEMPENTITY *pTemp, float scale, int color );
	void		( *R_Sprite_Smoke_Trail )		( float const *start, float const *end, int modelIndex, int count, int speed, float scale, float life, int random);
	void		( *R_Sprite_Spray_Lit )			( float const *pos, float const *dir, int modelIndex, int count, int speed, int iRand, int color, float scale, int brightness );//TODO: FIXME: TEST CALLING CONVENTION
	void		( *R_TracerEffect )				( float const *start, float const *end );
	particle_t *( *R_TracerParticles )			( float const *org, float const *vel, float life );
	void		( *R_UserTracerParticle )		( float const *org, float const *vel, float life, int colorIndex, float length, unsigned char deathcontext, void ( *deathfunc)( struct particle_s *particle ) );
	TEMPENTITY	*( *R_TempModel )				( float const *pos, float const *dir, float const* angles, float life, int modelIndex, int soundtype );
	TEMPENTITY	*( *R_DefaultSprite )			( float const *pos, int spriteIndex, float framerate );
	TEMPENTITY	*( *R_TempSprite )				( float const *pos, float const *dir, float scale, int modelIndex, int rendermode, int renderfx, float a, float life, int flags );
	int			( *CL_DecalIndexFromName )		( char const *name );
	void		( *R_DecalShoot )				( unsigned int textureIndex, int entity, int modelIndex, float const *position, int flags );
	BEAM		*( *R_BeamEntPoint )			( int startEnt, float const *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
	BEAM		*( *R_BeamEnts )				( int startEnt, int endEnt, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
	BEAM		*( *R_BeamLightning )			( float const *start, float const *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed );
	BEAM		*( *R_BeamPoints )				( float const *start, float const *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
	dlight_t	*( *CL_AllocDlight )			( int key );
	dlight_t	*( *CL_AllocElight )			( int key );
	TEMPENTITY	*( *CL_TempEntAlloc )			( float const *org, struct model_s *model );
	TEMPENTITY	*( *CL_TempEntAllocNoModel )	( float const *org );
	TEMPENTITY	*( *CL_TempEntAllocHigh )		( float const *org, struct model_s *model );
	TEMPENTITY	*( *CL_AllocCustomTempEntity )	( float const *origin, struct model_s *model, int high, void ( *callback ) ( struct tempent_s *ent, float frametime, float currenttime ) );
	void		 ( *R_ToggleNightvision )		( int enable );
	void		 ( *R_ToggleXrayVision )		( int enable );
	void		 ( *R_ToggleInfraredVision )	( int enable );
};

extern efx_api_t efx;

#endif
