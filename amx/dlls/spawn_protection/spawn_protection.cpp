#include "spawn_protection.h"
#include "DylansHooks.h"
cvar_t* spawnprotect = nullptr;
cvar_t* spawnprotect_time = nullptr;

BOOL callOnBasePlayerSpawn(C_BasePlayer *pPlayer) {
	int index = ENTINDEX(pPlayer->edict());
	if ( spawnprotect->GetBool() ) {
		plinfo[index].iTeam = pPlayer->team;
		plinfo[index].flSpawnProtectStart = gpGlobals->time;
		plinfo[index].bSpawnProtect = true;
		UTIL_Create_TE_ELIGHT(pPlayer->edict(), pPlayer->edict()->v.origin, 50, Color(0, 255, 0), (int)(spawnprotect_time->GetFloat() * 10), 2 );
	}
	plinfo[index].flNextTeamChange = gpGlobals->time + 30.0f;
	return TRUE;
}

void OnModuleInitialize(void) {
	if (!spawnprotect)
	{
		cvar_t_small sv_spawnprotect_time = { CVAR_FLOAT, "sv_spawnprotect_time", "Amount of time for spawn protection", "4.0", FCVAR_EXTDLL | FCVAR_SERVER };
		cvar_t_small sv_spawnprotect = { CVAR_BOOL, "sv_spawnprotect", "Enable spawn protection", "1", FCVAR_EXTDLL | FCVAR_SERVER };
		spawnprotect = (cvar_t*)CVAR_REGISTER(sv_spawnprotect);
		spawnprotect_time = (cvar_t*)CVAR_REGISTER(sv_spawnprotect_time);
	}
	//REG_SVR_COMMAND("de_endround", Cmd_EndRound);
	printf("Spawn Protection Initiated\n");
	ADD_DLL_HOOK(&callOnBasePlayerSpawn, ADD_HOOK_TYPES::H_BASEPLAYERSPAWN);
}

void OnKeyValue(edict_t *pEntity, KeyValueData *pkvd) {
}

void OnServerDeactivate(void) {
}

void OnServerActivate(void) {
}

void OnClientKill(edict_t *pEntity) {
	//suicide only, we hooked the "Killed" function manually
	if ( spawnprotect->GetBool() ) {
		int index = ENTINDEX(pEntity);
		if ( plinfo[index].bSpawnProtect ) {
			plinfo[index].bSpawnProtect = false;
			pEntity->v.takedamage = DAMAGE_AIM;
		}
	}
}

void OnClientConnect(edict_t *pEntity) {
	plinfo[ENTINDEX(pEntity)].flNextTeamChange = gpGlobals->time;
}

void OnClientDisconnect(edict_t *pEntity) {
	plinfo[ENTINDEX(pEntity)].flNextTeamChange = gpGlobals->time;
}


//Called when a server runs a frame
void OnStartFrame(void) {
}

//Called after a server runs a frame
void OnPostFrame(void) {
}

//Called when server thinks on a player
void OnPlayerPreThink(edict_t *pEntity) {
	if ( spawnprotect->GetBool() ) {
		int index = ENTINDEX(pEntity);
		if ( plinfo[index].bSpawnProtect ) {
			if ( gpGlobals->time - plinfo[index].flSpawnProtectStart >= spawnprotect_time->GetFloat() /*|| pEntity->v.button & IN_ATTACK || pEntity->v.velocity.Length2D() > 0*/ ) {
				plinfo[index].bSpawnProtect = false;
				pEntity->v.takedamage = DAMAGE_AIM;
			} else {
				pEntity->v.takedamage = DAMAGE_NO;
			}
		}
	}
}

//Called when a client types a command in the console
META_RES OnClientCommand(edict_t *pEntity, int argl, const char *szCommand) {
	if ( !strcmpi(szCommand, "teamchoice") )
	{
		if (gpGlobals->time < plinfo[ENTINDEX(pEntity)].flNextTeamChange)
		{
			char msg[768];
			sprintf(msg, "Blocked team change from %s", GetClientPointerFromIndex(ENTINDEX(pEntity) - 1)->name);
			SayTextAll(msg);
			return MRES_SUPERCEDE;
		}
		plinfo[ENTINDEX(pEntity)].flNextTeamChange = gpGlobals->time + 30.0f;
	}
	return MRES_IGNORED;
}

META_RES OnUserInfoChanged(edict_t *pEntity, char *infobuffer)
{
	char *str = INFOKEY_VALUE(infobuffer, "teamchoice");
	int index = ENTINDEX(pEntity);
	if ( str && strlen(str) > 0 ) {
		StringList *list = SplitChar(str, " ");
		int team = atoi(list->Get(0));
		if (team != plinfo[index].iTeamChoice)
		{
			if (gpGlobals->time >= plinfo[index].flNextTeamChange)
			{
				plinfo[index].iTeamChoice = team;
				plinfo[index].flNextTeamChange = gpGlobals->time + 20.0f;
				delete list;
			}
			else
			{
				plinfo[index].iTeamChoice = team;
				char msg[768];
				sprintf(msg, "Server blocked team change from %s", GetClientPointerFromIndex(ENTINDEX(pEntity) - 1)->name);
				SayTextAll(msg);
				delete list;
				return MRES_SUPERCEDE;
			}
		}
	}

	return MRES_IGNORED;
}