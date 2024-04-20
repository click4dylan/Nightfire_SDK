/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

Class Hierachy

CBaseEntity
	CBaseDelay
		CBaseToggle
			CBaseItem
			CBaseCharacter
				CBaseCycler
				CBasePlayer
				CBaseGroup
*/
#define		MAX_PATH_SIZE	10 // max number of nodes available for a path.

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
#define		FCAP_CUSTOMSAVE				0x00000001
#define		FCAP_ACROSS_TRANSITION		0x00000002		// should transfer between transitions
#define		FCAP_MUST_SPAWN				0x00000004		// Spawn after restore
#define		FCAP_DONT_SAVE				0x80000000		// Don't save this
#define		FCAP_IMPULSE_USE			0x00000008		// can be used by the player
#define		FCAP_CONTINUOUS_USE			0x00000010		// can be used by the player
#define		FCAP_ONOFF_USE				0x00000020		// can be used by the player
#define		FCAP_DIRECTIONAL_USE		0x00000040		// Player sends +/- 1 when using (currently only tracktrains)
#define		FCAP_MASTER					0x00000080		// Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
#define		FCAP_FORCE_TRANSITION		0x00000080		// ALWAYS goes across transitions



//#include "engine.h" //dylan added for vector and other things

#include "archtypes.h"     // DAL
#include "saverestore.h"
#include "schedule.h"

#ifndef MONSTEREVENT_H
#include "monsterevent.h"
#endif

// C functions for external declarations that call the appropriate C++ methods

#ifndef CBASE_DLLEXPORT
#ifdef _WIN32
#define CBASE_DLLEXPORT _declspec( dllexport )
#else
#define CBASE_DLLEXPORT __attribute__ ((visibility("default")))
#endif
#endif

#define EXPORT CBASE_DLLEXPORT

//extern "C" CBASE_DLLEXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion );
extern "C" CBASE_DLLEXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );

extern int DispatchSpawn( edict_t *pent );
extern void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd );
extern void DispatchTouch( edict_t *pentTouched, edict_t *pentOther );
extern void DispatchUse( edict_t *pentUsed, edict_t *pentOther );
extern void DispatchThink( edict_t *pent );
extern void DispatchBlocked( edict_t *pentBlocked, edict_t *pentOther );
extern void DispatchSave( edict_t *pent, SAVERESTOREDATA *pSaveData );
extern int  DispatchRestore( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity );
extern void	DispatchObjectCollsionBox( edict_t *pent );
extern void SaveWriteFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount );
extern void SaveReadFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount );
extern void SaveGlobalState( SAVERESTOREDATA *pSaveData );
extern void RestoreGlobalState( SAVERESTOREDATA *pSaveData );
extern void ResetGlobalState( void );

typedef enum { USE_OFF = 0, USE_ON = 1, USE_SET = 2, USE_TOGGLE = 3 } USE_TYPE;

extern void FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

typedef void (CBaseEntity::*BASEPTR)(void);
typedef void (CBaseEntity::*ENTITYFUNCPTR)(CBaseEntity *pOther );
typedef void (CBaseEntity::*USEPTR)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

// For CLASSIFY
#define	CLASS_NONE				0
#define CLASS_MACHINE			1
#define CLASS_PLAYER			2
#define	CLASS_HUMAN_PASSIVE		3
#define CLASS_HUMAN_MILITARY	4
#define CLASS_ALIEN_MILITARY	5
#define CLASS_ALIEN_PASSIVE		6
#define CLASS_ALIEN_MONSTER		7
#define CLASS_ALIEN_PREY		8
#define CLASS_ALIEN_PREDATOR	9
#define CLASS_INSECT			10
#define CLASS_PLAYER_ALLY		11
#define CLASS_PLAYER_BIOWEAPON	12 // hornets and snarks.launched by players
#define CLASS_ALIEN_BIOWEAPON	13 // hornets and snarks.launched by the alien menace
#define	CLASS_BARNACLE			99 // special because no one pays attention to it, and it eats a wide cross-section of creatures.

class CBaseEntity;
class CBaseCharacter;
class CBasePlayerItem;
class CSquadCharacter;
class CSearchLight;
class CDeathCamera;
class CCycler;

#define SF_ALLOW_BULLETS ( 1 << 16 ) // nightfire added this. it's the "Allow Bullets" spawnflags on brush objects
#define	SF_NORESPAWN	( 1 << 30 )// !!!set this bit on guns and stuff that should never respawn.


//
// EHANDLE. Safe way to point to CBaseEntities who may die between frames
//
class EHANDLE
{
private:
	edict_t *m_pent;
	int		m_serialnumber;
public:
	edict_t *Get( void );
	edict_t *Set( edict_t *pent );

	operator int ();

	operator CBaseEntity *();

	CBaseEntity * operator = (CBaseEntity *pEntity);
	CBaseEntity * operator ->();
};


//
// Base Entity.  All entity types derive from this
//
typedef class CBaseEntity
{
public:
	// Constructor.  Set engine to use C/C++ callback functions
	// pointers to engine data

	entvars_t *pev;		// Don't need to save/restore this pointer, the engine resets it

	// path corners
	CBaseEntity			*m_pGoalEnt;// path corner we are heading towards //0x8, correct
	CBaseEntity			*m_pLink;// used for temporary link-list operations. //0xc, correct

	// initialization functions
	virtual void	Spawn( void ) { return; }
	virtual void	Precache( void ) { return; }
	virtual void	KeyValue( KeyValueData* pkvd) { pkvd->fHandled = FALSE; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	virtual int		ObjectCaps( void ) { return FCAP_ACROSS_TRANSITION; }
	virtual void	Activate( void ) {}
	
	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	virtual void	SetObjectCollisionBox( void );

// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
// still realize that they are teammates. (overridden for monsters that form groups)
	virtual int Classify ( void ) { return CLASS_NONE; };
	virtual void DeathNotice ( entvars_t *pevChild ) {}// monster maker children use this to tell the monster maker that they have died.


	static	TYPEDESCRIPTION m_SaveData[];

	virtual void	TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	virtual int		TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	virtual int		TakeHealth( float flHealth, int bitsDamageType );
	virtual void	Killed( entvars_t *pevAttacker, int iGib );
	virtual int		BloodColor( void ) { return DONT_BLEED; }
	virtual void	TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	virtual BOOL    IsTriggered( CBaseEntity *pActivator ) {return TRUE;}
	virtual CBaseCharacter *MyCharacterPointer( void ) { return NULL;}
	virtual CSquadCharacter *MySquadCharacterPointer( void ) { return NULL;}
	virtual CSearchLight* MySearchLightPointer(void) { return NULL; }
	virtual CDeathCamera* MyDeathCameraPointer(void) { return NULL; }
	virtual CCycler* MyCyclerPointer(void) { return NULL; }
	virtual	int		GetToggleState( void ) { return TS_AT_TOP; }
	virtual void	AddPoints( int score, BOOL bAllowNegativeScore ) {}
	virtual void	AddPointsToTeam( int score, BOOL bAllowNegativeScore ) {}
	virtual BOOL	AddPlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual BOOL	RemovePlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual int 	GiveAmmo( int iAmount, const char *szName, int iMax ) { return -1; };
	virtual float	GetDelay( void ) { return 0; }
	virtual int		IsMoving( void ) { return pev->velocity != g_vecZero; }
	virtual void	OverrideReset( void ) {}
	
	//virtual int		DamageDecal( int bitsDamageType );
	// This is ONLY used by the node graph to test movement through a door
	virtual void	SetToggleState( int state ) {}
	//virtual void    StartSneaking( void ) {}
	//virtual void    StopSneaking( void ) {}
	virtual BOOL	OnControls( entvars_t *pev ) { return FALSE; }
	//virtual BOOL    IsSneaking( void ) { return FALSE; }
	virtual BOOL	IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL	IsBSPModel( void ) { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual BOOL	ReflectGauss( void ) { return ( IsBSPModel() && !pev->takedamage ); }
	virtual BOOL	HasTarget( string_t targetname ) { return FStrEq(STRING(targetname), STRING(pev->targetname) ); }
	virtual BOOL    IsInWorld( void );
	virtual	BOOL	IsPlayer( void ) { return FALSE; }
	virtual BOOL    IsEnemy( void ) { return FALSE; }
	virtual BOOL	IsNetClient( void ) { return FALSE; }
	virtual const char *TeamID( void ) { return ""; }
	virtual void BlowUpExplosive(void) { };
//	virtual void	SetActivator( CBaseEntity *pActivator ) {}
	virtual CBaseEntity *GetNextTarget( void );
	virtual BOOL ShouldCollide(edict_t* ent) { return TRUE; }
	virtual BOOL OnSaveGame() { return TRUE; }
	virtual void Think(void) { if (m_pfnThink) (this->*m_pfnThink)(); };
	virtual void Touch(CBaseEntity* pOther) { if (m_pfnTouch) (this->*m_pfnTouch)(pOther); };
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
	{
		if (m_pfnUse)
			(this->*m_pfnUse)(pActivator, pCaller, useType, value);
	}
	virtual void Blocked(CBaseEntity* pOther) { if (m_pfnBlocked) (this->*m_pfnBlocked)(pOther); };
	virtual BOOL BlockedTest(CBaseEntity* pOther) { if (m_pfnBlockedTest) return (this->*m_pfnBlockedTest)(pOther); return TRUE; };
	virtual void UpdateOnRemove(void);
	virtual CBaseEntity* Respawn(void) { return NULL; }

	// used by monsters that are created by the MonsterMaker
	virtual	void UpdateOwner(void) { return; };

	virtual BOOL FBecomeProne(void) { return FALSE; };

	virtual Vector Center() { return (pev->absmax + pev->absmin) * 0.5; }; // center point of entity
	virtual Vector EyePosition() { return pev->origin + pev->view_ofs; };			// position of eyes
	virtual Vector EarPosition() { return pev->origin + pev->view_ofs; };			// position of ears
	virtual Vector BodyTarget(const Vector& posSrc) { return Center(); };		// position to shoot at

	virtual int Illumination() { return GETENTITYILLUM(ENT(pev)); };

	virtual	BOOL FVisible(CBaseEntity* pEntity);
	virtual	BOOL FVisible(const Vector& vecOrigin);

	virtual BOOL ActivateAIEvent(CBaseEntity* pEntity, CBaseEntity* pEntity2) { return FALSE; }
	
	virtual void ActivateNightvision(bool enable) { }
	virtual void ActivateXRay(bool enable) { }
	virtual void ActivateHeat(bool enable) { }

	virtual BOOL IsGrappleTarget() { return FALSE; }
	virtual BOOL CanAnimate() { return FALSE; }
	virtual void ItemDropped() { }
	virtual int DisplayHudInformation(CBaseEntity* other) { return 0; }
	
	// fundamental callbacks
	void (CBaseEntity ::*m_pfnThink)(void); //16 dec on PC/MAC
	void (CBaseEntity ::*m_pfnTouch)( CBaseEntity *pOther ); //20 dec on PC/MAC
	void (CBaseEntity ::*m_pfnUse)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ); //24 dec on PC/MAC
	void (CBaseEntity ::*m_pfnBlocked)( CBaseEntity *pOther ); //28 dec on PC/MAC
	BOOL (CBaseEntity ::*m_pfnBlockedTest)(CBaseEntity* pOther); //32 dec on PC/MAC


	//FIXME todo: dylan: unknown if these are used in Nightfire
#if 1

	// allow engine to allocate instance data
    void *operator new( size_t stAllocateBlock, entvars_t *pev )
	{
		return (void *)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
	};

	// don't use this.
#if defined(_MSC_VER) && _MSC_VER >= 1200 // only build this code if MSVC++ 6.0 or higher
	void operator delete(void *pMem, entvars_t *pev)
	{
		pev->flags |= FL_KILLME;
	};
#endif
#endif

	// common member functions
	void EXPORT SUB_Remove( void );
	void EXPORT SUB_DoNothing( void );
	void EXPORT SUB_StartFadeOut ( void );
	void EXPORT SUB_FadeOut ( void );
	void EXPORT SUB_CallUseToggle( void ) { this->Use( this, this, USE_TOGGLE, 0 ); }
	int			ShouldToggle( USE_TYPE useType, BOOL currentState );
	void		FireBullets( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL  );
	//Vector		FireBulletsPlayer( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL, int shared_rand = 0 );

	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );
	// Do the bounding boxes of these two intersect?
	int		Intersects( CBaseEntity *pOther );
	void	MakeDormant( void );
	int		IsDormant( void );
	BOOL    IsLockedByMaster( void ) { return FALSE; }

	static CBaseEntity *Instance( edict_t *pent )
	{ 
		if ( !pent )
			pent = ENT(0);
		CBaseEntity *pEnt = (CBaseEntity *)GET_PRIVATE(pent); 
		return pEnt; 
	}

	static CBaseEntity *Instance( entvars_t *pev ) { return Instance( ENT( pev ) ); }
	static CBaseEntity *Instance( int eoffset) { return Instance( ENT( eoffset) ); }

	CBaseCharacter *GetCharacterPointer( entvars_t *pevMonster ) 
	{ 
		CBaseEntity *pEntity = Instance( pevMonster );
		if ( pEntity )
			return pEntity->MyCharacterPointer();
		return NULL;
	}
	CBaseCharacter * GetCharacterPointer( edict_t *pentMonster )
	{ 
		CBaseEntity *pEntity = Instance( pentMonster );
		if ( pEntity )
			return pEntity->MyCharacterPointer();
		return NULL;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
	//Dylan commented
	/*
#ifdef _DEBUG
	void FunctionCheck( void *pFunction, char *name ) 
	{ 
		if (pFunction && !NAME_FOR_FUNCTION((uint32)pFunction) )
			ALERT( at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (uint32)pFunction );
	}

	BASEPTR	ThinkSet( BASEPTR func, char *name ) 
	{ 
		m_pfnThink = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnThink)))), name ); 
		return func;
	}
	ENTITYFUNCPTR TouchSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnTouch = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnTouch)))), name ); 
		return func;
	}
	USEPTR	UseSet( USEPTR func, char *name ) 
	{ 
		m_pfnUse = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnUse)))), name ); 
		return func;
	}
	ENTITYFUNCPTR	BlockedSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnBlocked = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnBlocked)))), name ); 
		return func;
	}

#endif
	*/

	// virtual functions used by a few classes
	


	//
	static CBaseEntity *Create( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner = NULL );

	edict_t *edict() { return ENT( pev ); };
	EOFFSET eoffset( ) { return OFFSET( pev ); };
	int	  entindex( ) { return ENTINDEX( edict() ); };

	//We use this variables to store each ammo count.
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;
	//Special stuff for grenades and satchels.
	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_chargeReady;
	int m_fInAttack;

	enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
	int m_fireState;
} C_BaseEntity; 


#if 0

typedef class CBaseEntityCustom
{
public:
	// Constructor.  Set engine to use C/C++ callback functions
	// pointers to engine data

	//entvars_t *pev;		// Don't need to save/restore this pointer, the engine resets it
	entvars_t *pev; //dylan added //0x4

	// path corners
	CBaseEntityCustom			*m_pGoalEnt;// path corner we are heading towards //0x8, correct
	CBaseEntityCustom			*m_pLink;// used for temporary link-list operations. //0xc, correct

	// initialization functions
	virtual void	Spawn(void) { return; }
	virtual void	Precache(void) { return; }
	virtual void	KeyValue(KeyValueData* pkvd) { pkvd->fHandled = FALSE; }
	virtual int		Save(CSave &save) { return 0; }
	virtual int		Restore(CRestore &restore) { return 0; }
	virtual int		ObjectCaps(void) { return FCAP_ACROSS_TRANSITION; }
	virtual void	Activate(void) {}

	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	virtual void	SetObjectCollisionBox(void) { return; }

	// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
	// still realize that they are teammates. (overridden for monsters that form groups)
	virtual int Classify(void) { return CLASS_NONE; };
	virtual void DeathNotice(entvars_t *pevChild) {}// monster maker children use this to tell the monster maker that they have died.


	static	TYPEDESCRIPTION m_SaveData[];

	virtual void	TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { 
		TraceResult *trptr = ptr;
		__asm {
			push bitsDamageType
			push trptr
			push vecDir.z
			push vecDir.y
			push vecDir.x
			push flDamage
			push pevAttacker
			mov ecx, this
			mov eax, 0x42040670 //original TraceAttack
			call eax
		}
		return;
	}
	virtual int		TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) { return 0; }
	virtual int		TakeHealth(float flHealth, int bitsDamageType) { return 0; }
	virtual void	Killed(entvars_t *pevAttacker, int iGib) { return; } //Fixme
	virtual int		BloodColor(void) { return DONT_BLEED; }
	virtual void	TraceBleed(float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { return; }
	virtual BOOL    IsTriggered(CBaseEntityCustom *pActivator) { return TRUE; }
	virtual CBaseMonster *MyMonsterPointer(void) { return NULL; }
	virtual CSquadMonster *MySquadMonsterPointer(void) { return NULL; }
	virtual	int		GetToggleState(void) { return TS_AT_TOP; }
	virtual int     Unknown1(void) { 
		return 0; 
	}
	virtual int     Unknown2(void) { 
		return 0; 
	}
	virtual int     Unknown3(void) { 
		//CBaseEntity is supposed to have this function NULL!!!!!!!!!!!!!!!!!!!!!!!!
		int retVal;
		__asm {
			mov eax, this
			mov eax, dword ptr ds:[eax + 0x78]
			mov retVal, eax
		}
		return retVal;
	}
	virtual void	AddPoints(int score, BOOL bAllowNegativeScore) {}
	virtual void	AddPointsToTeam(int score, BOOL bAllowNegativeScore) {}
	virtual BOOL	AddPlayerItem(CBasePlayerItem *pItem) { return 0; }
	virtual BOOL	RemovePlayerItem(CBasePlayerItem *pItem) { return 0; }
	virtual int 	GiveAmmo(int iAmount, const char *szName, int iMax) { return -1; };
	virtual float	GetDelay(void) { 
		return 0; 
	}
	virtual int		IsMoving(void) { 
		return pev->velocity != g_vecZero; 
	}
	virtual void	OverrideReset(void) {
	}
	virtual int		DamageDecal(int bitsDamageType) { 
		return 1; 
	}
	// This is ONLY used by the node graph to test movement through a door
	//virtual void	SetToggleState( int state ) {}
	//virtual void    StartSneaking( void ) {}
	//virtual void    StopSneaking( void ) {}
	virtual BOOL	OnControls(entvars_t *pev) { 
		return FALSE; 
	}
	//virtual BOOL    IsSneaking( void ) { return FALSE; }
	virtual BOOL	IsAlive(void) { 
		return (pev->deadflag == DEAD_NO) && pev->health > 0; 
	}
	virtual BOOL	IsBSPModel( void ) { 
		return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; 
	}
	//virtual BOOL	ReflectGauss( void ) { return ( IsBSPModel() && !pev->takedamage ); }
	//virtual BOOL	ReflectGauss(void) { 
		//return ((pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP) && !pev->takedamage); 
	//}

	//Unknown what this function is at this time
	virtual BOOL	IsAlive2(void) { 
		float zero = 0.0f;
		BOOL retVal = 0;
		__asm {
			    push esi
				push ecx
			    mov esi, this
				mov eax, dword ptr ds : [esi]
				call dword ptr ds : [eax + 0x88]
				test eax, eax
				jz exitjump
				mov ecx, dword ptr ds:[esi+4]
				fld dword ptr ds:[ecx+0x16c]
				fcomp dword ptr ds:[zero]
				fstsw ax
				test ah, 44
				jpe exitjump
				//mov eax, 1
				mov dword ptr ds:[retVal], 1
				pop ecx
				pop esi
				//retn
			exitjump:
			    //xor eax, eax
				pop ecx
				pop esi
//				retn
		}
		return retVal;
	}

	virtual BOOL	HasTarget(string_t targetname) { 
		return FStrEq(STRING(targetname), STRING(pev->targetname)); 
	}
	virtual BOOL    IsInWorld(void) { 
		return TRUE; 
	}
	virtual	BOOL	IsPlayer(void) { 
		return FALSE; 
	}
	virtual BOOL	IsNetClient(void) {
		return FALSE;
	}
	virtual int	Teamchoice(void) { return 0; }
	virtual const char *TeamID(void) { return ""; }

	virtual void	SetActivator( CBaseEntityCustom *pActivator ) {}
	virtual CBaseEntityCustom *GetNextTarget(void) { return (CBaseEntityCustom*)NULL; }

	virtual BOOL ShouldCollide(int unknown) { 
		return TRUE; 
	}
	virtual byte HasPowerups(void) {
		byte retVal = 0;
		__asm {
			mov ecx, this
			mov eax, dword ptr ds:[ecx + 0x0be0]
			test eax, eax
			jnz exit
			mov eax, dword ptr ds:[ecx + 4]
			cmp dword ptr ds:[eax + 0x108], 0x0F
			je exit
			mov retVal, 1
		    exit :
		}
		return retVal;
	}

	// fundamental callbacks
	void (CBaseEntityCustom ::*m_pfnThink)(void);
	void (CBaseEntityCustom ::*m_pfnTouch)(CBaseEntityCustom *pOther);
	void (CBaseEntityCustom ::*m_pfnUse)(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value);
	void (CBaseEntityCustom ::*m_pfnBlocked)(CBaseEntityCustom *pOther);
	void (CBaseEntityCustom ::*m_pfnUnknown)(CBaseEntityCustom *pOther);

	//Dylan says: Everything after is not guaranteed, table not complete

	virtual void Think(void) { 
		if (m_pfnThink) (this->*m_pfnThink)(); 
	};
	virtual void Touch(CBaseEntityCustom *pOther) { if (m_pfnTouch) (this->*m_pfnTouch)(pOther); };
	virtual void Use(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value)
	{
		if (m_pfnUse)
			(this->*m_pfnUse)(pActivator, pCaller, useType, value);
	}
	virtual void Blocked(CBaseEntityCustom *pOther) { if (m_pfnBlocked) (this->*m_pfnBlocked)(pOther); };
	virtual byte Unknown(CBaseEntityCustom *pOther) { 
		if (m_pfnUnknown) {
			(this->*m_pfnUnknown)(pOther);
			return 0;
		}
		return 1;
	};

	// allow engine to allocate instance data
	void *operator new(size_t stAllocateBlock, entvars_t *pev)
	{
		return (void *)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
	};

	// don't use this.
#if defined(_MSC_VER) && _MSC_VER >= 1200 // only build this code if MSVC++ 6.0 or higher
	void operator delete(void *pMem, entvars_t *pev)
	{
		pev->flags |= FL_KILLME;
	};
#endif

	virtual void UpdateOnRemove(void) {  //dylan changed this to virtual, all entities seem to have it
		__asm {
			mov ecx, this
			mov eax, 0x420C7BC0 //original CBaseEntity::UpdateOnRemove
			call eax
		}
	}

	// common member functions
	void EXPORT SUB_Remove(void);
	void EXPORT SUB_DoNothing(void);
	void EXPORT SUB_StartFadeOut(void);
	void EXPORT SUB_FadeOut(void);
	void EXPORT SUB_CallUseToggle(void) { this->Use(this, this, USE_TOGGLE, 0); }
	int			ShouldToggle(USE_TYPE useType, BOOL currentState);
	void		FireBullets(ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting, Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL);
	Vector		FireBulletsPlayer(ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting, Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL, int shared_rand = 0);

	virtual CBaseEntityCustom *Respawn(void) { return NULL; }

	void SUB_UseTargets(CBaseEntityCustom *pActivator, USE_TYPE useType, float value);
	// Do the bounding boxes of these two intersect?
	int		Intersects(CBaseEntityCustom *pOther);
	void	MakeDormant(void);
	int		IsDormant(void);
	BOOL    IsLockedByMaster(void) { return FALSE; }

	static CBaseEntityCustom *Instance(edict_t *pent)
	{
		if (!pent)
			pent = ENT(0);
		CBaseEntityCustom *pEnt = (CBaseEntityCustom *)GET_PRIVATE(pent);
		return pEnt;
	}

	static CBaseEntityCustom *Instance(entvars_t *pev) { return Instance(ENT(pev)); }
	static CBaseEntityCustom *Instance(int eoffset) { return Instance(ENT(eoffset)); }

	CBaseMonster *GetMonsterPointer(entvars_t *pevMonster)
	{
		CBaseEntityCustom *pEntity = Instance(pevMonster);
		if (pEntity)
			return pEntity->MyMonsterPointer();
		return NULL;
	}
	CBaseMonster *GetMonsterPointer(edict_t *pentMonster)
	{
		CBaseEntityCustom *pEntity = Instance(pentMonster);
		if (pEntity)
			return pEntity->MyMonsterPointer();
		return NULL;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
	//Dylan commented
	/*
	#ifdef _DEBUG
	void FunctionCheck( void *pFunction, char *name )
	{
	if (pFunction && !NAME_FOR_FUNCTION((uint32)pFunction) )
	ALERT( at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (uint32)pFunction );
	}

	BASEPTR	ThinkSet( BASEPTR func, char *name )
	{
	m_pfnThink = func;
	FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntityCustom,m_pfnThink)))), name );
	return func;
	}
	ENTITYFUNCPTR TouchSet( ENTITYFUNCPTR func, char *name )
	{
	m_pfnTouch = func;
	FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntityCustom,m_pfnTouch)))), name );
	return func;
	}
	USEPTR	UseSet( USEPTR func, char *name )
	{
	m_pfnUse = func;
	FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntityCustom,m_pfnUse)))), name );
	return func;
	}
	ENTITYFUNCPTR	BlockedSet( ENTITYFUNCPTR func, char *name )
	{
	m_pfnBlocked = func;
	FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntityCustom,m_pfnBlocked)))), name );
	return func;
	}

	#endif
	*/

	// virtual functions used by a few classes

	// used by monsters that are created by the MonsterMaker
	virtual	void UpdateOwner(void) { return; };


	//
	static CBaseEntityCustom *Create(const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner = NULL) {
		// NOTE: szName must be a pointer to constant memory, e.g. "monster_class" because the entity
		// will keep a pointer to it after this call.
		edict_t	*pent;
		CBaseEntityCustom *pEntity;

		pent = CREATE_NAMED_ENTITY(MAKE_STRING(szName));
		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}
		pEntity = Instance(pent);
		pEntity->pev->owner = pentOwner;
		pEntity->pev->origin = vecOrigin;
		pEntity->pev->angles = vecAngles;
		((void(__cdecl*)(edict_t*))0x420324B0)(pEntity->edict());
		//DispatchSpawn(pEntity->edict());
		return pEntity;
	}

	//virtual BOOL FBecomeProne(void) { return TRUE; };
	edict_t *edict() { return ENT(pev); };
	EOFFSET eoffset() { return OFFSET(pev); };
	int	  entindex() { return ENTINDEX(edict()); };

	//virtual void UNKNOWN_FIXME1(void) {	
		//return; 
	//}
	virtual BOOL FBecomeProne(void) { return TRUE; };

	virtual Vector Center() { 
		return (pev->absmax + pev->absmin) * 0.5; 
	}; // center point of entity
	virtual Vector EyePosition() { 
		return pev->origin + pev->view_ofs; 
	};			// position of eyes
	virtual Vector EarPosition() { 
		return pev->origin + pev->view_ofs; 
	};			// position of ears
	virtual Vector BodyTarget(const Vector &posSrc) { 
		return Center(); 
	};		// position to shoot at

	virtual int Illumination() { 
		return GETENTITYILLUM(ENT(pev)); 
	};

	virtual	BOOL FVisible(CBaseEntityCustom *pEntity) { 
		return FALSE; 
	}
	virtual	BOOL FVisible(const Vector &vecOrigin) { 
		return FALSE; 
	}

	virtual byte FUnknown(int unknown1, int unknown2) {
		return 0;
	}

	virtual void FUnknownQspecs(int unknown1) {
		return;
	}

	virtual void FUnknown2(int unknown1) {
		return;
	}

	virtual void FUnknown3(int unknown1) {
		return;
	}

	virtual byte FUnknown4(void) {
		return 0;
	}

	virtual byte FUnknown5(void) {
		return 0;
	}

	virtual void FUnknown6(void) {
		return;
	}

	virtual BOOL FUnknown7(int unknown) {
		return 0;
	}

	//Real CBaseEntity ends here^

	virtual void UnknownSequenceFunc(void) {
		return;
	}

	virtual void FUnknown8(int unknown) {
		return;
	}

	virtual void FUnknown9(int unknown) {
		return;
	}

	virtual void FUnknown10(int unknown) {
		return;
	}

	//We use this variables to store each ammo count.
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;
	//Special stuff for grenades and satchels.
	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_chargeReady;
	int m_fInAttack;

	enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
	int m_fireState;
	//Custom functions dylan added to make it more Source-Like for entity creation below
	CBaseEntityCustom *GetOwnerEntity(void) { 
		if (!pev->owner)
			return NULL;
		return CBaseEntityCustom::Instance(pev->owner); 
	}
	void SetOwnerEntity(CBaseEntityCustom* entity) { pev->owner = entity == NULL ? NULL : entity->edict(); }
	void SetMoveType(int type) { pev->movetype = type; }
	int GetMoveType(void) {
		return pev->movetype;
	}
	void SetFriction(float friction) { pev->friction = friction; }
	float GetFriction(void) {
		return pev->friction;
	}
	float GetGravity(void) {
		return pev->gravity;
	}
	void SetGravity(float grav) {
		pev->gravity = grav;
	}
	void SetSequence(int seq) { pev->sequence = seq; }
	void SetSolid(int type) { pev->solid = type; }
	int GetSolid(void) {
		return pev->solid;
	}
	void AddFlag(int flags) { pev->flags |= flags; }
	int GetFlags(void) { return pev->flags; }
	void SetModel(const char* str) { SET_MODEL(edict(), str); }
	const char* GetModel(void) { 
		return STRING(pev->model);
	}
	Vector GetLocalOrigin(void) { return pev->origin; }
	void SetLocalOrigin(Vector org) { 
		pev->origin = org;
	}
	Vector GetAbsOrigin(void) { return pev->origin; }
	void SetAbsOrigin(Vector org) {
		pev->origin = org;
		((void(__cdecl*)(edict_t*, qboolean))0x430994D0)(edict(), 1); //SV_LinkEdict, see dylans_functions.h
	}
	Vector GetLocalAngles(void) { return pev->angles; }
	void SetLocalAngles(Vector ang) {
		pev->angles = ang;
	}
	Vector GetAbsAngles(void) { return pev->angles; }
	void SetAbsAngles(Vector ang) {
		pev->angles = ang;
		((void(__cdecl*)(edict_t*, qboolean))0x430994D0)(edict(), 1); //SV_LinkEdict, dylans_functions.h
	}
	void AddEffects(int ef) { pev->effects |= ef; }
	void RemoveEffects(int ef) {
		pev->effects &= ~ef;
	}
	void SetAbsVelocity(Vector vel) { pev->velocity = vel; }
	Vector GetAbsVelocity(void) { return pev->velocity; }
	void SetHealth(float health) { pev->health = health; }
	float GetHealth(void) { return pev->health; }
	void SnapEyeAngles(const Vector &viewAngles) {
		pev->v_angle = viewAngles;
		pev->fixangle = 1;
	}
} C_BaseEntityCustom;

#endif

#if 0
typedef class CBaseEntityCustom
{
public:
	// Constructor.  Set engine to use C/C++ callback functions
	// pointers to engine data

	//entvars_t *pev;		// Don't need to save/restore this pointer, the engine resets it
	entvars_t *pev; //dylan added //0x4

	// path corners
	CBaseEntityCustom			*m_pGoalEnt;// path corner we are heading towards //0x8, correct
	CBaseEntityCustom			*m_pLink;// used for temporary link-list operations. //0xc, correct

	// initialization functions
	virtual void	Spawn( void ) { return; }
	virtual void	Precache( void ) { return; }
	virtual void	KeyValue( KeyValueData* pkvd) { pkvd->fHandled = FALSE; }
	//virtual int		Save( CSave &save );
	//virtual int		Restore( CRestore &restore );
	virtual int		ObjectCaps( void ) { return FCAP_ACROSS_TRANSITION; }
	virtual void	Activate( void ) {}
	
	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	//virtual void	SetObjectCollisionBox( void );

// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
// still realize that they are teammates. (overridden for monsters that form groups)
	virtual int Classify ( void ) { return CLASS_NONE; };
	virtual void DeathNotice ( entvars_t *pevChild ) {}// monster maker children use this to tell the monster maker that they have died.


	static	TYPEDESCRIPTION m_SaveData[];

	//virtual void	TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	//virtual int		TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	//virtual int		TakeHealth( float flHealth, int bitsDamageType );
	//virtual void	Killed( entvars_t *pevAttacker, int iGib );
	virtual int		BloodColor( void ) { return DONT_BLEED; }
	//virtual void	TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	virtual BOOL    IsTriggered( CBaseEntityCustom *pActivator ) {return TRUE;}
	virtual CBaseMonster *MyMonsterPointer( void ) { return NULL;}
	virtual CSquadMonster *MySquadMonsterPointer( void ) { return NULL;}
	virtual	int		GetToggleState( void ) { return TS_AT_TOP; }
	virtual void	AddPoints( int score, BOOL bAllowNegativeScore ) {}
	virtual void	AddPointsToTeam( int score, BOOL bAllowNegativeScore ) {}
	virtual BOOL	AddPlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual BOOL	RemovePlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual int 	GiveAmmo( int iAmount, const char *szName, int iMax ) { return -1; };
	virtual float	GetDelay( void ) { return 0; }
	//virtual int		IsMoving( void ) { return pev->velocity != g_vecZero; }
	virtual void	OverrideReset( void ) {}
	//virtual int		DamageDecal( int bitsDamageType );
	
	virtual BOOL	OnControls( entvars_t *pev ) { return FALSE; }

	virtual BOOL	IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }

	virtual BOOL	ReflectGauss( void ) { return ( (pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP) && !pev->takedamage ); }

	virtual BOOL	IsAlive2( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; } //?

	virtual BOOL	HasTarget( string_t targetname ) { return FStrEq(STRING(targetname), STRING(pev->targetname) ); }
	//virtual BOOL    IsInWorld( void );
	virtual	BOOL	IsPlayer( void ) { return FALSE; }
	virtual BOOL	IsNetClient( void ) { return FALSE; }
	virtual int	Teamchoice(void) { return 0; }
	virtual const char *TeamID( void ) { return ""; }

	//virtual void Unknown(void);

	//virtual CBaseEntityCustom *GetNextTarget( void );

	virtual BOOL ShouldCollide(int unknown) { return TRUE; }
	virtual bool ShouldUnknown(void) { return true; }
	
	// fundamental callbacks
	void (CBaseEntityCustom ::*m_pfnThink)(void);
	void (CBaseEntityCustom ::*m_pfnTouch)( CBaseEntityCustom *pOther );
	void (CBaseEntityCustom ::*m_pfnUse)( CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value );
	void (CBaseEntityCustom ::*m_pfnBlocked)( CBaseEntityCustom *pOther );

	//Dylan says: Everything after is not guaranteed, table not complete

	virtual void Think( void ) { if (m_pfnThink) (this->*m_pfnThink)(); };
	virtual void Touch( CBaseEntityCustom *pOther ) { if (m_pfnTouch) (this->*m_pfnTouch)( pOther ); };
	virtual void Use( CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value ) 
	{ 
		if (m_pfnUse) 
			(this->*m_pfnUse)( pActivator, pCaller, useType, value );
	}
	virtual void Blocked( CBaseEntityCustom *pOther ) { if (m_pfnBlocked) (this->*m_pfnBlocked)( pOther ); };

	// allow engine to allocate instance data
    void *operator new( size_t stAllocateBlock, entvars_t *pev )
	{
		return (void *)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
	};

	// don't use this.
#if defined(_MSC_VER) && _MSC_VER >= 1200 // only build this code if MSVC++ 6.0 or higher
	void operator delete(void *pMem, entvars_t *pev)
	{
		pev->flags |= FL_KILLME;
	};
#endif

	void UpdateOnRemove( void );

	// common member functions
	void EXPORT SUB_Remove( void );
	void EXPORT SUB_DoNothing( void );
	void EXPORT SUB_StartFadeOut ( void );
	void EXPORT SUB_FadeOut ( void );
	void EXPORT SUB_CallUseToggle( void ) { this->Use( this, this, USE_TOGGLE, 0 ); }
	int			ShouldToggle( USE_TYPE useType, BOOL currentState );
	void		FireBullets( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL  );
	Vector		FireBulletsPlayer( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL, int shared_rand = 0 );

	virtual CBaseEntityCustom *Respawn( void ) { return NULL; }

	void SUB_UseTargets( CBaseEntityCustom *pActivator, USE_TYPE useType, float value );
	// Do the bounding boxes of these two intersect?
	int		Intersects( CBaseEntityCustom *pOther );
	void	MakeDormant( void );
	int		IsDormant( void );
	BOOL    IsLockedByMaster( void ) { return FALSE; }

	static CBaseEntityCustom *Instance( edict_t *pent )
	{ 
		if ( !pent )
			pent = ENT(0);
		CBaseEntityCustom *pEnt = (CBaseEntityCustom *)GET_PRIVATE(pent); 
		return pEnt; 
	}

	static CBaseEntityCustom *Instance( entvars_t *pev ) { return Instance( ENT( pev ) ); }
	static CBaseEntityCustom *Instance( int eoffset) { return Instance( ENT( eoffset) ); }

	CBaseMonster *GetMonsterPointer( entvars_t *pevMonster ) 
	{ 
		CBaseEntityCustom *pEntity = Instance( pevMonster );
		if ( pEntity )
			return pEntity->MyMonsterPointer();
		return NULL;
	}
	CBaseMonster *GetMonsterPointer( edict_t *pentMonster ) 
	{ 
		CBaseEntityCustom *pEntity = Instance( pentMonster );
		if ( pEntity )
			return pEntity->MyMonsterPointer();
		return NULL;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
	//Dylan commented
	/*
#ifdef _DEBUG
	void FunctionCheck( void *pFunction, char *name ) 
	{ 
		if (pFunction && !NAME_FOR_FUNCTION((uint32)pFunction) )
			ALERT( at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (uint32)pFunction );
	}

	BASEPTR	ThinkSet( BASEPTR func, char *name ) 
	{ 
		m_pfnThink = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntityCustom,m_pfnThink)))), name ); 
		return func;
	}
	ENTITYFUNCPTR TouchSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnTouch = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntityCustom,m_pfnTouch)))), name ); 
		return func;
	}
	USEPTR	UseSet( USEPTR func, char *name ) 
	{ 
		m_pfnUse = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntityCustom,m_pfnUse)))), name ); 
		return func;
	}
	ENTITYFUNCPTR	BlockedSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnBlocked = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntityCustom,m_pfnBlocked)))), name ); 
		return func;
	}

#endif
	*/

	// virtual functions used by a few classes
	
	// used by monsters that are created by the MonsterMaker
	virtual	void UpdateOwner( void ) { return; };


	//
	static CBaseEntityCustom *Create( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner = NULL );

	virtual BOOL FBecomeProne( void ) {return FALSE;};
	edict_t *edict() { return ENT( pev ); };
	EOFFSET eoffset( ) { return OFFSET( pev ); };
	int	  entindex( ) { return ENTINDEX( edict() ); };

	virtual Vector Center( ) { return (pev->absmax + pev->absmin) * 0.5; }; // center point of entity
	virtual Vector EyePosition( ) { return pev->origin + pev->view_ofs; };			// position of eyes
	virtual Vector EarPosition( ) { return pev->origin + pev->view_ofs; };			// position of ears
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ); };		// position to shoot at

	virtual int Illumination( ) { return GETENTITYILLUM( ENT( pev ) ); };

	//virtual	BOOL FVisible ( CBaseEntityCustom *pEntity );
	//virtual	BOOL FVisible ( const Vector &vecOrigin );

	//We use this variables to store each ammo count.
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;
	//Special stuff for grenades and satchels.
	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_chargeReady;
	int m_fInAttack;

	enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
	int m_fireState;
} C_BaseEntityCustom; 
#endif

























/*
class CBaseCharacter //: public CBaseToggle
{
public:
	entvars_t *pev; //dylan added //0x4

	Activity			m_Activity;// what the monster is doing (animation)
	Activity			m_IdealActivity;// monster should switch to this activity
	int					m_LastHitGroup; // the last body region that took damage
	int					m_bitsDamageType;	// what types of damage has monster (player) taken
	BYTE				m_rgbTimeBasedDamage[CDMG_TIMEBASED];
	MONSTERSTATE		m_MonsterState;// monster's current state
	MONSTERSTATE		m_IdealMonsterState;// monster should change to this state
	int					m_afConditions;
	int					m_afMemory;
	float				m_flNextAttack;		// cannot attack again until this time
	EHANDLE				m_hEnemy;		 // the entity that the monster is fighting.
	EHANDLE				m_hTargetEnt;	 // the entity that the monster is trying to reach
	float				m_flFieldOfView;// width of monster's field of view ( dot product )
	int					m_bloodColor;		// color of blood particless
	Vector				m_HackedGunPos;	// HACK until we can query end of gun
	Vector				m_vecEnemyLKP;// last known position of enemy. (enemy's origin)


	void KeyValue( KeyValueData *pkvd );

	void MakeIdealYaw( Vector vecTarget );
	virtual float ChangeYaw ( int speed );
	virtual BOOL HasHumanGibs( void );
	virtual BOOL HasAlienGibs( void );
	virtual void FadeMonster( void );	// Called instead of GibMonster() when gibs are disabled
	virtual void GibMonster( void );
	virtual Activity GetDeathActivity ( void );
	Activity GetSmallFlinchActivity( void );
	virtual void BecomeDead( void );
	BOOL		 ShouldGibMonster( int iGib );
	void		 CallGibMonster( void );
	virtual BOOL	ShouldFadeOnDeath( void );
	BOOL FCheckAITrigger( void );// checks and, if necessary, fires the monster's trigger target. 
	virtual int IRelationship ( CBaseEntity *pTarget );
	virtual int TakeHealth( float flHealth, int bitsDamageType );
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	int			DeadTakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	float DamageForce( float damage );
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	virtual void PainSound ( void ) { return; };

	void RadiusDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );
	void RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );

	inline void	SetConditions( int iConditions ) { m_afConditions |= iConditions; }
	inline void	ClearConditions( int iConditions ) { m_afConditions &= ~iConditions; }
	inline BOOL HasConditions( int iConditions ) { if ( m_afConditions & iConditions ) return TRUE; return FALSE; }
	inline BOOL HasAllConditions( int iConditions ) { if ( (m_afConditions & iConditions) == iConditions ) return TRUE; return FALSE; }

	inline void	Remember( int iMemory ) { m_afMemory |= iMemory; }
	inline void	Forget( int iMemory ) { m_afMemory &= ~iMemory; }
	inline BOOL HasMemory( int iMemory ) { if ( m_afMemory & iMemory ) return TRUE; return FALSE; }
	inline BOOL HasAllMemories( int iMemory ) { if ( (m_afMemory & iMemory) == iMemory ) return TRUE; return FALSE; }

	// This will stop animation until you call ResetSequenceInfo() at some point in the future
	inline void StopAnimation( void ) { pev->framerate = 0; }

	virtual void ReportAIState( void );
	virtual void MonsterInitDead( void );	// Call after animation/pose is set up
	void EXPORT CorpseFallThink( void );

	virtual void Look ( int iDistance );// basic sight function for monsters
	virtual CBaseEntity* BestVisibleEnemy ( void );// finds best visible enemy for attack
	CBaseEntity *CheckTraceHullAttack( float flDist, int iDamage, int iDmgType );
	virtual BOOL FInViewCone ( CBaseEntity *pEntity );// see if pEntity is in monster's view cone
	virtual BOOL FInViewCone ( Vector *pOrigin );// see if given location is in monster's view cone
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	void MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir );
	virtual BOOL	IsAlive( void ) { return (pev->deadflag != DEAD_DEAD); }

};*/

class C_BasePlayer //: public CBaseCharacter //Dylan added
{
public:
	// Constructor.  Set engine to use C/C++ callback functions
	// pointers to engine data

	//entvars_t *pev;		// Don't need to save/restore this pointer, the engine resets it
	entvars_t *pev; //dylan added //0x4

	// path corners
	C_BasePlayer			*m_pGoalEnt;// path corner we are heading towards //0x8, correct
	C_BasePlayer			*m_pLink;// used for temporary link-list operations. //0xc, correct

	// initialization functions
	virtual void	Spawn( void ) { return; }
	virtual void	Precache( void ) { return; }
	virtual void	KeyValue( KeyValueData* pkvd) { pkvd->fHandled = FALSE; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
// still realize that they are teammates. (overridden for monsters that form groups)
	virtual int Classify ( void ) { return CLASS_NONE; };

	virtual void	Activate( void ) {}
	
	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	virtual void	SetObjectCollisionBox( void );
	virtual int		ObjectCaps( void ) { return FCAP_ACROSS_TRANSITION; }

	virtual void DeathNotice ( entvars_t *pevChild ) {}// monster maker children use this to tell the monster maker that they have died.


	static	TYPEDESCRIPTION m_SaveData[];

	virtual void	TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	virtual int		TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	virtual int		TakeHealth( float flHealth, int bitsDamageType );
	virtual void	Killed( entvars_t *pevAttacker, int iGib );
	virtual int		BloodColor( void ) { return DONT_BLEED; }
	virtual void	TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	virtual BOOL    IsTriggered( C_BasePlayer *pActivator ) {return TRUE;}
	virtual CBaseCharacter *MyCharacterPointer( void ) { return NULL;}
	virtual CSquadCharacter *MySquadCharacterPointer( void ) { return NULL;}

	virtual int		Unknown1(void) { return NULL; };
	virtual int		Unknown2(void) { return NULL; };
	virtual int		Unknown3(void) { return NULL; };

	virtual	int		GetToggleState( void ) { return TS_AT_TOP; }

	virtual void	AddPoints( int score, BOOL bAllowNegativeScore ) {}
	virtual void	AddPointsToTeam( int score, BOOL bAllowNegativeScore ) {}
	virtual BOOL	AddPlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual BOOL	RemovePlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual int 	GiveAmmo( int iAmount, const char *szName, int iMax ) { return -1; };
	virtual float	GetDelay( void ) { return 0; }
	virtual int		IsMoving( void ) { return pev->velocity != g_vecZero; }
	virtual void	OverrideReset( void ) {}
	virtual int		DamageDecal( int bitsDamageType );
	// This is ONLY used by the node graph to test movement through a door
	virtual void	SetToggleState( int state ) {}
	//virtual void    StartSneaking( void ) {}
	//virtual void    StopSneaking( void ) {}
	virtual BOOL	IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; } //?
	//virtual BOOL	OnControls( entvars_t *pev ) { return FALSE; }
	//virtual BOOL    IsSneaking( void ) { return FALSE; }
	virtual BOOL	IsBSPModel( void ) { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	//virtual BOOL	ReflectGauss( void ) { return ( IsBSPModel() && !pev->takedamage ); }
	//virtual BOOL	ReflectGauss( void ) { return ( (pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP) && !pev->takedamage ); }

	virtual BOOL	IsAlive2( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; } //?


	virtual BOOL	HasTarget( string_t targetname ) { return FStrEq(STRING(targetname), STRING(pev->targetname) ); }
	virtual BOOL    IsInWorld( void );
	virtual	BOOL	IsPlayer( void ) { return TRUE; }
	virtual BOOL	IsNetClient( void ) { return FALSE; }
	virtual bool	ShouldUnknown(void) { return true; }
	virtual const char *TeamID( void ) { return ""; }
	virtual void Unknown4(void ) { }


//	virtual void	SetActivator( C_BasePlayer *pActivator ) {}
	virtual C_BasePlayer *GetNextTarget( void );
	
	virtual BOOL Unknown5 ( int Unknown ) { return TRUE; }

	virtual BOOL HasPowerups (void) { return FALSE; }


	// fundamental callbacks
	void (C_BasePlayer ::*m_pfnThink)(void);
	void (C_BasePlayer ::*m_pfnTouch)( C_BasePlayer *pOther );
	void (C_BasePlayer ::*m_pfnUse)( C_BasePlayer *pActivator, C_BasePlayer *pCaller, USE_TYPE useType, float value );
	void (C_BasePlayer ::*m_pfnBlocked)( C_BasePlayer *pOther );

	//DYLAN TOFIX: everything after here is not guaranteed, didn't finish

	virtual void Think( void ) { if (m_pfnThink) (this->*m_pfnThink)(); };
	virtual void Touch( C_BasePlayer *pOther ) { if (m_pfnTouch) (this->*m_pfnTouch)( pOther ); };
	virtual void Use( C_BasePlayer *pActivator, C_BasePlayer *pCaller, USE_TYPE useType, float value ) 
	{ 
		if (m_pfnUse) 
			(this->*m_pfnUse)( pActivator, pCaller, useType, value );
	}
	virtual void Blocked( C_BasePlayer *pOther ) { if (m_pfnBlocked) (this->*m_pfnBlocked)( pOther ); };

	// allow engine to allocate instance data
    void *operator new( size_t stAllocateBlock, entvars_t *pev )
	{
		return (void *)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
	};

	// don't use this.
#if defined(_MSC_VER) && _MSC_VER >= 1200 // only build this code if MSVC++ 6.0 or higher
	void operator delete(void *pMem, entvars_t *pev)
	{
		pev->flags |= FL_KILLME;
	};
#endif

	void UpdateOnRemove( void );

	// common member functions
	void EXPORT SUB_Remove( void );
	void EXPORT SUB_DoNothing( void );
	void EXPORT SUB_StartFadeOut ( void );
	void EXPORT SUB_FadeOut ( void );
	void EXPORT SUB_CallUseToggle( void ) { this->Use( this, this, USE_TOGGLE, 0 ); }
	int			ShouldToggle( USE_TYPE useType, BOOL currentState );
	virtual void		FireBullets( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL  );
	Vector		FireBulletsPlayer( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL, int shared_rand = 0 );

	virtual C_BasePlayer *Respawn( void ) { return NULL; }

	void SUB_UseTargets( C_BasePlayer *pActivator, USE_TYPE useType, float value );
	// Do the bounding boxes of these two intersect?
	int		Intersects( C_BasePlayer *pOther );
	void	MakeDormant( void );
	int		IsDormant( void );
	BOOL    IsLockedByMaster( void ) { return FALSE; }

	static C_BasePlayer *Instance( edict_t *pent )
	{ 
		if ( !pent )
			pent = ENT(0);
		C_BasePlayer *pEnt = (C_BasePlayer *)GET_PRIVATE(pent); 
		return pEnt; 
	}

	static C_BasePlayer *Instance( entvars_t *pev ) { return Instance( ENT( pev ) ); }
	static C_BasePlayer *Instance( int eoffset) { return Instance( ENT( eoffset) ); }

	CBaseCharacter *GetMonsterPointer( entvars_t *pevMonster ) 
	{ 
		C_BasePlayer *pEntity = Instance( pevMonster );
		if ( pEntity )
			return pEntity->MyCharacterPointer();
		return NULL;
	}
	CBaseCharacter *GetMonsterPointer( edict_t *pentMonster ) 
	{ 
		C_BasePlayer *pEntity = Instance( pentMonster );
		if ( pEntity )
			return pEntity->MyCharacterPointer();
		return NULL;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
	//Dylan commented
	/*
#ifdef _DEBUG
	void FunctionCheck( void *pFunction, char *name ) 
	{ 
		if (pFunction && !NAME_FOR_FUNCTION((uint32)pFunction) )
			ALERT( at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (uint32)pFunction );
	}

	BASEPTR	ThinkSet( BASEPTR func, char *name ) 
	{ 
		m_pfnThink = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(C_BasePlayer,m_pfnThink)))), name ); 
		return func;
	}
	ENTITYFUNCPTR TouchSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnTouch = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(C_BasePlayer,m_pfnTouch)))), name ); 
		return func;
	}
	USEPTR	UseSet( USEPTR func, char *name ) 
	{ 
		m_pfnUse = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(C_BasePlayer,m_pfnUse)))), name ); 
		return func;
	}
	ENTITYFUNCPTR	BlockedSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnBlocked = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(C_BasePlayer,m_pfnBlocked)))), name ); 
		return func;
	}

#endif
	*/

	// virtual functions used by a few classes
	
	// used by monsters that are created by the MonsterMaker
	virtual	void UpdateOwner( void ) { return; };


	//
	static C_BasePlayer *Create( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner = NULL );

	virtual BOOL FBecomeProne( void ) {return FALSE;};
	edict_t *edict() { return ENT( pev ); };
	EOFFSET eoffset( ) { return OFFSET( pev ); };
	int	  entindex( ) { return ENTINDEX( edict() ); };

	virtual Vector Center( ) { return (pev->absmax + pev->absmin) * 0.5; }; // center point of entity
	virtual Vector EyePosition( ) { return pev->origin + pev->view_ofs; };			// position of eyes
	virtual Vector EarPosition( ) { return pev->origin + pev->view_ofs; };			// position of ears
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ); };		// position to shoot at

	virtual int Illumination( ) { return GETENTITYILLUM( ENT( pev ) ); };

	virtual	BOOL FVisible ( C_BasePlayer *pEntity );
	virtual	BOOL FVisible ( const Vector &vecOrigin );

	//We use this variables to store each ammo count.
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;
	//Special stuff for grenades and satchels.
	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_chargeReady;
	int m_fInAttack;

	enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
	int m_fireState;
	unsigned char unknownPlayerTableEntries[2780];
	int deaths;
	unsigned char unknownPlayerTableEntries2[40];
	int team;
	int unknownPlayerTableEntry1;
	int unknownPlayerTableEntry2;
	int ctf_score;
	int unknownPlayerTableEntry3;
	int unknownPlayerTableEntry4;
	int powerup_flags;
	unsigned char unknownPlayertableEntries3[196];
	bool IsOddjob;
}; //Dylan added C_BasePlayer



// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a 
// member function of a base class.  static_cast is a sleezy way around that problem.
#define SetThinkCustom( a ) m_pfnThink = static_cast <void (CBaseEntityCustom::*)(void)> (a)
#define SetTouchCustom( a ) m_pfnTouch = static_cast <void (CBaseEntityCustom::*)(CBaseEntityCustom *)> (a)
#define SetUseCustom( a ) m_pfnUse = static_cast <void (CBaseEntityCustom::*)( CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value )> (a)
#define SetBlockedCustom( a ) m_pfnBlocked = static_cast <void (CBaseEntityCustom::*)(CBaseEntityCustom *)> (a)
#define SetNextThink( cbaseentity, time ) cbaseentity->pev->nextthink = time;

#ifdef _DEBUG

#define SetThink( a ) ThinkSet( static_cast <void (CBaseEntity::*)(void)> (a), #a )
#define SetTouch( a ) TouchSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )
#define SetUse( a ) UseSet( static_cast <void (CBaseEntity::*)(	CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a), #a )
#define SetBlocked( a ) BlockedSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )

#else

#define SetThink( a ) m_pfnThink = static_cast <void (CBaseEntity::*)(void)> (a)
#define SetTouch( a ) m_pfnTouch = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)
#define SetUse( a ) m_pfnUse = static_cast <void (CBaseEntity::*)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a)
#define SetBlocked( a ) m_pfnBlocked = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)

#endif


class CPointEntity : public CBaseEntity
{
public:
	void	Spawn( void );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
private:
};


typedef struct locksounds			// sounds that doors and buttons make when locked/unlocked
{
	string_t	sLockedSound;		// sound a door makes when it's locked
	string_t	sLockedSentence;	// sentence group played when door is locked
	string_t	sUnlockedSound;		// sound a door makes when it's unlocked
	string_t	sUnlockedSentence;	// sentence group played when door is unlocked

	int		iLockedSentence;		// which sentence in sentence group to play next
	int		iUnlockedSentence;		// which sentence in sentence group to play next

	float	flwaitSound;			// time delay between playing consecutive 'locked/unlocked' sounds
	float	flwaitSentence;			// time delay between playing consecutive sentences
	BYTE	bEOFLocked;				// true if hit end of list of locked sentences
	BYTE	bEOFUnlocked;			// true if hit end of list of unlocked sentences
} locksound_t;

void PlayLockSounds(entvars_t *pev, locksound_t *pls, int flocked, int fbutton);

//
// MultiSouce
//

#define MAX_MULTI_TARGETS	16 // maximum number of targets a single multi_manager entity may be assigned.
#define MS_MAX_TARGETS 32

class CMultiSource : public CPointEntity
{
public:
	void Spawn( );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int	ObjectCaps( void ) { return (CPointEntity::ObjectCaps() | FCAP_MASTER); }
	BOOL IsTriggered( CBaseEntity *pActivator );
	void EXPORT Register( void );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE		m_rgEntities[MS_MAX_TARGETS];
	int			m_rgTriggered[MS_MAX_TARGETS];

	int			m_iTotal;
	string_t	m_globalstate;
};


//
// generic Delay entity.
//
class CBaseDelay : public CBaseEntity
{
public:
	float		m_flDelay;
	int			m_iszKillTarget;

	virtual void	KeyValue( KeyValueData* pkvd);
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];
	// common member functions
	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );
	void EXPORT DelayThink( void );
};


class CBaseAnimating : public CBaseDelay
{
public:
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	// Basic Monster Animation functions
	float StudioFrameAdvance( float flInterval = 0.0 ); // accumulate animation frame time from last time called until now
	int	 GetSequenceFlags( void );
	int  LookupActivity ( int activity );
	int  LookupActivityHeaviest ( int activity );
	int  LookupSequence ( const char *label );
	void ResetSequenceInfo ( );
	void DispatchAnimEvents ( float flFutureInterval = 0.1 ); // Handle events that have happend since last time called up until X seconds into the future
	virtual void HandleAnimEvent( MonsterEvent_t *pEvent ) { return; };
	float SetBoneController ( int iController, float flValue );
	void InitBoneControllers ( void );
	float SetBlending ( int iBlender, float flValue );
	void GetBonePosition ( int iBone, Vector &origin, Vector &angles );
	void GetAutomovement( Vector &origin, Vector &angles, float flInterval = 0.1 );
	int  FindTransition( int iEndingSequence, int iGoalSequence, int *piDir );
	void GetAttachment ( int iAttachment, Vector &origin, Vector &angles );
	void SetBodygroup( int iGroup, int iValue );
	int GetBodygroup( int iGroup );
	int ExtractBbox( int sequence, float *mins, float *maxs );
	void SetSequenceBox( void );

	// animation needs
	float				m_flFrameRate;		// computed FPS for current sequence
	float				m_flGroundSpeed;	// computed linear movement rate for current sequence
	float				m_flLastEventCheck;	// last time the event list was checked
	BOOL				m_fSequenceFinished;// flag set when StudioAdvanceFrame moves across a frame boundry
	BOOL				m_fSequenceLoops;	// true if the sequence loops
};


//
// generic Toggle entity.
//
#define	SF_ITEM_USE_ONLY	256 //  ITEM_USE_ONLY = BUTTON_USE_ONLY = DOOR_USE_ONLY!!! 

class CBaseToggle : public CBaseAnimating
{
public:
	void				KeyValue( KeyValueData *pkvd );

	TOGGLE_STATE		m_toggle_state;
	float				m_flActivateFinished;//like attack_finished, but for doors
	float				m_flMoveDistance;// how far a door should slide or rotate
	float				m_flWait;
	float				m_flLip;
	float				m_flTWidth;// for plats
	float				m_flTLength;// for plats

	Vector				m_vecPosition1;
	Vector				m_vecPosition2;
	Vector				m_vecAngle1;
	Vector				m_vecAngle2;

	int					m_cTriggersLeft;		// trigger_counter only, # of activations remaining
	float				m_flHeight;
	EHANDLE				m_hActivator;
	void (CBaseToggle::*m_pfnCallWhenMoveDone)(void);
	Vector				m_vecFinalDest;
	Vector				m_vecFinalAngle;

	int					m_bitsDamageInflict;	// DMG_ damage type that the door or tigger does

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	virtual int		GetToggleState( void ) { return m_toggle_state; }
	virtual float	GetDelay( void ) { return m_flWait; }

	// common member functions
	void LinearMove( Vector	vecDest, float flSpeed );
	void EXPORT LinearMoveDone( void );
	void AngularMove( Vector vecDestAngle, float flSpeed );
	void EXPORT AngularMoveDone( void );
	BOOL IsLockedByMaster( void );

	static float		AxisValue( int flags, const Vector &angles );
	static void			AxisDir( entvars_t *pev );
	static float		AxisDelta( int flags, const Vector &angle1, const Vector &angle2 );

	string_t m_sMaster;		// If this button has a master switch, this is the targetname.
							// A master switch must be of the multisource type. If all 
							// of the switches in the multisource have been triggered, then
							// the button will be allowed to operate. Otherwise, it will be
							// deactivated.
};
#define SetMoveDone( a ) m_pfnCallWhenMoveDone = static_cast <void (CBaseToggle::*)(void)> (a)


// people gib if their health is <= this at the time of death
#define	GIB_HEALTH_VALUE	-30

#define	ROUTE_SIZE			8 // how many waypoints a monster can store at one time
#define MAX_OLD_ENEMIES		4 // how many old enemies to remember

#define	bits_CAP_DUCK			( 1 << 0 )// crouch
#define	bits_CAP_JUMP			( 1 << 1 )// jump/leap
#define bits_CAP_STRAFE			( 1 << 2 )// strafe ( walk/run sideways)
#define bits_CAP_SQUAD			( 1 << 3 )// can form squads
#define	bits_CAP_SWIM			( 1 << 4 )// proficiently navigate in water
#define bits_CAP_CLIMB			( 1 << 5 )// climb ladders/ropes
#define bits_CAP_USE			( 1 << 6 )// open doors/push buttons/pull levers
#define bits_CAP_HEAR			( 1 << 7 )// can hear forced sounds
#define bits_CAP_AUTO_DOORS		( 1 << 8 )// can trigger auto doors
#define bits_CAP_OPEN_DOORS		( 1 << 9 )// can open manual doors
#define bits_CAP_TURN_HEAD		( 1 << 10)// can turn head, always bone controller 0

#define bits_CAP_RANGE_ATTACK1	( 1 << 11)// can do a range attack 1
#define bits_CAP_RANGE_ATTACK2	( 1 << 12)// can do a range attack 2
#define bits_CAP_MELEE_ATTACK1	( 1 << 13)// can do a melee attack 1
#define bits_CAP_MELEE_ATTACK2	( 1 << 14)// can do a melee attack 2

#define bits_CAP_FLY			( 1 << 15)// can fly, move all around

#define bits_CAP_DOORS_GROUP    (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS)

// used by suit voice to indicate damage sustained and repaired type to player

// instant damage

#define DMG_GENERIC			0			// generic damage was done
#define DMG_CRUSH			(1 << 0)	// crushed by falling or moving object
#define DMG_BULLET			(1 << 1)	// shot
#define DMG_SLASH			(1 << 2)	// cut, clawed, stabbed
#define DMG_BURN			(1 << 3)	// heat burned
#define DMG_FREEZE			(1 << 4)	// frozen
#define DMG_FALL			(1 << 5)	// fell too far
#define DMG_BLAST			(1 << 6)	// explosive blast damage
#define DMG_CLUB			(1 << 7)	// crowbar, punch, headbutt
#define DMG_SHOCK			(1 << 8)	// electric shock
#define DMG_SONIC			(1 << 9)	// sound pulse shockwave
#define DMG_ENERGYBEAM		(1 << 10)	// laser or other high energy beam 
#define DMG_NEVERGIB		(1 << 12)	// with this bit OR'd in, no damage type will be able to gib victims upon death
#define DMG_ALWAYSGIB		(1 << 13)	// with this bit OR'd in, any damage type can be made to gib victims upon death.
#define DMG_DROWN			(1 << 14)	// Drowning
// time-based damage
#define DMG_TIMEBASED		(~(0x3fff))	// mask for time-based damage

#define DMG_PARALYZE		(1 << 15)	// slows affected creature down
#define DMG_NERVEGAS		(1 << 16)	// nerve toxins, very bad
#define DMG_POISON			(1 << 17)	// blood poisioning
#define DMG_RADIATION		(1 << 18)	// radiation exposure
#define DMG_DROWNRECOVER	(1 << 19)	// drowning recovery
#define DMG_ACID			(1 << 20)	// toxic chemicals or acid burns
#define DMG_SLOWBURN		(1 << 21)	// in an oven
#define DMG_SLOWFREEZE		(1 << 22)	// in a subzero freezer
#define DMG_MORTAR			(1 << 23)	// Hit by air raid (done to distinguish grenade from mortar)

// these are the damage types that are allowed to gib corpses
#define DMG_GIB_CORPSE		( DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB )

// these are the damage types that have client hud art
#define DMG_SHOWNHUD		(DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK)

// NOTE: tweak these values based on gameplay feedback:

#define PARALYZE_DURATION	2		// number of 2 second intervals to take damage
#define PARALYZE_DAMAGE		1.0		// damage to take each 2 second interval

#define NERVEGAS_DURATION	2
#define NERVEGAS_DAMAGE		5.0

#define POISON_DURATION		5
#define POISON_DAMAGE		2.0

#define RADIATION_DURATION	2
#define RADIATION_DAMAGE	1.0

#define ACID_DURATION		2
#define ACID_DAMAGE			5.0

#define SLOWBURN_DURATION	2
#define SLOWBURN_DAMAGE		1.0

#define SLOWFREEZE_DURATION	2
#define SLOWFREEZE_DAMAGE	1.0


#define	itbd_Paralyze		0		
#define	itbd_NerveGas		1
#define	itbd_Poison			2
#define	itbd_Radiation		3
#define	itbd_DrownRecover	4
#define	itbd_Acid			5
#define	itbd_SlowBurn		6
#define	itbd_SlowFreeze		7
#define CDMG_TIMEBASED		8

// when calling KILLED(), a value that governs gib behavior is expected to be 
// one of these three values
#define GIB_NORMAL			0// gib if entity was overkilled
#define GIB_NEVER			1// never gib, no matter how much death damage is done ( freezing, etc )
#define GIB_ALWAYS			2// always gib ( Houndeye Shock, Barnacle Bite )

class CBaseCharacter;
class CCineMonster;
class CSound;

#include "basemonster.h"


const char *ButtonSound( int sound );				// get string of button sound number


//
// Generic Button
//
class CBaseButton : public CBaseToggle
{
public:
	void Spawn( void );
	virtual void Precache( void );
	void RotSpawn( void );
	virtual void KeyValue( KeyValueData* pkvd);

	void ButtonActivate( );
	void SparkSoundCache( void );

	void EXPORT ButtonShot( void );
	void EXPORT ButtonTouch( CBaseEntity *pOther );
	void EXPORT ButtonSpark ( void );
	void EXPORT TriggerAndWait( void );
	void EXPORT ButtonReturn( void );
	void EXPORT ButtonBackHome( void );
	void EXPORT ButtonUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int		TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	enum BUTTON_CODE { BUTTON_NOTHING, BUTTON_ACTIVATE, BUTTON_RETURN };
	BUTTON_CODE	ButtonResponseToTouch( void );
	
	static	TYPEDESCRIPTION m_SaveData[];
	// Buttons that don't take damage can be IMPULSE used
	virtual int	ObjectCaps( void ) { return (CBaseToggle:: ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | (pev->takedamage?0:FCAP_IMPULSE_USE); }

	BOOL	m_fStayPushed;	// button stays pushed in until touched again?
	BOOL	m_fRotating;		// a rotating button?  default is a sliding button.

	string_t m_strChangeTarget;	// if this field is not null, this is an index into the engine string array.
							// when this button is touched, it's target entity's TARGET field will be set
							// to the button's ChangeTarget. This allows you to make a func_train switch paths, etc.

	locksound_t m_ls;			// door lock sounds
	
	BYTE	m_bLockedSound;		// ordinals from entity selection
	BYTE	m_bLockedSentence;	
	BYTE	m_bUnlockedSound;	
	BYTE	m_bUnlockedSentence;
	int		m_sounds;
};

//
// Weapons 
//

#define	BAD_WEAPON 0x00007FFF

//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
#if 1
template <class T> __forceinline T * GetClassPtr( T *a )
{
	entvars_t *pev = (entvars_t *)a;

	// allocate entity if necessary
	if (pev == NULL) {
		pev = VARS(CREATE_ENTITY());
	}

	// get the private data
	a = (T *)GET_PRIVATE(ENT(pev));

	if (a == NULL)
	{
		// allocate private data 
		a = new(pev)T;
		a->pev = pev;
	}
	return a;
}
#else
template <class T> __forceinline T * GetClassPtr(T *a)
{
	//Recreation of GetClassPtr
	pvPrivateData_t *pvPrivateData;
	entvars_t *vars = (entvars_t*)a;
	if (a == NULL)
		vars = VARS(CREATE_ENTITY());
	
	edict_t *pEntity = vars->pContainingEntity;
	//int wrong = (int)&a->weapons;
	//int wrong2 = (int)a;
	//int fuck = wrong - wrong2;

	if (pEntity != NULL)
	{
		pvPrivateData = pEntity->v.pvPrivateData; //(pvPrivateData_t*)GET_PRIVATE(pEntity);
		if (pvPrivateData != NULL)
			return (T*)pvPrivateData;
	}
	pvPrivateData = (pvPrivateData_t*)ALLOC_PRIVATE(pEntity, sizeof(CBombTarget));
	CBombTarget *pEnt;

	if (pvPrivateData != NULL)
	{
		pvPrivateData->MyClassPtr = &a->Spawn;
		pvPrivateData->pvPrivateData = a; //vars?
		return (T*)pvPrivateData;
	}
	else
	{
		pvPrivateData->pvPrivateData = a; //vars?
		return NULL;
	}
}
#endif
/*
bit_PUSHBRUSH_DATA | bit_TOGGLE_DATA
bit_MONSTER_DATA
bit_DELAY_DATA
bit_TOGGLE_DATA | bit_DELAY_DATA | bit_MONSTER_DATA
bit_PLAYER_DATA | bit_MONSTER_DATA
bit_MONSTER_DATA | CYCLER_DATA
bit_LIGHT_DATA
path_corner_data
bit_MONSTER_DATA | wildcard_data
bit_MONSTER_DATA | bit_GROUP_DATA
boid_flock_data
boid_data
CYCLER_DATA
bit_ITEM_DATA
bit_ITEM_DATA | func_hud_data
bit_TOGGLE_DATA | bit_ITEM_DATA
EOFFSET
env_sound_data
env_sound_data
push_trigger_data
*/

#define TRACER_FREQ		4			// Tracers fire every 4 bullets

typedef struct _SelAmmo
{
	BYTE	Ammo1Type;
	BYTE	Ammo1;
	BYTE	Ammo2Type;
	BYTE	Ammo2;
} SelAmmo;


// this moved here from world.cpp, to allow classes to be derived from it
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================
class CWorld : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );
};
