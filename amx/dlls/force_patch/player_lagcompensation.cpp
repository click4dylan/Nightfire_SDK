#include "player_lagcompensation.h"
#include "ilagcompensationmanager.h"
#include "utlvector.h"
#include "bitvec.h"
bool* allow_rzmin;
bool* allow_oldpatches;

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
	cvar_t allow_rzmin = {CVAR_BOOL, "allow_rzmin", "Allow players to set r_zmin", (int*)"0", FCVAR_EXTDLL | FCVAR_SERVER };
	cvar_t allow_oldpatches = {CVAR_BOOL, "allow_oldpatches", "Allows old client patches to join the server", (int*)"0", FCVAR_EXTDLL | FCVAR_SERVER };
	
	allow_rzmin = (bool*)CVAR_REGISTER(sv_unlag_fixstuck);
	allow_oldpatches = (bool*)CVAR_REGISTER(sv_unlag_frametime);
	printf("Force Client Patch Initiated\n");
}

void OnKeyValue(edict_t *pEntity, KeyValueData *pkvd) {
}

void OnServerDeactivate(void) {
	OnModuleInitialize();
}

void OnServerActivate(void) {
	OnModuleInitialize();
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
}

//Called when server thinks on a player
void OnPlayerPreThink(edict_t *pEntity) {
}

//Called when a client types a command in the console
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand) {
	//if ( !strcmpi(szCommand, "dropbomb") ) {
	//}
}