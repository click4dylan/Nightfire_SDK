#ifndef CSTRIKE_H
#define CSTRIKE_H
#include "engine.h"
#include "Dylans_Functions.h"
#include "DynamicLight.h"

void callOnHudInit(edict_t *pEntity);
//void hookOnPlayerTakeDamage();
//void callOnPlayerTakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
void callOnPlayerKilled(entvars_t *pevAttacker, int iGib, C_BasePlayer *pVictim);
FARPROC callOnGetEntityInit(LPCSTR lpProcName); //commentme


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

enum WeaponType : int{
	Gun = 0,
	Shotgun,
	Explosion
};

class CBlood : public CBaseEntityCustom 
{
public:
	CBlood::CBlood(); //Constructor
	CBlood::~CBlood(); //Destructor
	void Precache(void);
	void Spawn(void);
	void Think(void);
	static CBlood* Create(Vector vecOrigin, WeaponType weapontype);
	WeaponType hittype;
	int bloodsize1;
	int bloodsize2;
	int bloodsize3;
	Vector vecInitialOrigin;
};

void OnKeyValue(edict_t *pentKeyvalue, KeyValueData *pkvd);

#endif