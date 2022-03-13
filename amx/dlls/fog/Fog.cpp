#include "Fog.h"

void OnModuleInitialize(void) {
	cvar_t_small setfog_color = {CVAR_STRING, "setfog_color", "Sets fog color", "none", FCVAR_EXTDLL | FCVAR_SERVER };
	cvar_t_small setfog_start = {CVAR_FLOAT, "setfog_start", "Sets fog start", "512", FCVAR_EXTDLL | FCVAR_SERVER };
	cvar_t_small setfog_end = {CVAR_FLOAT, "setfog_end", "Sets fog end", "2048", FCVAR_EXTDLL | FCVAR_SERVER };
	CVAR_REGISTER(setfog_color);
	CVAR_REGISTER(setfog_start);
	CVAR_REGISTER(setfog_end);
	REG_SVR_COMMAND("setfog", DoSetFog);
	printf("SetFog Initialized\n");
}

void DoSetFog() {
	const char *value = CVAR_GET_STRING("setfog_color");
	if ( !value || !strcmp(value, "") || !strcmpi(value, "none") ) {
		SetFog(Color(0,0,0), 0, 0, true);
	} else {
		StringList *list = SplitChar(value, "  ");//space and alt + 0160
		if ( list->GetSize() == 3 ) {
			SetFog(Color(atoi(list->Get(0)), atoi(list->Get(1)), atoi(list->Get(2))), CVAR_GET_FLOAT("setfog_start"), CVAR_GET_FLOAT("setfog_end"), false);
		}
		delete list;
	}
}

void ConsoleCommand(edict_t *pEntity, int argl) {
	/*
	if ( argl > 2 ) { 
		SetLights(CMD_ARGV(1));
	} else if ( glinfo.bCheckLights ) {
		SetLights("", true);
	}
	*/
}

void SetFog(Color col, float fogstart, float fogend, bool setdefault) {
	if (setdefault) {
		for ( int i = 0; i < gpGlobals->maxClients; i++ ) {
			client_t *pClient = GetClientPointerFromIndex(i);
			if ( pClient->active && pClient->connected && !pClient->fakeclient && pClient->spawned )
				CLIENT_COMMAND(pClient->edict, "r_fogdev 0\n");
		}
	} else {
		for ( int i = 0; i < gpGlobals->maxClients; i++ ) {
			client_t *pClient = GetClientPointerFromIndex(i);
			if ( pClient->active && pClient->connected && !pClient->fakeclient ) {
				char str[160];
				sprintf(str, "r_fogcolor \"%i %i %i\";r_fogstart %f;r_fogend %f;r_fogdev 1\n", col.red, col.green, col.blue, fogstart, fogend);
				CLIENT_COMMAND(pClient->edict, str);
			}
		}		
	}
}