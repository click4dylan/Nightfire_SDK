#include "force_patch.h"
#include "ilagcompensationmanager.h"
#include "utlvector.h"
#include "bitvec.h"
cvar_t* allow_rzmin = nullptr;
cvar_t* allow_oldpatches = nullptr;

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
	if (!allow_rzmin)
	{
		cvar_t_small svallow_rzmin = { CVAR_BOOL, "allow_rzmin", "Allow players to set r_zmin", "0", FCVAR_EXTDLL | FCVAR_SERVER };
		cvar_t_small svallow_oldpatches = { CVAR_BOOL, "allow_oldpatches", "Allows old client patches to join the server", "0", FCVAR_EXTDLL | FCVAR_SERVER };

		allow_rzmin = (cvar_t*)CVAR_REGISTER(svallow_rzmin);
		allow_oldpatches = (cvar_t*)CVAR_REGISTER(svallow_oldpatches);
	}
	printf("Force Client Patch Initiated\n");
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
	int index = ENTINDEX(pEntity);
	plinfo[index].bOldVersion = false;
	plinfo[index].flTimeToKickForOldVersion = 0;
	plinfo[index].flTimeToRemindPlayerForOldVersion = 0;
	plinfo[index].flNextThink = 0;
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
	int index = ENTINDEX(pEntity);
	plinfo[index].bOldVersion = false;
	plinfo[index].flTimeToKickForOldVersion = 0;
	plinfo[index].flTimeToRemindPlayerForOldVersion = 0;
	plinfo[index].flNextThink = 0;
}


//Called when a server runs a frame
void OnStartFrame(void) {
}

//Called after a server runs a frame
void OnPostFrame(void) {
}

//Called when server thinks on a player
void OnPlayerPreThink(edict_t *pEntity) {
	if ( !allow_rzmin->GetBool() ) {
		client_t *pClient = EDICT_TO_CLIENT(pEntity);
		if ( !pClient->fakeclient ) {
			int index = ENTINDEX(pEntity);
			if ( gpGlobals->time >= plinfo[index].flNextThink ) {
				plinfo[index].flNextThink = gpGlobals->time + 0.1f;
				CLIENT_COMMAND(pEntity, "r_zmin 1\n");
				if ( plinfo[index].bOldVersion ) {
					if ( gpGlobals->time >= plinfo[index].flTimeToKickForOldVersion ) {
						char msg[255];
						sprintf(msg, "%s was kicked for using an old game version", pClient->name);
						SayTextAll(msg);
						CLIENT_PRINTF(pEntity, print_console, "\n\nCONNECTION REJECTED:\n\nThis server only allows players with the latest game version to join\nPlease update to Engine version 5.9.3.0 or higher to play on this server\n\nVisit www.nightfirepc.com or www.nightfiresource.com for more information\n\nYour game version is:\n");
						CLIENT_COMMAND(pEntity, "version;console 1;con_toggleconsole;wait;wait;echo Press escape to exit to the main menu;cl_disconnect\n");
					} else if ( gpGlobals->time >= plinfo[index].flTimeToRemindPlayerForOldVersion ) {
						plinfo[index].flTimeToRemindPlayerForOldVersion = gpGlobals->time + 5;
						char msg[255];
						sprintf(msg, "Kicking %s in %.1f secs for using an old game version!", pClient->name, plinfo[index].flTimeToKickForOldVersion - gpGlobals->time);
						SayTextAll(msg);
					}
				}
			}
		}
	}
}

//Called when a client types a command in the console
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand) {
	//if ( !strcmpi(szCommand, "dropbomb") ) {
	//}
}

void OnHandleClientUserInfoChanged( edict_t *pEntity, char *infobuffer ) {
	//If player's game version is 5.82 or newer then they support dynamic lights
	if ( !allow_oldpatches->GetBool() ) {
		client_t *pClient = EDICT_TO_CLIENT(pEntity);
		if ( !pClient->fakeclient ) {
			unsigned int wonid = g_engfuncs.pfnGetPlayerWONId(pEntity);
			int index = ENTINDEX(pEntity);
			plinfo[index].bOldVersion = false;
			plinfo[index].flTimeToKickForOldVersion = 0;
			plinfo[index].flTimeToRemindPlayerForOldVersion = 0;
			if ( wonid == -1 ) {
				plinfo[index].bOldVersion = true;
				plinfo[index].flTimeToKickForOldVersion = gpGlobals->time + 20.0f;
				plinfo[index].flTimeToRemindPlayerForOldVersion = gpGlobals->time + 2;
			}
		}
	}
	/*
	char *versionstr = INFOKEY_VALUE(infobuffer, "gamever");
	int index = ENTINDEX(pEntity);
	if ( strlen(versionstr) > 0 ) {
		StringList *list = SplitChar(versionstr, " ");
		if ( atof(list->Get(0)) >= 5.93f) {
			plinfo[index].bSupportsDynamicLights = true;
		} else {
			//version too old to support Dlights
			if ( plinfo[index].bSupportsDynamicLights ) { //at connect we default to true so we can increment the counter properly
				glinfo.NumPlayersWithoutDynamicLightSupport++;
				plinfo[index].bSupportsDynamicLights = false;
			}
		}
		delete list;
	} else {
		//1.0 or really old player
		if ( plinfo[index].bSupportsDynamicLights ) { //at connect we default to true so we can increment the counter properly
			glinfo.NumPlayersWithoutDynamicLightSupport++;
			plinfo[index].bSupportsDynamicLights = false;
		}
	}
	*/
}