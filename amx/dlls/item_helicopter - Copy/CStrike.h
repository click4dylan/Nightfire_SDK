#ifndef CSTRIKE_H
#define CSTRIKE_H
#include "engine.h"
#include "Dylans_Functions.h"
#include "DynamicLight.h"

#define ADR_PLAYERKILLED				0x420AACA0
#define ADR_GETENTITYINIT				0x43095040
#define ADR_SPAWNBLOOD                  0x420D07D0
#define ADR_INIT_HUD					0x42081F32 //After player spawns for the first time

void hookOnHudInit();
void callOnHudInit(edict_t *pEntity);
void hookOnGetEntityInit();
void hookOnPlayerKilled();
void hookOnSpawnBlood();
void callOnSpawnBlood(float X, float Y, float Z, int bloodColor, float flDamage);
//void hookOnPlayerTakeDamage();
//void callOnPlayerTakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
void callOnPlayerKilled(entvars_t *pevAttacker, int iGib, C_BasePlayer *pVictim);
FARPROC callOnGetEntityInit(LPCSTR lpProcName); //commentme

//C_DLLEXPORT void* bomb_target(entvars_t *a); //commentme

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

void OnModuleInitialize(void);
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand);
void OnServerActivate(void);
void OnServerDeactivate(void);
void OnClientKill(edict_t *pEntity);
void OnClientConnect(edict_t *pEntity);
void OnClientDisconnect(edict_t *pEntity);
void FindAndResetHelicopters(edict_t *pEntity);
int OnAddToFullPack(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet);
void OnStartFrame(void);
inline void NormalizeAngles(Vector& angles)
{
	int i;

	// Normalize angles to -180 to 180 range
	for (i = 0; i < 3; i++)
	{
		if (angles[i] > 180.0)
		{
			angles[i] -= 360.0;
		}
		else if (angles[i] < -180.0)
		{
			angles[i] += 360.0;
		}
	}
}

#define PHELICOPTER_LIGHT_UPDATE_SECS 2.0f
#define PHELICOPTER_HELICOPTER_MODEL			"models/mini_helicopter.mdl"
#define PHELICOPTER_HELICOPTER_MODEL_MI6			"models/mini_helicopter_mi6.mdl"
#define PHELICOPTER_HELICOPTER_MODEL_PHX			"models/mini_helicopter_phx.mdl"
#define PHELICOPTER_MAX_ROLL_DEG			55.0f
#define PHELICOPTER_MAX_PITCH_DEG			90.0f
#define PHELICOPTER_HULL_MINS				Vector(-10,-7,-10)
#define PHELICOPTER_HULL_MAXS				Vector(10,-7,10)
//#define PHELICOPTER_MAX_UPSPEED				75 //variable below
#define PHELICOPTER_MAX_ROLLSPEED			0.8f  //0.4
#define PHELICOPTER_ROLLSPEED_RATE			0.02f //0.025
#define PHELICOPTER_MAX_SPEED				300.0f
#define PHELICOPTER_MIN_SPEED				0.0f
#define PHELICOPTER_MAX_UPSIDEDOWN_FALLSPEED 5.0f
#define PHELICOPTER_BLOWUP_TOUCH_SPEED		(PHELICOPTER_MAX_SPEED - 5.0f)
#define PHELICOPTER_TURN_RATE				400.0f
#define PHELICOPTER_DIE_TIME				2.0f
#define PHELICOPTER_TRAIL_LIFE				6.0f
#define PHELICOPTER_LOSE_CONTROL_DIST		30.0f
#define PHELICOPTER_DANGER_SOUND_DURATION	0.1
#define PHELICOPTER_EXPLODESOUND "explosions/chopper_explode.wav"
#define PHELICOPTER_SOUND "common/helicopter02_loop.wav"
#define PHELICOPTER_GUNSOUND1 "tower/ac_fire1.wav"
#define PHELICOPTER_GUNSOUND2 "tower/ac_fire2.wav"
#define PHELICOPTER_GUNSOUND3 "tower/ac_fire3.wav"
#define PHELICOPTER_GUNSOUND4 "tower/ac_fire4.wav"
#define PHELICOPTER_ROCKETSOUND "tower/ac_missile1.wav"
#define PHELICOPTER_HITWALLSOUND1 "debris/car1.wav"
#define PHELICOPTER_HITWALLSOUND2 "debris/car2.wav"
#define PHELICOPTER_HITWALLSOUND3 "debris/car3.wav"

//Player Camera that controls the helicopter
class CItemHelicopterCamera : public CBaseEntityCustom {//, C_BaseEntity {
public:
	CItemHelicopterCamera::CItemHelicopterCamera(); //Constructor
	CItemHelicopterCamera::~CItemHelicopterCamera(); //Destructor
	virtual int		ObjectCaps(void) { return FCAP_IMPULSE_USE; }
	void IdleThink(void);
	void Precache(void);
	//virtual void Think(void);
	void KeyValue(KeyValueData *pkvd);
	void Spawn(void);// { printf("SPAWNED BOMBTARGET!\nSPAWNED BOMBTARGET!\nSPAWNED BOMBTARGET!\nSPAWNED BOMBTARGET!\nSPAWNED BOMBTARGET!\n"); }
	void CItemHelicopterCamera::UseDoNothing(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value);
	void SetObjectCollisionBox(void);
	void Use(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value);
	void Launch(void);
	void FlyThink(void);
	void RespawnThink(void);
	void ExplodeThink(void);
	void RemoveThink(void);
	void FlyTouch(CBaseEntityCustom *pOther);
	void InputActivate(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller);
	void InputDeactivate(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller);
	void LoseHelicopterControl();
	void BlowUp(entvars_t *pevAttacker);
	void Killed(entvars_t *pevAttacker, int iGib);
	int	TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	float			PHELICOPTER_SPEED;
	float			PHELICOPTER_MAX_UPSPEED;
	float			m_flLaunchDelay;
	float			m_flDamage;
	float			m_flDamageRadius;
	//CHandle<SmokeTrail>	m_hSmokeTrail;
	Vector			m_vSpawnPos;
	Vector			m_vSpawnAng;
	Vector			m_vBounceVel;
	bool			m_bShake;
	bool			m_bRecoil;
	float			m_flStatic;
	float			m_flNextDangerTime;
	float			lasttime_movedforward;
	float			lasttime_sloweddown;
	float			lasttime_resetmouse;
	float			lasttime_movedleft;
	float			lasttime_movedright;
	float			lasttime_hoveredup;
	float			lasttime_hovereddown;
	float			lasttime_resetleft;
	float			lasttime_resetup;
	float			lasttime_animated;
	float			time_pressedusekey;
	float			lasttime_leftclicked;
	float			lasttime_rightclicked;
	float			lasttime_printedhealth;
	float			lasttime_tookdamage;
	float			lasttime_pressedattack3;
	float			time_blewup;
	float			m_flRollSideSpeed;
	float			m_flUpSpeed;
	float			m_flFallSpeed;
	float			lasttime_fell;
	float			UPDATE_RATE;
	double           ClientConnectTime;
	int			HIGH_SPEED;
	float           m_flTimeCreatedElight;
	bool m_bActive;

	int					m_nSaveFOV;
	Vector				m_vSaveOrigin;
	Vector m_vSaveEyeAngles;
	Vector				m_vSaveAngles;
	int			m_nSaveMoveType;
	int m_nSaveTakeDamage;
	int m_nSaveEffects;
	int m_nSaveSolid;
	//MoveCollide_t		m_nSaveMoveCollide;
	Vector				m_vSaveViewOffset;
	//CBaseCombatWeapon*	m_pSaveWeapon;
	edict_t *pVisibleHeliModelEdict;
	edict_t *pCamEdict;
	int SmokeIndex;
	int team;
	entvars_t *pevAttackerEnt;
	int rocketsshot;
};

//Actual helicopter model that players see in the world
class CItemHelicopter : public CBaseEntityCustom {//, C_BaseEntity {
public:
	void Precache(void);
	void Spawn(void);
	static CItemHelicopter* Create(Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CItemHelicopterCamera *pCamera);
	CItemHelicopterCamera *pCamera;
	int team;
};

#endif