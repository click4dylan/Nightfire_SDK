#include "MapSpecifics.h"

float nexttemporarymessagetime = 0;

void OnModuleInitialize(void) {
	REG_SVR_COMMAND("togglemapspecifics", MapSpecificsCommand);
	printf("MapSpecifics Initialized\n");
}

void MapSpecificsCommand(void) {
	const char *value = CMD_ARGV(1);
	bool enable;
	if ( !strcmp(value, "1") ) {
		//togglemapspecifics 1
		enable = true;
	} else if ( !strcmp(value, "0") ) {
		//togglemapspecifics 0
		enable = false;
	} else {
		//toggle it
		if ( glinfo.bMapSpecificsEnabled ) {
			enable = false;
		} else {
			enable = true;
		}
	}

	if ( enable ) {
		SayTextAll("Map Specifics Enabled");
		glinfo.bMapSpecificsEnabled = true;
		SERVER_COMMAND("setfog_color \"100 120 140\";setfog_start 384;setfog_end 2048;setfog;setlights_amt c;setlights\n");
	} else {
		SayTextAll("Map Specifics Disabled");
		glinfo.bMapSpecificsEnabled = false;
		SERVER_COMMAND("setfog_color \"none\";setfog;setlights_amt none;setlights\n");
	}
}

void OnMapSpecifics(void) {
	if ( glinfo.iPlayersConnecting > 0 ) {
		for ( int i = 1; i <= gpGlobals->maxClients; i++ ) {
			if ( plinfo[i].bConnecting ) {
				client_t *pClient = GetClientPointerFromIndex(i - 1);
				if ( pClient->active && pClient->connected && pClient->spawned ) {
					plinfo[i].bConnecting = false;
					glinfo.iPlayersConnecting--;
				}
			}
		}
		glinfo.flNextMapSpecificsThinkTime = gpGlobals->time + 1;
	} else { 
		glinfo.flNextMapSpecificsThinkTime = gpGlobals->time + 60;
		SERVER_COMMAND("setfog_color \"100 120 140\";setfog_start 384;setfog_end 2048;setfog;setlights_amt c;setlights\n");
		if ( gpGlobals->time >= nexttemporarymessagetime ) {
			nexttemporarymessagetime = gpGlobals->time + 290;
			SayTextAll("This message was sent by AMX Mod X\nType flashlight in console to enable flashlight\nSpectate buttons: jump, duck, primary attack, secondary attack\nType spectate in console to specate");
		}
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