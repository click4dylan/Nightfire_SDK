#include "SpawnTimeFix.h"
#include "DylansHooks.h"

#include <math.h>

cvar_t* sv_minspawntime = nullptr;

DLL_GLOBAL const Vector g_vecZero = Vector(0, 0, 0);

BOOL callOnFPlayerCanRespawn (C_BasePlayer* pPlayer) {
	//returns if player is allowed to respawn
	PlayerInfo *info = &plinfo[ENTINDEX(pPlayer->edict())];
	float spawn_time_delta = fabsf(gpGlobals->time - info->flLastDeathTime);
	BOOL result = spawn_time_delta >= sv_minspawntime->GetFloat() ? TRUE : FALSE;
	//printf("callOnFPlayerCanRespawn delta %f, limit %f, returning %i, adr %#010x\n", spawn_time_delta, *sv_minspawntime, result, (DWORD)sv_minspawntime);
	return result;
}

void callOnPlayerKilled(entvars_t2 *pevAttacker, int iGib, C_BasePlayer *pVictim) {
	PlayerInfo *info = &plinfo[ENTINDEX(pVictim->edict())];
	info->flLastDeathTime = gpGlobals->time;
}

void OnModuleInitialize(void) {
	static bool regged = false;
	if (!regged)
	{
		cvar_t_small minspawntime = {CVAR_FLOAT, "sv_minspawntime", "minimum player spawn time in seconds", "3.0", FCVAR_EXTDLL | FCVAR_SERVER};
		sv_minspawntime = (cvar_t*)CVAR_REGISTER(minspawntime);
		printf("Spawn Time Limit Plugin Initialized\n");
		if (!ADD_DLL_HOOK(&callOnPlayerKilled, ADD_HOOK_TYPES::H_PLAYERKILLED))
			printf("Failed to hook callOnPlayerKilled\n");
		if (!ADD_DLL_HOOK(&callOnFPlayerCanRespawn, ADD_HOOK_TYPES::H_FPLAYERCANRESPAWN))
			printf("Failed to hook callOnFPlayerCanRespawn\n");
		regged = true;
	}
}

void OnServerDeactivate(void) {
}

void OnServerActivate(void) {
	printf("Spawn Time Limit Activated\n");
}

void OnClientKill(edict_t *pEntity) {
	//suicide only, we hooked the "Killed" function manually
}

void OnClientConnect(edict_t *pEntity) {
}

void OnClientDisconnect(edict_t *pEntity) {
}

//Called when client's user info string changes
void CStrike_HandleClientUserInfOChanged( edict_t *pEntity, char *infobuffer ) {
	//int teamchoice = atoi(INFOKEY_VALUE(infobuffer, "teamchoice"));
}

int OnAddToFullPack(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet) {
	return -1;
#if 0
	if ((ent->v.effects == EF_NODRAW) &&
		(ent != host))
		return 0;

	// Ignore ents without valid / visible models
	if (!ent->v.modelindex || !STRING(ent->v.model))
		return 0;

	// Don't send spectators to other players
	if ((ent->v.flags & FL_SPECTATOR) && (ent != host))
	{
		return 0;
	}

	const char *classname = STRING(ent->v.classname);
	if (!strcmp(classname, "dumb_rocket")) {
		if (ent->v.owner && ent->v.owner->v.owner == host)
			return 1;
		/*if (!strcmp(STRING(ent->v.owner->v.classname), "item_helicopter")) {

			edict_t *pHeli = FIND_ENTITY_BY_CLASSNAME(NULL, "item_helicopter");
			while (!FNullEnt(pHeli)) {
				if (ent->v.owner == pHeli) {
					if (pHeli->v.owner == host) {
						return 1;
					}
				}
				pHeli = FIND_ENTITY_BY_CLASSNAME(pHeli, "item_helicopter");
			}
		}
		*/
	}
	else if (!strcmp(classname, "item_helicopter"))
	{
		if (ent->v.owner == host)
			return 1;
	}
	else if (!strcmp(classname, "item_helicopter_model"))
	{
		if (ent->v.owner == host)
			return -1;
	}
	return -1;
#endif
}

//Called when a server runs a frame
void OnStartFrame(void) {
}

//Called when server thinks on a player
void OnPlayerPreThink(edict_t *pEntity) {
}

//Called when a client types a command in the console
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand) {
}