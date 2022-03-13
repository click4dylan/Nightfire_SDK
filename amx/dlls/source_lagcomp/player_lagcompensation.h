#ifndef CSTRIKE_H
#define CSTRIKE_H
#include "engine.h"
#include "Dylans_Functions.h"
#include "DynamicLight.h"

#define ADR_SV_SETUPMOVE		0x430925F0
#define ADR_SV_RESTOREMOVE		0x43091870 //SV_RestoreMove
#define ADR_SV_COMPUTELATENCY 0x430925D0
#define ADR_SV_CALCCLIENTTIME 0x43092430

float SV_CalcClientTime(client_t *cl);
void SV_ComputeLatency(client_t *cl);

float hookOnSV_CalcClientTime(client_t *cl);
void hookOnSV_ComputeLatency(client_t *cl);
void hookOnSV_SetupMove(client_t *player);
void hookOnSV_RestoreMove(client_t *player);
void SV_SetupMove( client_t *player );
void SV_RestoreMove( client_t *player );
void callOnHudInit(edict_t *pEntity);

entity_state_t *SV_FindEntInPack(int index, packet_entities_t *pack);

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
void OnKeyValue(edict_t *pEntity, KeyValueData *pkvd);
void OnModuleInitialize(void);
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand);
void OnServerActivate(void);
void OnServerDeactivate(void);
void OnClientKill(edict_t *pEntity);
void OnClientConnect(edict_t *pEntity);
void OnClientDisconnect(edict_t *pEntity);
void OnStartFrame(void);
void OnPostFrame(void);
void OnPlayerPreThink(edict_t *pEntity);
#endif