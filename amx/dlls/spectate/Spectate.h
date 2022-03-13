#ifndef SPECTATE_H
#define SPECTATE_H
#include "engine.h"
#include "Dylans_Functions.h"
#define MODE_THIRDPERSON 0
#define MODE_FREELOOK 1
#define MODE_FIRSTPERSON 2

void OnModuleInitialize(void);
void SpectateConsoleCommand(edict_t *pEntity, int argl);
void ExitSpectate(edict_t *pEntity);
void CreateCamera(edict_t *&pEntity);
void DeleteCamera(edict_t *pEntity);
void UpdateCamera(int index, edict_t *pEntity);
void CheckCameraButtons(int index, edict_t *pEntity);
void EnableFreeLook(edict_t *pEntity);
void DisableFreeLook(edict_t *pEntity);
bool SpectateLastPlayer(edict_t *pEntity);
bool CycleToNextPlayer(edict_t *pEntity, edict_t *pCurrentTargetPlayer, int index);
bool CycleToPreviousPlayer(edict_t *pEntity, edict_t *pCurrentTargetPlayer, int index);
inline void SpectatorKill(edict_t *pEntity) {
	if ( pEntity->v.deadflag != DEAD_DYING && pEntity->v.deadflag != DEAD_DEAD ) {
		MDLL_ClientKill(pEntity);
	
		pEntity->v.frags++;
		SetPlayerDeaths(pEntity, GetPlayerDeaths(pEntity) - 1);
		//Update scoreboard
		MESSAGE_BEGIN(MSG_ALL, GET_USER_MSG_ID(PLID, "ScoreInfo", NULL));
		WRITE_BYTE( ENTINDEX(pEntity) );
		WRITE_SHORT( (int)pEntity->v.frags );
		WRITE_SHORT( GetPlayerDeaths(pEntity) );
		WRITE_SHORT( 0 );
		WRITE_SHORT( MF_GetPlayerTeamID(ENTINDEX(pEntity)) );
		MESSAGE_END();
	}
	plinfo[ENTINDEX(pEntity)].pSpectateLastTargetEdict = NULL;
}
inline void MakePlayerInvisible(edict_t *pEntity) {
	pEntity->v.rendermode = kRenderTransColor;
	pEntity->v.renderamt = 0;
	pEntity->v.renderfx = kRenderFxNone;
}
inline void MakePlayerVisible(edict_t *pEntity) {
	pEntity->v.rendermode = kRenderNormal;
	pEntity->v.renderamt = 0;
	pEntity->v.renderfx = kRenderFxNone;
}
inline void MakePlayerSolid(edict_t *pEntity) {
	pEntity->v.movetype = MOVETYPE_WALK;
	pEntity->v.gravity = 1.0f;
	pEntity->v.solid = SOLID_SLIDEBOX;
}
inline void MakePlayerNonSolid(edict_t *pEntity) {
	pEntity->v.movetype = MOVETYPE_NOCLIP;
	pEntity->v.gravity = 0;
	pEntity->v.solid = SOLID_NOT;
}
#endif