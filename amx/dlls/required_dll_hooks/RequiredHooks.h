#ifndef CSTRIKE_H
#define CSTRIKE_H
#include "engine.h"
#include "Dylans_Functions.h"
#include "DynamicLight.h"
#include "DylansHooks.h"


#define ADR_FIRE_BULLETS			0x42042DE0
#define ADR_PLAYERKILLED			0x420AACA0
#define ADR_GETENTITYINIT			0x43095040
#define ADR_SPAWNBLOOD              0x420D07D0
#define ADR_INIT_HUD				0x42081F32 //After player spawns for the first time
#define ADR_BASEPLAYERSPAWN			0x420A2490
#define ADR_PLAYERTAKEDAMAGE		0x420A66B0
#define ADR_FINDENTITYINSPHERE 0x4306C940
#define ADR_FPLAYERCANRESPAWN			0x4207FD80
#define ADR_CL_PM_INIT 0x41064950
#define ADR_CL_PM_MOVE 0x41066E80

edict_t* hookOnFindEntityInSphere(edict_t *pEdictStartSearchAfter, const Vector &org, float rad);
edict_t* callOnFindEntityInSphere(edict_t *pEdictStartSearchAfter, const Vector &org, float rad);

void hookOnHudInit();
void callOnHudInit(edict_t *pEntity);
void hookOnSpawnBlood();
void callOnSpawnBlood(float X, float Y, float Z, int bloodColor, float flDamage);
void hookOnBasePlayerSpawn();
BOOL callOnBasePlayerSpawn(C_BasePlayer *pPlayer);
BOOL __stdcall hookOnFPlayerCanRespawn(C_BasePlayer *pPlayer);
void hookOnPlayerTakeDamage();
void callOnPlayerTakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
void hookOnPlayerKilled();
void callOnPlayerKilled(entvars_t *pevAttacker, int iGib, C_BasePlayer *pVictim);
void hookOnGetEntityInit();
FARPROC callOnGetEntityInit(LPCSTR lpProcName);
void hookOnFireBullets(void);
void callOnFireBullets(edict_t *edict, Vector vecSrc, Vector vecShootDir, float dist);
void hookOnCL_PM_Init(struct playermove_s *ppmove);
void hookOnCL_PM_Move(struct playermove_s *ppmove, int server);


//C_DLLEXPORT void* bomb_target(entvars_t *a);

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
BYTE* getHookFuncJMPAddr(ADD_HOOK_TYPES hooktype);
void HookFunctions();
// END OF HOOKS

void OnModuleInitialize(void);
void OnServerActivate(void);
void OnServerDeactivate(void);
void OnClientConnect(edict_t *pEntity);
void OnClientDisconnect(edict_t *pEntity);

//find ADD_HOOK_TYPES in hlsdk/public/DylansHooks.h

/*enum ADD_HOOK_TYPES {
	H_FIREBULLETS = 0,
	H_HUDINIT,
	H_PLAYERKILLED,
	H_ENTITYINIT,
	H_BASEPLAYERSPAWN,
	H_BASEPLAYERTAKEDAMAGE,
	H_SPAWNBLOOD,
	H_FINDENTITYINSPHERE,
	H_FPLAYERCANRESPAWN//,
	//H_PM_INIT,
	//H_PM_MOVE
};/**/

struct globalhooks_s {
	hook_t *firebullets_t;
	hook_t *hudinit_t;
	hook_t *playerkilled_t;
	hook_t *entityinit_t;
	hook_t *baseplayerspawn_t;
	hook_t *playertakedamage_t;
	hook_t *spawnblood_t;
	hook_t *ffindentityinsphere_t;
	hook_t *fplayercanrespawn_t;
	//hook_t *fcl_pm_init;
	//hook_t *fcl_pm_move;
};

extern globalhooks_s globalhooks_t;

#define FOUR_BLANK_HOOKS \
{ (BYTE*)0, (DWORD)0, 0, (BYTE*)0, },\
{ (BYTE*)0, (DWORD)0, 0, (BYTE*)0, },\
{ (BYTE*)0, (DWORD)0, 0, (BYTE*)0, },\
{ (BYTE*)0, (DWORD)0, 0, (BYTE*)0, },
static HookCallInfo funcHooks[] = {
	{ (BYTE*)ADR_FIRE_BULLETS, (DWORD)hookOnFireBullets, 6, (BYTE*)ADR_FIRE_BULLETS + 0x5 },
	//FOUR_BLANK_HOOKS
	{ (BYTE*)ADR_INIT_HUD, (DWORD)hookOnHudInit, 6, (BYTE*)ADR_INIT_HUD + 0x5 },
	//FOUR_BLANK_HOOKS
	{ (BYTE*)ADR_PLAYERKILLED, (DWORD)hookOnPlayerKilled, 9, (BYTE*)ADR_PLAYERKILLED + 0x6 },
	//FOUR_BLANK_HOOKS
	{ (BYTE*)ADR_GETENTITYINIT, (DWORD)hookOnGetEntityInit, 5, (BYTE*)ADR_GETENTITYINIT + 0x5 },
	//FOUR_BLANK_HOOKS
	{ (BYTE*)ADR_BASEPLAYERSPAWN, (DWORD)hookOnBasePlayerSpawn, 5, (BYTE*)ADR_BASEPLAYERSPAWN + 0x5 },
	//FOUR_BLANK_HOOKS
	{ (BYTE*)ADR_PLAYERTAKEDAMAGE, (DWORD)hookOnPlayerTakeDamage, 6, (BYTE*)ADR_PLAYERTAKEDAMAGE + 0x6 },
	//FOUR_BLANK_HOOKS
	{ (BYTE*)ADR_SPAWNBLOOD, (DWORD)hookOnSpawnBlood, 5, (BYTE*)ADR_SPAWNBLOOD + 0x5 },
	//FOUR_BLANK_HOOKS
	{ (BYTE*)ADR_FINDENTITYINSPHERE, (DWORD)hookOnFindEntityInSphere, 5, (BYTE*)ADR_FINDENTITYINSPHERE + 0x5 },
	//FOUR_BLANK_HOOKS
	{(BYTE*)ADR_FPLAYERCANRESPAWN, (DWORD)hookOnFPlayerCanRespawn, 6, (BYTE*)ADR_FPLAYERCANRESPAWN+0x5},
	//FOUR_BLANK_HOOKS
	//{ (BYTE*)ADR_CL_PM_INIT, (DWORD)hookOnCL_PM_Init, 6, (BYTE*)ADR_CL_PM_INIT + 0x5 },
	//FOUR_BLANK_HOOKS
	//{ (BYTE*)ADR_CL_PM_MOVE, (DWORD)hookOnCL_PM_Move, 6, (BYTE*)ADR_CL_PM_MOVE + 0x5 },

	//{(BYTE*)ADR_PLAYERSPAWNCTF, (DWORD)hookOnPlayerSpawnCTF, 6, (BYTE*)ADR_PLAYERSPAWNCTF+0x6},
	//{(BYTE*)ADR_PLAYERSPAWNDM, (DWORD)hookOnPlayerSpawnDM, 5, (BYTE*)ADR_PLAYERSPAWNDM+0x5},
	
};

typedef struct functiontable_s
{
	uint32 pFunction;
	char *pFunctionName;
} functiontable_t;

/* 572 */
/* <8a793> ../engine/pr_dlls.h:48 */
typedef struct extensiondll_s
{
	void *lDLLHandle;
	functiontable_t *functionTable;
	int functionCount;
} extensiondll_t;

#ifdef _DEBUG
FORCEINLINE float clamp(float val, float minVal, float maxVal)
{
	if (maxVal < minVal)
		return maxVal;
	else if (val < minVal)
		return minVal;
	else if (val > maxVal)
		return maxVal;
	else
		return val;
}
#else // DEBUG
FORCEINLINE float clamp(float val, float minVal, float maxVal)
{
#if defined(__i386__) || defined(_M_IX86)
	_mm_store_ss(&val,
		_mm_min_ss(
		_mm_max_ss(
		_mm_load_ss(&val),
		_mm_load_ss(&minVal)),
		_mm_load_ss(&maxVal)));
#else
	val = fpmax(minVal, val);
	val = fpmin(maxVal, val);
#endif
	return val;
}
#endif // DEBUG

#endif