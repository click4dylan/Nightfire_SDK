#include "Blood.h"
#include "DylansHooks.h"

#include <math.h>

cvar_t* violence_hblood = nullptr;

DLL_GLOBAL const Vector g_vecZero = Vector(0, 0, 0);

FARPROC callOnGetEntityInit(LPCSTR lpProcName) {

	//if ( !strcmp(lpProcName, "func_bomb_target") ) {
        HMODULE hMod = GetModuleHandle("blood_amxx.dll");
		if ( hMod ) {
			//FARPROC test = GetProcAddress(hMod, "bomb_target");
			//return test;
			return  GetProcAddress(hMod, lpProcName);
		}
	//}
	return NULL;
}

void callOnSpawnBlood(float X, float Y, float Z, int bloodColor, float flDamage) {
	if (violence_hblood->GetBool())
	{
		if (bloodColor != DONT_BLEED) {
			CBlood *pBlood = (CBlood*)CBaseEntityCustom::Create("blood_hit", Vector(X, Y, Z), Vector(0,0,0), NULL);
#if 0
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
#endif
		}
	}
}

void CBlood::Precache(void) {
}

CBlood *CBlood::Create(Vector vecOrigin, WeaponType weapontype) {
	CBlood *pBlood = GetClassPtr((CBlood *)NULL);
	SET_ORIGIN(pBlood->edict(), vecOrigin);
	pBlood->Spawn();
	return pBlood;
}

LINK_ENTITY_TO_CLASS(blood_hit, CBlood);

CBlood::CBlood()
{
}

CBlood::~CBlood()
{
}

void CBlood::Spawn(void) {
	vecInitialOrigin = GetAbsOrigin();
	hittype = Gun;
	pev->spawnflags |= SF_NORESPAWN;
	pev->classname = MAKE_STRING("blood_hit");
	pev->targetname = MAKE_STRING("blood_hit");
	pev->flags |= EF_NODRAW;
	Precache();
	SetSolid(SOLID_NOT);
	//SET_MODEL(edict(), "");
	SET_SIZE(edict(), Vector(-1, -1, -1), Vector(1, 1, 1));
	SetObjectCollisionBox();
	//pev->mins = Vector(-25, -25, -25);
	//pev->absmin = Vector(-25, -25, -25);
	//pev->maxs = Vector(25, 25, 25);
	//pev->absmax = Vector(25, 25, 25);
	SetMoveType(MOVETYPE_NONE);
	//SetFriction(0.55f);
	//SetGravity(0.5f);
	//pev->max_health = helicopter_health->GetFloat();
	//pev->health = pev->max_health;
	pev->takedamage = DAMAGE_NO; //fixme
	
	bloodsize1 = RANDOM_LONG(150, 255);
	bloodsize2 = RANDOM_LONG(150, 175);
	bloodsize3 = RANDOM_LONG(25, 45);

	SetThinkCustom(&CBlood::Think);
	SetNextThink(this, gpGlobals->time);
}

void CBlood::Think(void)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecInitialOrigin);
	WRITE_BYTE(TE_SPRITE);
	WRITE_COORD(vecInitialOrigin.x);		// position (X)
	WRITE_COORD(vecInitialOrigin.y);		// position (Y)
	WRITE_COORD(vecInitialOrigin.z);		// position (Z)
	WRITE_SHORT(glinfo.BloodSprite);			// sprite index
	WRITE_BYTE(RANDOM_LONG(2, 5));
	WRITE_BYTE(bloodsize1);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecInitialOrigin);
	WRITE_BYTE(TE_SPRITE);
	WRITE_COORD(vecInitialOrigin.x);		// position (X)
	WRITE_COORD(vecInitialOrigin.y);		// position (Y)
	WRITE_COORD(vecInitialOrigin.z);		// position (Z)
	WRITE_SHORT(glinfo.BloodSprite);			// sprite index
	WRITE_BYTE(RANDOM_LONG(2, 3));
	WRITE_BYTE(bloodsize2);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecInitialOrigin);
	WRITE_BYTE(TE_SPRITE);
	WRITE_COORD(vecInitialOrigin.x);		// position (X)
	WRITE_COORD(vecInitialOrigin.y);		// position (Y)
	WRITE_COORD(vecInitialOrigin.z);		// position (Z)
	WRITE_SHORT(glinfo.BloodSprite);			// sprite index
	WRITE_BYTE(RANDOM_LONG(1, 2));
	WRITE_BYTE(bloodsize3);
	MESSAGE_END();

	bloodsize1 += 10;
	bloodsize2 += 10;
	bloodsize3 += 10;

	if (bloodsize1 >= 255 && bloodsize2 >= 175 && bloodsize3 >= 45)
	{
		pev->targetname = NULL;
		pev->flags |= FL_KILLME;
		SetThinkCustom(NULL);
		g_engfuncs.pfnRemoveEntity(ENT(pev));
	}
	else
	{
		SetNextThink(this, gpGlobals->time + 0.04f);
	}
}

void OnModuleInitialize(void) {
	static bool regged = false;
	if (!regged)
	{
		cvar_t_small violence = {CVAR_BOOL, "violence_hblood", "none", "1", FCVAR_ARCHIVE | FCVAR_EXTDLL | FCVAR_SERVER};
		violence_hblood = (cvar_t*)CVAR_REGISTER(violence);
		printf("Blood Plugin Initialized\n");
		regged = true;
		ADD_DLL_HOOK(&callOnGetEntityInit, ADD_HOOK_TYPES::H_ENTITYINIT);
		ADD_DLL_HOOK(&callOnSpawnBlood, ADD_HOOK_TYPES::H_SPAWNBLOOD);
	}
}

void OnServerDeactivate(void) {
}

void OnServerActivate(void) {
	glinfo.BloodSprite = PRECACHE_MODEL("sprites/redpapergibs.spz");
	printf("Blood Activated\n");
}

void OnClientKill(edict_t *pEntity) {
	//suicide only, we hooked the "Killed" function manually
}

void OnClientConnect(edict_t *pEntity) {
}

void OnClientDisconnect(edict_t *pEntity) {
}

//Called when client's user info string changes
void CStrike_HandleClientUserInfOChanged( edict_t *pEntity, char *infobuffer ) {
	//int teamchoice = atoi(INFOKEY_VALUE(infobuffer, "teamchoice"));
}

int OnAddToFullPack(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet) {
	return -1;
#if 0
	if ((ent->v.effects == EF_NODRAW) &&
		(ent != host))
		return 0;

	// Ignore ents without valid / visible models
	if (!ent->v.modelindex || !STRING(ent->v.model))
		return 0;

	// Don't send spectators to other players
	if ((ent->v.flags & FL_SPECTATOR) && (ent != host))
	{
		return 0;
	}

	const char *classname = STRING(ent->v.classname);
	if (!strcmp(classname, "dumb_rocket")) {
		if (ent->v.owner && ent->v.owner->v.owner == host)
			return 1;
		/*if (!strcmp(STRING(ent->v.owner->v.classname), "item_helicopter")) {

			edict_t *pHeli = FIND_ENTITY_BY_CLASSNAME(NULL, "item_helicopter");
			while (!FNullEnt(pHeli)) {
				if (ent->v.owner == pHeli) {
					if (pHeli->v.owner == host) {
						return 1;
					}
				}
				pHeli = FIND_ENTITY_BY_CLASSNAME(pHeli, "item_helicopter");
			}
		}
		*/
	}
	else if (!strcmp(classname, "item_helicopter"))
	{
		if (ent->v.owner == host)
			return 1;
	}
	else if (!strcmp(classname, "item_helicopter_model"))
	{
		if (ent->v.owner == host)
			return -1;
	}
	return -1;
#endif
}

//Called when a server runs a frame
void OnStartFrame(void) {
}

//Called when server thinks on a player
void OnPlayerPreThink(edict_t *pEntity) {
}

//Called when a client types a command in the console
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand) {
}