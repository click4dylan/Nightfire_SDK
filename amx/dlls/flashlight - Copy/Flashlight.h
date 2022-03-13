#ifndef FLASHLIGHT_H
#define FLASHLIGHT_H
#include "engine.h"
#include "Dylans_Functions.h"

void callOnFireBullets(edict_t *edict, Vector vecSrc, Vector vecShootDir, float dist);
void callOnPlayerKilled(entvars_t *pevAttacker, int iGib, C_BasePlayer *pVictim);
void OnClientConnect(edict_t *pEntity);
void OnClientDisconnect(edict_t *pEntity);
void OnModuleInitialize(void);
void HookFunctions(void);
void EnableLightBeam(edict_t *pEntity, int index);
void DisableLightBeam(edict_t  *pEntity, int index);
void SwitchLightBeam(edict_t *pEntity);
void ToggleFlashlight(edict_t *pEntity);
void DisableFlashlight(edict_t *pEntity);
void EnableFlashlight(edict_t *pEntity);
void FlashlightThink(edict_t *pEntity, int index);
void OnPlayerPreThink(edict_t *pEntity, int index);
/*
__forceinline void FlashlightFrame(void) {
	for ( int i = 1; i <= gpGlobals->maxClients; i++ ) {
		if ( plinfo[i].bUsingFlashlight ) {
			edict_t *pEntity = g_engfuncs.pfnPEntityOfEntIndex(i);
			FlashlightThink(pEntity, i);
		}
	}
}
*/
int callOnAddToFullPack(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet);
void callOnHudInit();
#endif