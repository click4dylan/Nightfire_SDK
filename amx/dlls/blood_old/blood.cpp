#include "blood.h"
#include "DylansHooks.h"

bool* violence_hblood = nullptr;

void OnServerDeactivate()
{

}

void callOnSpawnBlood(float X, float Y, float Z, int bloodColor, float flDamage) {
	if (CVAR_GETDIRECT(violence_hblood))
	{
		if (bloodColor != DONT_BLEED) {
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, Vector(X, Y, Z));
			WRITE_BYTE(TE_SPRITE);
			WRITE_COORD(X);		// position (X)
			WRITE_COORD(Y);		// position (Y)
			WRITE_COORD(Z);		// position (Z)
			WRITE_SHORT(glinfo.BloodSprite);			// sprite index
			WRITE_BYTE(RANDOM_LONG(2, 5));
			WRITE_BYTE(RANDOM_LONG(150, 255));
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, Vector(X, Y, Z));
			WRITE_BYTE(TE_SPRITE);
			WRITE_COORD(X);		// position (X)
			WRITE_COORD(Y);		// position (Y)
			WRITE_COORD(Z);		// position (Z)
			WRITE_SHORT(glinfo.BloodSprite);			// sprite index
			WRITE_BYTE(RANDOM_LONG(2, 3));
			WRITE_BYTE(RANDOM_LONG(150, 175));
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, Vector(X, Y, Z));
			WRITE_BYTE(TE_SPRITE);
			WRITE_COORD(X);		// position (X)
			WRITE_COORD(Y);		// position (Y)
			WRITE_COORD(Z);		// position (Z)
			WRITE_SHORT(glinfo.BloodSprite);			// sprite index
			WRITE_BYTE(RANDOM_LONG(1, 2));
			WRITE_BYTE(RANDOM_LONG(25, 45));
			MESSAGE_END();
		}
	}
}

void OnModuleInitialize(void) {
	static bool regged = false;
	if (!regged)
	{
		cvar_t_small violence = {CVAR_BOOL, "violence_hblood", "none", "1", FCVAR_ARCHIVE | FCVAR_EXTDLL | FCVAR_SERVER};
		violence_hblood = (bool*)CVAR_REGISTER(violence);
		printf("Blood Plugin Initialized\n");
		regged = true;
		ADD_DLL_HOOK(&callOnSpawnBlood, ADD_HOOK_TYPES::H_SPAWNBLOOD);
	}

	glinfo.BloodSprite = PRECACHE_MODEL("sprites/redpapergibs.spz");
}

int GetNumPlayers(bool IncludeBots)
{
	int numplayers = 0;
	for (int i = 0; i < gpGlobals->maxClients; i++) 
	{
		client_t *pClient = GetClientPointerFromIndex(i);
		if (pClient->active && pClient->connected && pClient->spawned && (!pClient->fakeclient || IncludeBots))
			numplayers++;
	}
	return numplayers;
}

void OnStartFrame()
{
}

void OnClientCommand(edict_t *pEntity, int argl, const char **str)
{
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