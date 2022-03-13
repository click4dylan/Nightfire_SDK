//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef ILAGCOMPENSATIONMANAGER_H
#define ILAGCOMPENSATIONMANAGER_H
#ifdef _WIN32
#pragma once
#endif

class C_BasePlayer;
class C_UserCmd;

//-----------------------------------------------------------------------------
// Purpose: This is also an IServerSystem
//-----------------------------------------------------------------------------
class ILagCompensationManager
{
public:
	// Called during player movement to set up/restore after lag compensation
	void SV_SetupMove(client_t *_host_client);
	void SV_RestoreMove(client_t *_host_client);
};

extern ILagCompensationManager *lagcompensation;

typedef struct sv_adjusted_positions_s
{
	int active;
	int needrelink;
	//vec3_t newangles;
	vec3_t neworg;
	vec3_t oldorg;
	vec3_t initial_correction_org;
	//vec3_t initial_correction_angles;
	vec3_t oldabsmin;
	vec3_t oldabsmax;
	//vec3_t oldangles;
	int deadflag;
	vec3_t temp_org;
	//vec3_t temp_angles;
	int temp_org_setflag;
} sv_adjusted_positions_t;

#define CMD_MAXBACKUP 64

#endif // ILAGCOMPENSATIONMANAGER_H
