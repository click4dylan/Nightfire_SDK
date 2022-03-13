#ifndef CSTRIKE_H
#define CSTRIKE_H
#include "engine.h"
#include "Dylans_Functions.h"
#include "DynamicLight.h"

#define ADR_PLAYERSPAWNDM				0x42080F28
#define ADR_PLAYERSPAWNCTF				0x420469C7
#define ADR_FPLAYERCANRESPAWN			0x4207FD80
#define ADR_PLAYERTAKEDAMAGE			0x420A66B0
#define ADR_PLAYERKILLED				0x420AACA0
#define ADR_GETENTITYINIT				0x43095040
#define ADR_BASEPLAYERSPAWN				0x420A2490
#define ADR_INIT_HUD					0x42081F32 //After player spawns for the first time

int ButtonObjectCaps();
int callOnButtonObjectCaps( C_BaseEntity *pEntity );
void HookButtonObjectCaps(void);
//void hookOnGetEntityInit();
void hookOnBasePlayerSpawn();
void hookOnPlayerSpawn();
void hookOnPlayerKilled();
//void hookOnPlayerTakeDamage();
//void callOnPlayerTakeDamage(entvars_t2 *pevInflictor, entvars_t2 *pevAttacker, float flDamage, int bitsDamageType);
void callOnPlayerKilled(entvars_t2 *pevAttacker, int iGib, C_BasePlayer *pVictim);
void callOnHudInit(edict_t *pEntity);
//FARPROC callOnGetEntityInit(LPCSTR lpProcName);

//C_DLLEXPORT void bomb_target(void);

BOOL callOnFPlayerCanRespawn(C_BasePlayer *pPlayer);
void callOnClientSpawnCTF(C_BasePlayer *pPlayer);
void callOnClientSpawnDM(C_BasePlayer *pPlayer);
BOOL callOnBasePlayerSpawn(C_BasePlayer *pPlayer);

//HOOKS
__inline void PlaceJMP( BYTE *bt_DetourAddress, DWORD dw_FunctionAddress, DWORD dw_Size)
{
	DWORD dw_OldProtection, dw_Distance;
	VirtualProtect(bt_DetourAddress, dw_Size, PAGE_EXECUTE_READWRITE, &dw_OldProtection);
	dw_Distance = (DWORD)(dw_FunctionAddress - (DWORD)bt_DetourAddress) - 5;
	*bt_DetourAddress = 0xE9;
	*(DWORD*)(bt_DetourAddress + 0x1) = dw_Distance;
	for(int i = 0x5; i < dw_Size; i++) *(bt_DetourAddress + i) = 0x90;
	VirtualProtect(bt_DetourAddress, dw_Size,  dw_OldProtection, NULL);
    return;
}

/*
__inline void SecondsToTime(const char *str, float input_seconds) {
	//http://www.cplusplus.com/forum/beginner/14357/
	//long int seconds;
	//int days, minutes, hours;
	int seconds, minutes, hours, days;
	days = ((int)floor(glinfo.flClockSeconds)) / 60 / 60 / 24;
	hours = ((int)(floor(glinfo.flClockSeconds)) / 60 / 60) % 24;
	minutes = ((int)floor(glinfo.flClockSeconds) / 60) % 60;
	seconds = (int)floor(glinfo.flClockSeconds) % 60;
}
*/

typedef struct {
	BYTE *address;
	DWORD retFunc;
	DWORD len;
	BYTE *retJMPAddr;
} HookCallInfo;

BYTE* getHookFuncJMPAddr(DWORD fnName);
void HookFunctions();
// END OF HOOKS

#ifdef DEBUG  // stop crashing edit-and-continue
FORCEINLINE float clamp( float val, float minVal, float maxVal )
{
	if ( maxVal < minVal )
		return maxVal;
	else if( val < minVal )
		return minVal;
	else if( val > maxVal )
		return maxVal;
	else
		return val;
}
#else // DEBUG
FORCEINLINE float clamp( float val, float minVal, float maxVal )
{
#if defined(__i386__) || defined(_M_IX86)
	_mm_store_ss( &val,
		_mm_min_ss(
			_mm_max_ss(
				_mm_load_ss(&val),
				_mm_load_ss(&minVal) ),
			_mm_load_ss(&maxVal) ) );
#else
	val = fpmax(minVal, val);
	val = fpmin(maxVal, val);
#endif
	return val;
}
#endif // DEBUG

//
// Returns a clamped value in the range [min, max].
//
/*
template< class T >
inline T clamp( T const &val, T const &minVal, T const &maxVal )
{
	if ( maxVal < minVal )
		return maxVal;
	else if( val < minVal )
		return minVal;
	else if( val > maxVal )
		return maxVal;
	else
		return val;
}

*/

void DeleteBomb(void);
inline bool IsTimeToThink(void) {
	return ( gpGlobals->time >= glinfo.flNextThink );
}
inline void SetNextThink(float secs) {
	float delta = (gpGlobals->time - glinfo.flNextThink);
	delta = (delta < 0 ? 0 : delta);
	delta = (delta > 1 ? 1: delta);
	glinfo.flNextThink = gpGlobals->time + secs - delta;
}
inline void ResetRoundSystem(void) {
	glinfo.flNextThink = 0;
	glinfo.flNextTimeDrawClock = 0;
	glinfo.flWarmupTimeEnd = 0;
	glinfo.flRoundTimeEnd = 0;
	glinfo.flRoundPreStartTimeEnd = 0;
	glinfo.flTimeToCheckBombSites = 0;
	glinfo.iRoundNumber = 1;
	glinfo.iNumPlayersAlivePhoenix = 0;
	glinfo.iNumPlayersAliveMI6 = 0;
	glinfo.iNumBombSites = 0;
	glinfo.bRoundThink = false;
	glinfo.bWarmupComplete = false;
	glinfo.bWarmupStarted = false;
	glinfo.bRoundEndThink = false;
	glinfo.bGameEndThink = false;
	glinfo.bRoundPreStartThink = false;
	glinfo.bAllowPlayerSpawn = true;
	glinfo.bReceivedBombSites = false;
	glinfo.pBombSiteA = NULL;
	glinfo.pBombSiteB = NULL;
	glinfo.bBombExplodeThink = false;
	glinfo.bBombExploded = false;
	glinfo.flBombExplodeTime = 0;
	glinfo.clBombPlanter = NULL;
	glinfo.iRoundsWonMI6 = 0;
	glinfo.iRoundsWonPhoenix = 0;
	glinfo.bBombDefused = false;
	glinfo.flBombDefuseTime = 0;
	glinfo.clBombDefuser = NULL;
	glinfo.flLastDefuseTime = 0;
	glinfo.bBombStartedPlanting = false;
	glinfo.flBombPlantTime = 0;
	glinfo.flBombSiteARadius = 0;
	glinfo.flBombSiteBRadius = 0;
	SetCTFCaptures(TEAM_MI6, 0);
	SetCTFCaptures(TEAM_PHOENIX, 0);
	glinfo.flPrevDefuserMaxSpeed = 0;
	glinfo.flPrevPlanterMaxSpeed = 0;
	glinfo.flClockSeconds = 0;
	glinfo.clBombCarrier = NULL;
	glinfo.flNextBeepSound = 0;
	glinfo.bPlayedWarningSound = false;
	glinfo.g_sModelIndexFireball = NULL;
	glinfo.iBombSitePlantingAt = -1;
	glinfo.bForcePlayerAliveCheck = false;
	glinfo.iNumPlayersAwaitingSpectate = 0;
	glinfo.iNumPlayersAwaitingSpawn = 0;
	glinfo.flNextTimeForceAliveCheck = 0;
	DeleteBomb();
}
__forceinline bool CheckedForBombSites(void) {
	if ( glinfo.bReceivedBombSites )
		return true;
	return false;
}
__forceinline bool IsTimeToCheckForBombSites(void) {
	if ( gpGlobals->time >= glinfo.flTimeToCheckBombSites )
		return true;
	return false;
}
void CreateBomb(void);
__forceinline void FindBombSites(void) {
	glinfo.flTimeToCheckBombSites = 0;
	glinfo.bReceivedBombSites = true;
	glinfo.pBombSiteA = FIND_ENTITY_BY_TARGETNAME((edict_t*)NULL, "BombSite A");
	if ( glinfo.pBombSiteA ) {
		glinfo.iNumBombSites++;
		glinfo.flBombSiteARadius = glinfo.pBombSiteA->v.renderamt;
	}
	glinfo.pBombSiteB = FIND_ENTITY_BY_TARGETNAME((edict_t*)NULL, "BombSite B");
	if ( glinfo.pBombSiteB ) {
		glinfo.iNumBombSites++;
		glinfo.flBombSiteBRadius = glinfo.pBombSiteB->v.renderamt;
	}
	HookButtonObjectCaps();
}
void CStrike_HandleClientUserInfOChanged( edict_t *pEntity, char *infobuffer );
void OnKeyValue(edict_t *pEntity, KeyValueData *pkvd);
void OnWarmupComplete(void);
void OnBombExplode(void);
void OnBombStartPlanting(edict_t *pBombSite);
void __stdcall BombThink(void);
void __stdcall BombUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
void __stdcall BombTouch(C_BaseEntity *pOther);
void BombPlantThink(void);
void BombPlantUseThink(edict_t *pBombSite);
void OnBombPlant(edict_t *pBombSite);
void BombExplodeThink(void);
void __stdcall DropBombEndThink(void);
void DropBomb(void);
void GiveBomb(void);
void NotifyBombCarrier(void);
void FreezePlayers(void);
void UnFreezePlayers(void);
void RespawnPlayers(void);
void DrawClock(void);
void Cmd_ResetGame();
void Cmd_EndRound();
void OnModuleInitialize(void);
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand);
void OnServerActivate(void);
void OnServerDeactivate(void);
void OnClientKill(edict_t *pEntity);
void OnClientConnect(edict_t *pEntity);
void OnClientDisconnect(edict_t *pEntity);
void OnStartFrame(void);
void OnPlayerPreThink(edict_t *pEntity);
void GameEndThink(void);
void GameEnd(void);
void RoundPreStart(void);
void RoundPreStartThink(void);
void RoundStart(void);
void RoundEndThink(void);
void RoundEnd(void);
void RoundThink(void);
__inline void GiveMoney(int index, int incMoney) {
	plinfo[index].iMoney = min(plinfo[index].iMoney + incMoney, 16000);
}
__inline void RemoveMoney(int index, int decMoney) {
	plinfo[index].iMoney = max(0, plinfo[index].iMoney - decMoney);
}
#endif