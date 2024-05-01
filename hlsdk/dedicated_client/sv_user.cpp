#include <Windows.h>
#include <vector>
//#include <extdll.h>	
//#include <CVector.h>
//#include "CString.h"
//#include "entity.h"
//#include <gpglobals.h>
//#include <const.h>
//#include <util.h>
//#include <cbase.h>
#include <extdll.h>	
#include "globals.h"
#include "pattern_scanner.h"
#include "MinHook/MinHook.h"
#include <event_api.h>
#include "MetaHook.h"
#include <usercmd.h>
#include <cvardef.h>
#include <clientsideentity.h>
#include <NightfireFileSystem.h>
//#include <r_studioint.h>

//typedef float vec_t;
//typedef float vec2_t[2];
//typedef float vec3_t[3];
//#include <eiface.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <platformdll.h>
//#include <enginefuncs.h>
//#include <eiface.h>
#include <globalvars.h>
//#include <server_static.h>

#include <server.h>
#include "nightfire_pointers.h"
//#include <eiface.h>

extern double* host_frametime;

#define HOST_CLIENT (*g_Pointers.host_client)

void SV_EstablishTimeBase(client_t *cl, usercmd_t *cmds, int dropped, int numbackup, int numcmds)
{
	g_Pointers.g_EntityInterface->pfnPM_Move(nullptr, TRUE);

	int		i;
	double	runcmd_time = 0.0;
	double	time_at_end = 0.0;
	constexpr int MAX_DROPPED_CMDS = 24;

	// If we haven't dropped too many packets, then run some commands
	if (dropped < MAX_DROPPED_CMDS)
	{
		if (dropped > numbackup)
		{
			// Con_Printf("%s: lost %i cmds\n", __func__, dropped - numbackup);
		}

		int droppedcmds = dropped;

		// Run the last known cmd for each dropped cmd we don't have a backup for
		while (droppedcmds > numbackup)
		{
			runcmd_time += cl->lastcmd.msec / 1000.0;
			droppedcmds--;
		}

		// Now run the "history" commands if we still have dropped packets
		while (droppedcmds > 0)
		{
			int cmdnum = numcmds + droppedcmds - 1;
			runcmd_time += cmds[cmdnum].msec / 1000.0;
			droppedcmds--;
		}
	}

	// Now run any new command(s). Go backward because the most recent command is at index 0
	for (i = numcmds - 1; i >= 0; i--)
	{
		time_at_end += cmds[i].msec / 1000.0;
	}

	cl->svtimebase = *host_frametime + g_Pointers.g_psv->time - (time_at_end + runcmd_time);
}

#if 0
void SV_RunCmd(usercmd_t* ucmd, int random_seed)
{
	usercmd_t cmd = *ucmd;
	int i;
	edict_t* ent;
	trace_t trace;
	float frametime;

	if (HOST_CLIENT->ignorecmdtime > *g_Pointers.realtime)
	{
		HOST_CLIENT->cmdtime = (double)ucmd->msec / 1000.0 + HOST_CLIENT->cmdtime;
		return;
	}


	HOST_CLIENT->ignorecmdtime = 0;
	if (cmd.msec > 50)
	{
		cmd.msec = (byte)(ucmd->msec / 2.0);
		SV_RunCmd(&cmd, random_seed);
		cmd.msec = (byte)(ucmd->msec / 2.0);
		cmd.impulse = 0;
		SV_RunCmd(&cmd, random_seed);
		return;
	}


	if (!HOST_CLIENT->fakeclient)
		SV_SetupMove(HOST_CLIENT);

#ifdef REHLDS_FIXES
	if (sv_usercmd_custom_random_seed.value)
	{
		float fltTimeNow = float(Sys_FloatTime() * 1000.0);
		random_seed = *reinterpret_cast<int*>((char*)&fltTimeNow);
	}
#endif
	
	gEntityInterface.pfnCmdStart(sv_player, ucmd, random_seed);
	frametime = float(ucmd->msec * 0.001);
	HOST_CLIENT->svtimebase = frametime + HOST_CLIENT->svtimebase;
	HOST_CLIENT->cmdtime = ucmd->msec / 1000.0 + HOST_CLIENT->cmdtime;
	if (ucmd->impulse)
	{
		sv_player->v.impulse = ucmd->impulse;

		// Disable fullupdate via impulse 204
#ifndef REHLDS_FIXES
		if (ucmd->impulse == 204)
			SV_ForceFullClientsUpdate();
#endif // REHLDS_FIXES
	}
	sv_player->v.clbasevelocity[0] = 0;
	sv_player->v.clbasevelocity[1] = 0;
	sv_player->v.clbasevelocity[2] = 0;
	sv_player->v.button = ucmd->buttons;
#ifdef REHLDS_FIXES
	sv_player->v.light_level = ucmd->lightlevel;
#endif
	SV_CheckMovingGround(sv_player, frametime);
	pmove->oldangles[0] = sv_player->v.v_angle[0];
	pmove->oldangles[1] = sv_player->v.v_angle[1];
	pmove->oldangles[2] = sv_player->v.v_angle[2];
	if (!sv_player->v.fixangle)
	{
		sv_player->v.v_angle[0] = ucmd->viewangles[0];
		sv_player->v.v_angle[1] = ucmd->viewangles[1];
		sv_player->v.v_angle[2] = ucmd->viewangles[2];
	}
	SV_PlayerRunPreThink(sv_player, (float)HOST_CLIENT->svtimebase);
	SV_PlayerRunThink(sv_player, frametime, HOST_CLIENT->svtimebase);
	if (Length(sv_player->v.basevelocity) > 0.0)
	{
		sv_player->v.clbasevelocity[0] = sv_player->v.basevelocity[0];
		sv_player->v.clbasevelocity[1] = sv_player->v.basevelocity[1];
		sv_player->v.clbasevelocity[2] = sv_player->v.basevelocity[2];
	}

	pmove->server = TRUE;
	pmove->multiplayer = (g_Pointers.psvs->maxclients > 1) ? TRUE : FALSE;
	pmove->time = float(1000.0 * HOST_CLIENT->svtimebase);
	pmove->usehull = (sv_player->v.flags & FL_DUCKING) == FL_DUCKING;
	pmove->maxspeed = sv_maxspeed.value;
	pmove->clientmaxspeed = sv_player->v.maxspeed;
	pmove->flDuckTime = (float)sv_player->v.flDuckTime;
	pmove->bInDuck = sv_player->v.bInDuck;
	pmove->flTimeStepSound = sv_player->v.flTimeStepSound;
	pmove->iStepLeft = sv_player->v.iStepLeft;
	pmove->flFallVelocity = sv_player->v.flFallVelocity;
	pmove->flSwimTime = (float)sv_player->v.flSwimTime;
	pmove->oldbuttons = sv_player->v.oldbuttons;

	strncpy(pmove->physinfo, HOST_CLIENT->physinfo, sizeof(pmove->physinfo) - 1);
	pmove->physinfo[sizeof(pmove->physinfo) - 1] = 0;

	pmove->velocity[0] = sv_player->v.velocity[0];
	pmove->velocity[1] = sv_player->v.velocity[1];
	pmove->velocity[2] = sv_player->v.velocity[2];

	pmove->movedir[0] = sv_player->v.movedir[0];
	pmove->movedir[1] = sv_player->v.movedir[1];
	pmove->movedir[2] = sv_player->v.movedir[2];

	pmove->angles[0] = sv_player->v.v_angle[0];
	pmove->angles[1] = sv_player->v.v_angle[1];
	pmove->angles[2] = sv_player->v.v_angle[2];

	pmove->basevelocity[0] = sv_player->v.basevelocity[0];
	pmove->basevelocity[1] = sv_player->v.basevelocity[1];
	pmove->basevelocity[2] = sv_player->v.basevelocity[2];

	pmove->view_ofs[0] = sv_player->v.view_ofs[0];
	pmove->view_ofs[1] = sv_player->v.view_ofs[1];
	pmove->view_ofs[2] = sv_player->v.view_ofs[2];

	pmove->punchangle[0] = sv_player->v.punchangle[0];
	pmove->punchangle[1] = sv_player->v.punchangle[1];
	pmove->punchangle[2] = sv_player->v.punchangle[2];

	pmove->deadflag = sv_player->v.deadflag;
	pmove->effects = sv_player->v.effects;
	pmove->gravity = sv_player->v.gravity;
	pmove->friction = sv_player->v.friction;
	pmove->spectator = 0;
	pmove->waterjumptime = sv_player->v.teleport_time;

	memcpy(&pmove->cmd, &cmd, sizeof(pmove->cmd));

	pmove->dead = sv_player->v.health <= 0.0;
	pmove->movetype = sv_player->v.movetype;
	pmove->flags = sv_player->v.flags;
	pmove->player_index = NUM_FOR_EDICT(sv_player) - 1;

	pmove->iuser1 = sv_player->v.iuser1;
	pmove->iuser2 = sv_player->v.iuser2;
	pmove->iuser3 = sv_player->v.iuser3;
	pmove->iuser4 = sv_player->v.iuser4;
	pmove->fuser1 = sv_player->v.fuser1;
	pmove->fuser2 = sv_player->v.fuser2;
	pmove->fuser3 = sv_player->v.fuser3;
	pmove->fuser4 = sv_player->v.fuser4;

	pmove->vuser1[0] = sv_player->v.vuser1[0];
	pmove->vuser1[1] = sv_player->v.vuser1[1];
	pmove->vuser1[2] = sv_player->v.vuser1[2];

	pmove->vuser2[0] = sv_player->v.vuser2[0];
	pmove->vuser2[1] = sv_player->v.vuser2[1];
	pmove->vuser2[2] = sv_player->v.vuser2[2];

	pmove->vuser3[0] = sv_player->v.vuser3[0];
	pmove->vuser3[1] = sv_player->v.vuser3[1];
	pmove->vuser3[2] = sv_player->v.vuser3[2];

	pmove->vuser4[0] = sv_player->v.vuser4[0];
	pmove->vuser4[1] = sv_player->v.vuser4[1];
	pmove->vuser4[2] = sv_player->v.vuser4[2];

	pmove->origin[0] = sv_player->v.origin[0];
	pmove->origin[1] = sv_player->v.origin[1];
	pmove->origin[2] = sv_player->v.origin[2];

	SV_AddLinksToPM(sv_areanodes, pmove->origin);

	pmove->frametime = frametime;
	pmove->runfuncs = TRUE;
	pmove->PM_PlaySound = PM_SV_PlaySound;
	pmove->PM_TraceTexture = PM_SV_TraceTexture;
	pmove->PM_PlaybackEventFull = PM_SV_PlaybackEventFull;

	g_Pointers.gEntityInterface->pfnPM_Move(pmove, TRUE);

	sv_player->v.deadflag = pmove->deadflag;
	sv_player->v.effects = pmove->effects;
	sv_player->v.teleport_time = pmove->waterjumptime;
	sv_player->v.waterlevel = pmove->waterlevel;
	sv_player->v.watertype = pmove->watertype;
	sv_player->v.flags = pmove->flags;
	sv_player->v.friction = pmove->friction;
	sv_player->v.movetype = pmove->movetype;
	sv_player->v.maxspeed = pmove->clientmaxspeed;
	sv_player->v.iStepLeft = pmove->iStepLeft;

	sv_player->v.view_ofs[0] = pmove->view_ofs[0];
	sv_player->v.view_ofs[1] = pmove->view_ofs[1];
	sv_player->v.view_ofs[2] = pmove->view_ofs[2];

	sv_player->v.movedir[0] = pmove->movedir[0];
	sv_player->v.movedir[1] = pmove->movedir[1];
	sv_player->v.movedir[2] = pmove->movedir[2];

	sv_player->v.punchangle[0] = pmove->punchangle[0];
	sv_player->v.punchangle[1] = pmove->punchangle[1];
	sv_player->v.punchangle[2] = pmove->punchangle[2];

	if (pmove->onground == -1)
	{
		sv_player->v.flags &= ~FL_ONGROUND;
	}
	else
	{
		sv_player->v.flags |= FL_ONGROUND;
		sv_player->v.groundentity = EDICT_NUM(pmove->physents[pmove->onground].info);
	}

	sv_player->v.origin[0] = pmove->origin[0];
	sv_player->v.origin[1] = pmove->origin[1];
	sv_player->v.origin[2] = pmove->origin[2];

	sv_player->v.velocity[0] = pmove->velocity[0];
	sv_player->v.velocity[1] = pmove->velocity[1];
	sv_player->v.velocity[2] = pmove->velocity[2];

	sv_player->v.basevelocity[0] = pmove->basevelocity[0];
	sv_player->v.basevelocity[1] = pmove->basevelocity[1];
	sv_player->v.basevelocity[2] = pmove->basevelocity[2];

	if (!sv_player->v.fixangle)
	{
		sv_player->v.v_angle[0] = pmove->angles[0];
		sv_player->v.v_angle[1] = pmove->angles[1];
		sv_player->v.v_angle[2] = pmove->angles[2];
		sv_player->v.angles[0] = float(-pmove->angles[0] / 3.0);
		sv_player->v.angles[1] = pmove->angles[1];
		sv_player->v.angles[2] = pmove->angles[2];
	}

	sv_player->v.bInDuck = pmove->bInDuck;
	sv_player->v.flDuckTime = (int)pmove->flDuckTime;
	sv_player->v.flTimeStepSound = pmove->flTimeStepSound;
	sv_player->v.flFallVelocity = pmove->flFallVelocity;
	sv_player->v.flSwimTime = (int)pmove->flSwimTime;
	sv_player->v.oldbuttons = pmove->cmd.buttons;

	sv_player->v.iuser1 = pmove->iuser1;
	sv_player->v.iuser2 = pmove->iuser2;
	sv_player->v.iuser3 = pmove->iuser3;
	sv_player->v.iuser4 = pmove->iuser4;
	sv_player->v.fuser1 = pmove->fuser1;
	sv_player->v.fuser2 = pmove->fuser2;
	sv_player->v.fuser3 = pmove->fuser3;
	sv_player->v.fuser4 = pmove->fuser4;

	sv_player->v.vuser1[0] = pmove->vuser1[0];
	sv_player->v.vuser1[1] = pmove->vuser1[1];
	sv_player->v.vuser1[2] = pmove->vuser1[2];

	sv_player->v.vuser2[0] = pmove->vuser2[0];
	sv_player->v.vuser2[1] = pmove->vuser2[1];
	sv_player->v.vuser2[2] = pmove->vuser2[2];

	sv_player->v.vuser3[0] = pmove->vuser3[0];
	sv_player->v.vuser3[1] = pmove->vuser3[1];
	sv_player->v.vuser3[2] = pmove->vuser3[2];

	sv_player->v.vuser4[0] = pmove->vuser4[0];
	sv_player->v.vuser4[1] = pmove->vuser4[1];
	sv_player->v.vuser4[2] = pmove->vuser4[2];

	SetMinMaxSize(sv_player, player_mins[pmove->usehull], player_maxs[pmove->usehull], 0);
	if (HOST_CLIENT->edict->v.solid)
	{
		SV_LinkEdict(sv_player, TRUE);
		vec3_t vel;

		vel[0] = sv_player->v.velocity[0];
		vel[1] = sv_player->v.velocity[1];
		vel[2] = sv_player->v.velocity[2];
		for (i = 0; i < pmove->numtouch; ++i)
		{
			pmtrace_t* tr = &pmove->touchindex[i];
			ent = EDICT_NUM(pmove->physents[tr->ent].info);
			SV_ConvertPMTrace(&trace, tr, ent);
			sv_player->v.velocity[0] = tr->deltavelocity[0];
			sv_player->v.velocity[1] = tr->deltavelocity[1];
			sv_player->v.velocity[2] = tr->deltavelocity[2];
			SV_Impact(ent, sv_player, &trace);
		}
		sv_player->v.velocity[0] = vel[0];
		sv_player->v.velocity[1] = vel[1];
		sv_player->v.velocity[2] = vel[2];
	}

	gGlobalVariables.time = (float)HOST_CLIENT->svtimebase;
	gGlobalVariables.frametime = frametime;
	gEntityInterface.pfnPlayerPostThink(sv_player);
	gEntityInterface.pfnCmdEnd(sv_player);

	if (!HOST_CLIENT->fakeclient)
		SV_RestoreMove(HOST_CLIENT);
}
#endif