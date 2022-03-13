#include "player_lagcompensation.h"
#include "ilagcompensationmanager.h"
#include "utlvector.h"
#include "bitvec.h"
//#include "../../plugins/include/hlsdk_const.inc"

//HOOKS
DWORD tempEAX;
DWORD tempEBX;
DWORD tempECX;
DWORD tempESI;
DWORD tempEDX;
DWORD tempEDI;
DWORD *tempRegister;
DWORD *tempRegister2;
DWORD *tempRegister3;
DWORD *tempRegister4;
float tempFloat;
#define sv_maxunlag		(0x449C9A70)
#define sv_unlag		(0x449CA0E8)
#define sv_unlagpush	(0x449C9EA8)
#define sv_unlagsamples (0x449C9A94)

//todo: sv_computelatency (sv_unlagsamples)


sv_adjusted_positions_t truepositions[MAX_CLIENTS];
BOOL nofind;


class CLagCompensationManager
{
public:
	// called after entities think
	virtual void FrameUpdatePostEntityThink();

	// ILagCompensationManager stuff

	int SV_UnlagCheckTeleport(vec_t *v1, vec_t *v2);
	// Called during player movement to set up/restore after lag compensation
	void SV_SetupMove(client_t *_host_client);
	void SV_RestoreMove(client_t *_host_client);

private:
	float			GetLatency( C_BasePlayer *player );
};

static CLagCompensationManager g_LagCompensationManager;
//ILagCompensationManager *lagcompensation = &g_LagCompensationManager; //dylan fix

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *player - 
// Output : float CLagCompensation::GetLatency
//-----------------------------------------------------------------------------
float CLagCompensationManager::GetLatency ( C_BasePlayer *player )
{
	int backtrack = CVAR_GETDIRECTINT(sv_unlagsamples);
	backtrack = max( 1,  backtrack );
	// Clamp to backup, or 16 at most
	backtrack = min( backtrack, gpGlobals->maxClients <= 1 ? 4 : 16 );

	float ping = GetPlayerPing( player->entindex(), backtrack ); 
	return ping;
}

//-----------------------------------------------------------------------------
// Purpose: Called once per frame after all entities have had a chance to think
//-----------------------------------------------------------------------------
void CLagCompensationManager::FrameUpdatePostEntityThink()
{
}

//-----------------------------------------------------------------------------
// Purpose: Simple linear interpolation
// Input  : frac - 
//			src - 
//			dest - 
//			output - 
//-----------------------------------------------------------------------------
static void InterpolateVector( float frac, const Vector& src, const Vector& dest, Vector& output )
{
	int i;

	for ( i = 0; i < 3; i++ )
	{
		output[ i ] = src[ i ] + frac * ( dest[ i ] - src[ i ] );
	}
}
/*
static void InterpolateAngles( float frac, const QAngle& src, const QAngle& dest, QAngle& output )
{
	int i;

	for ( i = 0; i < 3; i++ )
	{
		output[ i ] = src[ i ] + frac * ( dest[ i ] - src[ i ] );
	}
}
*/

int CLagCompensationManager::SV_UnlagCheckTeleport(vec_t *v1, vec_t *v2)
{
	for (int i = 0; i < 3; i++)
	{
		if (fabs(v1[i] - v2[i]) > 128)
			return 1;
	}

	return 0;
}

entity_state_t *SV_FindEntInPack(int index, packet_entities_t *pack)
{
	if (pack->num_entities <= 0)
		return NULL;

	for (int i = 0; i < pack->num_entities; i++)
	{
		if (pack->entities[i].number == index)
			return &pack->entities[i];
	}

	return NULL;
}

void SV_ComputeLatency(client_t *cl)
{
	cl->latency = SV_CalcClientTime(cl);
}

float SV_CalcClientTime(client_t *cl)
{
	float minping;
	float maxping;
	int backtrack;

	float ping = 0.0;
	int count = 0;
	backtrack = CVAR_GETDIRECTINT(sv_unlagsamples);

	if (backtrack < 1)
		backtrack = 1;

	int iSV_UPDATE_BACKUP = SV_UPDATE_BACKUP;
	int iSV_UPDATE_MASK = SV_UPDATE_MASK;

	if (backtrack >= (iSV_UPDATE_BACKUP <= 16 ? iSV_UPDATE_BACKUP : 16))
		backtrack = (iSV_UPDATE_BACKUP <= 16 ? iSV_UPDATE_BACKUP : 16);

	if (backtrack <= 0)
		return 0.0f;

	for (int i = 0; i < backtrack; i++)
	{
		client_frame_t *frame = &cl->frames[iSV_UPDATE_MASK & (cl->netchan.incoming_acknowledged - i)];
		if (frame->ping_time <= 0.0f)
			continue;

		++count;
		ping += frame->ping_time;
	}

	if (!count)
		return 0.0f;

	minping = 9999.0;
	maxping = -9999.0;
	ping /= count;

	for (int i = 0; i < (iSV_UPDATE_BACKUP <= 4 ? iSV_UPDATE_BACKUP : 4); i++)
	{
		client_frame_t *frame = &cl->frames[iSV_UPDATE_MASK & (cl->netchan.incoming_acknowledged - i)];
		if (frame->ping_time <= 0.0f)
			continue;

		if (frame->ping_time < minping)
			minping = frame->ping_time;

		if (frame->ping_time > maxping)
			maxping = frame->ping_time;
	}

	if (maxping < minping || fabs(maxping - minping) <= 0.2)
		return ping;

	return 0.0f;
}

// Called during player movement to set up/restore after lag compensation
void CLagCompensationManager::SV_SetupMove(client_t *_host_client)
{
	/*struct*/ client_t *cl;
	double cl_interptime;
	client_frame_t *nextFrame;
	entity_state_t *state;
	sv_adjusted_positions_t *pos;
	float frac;
	entity_state_t *pnextstate;
	int i;
	client_frame_t *frame;
	vec3_t origin;
	vec3_t delta;
	//vec3_t angles;
	//vec3_t deltaangles;

	double targettime;

	Q_memset(truepositions, 0, sizeof(truepositions));
	nofind = 1;
	
	//if (!gEntityInterface.pfnAllowLagCompensation())
		//return;

	if (CVAR_GETDIRECTFLOAT(sv_maxunlag) == 0.0f || !_host_client->lw || !_host_client->lc)
		return;

	if (gpGlobals->maxClients <= 1 || !_host_client->active)
		return;

	nofind = 0;
	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		cl = GetClientPointerFromIndex(i); //&g_psvs.clients[i];
		if (cl == _host_client || !cl->active)
			continue;

		truepositions[i].oldorg[0] = cl->edict->v.origin[0];
		truepositions[i].oldorg[1] = cl->edict->v.origin[1];
		truepositions[i].oldorg[2] = cl->edict->v.origin[2];
		truepositions[i].oldabsmin[0] = cl->edict->v.absmin[0];
		truepositions[i].oldabsmin[1] = cl->edict->v.absmin[1];
		truepositions[i].oldabsmin[2] = cl->edict->v.absmin[2];
		truepositions[i].oldabsmax[0] = cl->edict->v.absmax[0];
		truepositions[i].oldabsmax[1] = cl->edict->v.absmax[1];
		truepositions[i].active = 1;
		truepositions[i].oldabsmax[2] = cl->edict->v.absmax[2];
		/*
		truepositions[i].oldangles[0] = cl->edict->v.angles[0];
		truepositions[i].oldangles[1] = cl->edict->v.angles[1];
		truepositions[i].oldangles[2] = cl->edict->v.angles[2];*/
	}

	float clientLatency = _host_client->latency - _host_client->next_messageinterval;
	if (clientLatency > 1.5)
		clientLatency = 1.5f;

	if (CVAR_GETDIRECTFLOAT(sv_maxunlag) != 0.0f)
	{
		if (CVAR_GETDIRECTFLOAT(sv_maxunlag) < 0.0)
			CVAR_SETDIRECTFLOAT(sv_maxunlag, 0.0f);

		if (clientLatency >= CVAR_GETDIRECTFLOAT(sv_maxunlag))
			clientLatency = CVAR_GETDIRECTFLOAT(sv_maxunlag);
	}

	cl_interptime = _host_client->localtime; //lastcmd.lerp_msec / 1000.0f;
	
	bool forced = false;

	if (cl_interptime > 0.1f)
	{
		cl_interptime = 0.1f;
		forced = true;
	}

	if (_host_client->next_messageinterval > cl_interptime)
	{
		cl_interptime = _host_client->next_messageinterval;
		forced = true;
	}

	PlayerInfo *pl = &plinfo[ENTINDEX(_host_client->edict)];

	if (forced && gpGlobals->time - pl->fLastForceInterp > 1.0f)
	{
		char adjustedinterp[128];
		sprintf(adjustedinterp, "ex_interp %.6f;echo ex_interp forced to %.6f\n", cl_interptime, cl_interptime);
		CLIENT_COMMAND(_host_client->edict, adjustedinterp);
		pl->fLastForceInterp = gpGlobals->time;
	}

	float pushtime = CVAR_GETDIRECTFLOAT(sv_unlagpush);
	double time = realtime;

	usercmd_t* cmd = CURRENT_USER_CMD;
#if 1
	if (cmd && cmd->impact_index)
	{
		float clientTime = cmd->impact_position[1]; //use _host_client->svtimebase as alternative?
		float clientFrameTime = cmd->impact_position[0];
		double realLatency = gpGlobals->time - clientTime;
		double newtargettime = time - realLatency - cl_interptime + pushtime + _host_client->next_messageinterval + clientFrameTime;
		double correct = clientLatency + cl_interptime + pushtime;
		// calc difference between tick send by player and our latency based tick
		double deltaTime = correct - (time - newtargettime);
		if (fabs(deltaTime) > 0.2)
		{
			// difference between cmd time and latency is too big > 200ms, use time correction based on latency
			targettime = time - clientLatency - cl_interptime + pushtime;
		}
		else
		{
			targettime = newtargettime;
		}
	}
	else
#endif
	{
		targettime = time - clientLatency - cl_interptime + pushtime;
	}
	if (targettime > time)
		targettime = time;

	int iSV_UPDATE_BACKUP = SV_UPDATE_BACKUP;
	int iSV_UPDATE_MASK = SV_UPDATE_MASK;

	if (iSV_UPDATE_BACKUP <= 0)
	{
		Q_memset(truepositions, 0, sizeof(truepositions));
		nofind = 1;
		return;
	}

	frame = nextFrame = NULL;
	for (i = 0; i < iSV_UPDATE_BACKUP; i++, frame = nextFrame)
	{
		nextFrame = &_host_client->frames[iSV_UPDATE_MASK & (_host_client->netchan.outgoing_sequence + ~i)];
		for (int j = 0; j < nextFrame->entities.num_entities; j++)
		{
			state = &nextFrame->entities.entities[j];

			if (state->number <= 0)
				continue;

			if (state->number > gpGlobals->maxClients)
				break;

			//if (state->number <= 0 || state->number > gpGlobals->maxClients) //fix above
				//continue;

			pos = &truepositions[state->number - 1];
			if (pos->deadflag)
				continue;


			if (state->health <= 0)
				pos->deadflag = 1;

			if (state->effects & EF_NOINTERP)
				pos->deadflag = 1;

			if (pos->temp_org_setflag)
			{
				if (SV_UnlagCheckTeleport(state->origin, pos->temp_org))
					pos->deadflag = 1;
			}
			else
			{
				pos->temp_org_setflag = 1;
			}

			pos->temp_org[0] = state->origin[0];
			pos->temp_org[1] = state->origin[1];
			pos->temp_org[2] = state->origin[2];
			/*pos->temp_angles[0] = state->angles[0];
			pos->temp_angles[0] = state->angles[1];
			pos->temp_angles[0] = state->angles[2];*/
		}

		if (targettime > nextFrame->senttime)
			break;
	}

	if (i >= iSV_UPDATE_BACKUP || targettime - nextFrame->senttime > 1.0)
	{
		Q_memset(truepositions, 0, 0xB00u);
		nofind = 1;
		return;
	}

	if (frame)
	{
		float timeDiff = float(frame->senttime - nextFrame->senttime);
		if (timeDiff == 0.0)
			frac = 0.0;
		else
		{
			frac = float((targettime - nextFrame->senttime) / timeDiff);
			if (frac <= 1.0)
			{
				if (frac < 0.0)
					frac = 0.0;
			}
			else
				frac = 1.0;
		}
	}
	else
	{
		frame = nextFrame;
		frac = 0.0;
	}

	for (i = 0; i < nextFrame->entities.num_entities; i++)
	{
		state = &nextFrame->entities.entities[i];
		//if (state->number <= 0 || state->number > gpGlobals->maxClients)  //fix below
			//continue;
		if (state->number <= 0)
			continue;

		if (state->number > gpGlobals->maxClients)
			break;

		cl = GetClientPointerFromIndex(state->number - 1);//&g_psvs.clients[state->number - 1];
		if (cl == _host_client || !cl->active)
			continue;

		pos = &truepositions[state->number - 1];
		if (pos->deadflag)
			continue;

		if (!pos->active)
		{
			Warning("tried to store off position of bogus player %i/%s\n", i, cl->name);
			continue;
		}

		pnextstate = SV_FindEntInPack(state->number, &frame->entities);

		if (pnextstate)
		{
			delta[0] = pnextstate->origin[0] - state->origin[0];
			delta[1] = pnextstate->origin[1] - state->origin[1];
			delta[2] = pnextstate->origin[2] - state->origin[2];
			VectorMA(state->origin, frac, delta, origin);
			/*deltaangles[0] = pnextstate->angles[0] - state->angles[0];
			deltaangles[1] = pnextstate->angles[1] - state->angles[1];
			deltaangles[2] = pnextstate->angles[2] - state->angles[2];
			VectorMA(state->angles, frac, deltaangles, angles);*/
		}
		else
		{
			origin[0] = state->origin[0];
			origin[1] = state->origin[1];
			origin[2] = state->origin[2];
			/*angles[0] = state->angles[0];
			angles[1] = state->angles[1];
			angles[2] = state->angles[2];*/
		}
		pos->neworg[0] = origin[0];
		pos->neworg[1] = origin[1];
		pos->neworg[2] = origin[2];
		/*pos->newangles[0] = angles[0];
		pos->newangles[1] = angles[1];
		pos->newangles[2] = angles[2];*/
		pos->initial_correction_org[0] = origin[0];
		pos->initial_correction_org[1] = origin[1];
		pos->initial_correction_org[2] = origin[2];
		/*pos->initial_correction_angles[0] = angles[0];
		pos->initial_correction_angles[1] = angles[1];
		pos->initial_correction_angles[2] = angles[2];*/
		bool change = false;
		/*
		if (!VectorCompare(angles, cl->edict->v.angles))
		{
			change = true;
			cl->edict->v.angles[0] = angles[0];
			cl->edict->v.angles[1] = angles[1];
			cl->edict->v.angles[2] = angles[2];
		}*/
		if (!VectorCompareWithPrecision(origin, cl->edict->v.origin, 0.001f))
		{
			//check to see if backtracking will cause the player to get stuck
			bool stuck = false;

			//int hullNumber = human_hull;
			//if ( cl->edict->v.flags & FL_DUCKING )
			//	hullNumber = head_hull;

			//TraceResult tr;
			//g_engfuncs.pfnTraceHull(origin, origin, 0, hullNumber, 0, cl->edict, &tr);
			//if (tr.fStartSolid)
			{
				//Warning("backtracking player would get stuck #1 %i/%s\n", i, cl->name);
				stuck = true;
			}
			//else
			{
				/*
				g_engfuncs.pfnTraceModel(origin, origin, hullNumber, 0, _host_client->edict, &tr);

				if (tr.fStartSolid)
				{
					Warning("backtracking player would get stuck #3 %i/%s\n", i, cl->name);
					stuck = true;
				}
				else
				*/
				{
					//do dirty positional check..
					//float dist = (origin - _host_client->edict->v.origin).Length();
					//if (dist <= 40.0f)
					{
					//	Warning("backtracking player would get stuck #2 %i/%s\n", i, cl->name);
					//	stuck = true;
					}
				}
			}

			//if (stuck)
			//{
			//	pos->neworg[0] = pos->oldorg[0];
			//	pos->neworg[1] = pos->oldorg[1];
			//	pos->neworg[2] = pos->oldorg[2];
			//	pos->initial_correction_org[0] = pos->oldorg[0];
			//	pos->initial_correction_org[1] = pos->oldorg[1];
			//	pos->initial_correction_org[2] = pos->oldorg[2];
			//}
			//else
			//{
				//not stuck, backtrack away
				cl->edict->v.origin[0] = origin[0];
				cl->edict->v.origin[1] = origin[1];
				cl->edict->v.origin[2] = origin[2];
				SV_LinkEdict(cl->edict, 0);
				pos->needrelink = 1;
			//}
		}
	}
}

#define M_PI		3.14159265358979323846f
#define M_RADPI		57.295779513082f
#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI_F) )
#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI_F / 180.f) )

void VectorAngles(const Vector& forward, Vector &angles)
{
	float yaw, pitch;

	if (forward.y == 0 && forward.x == 0)
	{
		yaw = 0;
		pitch = float((forward.z > 0) ? 270 : 90);
	}
	else
	{
		yaw = RAD2DEG(atan2(forward.y, forward.x));

		if (yaw < 0.f) yaw += 360.f;

		pitch = RAD2DEG(atan2(-forward.z, forward.Length2D()));

		if (pitch < 0.f) pitch += 360.f;
	}

	angles.x = pitch;
	angles.y = yaw;
	angles.z = 0;
}

void FixPlayerStuck(int i, client_t *stuck_player)
{
	return;

	if (stuck_player->edict->v.movetype == MOVETYPE_NOCLIP
		/*|| stuck_player->edict->v.velocity.Length() >= 0.1f*/)
		return;

	//see if player final position is stuck, if so then try to fix their position
	int hullNumber = human_hull;
	if ( stuck_player->edict->v.flags & FL_DUCKING )
		hullNumber = head_hull;

	Vector vecDir;
	Vector vecUp;

	edict_t *entity_in_sphere = nullptr;
	edict_t *nearest_player = nullptr;
	edict_t *nearest_platform = nullptr;
	float closest_dist = FLT_MAX;
	float closest_platform_dist = FLT_MAX;

	while ((entity_in_sphere = g_engfuncs.pfnFindEntityInSphere(entity_in_sphere, stuck_player->edict->v.origin, 50.0f)) != nullptr)
	{
		if (FNullEnt(entity_in_sphere))
			break;

		if (entity_in_sphere != stuck_player->edict)
		{
			entvars_t *v = &entity_in_sphere->v;

			if (FClassnameIs(v, "player"))
			{
				float dist = (entity_in_sphere->v.origin - stuck_player->edict->v.origin).Length();
				if (dist < closest_dist)
				{
					nearest_player = entity_in_sphere;
					closest_dist = dist;
				}
			}
			else if (FClassnameIs(v, "func_train") || FClassnameIs(v, "func_rotating") || FClassnameIs(v, "func_door") || FClassnameIs(v, "func_door_rotating"))
			{
				float dist = (entity_in_sphere->v.origin - stuck_player->edict->v.origin).Length();
				if (dist < closest_platform_dist)
				{
					nearest_platform = entity_in_sphere;
					closest_platform_dist = dist;
				}
			}
		}
	}


	float yaw;
	if (!nearest_player)
	{
		yaw = stuck_player->edict->v.v_angle.y;	
	}
	else
	{
		if (nearest_player->v.velocity.Length2D() > 0.1f)
		{
			Vector moveangles;
			VectorAngles(nearest_player->v.velocity, moveangles);
			while (moveangles.y > 180.0f)
				moveangles.y -= 360.0f;
			while (moveangles.y < -180.0f)
				moveangles.y += 360.0f;

			yaw = moveangles.y;
		}
		else
		{
			yaw = nearest_player->v.v_angle.y;
		}
	}

	Vector angles = Vector(0.0f, yaw, 0.0f);
	g_engfuncs.pfnMakeVectors(angles);
	vecDir = gpGlobals->v_forward;
	vecDir = vecDir.Normalize();
	vecUp = gpGlobals->v_up;
	vecUp = vecUp.Normalize();
	bool stuck = false;
	bool fixed = false;
	Vector vecFixedPosition;

	TraceResult stucktrace;
	vecFixedPosition = stuck_player->edict->v.origin;
	g_engfuncs.pfnTraceHull(vecFixedPosition, vecFixedPosition, 0, hullNumber, 0, stuck_player->edict, &stucktrace);
	stuck = stucktrace.fStartSolid;

	if (stuck)
		printf("stuck\n");

	if (nearest_player || nearest_platform)
		printf("found nearestplayer||platform\n");

	if (stuck && (nearest_player || nearest_platform))
	{
		//try moving forwards, maybe they are against a wall

		int maxpush = 48;
		if (nearest_platform && (nearest_platform->v.velocity.Length() > 0.1f || !FClassnameIs(&nearest_platform->v, "func_train")))
		{
			maxpush = 16;
			if (!nearest_player)
				maxpush = 8;
		}


		for (int d = 1; d < maxpush; d++)
		{
			Vector vecPushBack = vecDir * (float)d;
			TraceResult tr;
			vecFixedPosition = stuck_player->edict->v.origin + vecPushBack;
			g_engfuncs.pfnTraceHull(vecFixedPosition, vecFixedPosition, 0, hullNumber, 0, stuck_player->edict, &tr);
			if (!tr.fStartSolid)
			{
				//make sure there is a floor below this position
				Vector oldorigin = stuck_player->edict->v.origin;
				int oldflags = stuck_player->edict->v.flags;
				edict_t *oldgroundent = stuck_player->edict->v.groundentity;

				stuck_player->edict->v.origin[0] = vecFixedPosition[0];
				stuck_player->edict->v.origin[1] = vecFixedPosition[1];
				stuck_player->edict->v.origin[2] = vecFixedPosition[2];
				SV_LinkEdict(stuck_player->edict, 0);

				bool outside_world = !g_engfuncs.pfnDropToFloor(stuck_player->edict);

				//restore original position
				stuck_player->edict->v.origin = oldorigin;
				stuck_player->edict->v.flags = oldflags;
				stuck_player->edict->v.groundentity = oldgroundent;
				SV_LinkEdict(stuck_player->edict, 0);

				if (!outside_world)
				{
					fixed = true;
					break;
				}
			}
			stuck = true;
		}

		if (stuck && !fixed)
		{
			//try moving backwards
			for (int d = 1; d < maxpush; d++)
			{
				Vector vecPushBack = vecDir * (float)-d;
				TraceResult tr;
				vecFixedPosition = stuck_player->edict->v.origin + vecPushBack;
				g_engfuncs.pfnTraceHull(vecFixedPosition, vecFixedPosition, 0, hullNumber, 0, stuck_player->edict, &tr);
				if (!tr.fStartSolid)
				{
					//make sure there is a floor below this position
					Vector oldorigin = stuck_player->edict->v.origin;
					int oldflags = stuck_player->edict->v.flags;
					edict_t *oldgroundent = stuck_player->edict->v.groundentity;

					stuck_player->edict->v.origin[0] = vecFixedPosition[0];
					stuck_player->edict->v.origin[1] = vecFixedPosition[1];
					stuck_player->edict->v.origin[2] = vecFixedPosition[2];
					SV_LinkEdict(stuck_player->edict, 0);

					bool outside_world = !g_engfuncs.pfnDropToFloor(stuck_player->edict);

					//restore original position
					stuck_player->edict->v.origin = oldorigin;
					stuck_player->edict->v.flags = oldflags;
					stuck_player->edict->v.groundentity = oldgroundent;
					SV_LinkEdict(stuck_player->edict, 0);

					if (!outside_world)
					{
						fixed = true;
						break;
					}
				}
				stuck = true;
			}
		}
	}

	if (stuck && !fixed)
	{
		//See if they are on a moving platform or a player
		//Can't get stuck if they are not on a movable surface!
		TraceResult groundtrace;
		g_engfuncs.pfnTraceLine(stuck_player->edict->v.origin + Vector(0.0f, 0.0f, 72.0f), stuck_player->edict->v.origin - Vector(0.0f, 0.0f, 1.0f), dont_ignore_glass, 0, stuck_player->edict, &groundtrace);
		if (groundtrace.pHit && (FClassnameIs(groundtrace.pHit, "player") || FClassnameIs(groundtrace.pHit, "func_train") || FClassnameIs(groundtrace.pHit, "func_rotating") 
			|| FClassnameIs(groundtrace.pHit, "func_door") || FClassnameIs(groundtrace.pHit, "func_door_rotating")))
		{

			//try moving upwards, maybe they are stuck in an elevator

			for (int d = 1; d < 64; d++)
			{
				Vector vecPushUp = vecUp * (float)d;
				TraceResult tr;
				vecFixedPosition = stuck_player->edict->v.origin + vecPushUp;
				g_engfuncs.pfnTraceHull(vecFixedPosition, vecFixedPosition, 0, hullNumber, 0, stuck_player->edict, &tr);
				if (!tr.fStartSolid)
				{
					//make sure there is a floor below this position
					Vector oldorigin = stuck_player->edict->v.origin;
					int oldflags = stuck_player->edict->v.flags;
					edict_t *oldgroundent = stuck_player->edict->v.groundentity;

					stuck_player->edict->v.origin[0] = vecFixedPosition[0];
					stuck_player->edict->v.origin[1] = vecFixedPosition[1];
					stuck_player->edict->v.origin[2] = vecFixedPosition[2];
					SV_LinkEdict(stuck_player->edict, 0);

					bool outside_world = !g_engfuncs.pfnDropToFloor(stuck_player->edict);

					//restore original position
					stuck_player->edict->v.origin = oldorigin;
					stuck_player->edict->v.flags = oldflags;
					stuck_player->edict->v.groundentity = oldgroundent;
					SV_LinkEdict(stuck_player->edict, 0);

					if (!outside_world)
					{
						fixed = true;
						break;
					}
				}
				stuck = true;
			}

			if (stuck && !fixed)
			{
				//try moving downwards, maybe they are stuck in an elevator

				for (int d = 1; d < 64; d++)
				{
					Vector vecPushDown = vecUp * (float)-d;
					TraceResult tr;
					vecFixedPosition = stuck_player->edict->v.origin + vecPushDown;
					g_engfuncs.pfnTraceHull(vecFixedPosition, vecFixedPosition, 0, hullNumber, 0, stuck_player->edict, &tr);
					if (!tr.fStartSolid)
					{
						//make sure there is a floor below this position
						Vector oldorigin = stuck_player->edict->v.origin;
						int oldflags = stuck_player->edict->v.flags;
						edict_t *oldgroundent = stuck_player->edict->v.groundentity;

						stuck_player->edict->v.origin[0] = vecFixedPosition[0];
						stuck_player->edict->v.origin[1] = vecFixedPosition[1];
						stuck_player->edict->v.origin[2] = vecFixedPosition[2];
						SV_LinkEdict(stuck_player->edict, 0);

						bool outside_world = !g_engfuncs.pfnDropToFloor(stuck_player->edict);

						//restore original position
						stuck_player->edict->v.origin = oldorigin;
						stuck_player->edict->v.flags = oldflags;
						stuck_player->edict->v.groundentity = oldgroundent;
						SV_LinkEdict(stuck_player->edict, 0);

						if (!outside_world)
						{
							fixed = true;
							break;
						}
					}
					stuck = true;
				}
			}
		}
	}

	if (stuck)
	{
		if (fixed)
		{
			stuck_player->edict->v.origin[0] = vecFixedPosition[0];
			stuck_player->edict->v.origin[1] = vecFixedPosition[1];
			stuck_player->edict->v.origin[2] = vecFixedPosition[2];
			SV_LinkEdict(stuck_player->edict, 0);

			Warning("player stuck, corrected pos %i/%s\n", i, stuck_player->name);
		}
		else
		{
			Warning("player stuck, failed to correct %i/%s\n", i, stuck_player->name);
		}
	}
}

void CLagCompensationManager::SV_RestoreMove(client_t *_host_client)
{
	sv_adjusted_positions_t *pos;
	client_t *cli;

	if (nofind)
	{
		nofind = 0;
		return;
	}

	//if (!gEntityInterface.pfnAllowLagCompensation())
		//return;

	if (gpGlobals->maxClients <= 1 || CVAR_GETDIRECTFLOAT(sv_maxunlag) == 0.0)
		return;

	if (!_host_client->lw || !_host_client->lc || !_host_client->active)
		return;

	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		cli = GetClientPointerFromIndex(i);//&g_psvs.clients[i];
		pos = &truepositions[i];

		if (cli == _host_client || !cli->active)
			continue;

		//if (VectorCompareWithPrecision(pos->neworg, pos->oldorg, 0.001f) || !pos->needrelink)
		//{
		//	FixPlayerStuck(i, cli);
		//	continue;
		//}

		if (!VectorCompareWithPrecision(pos->neworg, pos->oldorg, 0.001f) && pos->needrelink)
		{
			if (pos->active)
			{
				if (VectorCompareWithPrecision(pos->initial_correction_org, cli->edict->v.origin, 0.001f))
				{
					cli->edict->v.origin[0] = pos->oldorg[0];
					cli->edict->v.origin[1] = pos->oldorg[1];
					cli->edict->v.origin[2] = pos->oldorg[2];
					SV_LinkEdict(cli->edict, 0);
				}
			}
			else
			{
				Warning("SV_RestoreMove:  Tried to restore 'inactive' player %i/%s\n", i, &cli->name[4]);
			}
		}
	}
}

void SV_SetupMove(client_t *player) {
	g_LagCompensationManager.SV_SetupMove(player);
}

__declspec(naked) void hookOnSV_SetupMove(client_t *player) {
	__asm jmp SV_SetupMove
	/*
	__asm {
		mov eax, dword ptr ss:[esp + 4]
		mov tempEAX, eax
	}
	SV_SetupMove((client_t*)tempEAX);
	__asm retn
	*/
}


void SV_RestoreMove( client_t *player ) {
	g_LagCompensationManager.SV_RestoreMove(player);
}

__declspec(naked) void hookOnSV_RestoreMove(client_t *player) {
	__asm jmp SV_RestoreMove
	/*
	__asm {
		mov eax, dword ptr ss:[esp + 4]
		mov tempEAX, eax
	}
	SV_RestoreMove((client_t*)tempEAX);
	__asm retn
	*/
}

__declspec(naked) float hookOnSV_CalcClientTime(client_t *cl) {
	__asm {
		mov eax, dword ptr ss:[esp + 4]
		mov tempEAX, eax
	}
	tempFloat = SV_CalcClientTime((client_t*)tempEAX);
	__asm fld tempFloat
	__asm retn
}

__declspec(naked) void hookOnSV_ComputeLatency(client_t *cl) {
	__asm jmp SV_ComputeLatency
	/*
	__asm {
		mov dword ptr ds:[tempESI], esi
		mov esi, dword ptr ss : [esp + 4]
		mov tempEAX, esi
	}
	SV_ComputeLatency((client_t*)tempEAX);

	__asm mov esi, dword ptr ds:[tempESI]
	__asm retn
	*/
}

HookCallInfo funcHooks[] = {
	{ (BYTE*)ADR_SV_SETUPMOVE, (DWORD)hookOnSV_SetupMove, 6, (BYTE*)ADR_SV_SETUPMOVE + 0x6 },
	{ (BYTE*)ADR_SV_RESTOREMOVE, (DWORD)hookOnSV_RestoreMove, 6, (BYTE*)ADR_SV_RESTOREMOVE + 0x6 },
	{ (BYTE*)ADR_SV_CALCCLIENTTIME, (DWORD)hookOnSV_CalcClientTime, 6, (BYTE*)ADR_SV_CALCCLIENTTIME + 0x6 },
	{ (BYTE*)ADR_SV_COMPUTELATENCY, (DWORD)hookOnSV_ComputeLatency, 6, (BYTE*)ADR_SV_COMPUTELATENCY + 0x6 },
};

BYTE* getHookFuncJMPAddr(DWORD fnName) {
	for(int i=0;i<sizeof(funcHooks)/sizeof(HookCallInfo);i++) {
		if(funcHooks[i].retFunc == fnName) {
			return (BYTE*)funcHooks[i].retJMPAddr;
		}
	}
	printf("Error: One or more of the provided function addresses weren't found\n");
	return 0;
}

void HookFunctions() {
	for(int i=0;i<sizeof(funcHooks)/sizeof(HookCallInfo);i++) {
		PlaceJMP((BYTE*)funcHooks[i].address, funcHooks[i].retFunc, funcHooks[i].len);
	}
}

// END OF HOOKS

/*
class CBombTarget : public CBaseEntityCustom {
	void Spawn (void) { printf("SPAWNED BOMBTARGET!\n"); }
};

C_DLLEXPORT void bomb_target(void) {
	//this works but causes a crash because I think it needs LINK_ENTITY_TO_CLASS which also crashes because pContainingEntity is invalid
	printf("Bomb Target Found\n");
}

LINK_ENTITY_TO_CLASS(func_bomb_target, CBombTarget ); //0x4212E584 CBaseEntity
*/

/*void callOnPlayerTakeDamage(entvars_t2 *pevInflictor, entvars_t2 *pevAttacker, float flDamage, int bitsDamageType) {
	//Todo: What do we need here?
}*/

void callOnPlayerKilled(entvars_t2 *pevAttacker, int iGib, C_BasePlayer *pVictim) {
	/*
	C_BasePlayer *pAttacker = C_BasePlayer::Instance(pevAttacker->pContainingEntity);
	if ( pAttacker && pAttacker->IsPlayer() && !pevAttacker->team == pVictim->team ) {
		//Give the attacker some points for killing this player
		SetCTFScore(pAttacker, GetCTFScore(pAttacker) + 1);
		//Give money to attacker
	}
	*/
}

void OnModuleInitialize(void) {
	HookFunctions();
	//REG_SVR_COMMAND("de_endround", Cmd_EndRound);
	printf("Custom Lag Compensation Initiated\n");
}

void OnKeyValue(edict_t *pEntity, KeyValueData *pkvd) {
}

void OnServerDeactivate(void) {
}

void OnServerActivate(void) {
}

void OnClientKill(edict_t *pEntity) {
	//suicide only, we hooked the "Killed" function manually
}

void OnClientConnect(edict_t *pEntity) {
	//client_t *pClient = EDICT_TO_CLIENT(pEntity);
	/*
	int index = ENTINDEX(pEntity);
	client_t *pClient = EDICT_TO_CLIENT(pEntity);
	if ( !pClient->fakeclient ) {
		glinfo.iNumPlayers++;
		plinfo[index].bFakeClient = false;
	} else {
		plinfo[index].bFakeClient = true;
	}
	*/
}

void OnClientDisconnect(edict_t *pEntity) {
	/*
	int index = ENTINDEX(pEntity);
	client_t *pClient = EDICT_TO_CLIENT(pEntity);
	if ( !pClient->fakeclient )
		glinfo.iNumPlayers--;
	plinfo[index].bFakeClient = false;
	plinfo[index].iTeam = 0;
	*/
}


//Called when a server runs a frame
void OnStartFrame(void) {
}

//Called after a server runs a frame
void OnPostFrame(void) {
	//g_LagCompensationManager.FrameUpdatePostEntityThink();
}

//Called when server thinks on a player
void OnPlayerPreThink(edict_t *pEntity) {
}

//Called when a client types a command in the console
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand) {
	//if ( !strcmpi(szCommand, "dropbomb") ) {
	//}
}