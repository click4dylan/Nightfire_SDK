#include "Cstrike.h"

//HOOKS
DWORD tempEAX;
DWORD tempEBX;
DWORD tempECX;
DWORD tempESI;
DWORD tempEDX;
DWORD tempEDI;
DWORD *tempRegister;
DWORD *tempRegister2;
DWORD *tempRegister3;
DWORD *tempRegister4;
float tempFloat;
#include <math.h>

bool* allow_helicopters;
float* helicopter_health;

DLL_GLOBAL const Vector g_vecZero = { 0, 0, 0 };

BYTE *retJMP;

__declspec(naked) void hookOnHudInit() {
	__asm {
		push ebp //create stack frame
			mov ebp, esp
			sub esp, 0x10
			mov dword ptr ss : [esp], eax
			mov dword ptr ss : [esp + 4], ecx
			mov dword ptr ss : [esp + 8], edx
			mov dword ptr ds : [esp + 0xC], edi

			mov edi, dword ptr ds : [edi + 0x4]   //EDICT_NUM
			mov edi, dword ptr ds : [edi + 0x204] //EDICT_NUM
			mov tempRegister, edi
	}
	callOnHudInit((edict_t*)tempRegister);
	retJMP = getHookFuncJMPAddr((DWORD)hookOnHudInit);
	__asm {
		mov eax, dword ptr ss : [esp]
			mov ecx, dword ptr ss : [esp + 4]
			mov edx, dword ptr ss : [esp + 8]
			mov edi, dword ptr ss : [esp + 0xC]
			mov esp, ebp
			pop ebp

			MOV ECX, DWORD PTR DS : [0x4217CA9C]
			jmp retJMP //jump to original code
	}
}

void callOnHudInit(edict_t *pEntity) {
	edict_t *pHeliModel = FIND_ENTITY_BY_CLASSNAME(NULL, "item_helicopter_model");
	while (!FNullEnt(pHeliModel)) {
		if (pHeliModel->v.owner) {
			StopSound(pHeliModel->v.owner, CHAN_AUTO, PHELICOPTER_SOUND);
			g_engfuncs.pfnEmitSound(pHeliModel->v.owner, CHAN_AUTO, PHELICOPTER_SOUND, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
		pHeliModel = FIND_ENTITY_BY_CLASSNAME(pHeliModel, "item_helicopter_model");
	}
}

__declspec(naked) void hookOnPlayerKilled() {
	__asm {
		mov tempECX, ecx
		mov tempEBX, ebx
		mov tempESI, esi
		mov eax, dword ptr ss:[esp + 4]
		mov tempRegister, eax
		mov eax, dword ptr ss:[esp + 8]
		mov tempRegister2, eax
		mov tempRegister3, ecx
	}

	retJMP = getHookFuncJMPAddr((DWORD)hookOnPlayerKilled);
	callOnPlayerKilled((entvars_t *)tempRegister, (int)tempRegister2, (C_BasePlayer *)tempRegister3);

	__asm {
		mov ecx, tempECX
		mov eax, tempEAX
		mov esi, tempESI
		//restore intercepted code below
		PUSH EBX
		PUSH ESI
		MOV ESI, ECX

		jmp retJMP
	}
}

__declspec(naked) void hookOnGetEntityInit() {
	__asm {
		mov tempEBX, ebx
		mov tempESI, esi
		mov tempEDI, edi
		mov ebx, dword ptr ss:[esp + 4]
		mov tempRegister, ebx
	}
	retJMP = getHookFuncJMPAddr((DWORD)hookOnGetEntityInit);
	callOnGetEntityInit((LPCSTR)tempRegister);
	__asm {
		test eax, eax
		je CustomEntityNotFound
			//Custom entity found! Just return so the game doesn't think it's unknown
			mov ebx, tempEBX
			mov esi, tempESI
			mov edi, tempEDI
			retn
		CustomEntityNotFound:
		mov ebx, tempEBX
		mov esi, tempESI
		mov edi, tempEDI
		//restore intercepted code
		MOV EAX,DWORD PTR DS:[0x44B7B1B8]
		jmp retJMP
	}
}

FARPROC callOnGetEntityInit(LPCSTR lpProcName) {
	//if ( !strcmp(lpProcName, "func_bomb_target") ) {
        HMODULE hMod = GetModuleHandle("item_helicopter_amxx.dll");
		if ( hMod ) {
			//FARPROC test = GetProcAddress(hMod, "bomb_target");
			//return test;
			FARPROC test2 = GetProcAddress(hMod, lpProcName);
			return test2;
		}
	//}
	return NULL;
}

float tempFloatX, tempFloatY, tempFloatZ;
__declspec(naked) void hookOnSpawnBlood() {
	__asm {
		mov eax, dword ptr ss : [esp + 4]
			mov tempFloatX, eax
			mov eax, dword ptr ss : [esp + 8]
			mov tempFloatY, eax
			mov eax, dword ptr ss : [esp + 0xc]
			mov tempFloatZ, eax
			mov eax, dword ptr ss : [esp + 0x10]
			mov tempEAX, eax
			mov eax, dword ptr ss : [esp + 0x14]
			mov tempFloat, eax
	}
	callOnSpawnBlood(tempFloatX, tempFloatY, tempFloatZ, tempEAX, tempFloat);
	__asm{
		retn
	}
}

void callOnSpawnBlood(float X, float Y, float Z, int bloodColor, float flDamage) {
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

HookCallInfo funcHooks[] = {
	//{(BYTE*)ADR_PLAYERSPAWNCTF, (DWORD)hookOnPlayerSpawnCTF, 6, (BYTE*)ADR_PLAYERSPAWNCTF+0x6},
	//{(BYTE*)ADR_PLAYERSPAWNDM, (DWORD)hookOnPlayerSpawnDM, 5, (BYTE*)ADR_PLAYERSPAWNDM+0x5},
	//{(BYTE*)ADR_FPLAYERCANRESPAWN, (DWORD)hookOnFPlayerCanRespawn, 6, (BYTE*)ADR_FPLAYERCANRESPAWN+0x5},
	//{(BYTE*)ADR_PLAYERTAKEDAMAGE, (DWORD)hookOnPlayerTakeDamage, 6, (BYTE*)ADR_PLAYERTAKEDAMAGE+0x6}, //not needed right now
	{(BYTE*)ADR_PLAYERKILLED, (DWORD)hookOnPlayerKilled, 6, (BYTE*)ADR_PLAYERKILLED+0x6},
	{(BYTE*)ADR_GETENTITYINIT, (DWORD)hookOnGetEntityInit, 5, (BYTE*)ADR_GETENTITYINIT+0x5}, //comment me
	{(BYTE*)ADR_SPAWNBLOOD, (DWORD)hookOnSpawnBlood, 5, (BYTE*)ADR_SPAWNBLOOD + 0x5},
	//{(BYTE*)ADR_BASEPLAYERSPAWN, (DWORD)hookOnBasePlayerSpawn, 5, (BYTE*)ADR_BASEPLAYERSPAWN+0x5},
	{(BYTE*)ADR_INIT_HUD, (DWORD)hookOnHudInit, 6, (BYTE*)ADR_INIT_HUD+0x5},
};

BYTE* getHookFuncJMPAddr(DWORD fnName) {
	for(int i=0;i<sizeof(funcHooks)/sizeof(HookCallInfo);i++) {
		if(funcHooks[i].retFunc == fnName) {
			return (BYTE*)funcHooks[i].retJMPAddr;
		}
	}
	printf("Error: One or more of the provided function addresses weren't found\n");
	return 0;
}

void HookFunctions() {
	for(int i=0;i<sizeof(funcHooks)/sizeof(HookCallInfo);i++) {
		PlaceJMP((BYTE*)funcHooks[i].address, funcHooks[i].retFunc, funcHooks[i].len);
	}
}

// END OF HOOKS

//HELICOPTER MODEL

void CItemHelicopter::Precache(void) {
	PRECACHE_MODEL(PHELICOPTER_HELICOPTER_MODEL);
	PRECACHE_MODEL(PHELICOPTER_HELICOPTER_MODEL_MI6);
	PRECACHE_MODEL(PHELICOPTER_HELICOPTER_MODEL_PHX);
}

void CItemHelicopter::Spawn(void) {
	Precache();
	if (team == TEAM_MI6)
		SET_MODEL(edict(), PHELICOPTER_HELICOPTER_MODEL_MI6);
	else if (team == TEAM_PHOENIX)
		SET_MODEL(edict(), PHELICOPTER_HELICOPTER_MODEL_PHX);
	else
		SET_MODEL(edict(), PHELICOPTER_HELICOPTER_MODEL);
	SetSequence(LookupSequence(edict(), "flying"));
	pev->framerate = 30.0f;
	//StudioFrameAdvance((CBaseEntity*)this, 1);
	SET_SIZE(this->edict(), g_vecZero, g_vecZero);
	SetSolid(SOLID_BBOX);
	AddEffects(EF_NODRAW);
}

CItemHelicopter *CItemHelicopter::Create(Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CItemHelicopterCamera *pCamera) {
	CItemHelicopter *pHeliModel = GetClassPtr((CItemHelicopter *)NULL);
	SET_ORIGIN(pHeliModel->edict(), vecOrigin);
	pHeliModel->pev->classname = MAKE_STRING("item_helicopter_model");
	pHeliModel->pev->angles = vecAngles;
	if (pOwner) {
		pHeliModel->team = ((C_BasePlayer*)pOwner)->team;
	}
	pHeliModel->Spawn();
	//pHeliModel->SetTouch(CItemHelicopter::Touch);
	pHeliModel->pCamera = pCamera;// remember what camera created me
	pHeliModel->pev->owner = pOwner->edict();
	return pHeliModel;
}

LINK_ENTITY_TO_CLASS(item_helicopter_model, CItemHelicopter);


//HELICOPTER CAMERA

CItemHelicopterCamera::CItemHelicopterCamera()
{
	m_vSpawnPos = g_vecZero;
	m_vSpawnAng = g_vecZero;
	m_vBounceVel = g_vecZero;
}

CItemHelicopterCamera::~CItemHelicopterCamera()
{
	if (pVisibleHeliModelEdict) {
		REMOVE_ENTITY(pVisibleHeliModelEdict);
		pVisibleHeliModelEdict = NULL;
	}
	if (pCamEdict) {
		REMOVE_ENTITY(pCamEdict);
		pCamEdict = NULL;
	}
}

void CItemHelicopterCamera::KeyValue(KeyValueData *pkvd) {
	if (FStrEq(pkvd->szKeyName, "team"))
	{
		team = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntityCustom::KeyValue(pkvd);
}

void CItemHelicopterCamera::Spawn(void) {
	if (team != 0) {
		SetThinkCustom(&CItemHelicopterCamera::IdleThink);
		SetNextThink(this, gpGlobals->time);
	}
	pev->classname = MAKE_STRING("item_helicopter");
	pev->targetname = MAKE_STRING("helicopter");
	//pev->flags = 0xCD39D42E;
	Precache();
	SetSolid(SOLID_NOT);
	if (team == TEAM_MI6)
		SET_MODEL(edict(), PHELICOPTER_HELICOPTER_MODEL_MI6);
	else if (team == TEAM_PHOENIX)
		SET_MODEL(edict(), PHELICOPTER_HELICOPTER_MODEL_PHX);
	else
		SET_MODEL(edict(), PHELICOPTER_HELICOPTER_MODEL);
	SET_SIZE(edict(), Vector(-40, -30, -10), Vector(40, 30, 35));
	SetObjectCollisionBox();
	//pev->mins = Vector(-25, -25, -25);
	//pev->absmin = Vector(-25, -25, -25);
	//pev->maxs = Vector(25, 25, 25);
	//pev->absmax = Vector(25, 25, 25);
	SetMoveType(MOVETYPE_BOUNCE);
	SetFriction(0.55f);
	SetGravity(0.5f);
	pev->max_health = CVAR_GETDIRECT(helicopter_health);
	pev->health = pev->max_health;
	UPDATE_RATE = 0.02f;
	float frametime = 1 / CVAR_GET_FLOAT("fps_max");
	if (frametime > 0.0078125) {
		UPDATE_RATE = fmax(0.0001f, UPDATE_RATE - (frametime));
	}
	pev->takedamage = DAMAGE_AIM; //fixme
	HIGH_SPEED = 1;
	PHELICOPTER_MAX_UPSPEED = 75;
	PHELICOPTER_SPEED = 0;
	lasttime_pressedattack3 = 0;
	lasttime_movedforward = 0;
	lasttime_sloweddown = 0;
	lasttime_resetmouse = 0;
	lasttime_movedleft = 0;
	lasttime_movedright = 0;
	lasttime_hoveredup = 0;
	lasttime_animated = 0;
	lasttime_leftclicked = 0;
	lasttime_rightclicked = 0;
	lasttime_printedhealth = 0;
	lasttime_tookdamage = 0;
	lasttime_resetleft = 0;
	lasttime_resetup = 0;
	m_flRollSideSpeed = 0;
	m_flUpSpeed = 0;
	m_flFallSpeed = 0;
	lasttime_fell = 0;
	time_pressedusekey = 0;
	time_blewup = 0;
	rocketsshot = 0;

	//SetCollisionGroup(COLLISION_GROUP_PASSABLE_DOOR);
	//AddSolidFlags(FSOLID_TRIGGER);
	//AddEffects( EF_NODRAW );
	//AddFlag(FL_OBJECT); // So shows up in NPC Look()

	//SetModel(PHELICOPTER_HELICOPTER_MODEL);
	//UTIL_SetSize(this, mobb / 2.2, mxobb / 2.2);

	m_bActive = false;
	m_vSpawnPos = GetLocalOrigin();
	m_vSpawnAng = GetLocalAngles();
}

void CItemHelicopterCamera::IdleThink(void) {
	//Color col = Color(255, 255, 255);
	//if (m_bActive)
		//g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, PHELICOPTER_SOUND, VOL_NORM, ATTN_NORM, SND_CHANGE_VOL, PITCH_NORM);
	//else
		//StopSound(edict(), CHAN_AUTO, PHELICOPTER_SOUND);
	m_flTimeCreatedElight = gpGlobals->time;
	Vector origin = GetAbsOrigin();
	origin.z -= 10.0f;
	if (team == TEAM_MI6) {
		//pev->rendermode = kRenderTransColor;
		//pev->rendercolor = Vector(0, 162, 232);
		//col = Color(0, 162, 232);
		UTIL_Create_TE_ELIGHT(edict(), origin, 120, Color(0, 162, 232), (int)(PHELICOPTER_LIGHT_UPDATE_SECS * 10), 0);
	}
	else {
		//pev->rendermode = kRenderTransColor;
		//pev->rendercolor = Vector(255, 127, 39);
		//col = Color(255, 127, 39);
		UTIL_Create_TE_ELIGHT(edict(), origin, 120, Color(255, 127, 39), (int)(PHELICOPTER_LIGHT_UPDATE_SECS * 10), 0);
	}
	SetNextThink(this, gpGlobals->time + PHELICOPTER_LIGHT_UPDATE_SECS);
}

void CItemHelicopterCamera::Precache(void) {
	PRECACHE_MODEL(PHELICOPTER_HELICOPTER_MODEL);
	PRECACHE_MODEL(PHELICOPTER_HELICOPTER_MODEL_MI6);
	PRECACHE_MODEL(PHELICOPTER_HELICOPTER_MODEL_PHX);
	PRECACHE_SOUND(PHELICOPTER_HITWALLSOUND1);
	PRECACHE_SOUND(PHELICOPTER_HITWALLSOUND2);
	PRECACHE_SOUND(PHELICOPTER_HITWALLSOUND3);
	PRECACHE_SOUND(PHELICOPTER_EXPLODESOUND);
	PRECACHE_SOUND(PHELICOPTER_SOUND);
	PRECACHE_SOUND(PHELICOPTER_GUNSOUND1);
	PRECACHE_SOUND(PHELICOPTER_GUNSOUND2);
	PRECACHE_SOUND(PHELICOPTER_GUNSOUND3);
	PRECACHE_SOUND(PHELICOPTER_GUNSOUND4);
	PRECACHE_SOUND(PHELICOPTER_ROCKETSOUND);
	SmokeIndex = PRECACHE_MODEL("sprites/smoke.spz");
	glinfo.SpriteIndex = PRECACHE_MODEL("sprites/zerogxplode.spz");// Bomb Explosion Sprite
}

void CItemHelicopterCamera::InputActivate(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller) {
	CItemHelicopter *pHeliModel = CItemHelicopter::Create(GetAbsOrigin(), GetAbsAngles(), (CBaseEntity*)pActivator, this);
	if (!pHeliModel) {
		printf("WARNING: Could not create item_helicopter_model!");
		REMOVE_ENTITY(this->edict());
		return;
	}
	pCamEdict = CREATE_NAMED_ENTITY(MAKE_STRING("cycler_sprite"));
	if (!pCamEdict) {
		printf("WARNING: Could not create heli cycler_sprite!");
		REMOVE_ENTITY(this->edict());
		return;
	}
	pCamEdict->v.rendermode = kRenderTransColor;
	pCamEdict->v.renderamt = 0;
	pCamEdict->v.movetype = MOVETYPE_NOCLIP;
	pCamEdict->v.solid = SOLID_NOT;
	pCamEdict->v.takedamage = DAMAGE_NO;
	pCamEdict->v.origin = GetAbsOrigin();
	pCamEdict->v.angles = GetAbsAngles();
	SET_MODEL(pCamEdict, PHELICOPTER_HELICOPTER_MODEL);
	pCamEdict->v.effects &= ~EF_NODRAW;
	SET_SIZE(pCamEdict, g_vecZero, g_vecZero);

	pVisibleHeliModelEdict = pHeliModel->edict(); //CREATE_NAMED_ENTITY(MAKE_STRING("item_helicopter_model"));
	//AddEffects(EF_NODRAW);
	pVisibleHeliModelEdict->v.movetype = MOVETYPE_NOCLIP;
	pVisibleHeliModelEdict->v.solid = SOLID_NOT;
	pVisibleHeliModelEdict->v.takedamage = DAMAGE_NO;
	pVisibleHeliModelEdict->v.gravity = 0;
	pVisibleHeliModelEdict->v.effects &= ~EF_NODRAW;
	pVisibleHeliModelEdict->v.origin = GetAbsOrigin();
	pVisibleHeliModelEdict->v.angles = GetAbsAngles();
	pev->rendermode = kRenderTransColor;
	pev->renderamt = 0;
	//pVisibleHeliModelEdict->v.renderfx = kRenderFxNone;

	//pev->flags |= FL_HELICOPTER;
	
	SetMoveType(MOVETYPE_BOUNCE);
	SetSolid(SOLID_BBOX);
	SET_SIZE(edict(), Vector(-40, -30, -10), Vector(40, 30, 35));
	SetObjectCollisionBox();

	SetOwnerEntity(pActivator);
	m_bActive = true;
	//m_nPClipTraceDir = PC_TRACE_LAST;
	//m_flCurrMinClipDist = PCONTROL_CLIP_DIST;

	C_BasePlayer* pPlayer = (C_BasePlayer*)pActivator;

	// Save Data
	//m_nSaveFOV = pPlayer->GetFOV();
	m_vSaveOrigin = pPlayer->pev->origin;
	m_vSaveEyeAngles = pPlayer->pev->v_angle;
	m_vSaveAngles = pPlayer->pev->angles;
	m_nSaveMoveType = pPlayer->pev->movetype;
	pPlayer->pev->movetype = MOVETYPE_FOLLOW;
	m_nSaveEffects = pPlayer->pev->effects;
	pPlayer->pev->effects |= EF_NODRAW;
	m_nSaveTakeDamage = pPlayer->pev->takedamage;
	pPlayer->pev->takedamage = DAMAGE_NO;
	m_nSaveSolid = pPlayer->pev->solid;
	pPlayer->pev->solid = SOLID_NOT;
	pPlayer->pev->aiment = edict();
	//m_nSaveMoveCollide = pPlayer->GetMoveCollide();
	//m_vSaveViewOffset = pPlayer->GetViewOffset();
	//m_pSaveWeapon = pPlayer->GetActiveWeapon();
	//pPlayer->SetFOV( this, m_nSaveFOV );
	//pPlayer->AddFlag(FL_FREEZEWEAPONS);
	//pPlayer->AddSolidFlags( FSOLID_NOT_SOLID );
	//pPlayer->SetLocalOrigin( GetLocalOrigin() );
	((CBaseEntityCustom*)pPlayer)->SnapEyeAngles(GetLocalAngles());//(g_vecZero); //use veczero if using slow turning method
	pPlayer->pev->angles = GetLocalAngles();//g_vecZero;
	//pPlayer->pev->flags |= FL_FROZEN;
	SetUseCustom(&CItemHelicopterCamera::UseDoNothing);
	HIGH_SPEED = 1;
	//pPlayer->SetViewOffset( vec3_origin );
	//SV_LinkEdict(pPlayer->edict(), 1);
	//SV_LinkEdict(edict(), 1);
}

void CItemHelicopterCamera::InputDeactivate(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller) {
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	//pev->flags &= ~FL_HELICOPTER;

	if (pVisibleHeliModelEdict) {
		REMOVE_ENTITY(pVisibleHeliModelEdict);
		pVisibleHeliModelEdict = NULL;
	}

	if (pCamEdict) {
		REMOVE_ENTITY(pCamEdict);
		pCamEdict = NULL;
	}

	C_BasePlayer *pPlayer = (C_BasePlayer*)GetOwnerEntity();
	client_t *pClient;
	if (pPlayer) {
		pClient = EDICT_TO_CLIENT(pPlayer->edict());
		if (pClient && pClient->connected)
			SET_VIEW(pPlayer->edict(), pPlayer->edict());
	}
	//SetMoveType(MOVETYPE_NONE);
	SetOwnerEntity(NULL);
	m_bActive = false;
	SetTouchCustom(NULL);
	if (pPlayer && pClient && pClient->connected) {
		pPlayer->pev->angles = m_vSaveAngles;
		((CBaseEntityCustom*)pPlayer)->SnapEyeAngles(m_vSaveEyeAngles);
		pPlayer->pev->origin = m_vSaveOrigin; //added
		pPlayer->pev->aiment = pClient->edict;
		pPlayer->pev->movetype = m_nSaveMoveType;
		pPlayer->pev->effects = m_nSaveEffects;
		pPlayer->pev->takedamage = m_nSaveTakeDamage;
		pPlayer->pev->solid = m_nSaveSolid;
		//SV_LinkEdict(pPlayer->edict(), 1);
		edict_t *pPlayerEquip = FIND_ENTITY_BY_CLASSNAME(NULL, "game_player_equip");
		while (!FNullEnt(pPlayerEquip)) {
			C_BaseEntity *pEquip = C_BaseEntity::Instance(pPlayerEquip);
			pEquip->Touch((C_BaseEntity*)C_BasePlayer::Instance(pClient->edict));
			pPlayerEquip = FIND_ENTITY_BY_CLASSNAME(pPlayerEquip, "game_player_equip");
		}
	}
	SetSolid(SOLID_NOT);
	//SV_LinkEdict(edict(), 1);
	StopSound(edict(), CHAN_AUTO, PHELICOPTER_SOUND);
	PHELICOPTER_SPEED = 0;
	rocketsshot = 0;
	SetUseCustom(&CItemHelicopterCamera::UseDoNothing);
	SetThinkCustom(&CItemHelicopterCamera::RespawnThink);
	SetNextThink(this, gpGlobals->time + 30.0f);
}

void CItemHelicopterCamera::LoseHelicopterControl(void) {
	//StopSound(edict(), CHAN_AUTO, PHELICOPTER_SOUND);
	//pev->takedamage = DAMAGE_NO;
	//pev->deadflag = DEAD_DEAD;
	InputDeactivate(GetOwnerEntity(), GetOwnerEntity());
}

void CItemHelicopterCamera::BlowUp(entvars_t *pevAttacker) {
	C_BasePlayer *pPlayer = (C_BasePlayer*)GetOwnerEntity();
	if (pPlayer) {
		CLIENT_PRINTF(((C_BaseEntity*)pPlayer)->edict(), print_center, "HELICOPTER IS DESTROYED");
	}
	pevAttackerEnt = pevAttacker;
	StopSound(edict(), CHAN_AUTO, PHELICOPTER_SOUND);
	g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, PHELICOPTER_EXPLODESOUND, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	Vector explodeorigin = GetAbsOrigin();
	explodeorigin.z += 20;
	Create_TE_EXPLOSION(edict(), explodeorigin, glinfo.SpriteIndex, 40, 15, TE_EXPLFLAG_NONE);
	Vector vecForward = GetAbsVelocity();
	//VectorNormalize(vecForward);
	TraceResult tr;
	TRACE_LINE(GetAbsOrigin(), GetAbsOrigin() + 60 * vecForward, dont_ignore_monsters, 0, this->edict(), &tr);

	//UTIL_DecalTrace(&tr, "Scorch");

	//UTIL_ScreenShake(GetAbsOrigin(), 25.0, 150.0, 1.0, 750, 0);
	//CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), 1024, 3.0); //possible crash source

	//RadiusDamage(CTakeDamageInfo(this, this, m_flDamage, DMG_BLAST), GetAbsOrigin(), m_flDamageRadius, CLASS_NONE, NULL);

	time_blewup = gpGlobals->time;
	if (pPlayer)
		UTIL_ScreenFadeOne(pPlayer->edict(), 0, 0, 0, 255, 2, 0.1, 1); //FADE OUT

	SetThinkCustom(&CItemHelicopterCamera::ExplodeThink);
	SetNextThink(this, gpGlobals->time);
	SetTouchCustom(NULL);
	m_vBounceVel = -0.2 * GetAbsVelocity();
	//AddSolidFlags(FSOLID_NOT_SOLID);
}

void CItemHelicopterCamera::Killed(entvars_t *pevAttacker, int iGib) {
	BlowUp(pevAttacker);
}

void CItemHelicopterCamera::RemoveThink(void) {
	InputDeactivate(GetOwnerEntity(), GetOwnerEntity());
}

void CItemHelicopterCamera::ExplodeThink(void) {
	if (gpGlobals->time - m_flTimeCreatedElight >= PHELICOPTER_LIGHT_UPDATE_SECS)
		IdleThink();
	SetAbsVelocity(m_vBounceVel);
	Vector angles = GetLocalAngles(); //spin us around and around!
	angles.y -= 2;
	SetLocalAngles(angles);

	C_BasePlayer*	pPlayer = (C_BasePlayer*)GetOwnerEntity();

	if (!pPlayer){ //crashfix on disconnect
		PHELICOPTER_SPEED = 0;
		SetAbsVelocity(g_vecZero);
		SetLocalAngles(m_vSpawnAng);
		SetLocalOrigin(m_vSpawnPos);
		SetOwnerEntity(NULL);
		if (pVisibleHeliModelEdict) REMOVE_ENTITY(pVisibleHeliModelEdict);
		pVisibleHeliModelEdict = NULL;
		if (pCamEdict) REMOVE_ENTITY(pCamEdict);
		pCamEdict = NULL;
		RemoveEffects(EF_NODRAW);
		pev->rendermode = kRenderNormal;
		pev->renderamt = 255;
		return;
	}

	pVisibleHeliModelEdict->v.velocity = GetAbsVelocity();
	pVisibleHeliModelEdict->v.origin = GetAbsOrigin();
	//pVisibleHeliModelEdict->v.origin.z -= 25.0f;
	pVisibleHeliModelEdict->v.angles = GetAbsAngles();
	pVisibleHeliModelEdict->v.angles.x = -pVisibleHeliModelEdict->v.angles.x;

	//if (gpGlobals->time - time_blewup < 0.1f) //hacky way to fade once
		//UTIL_ScreenFadeOne(pPlayer->edict(), 0, 0, 0, 255, 2, 0.1, 1); //FADE OUT
		//UTIL_ScreenFade(pPlayer, black, 2.0, 0.1, FFADE_OUT);

	if (gpGlobals->time - time_blewup > PHELICOPTER_DIE_TIME){
		if (pPlayer && pevAttackerEnt) {
			pPlayer->TakeDamage(pevAttackerEnt, pevAttackerEnt, 300.0f, DMG_BULLET);
		}
		pevAttackerEnt = NULL;
		SetThinkCustom(&CItemHelicopterCamera::RemoveThink);
		SetNextThink(this, gpGlobals->time);
	}
	else{
		SetNextThink(this, gpGlobals->time + 0.01f);
	}
}

void CItemHelicopterCamera::UseDoNothing(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value) {
}

void CItemHelicopterCamera::SetObjectCollisionBox(void) {
	if ((pev->solid == SOLID_BSP) &&
		(pev->angles.x || pev->angles.y || pev->angles.z))
	{	// expand for rotation
		float		max, v;
		int			i;

		max = 0;
		for (i = 0; i<3; i++)
		{
			v = fabs(pev->mins[i]);
			if (v > max)
				max = v;
			v = fabs(pev->maxs[i]);
			if (v > max)
				max = v;
		}
		for (i = 0; i<3; i++)
		{
			pev->absmin[i] = pev->origin[i] - max;
			pev->absmax[i] = pev->origin[i] + max;
		}
	}
	else
	{
		pev->absmin = pev->origin + pev->mins;
		pev->absmax = pev->origin + pev->maxs;
	}

	pev->absmin.x -= 1;
	pev->absmin.y -= 1;
	pev->absmin.z -= 1;
	pev->absmax.x += 1;
	pev->absmax.y += 1;
	pev->absmax.z += 1;
}

void CItemHelicopterCamera::Use(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value) {
	if (m_pfnUse && m_pfnUse != &CItemHelicopterCamera::Use) {
		(this->*m_pfnUse)(pActivator, pCaller, useType, value);
		return;
	}

	if (CVAR_GETDIRECT(allow_helicopters) == 0)
		return; //don't allow getting in helis if it's disabled by the server admin

	if (!pActivator)
		return;
	if (!pActivator->IsPlayer())
		return;

	if (m_bActive)
		return;

	C_BasePlayer* pPlayer = (C_BasePlayer*)pActivator;
	if (team != 0 && pPlayer->team != team) {
		char teammsg[256];
		sprintf(teammsg, "This helicopter is only for team %s!", team == TEAM_MI6 ? "MI6" : "Phoenix");
		CLIENT_PRINTF(((C_BaseEntity*)pActivator)->edict(), print_center, teammsg);
		return;
	}

	if (!GetOwnerEntity()) {
		StripWeapons(pActivator->edict(), 0);
		time_pressedusekey = gpGlobals->time;
		ClientConnectTime = EDICT_TO_CLIENT(pActivator->edict())->connection_started;
		InputActivate(pActivator, pCaller);
		Bond_HudMsg(pActivator->edict(), "Special Helicopter Controls:\nReload: Change Speed\nFire: Shoot\nAlt Fire: Launch Missile", 3, SK_OPERATIVE);
		/*
		if (pVisibleHeliModelEdict) {
			SetSequence(LookupSequence(pVisibleHeliModelEdict, "flying"));
			pVisibleHeliModelEdict->v.framerate = 30.0f;
			StudioFrameAdvance(C_BaseEntity::Instance(pVisibleHeliModelEdict), 1);
		}*/
		SET_VIEW(pActivator->edict(), pCamEdict);
		SetThinkCustom(&CItemHelicopterCamera::Launch);
		SetNextThink(this, gpGlobals->time + 0.1f);
	}
}

void CItemHelicopterCamera::Launch(void) {
	//MAKE_VECTORS(GetLocalAngles());
	//Vector vecDir = gpGlobals->v_up * 50.0f;
	//if (pVisibleHeliModelEdict)
		//pVisibleHeliModelEdict->v.velocity = vecDir;
	//pev->velocity = vecDir;
	pev->deadflag = DEAD_NO;
	pev->takedamage = DAMAGE_AIM;
	pev->health = pev->max_health;
	//pev->flags &= ~FL_ONGROUND;
	//pev->groundentity = NULL;
	g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, PHELICOPTER_SOUND, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	SetThinkCustom(&CItemHelicopterCamera::FlyThink);
	SetTouchCustom(&CItemHelicopterCamera::FlyTouch);
	SetNextThink(this, gpGlobals->time + 0.1f);
}

void CItemHelicopterCamera::FlyThink(void) {
	if (!m_bActive)
		return;
	float newtime = gpGlobals->time;
	if (newtime - m_flTimeCreatedElight >= PHELICOPTER_LIGHT_UPDATE_SECS)
		IdleThink();

	SetNextThink(this, newtime + (gpGlobals->frametime));
	//if (newtime - lasttime_animated >= 0.033333333f){ //30fps
		//Play the helicopter blade spinning animation
		//lasttime_animated = newtime;
		//StudioFrameAdvance();
	//}

	C_BasePlayer*	pPlayer = (C_BasePlayer*)GetOwnerEntity();
	if (!pPlayer) {
		//fix crash on disconnect
		LoseHelicopterControl();
		return;
	}

	client_t *pClient = EDICT_TO_CLIENT(pPlayer->edict());

	if (pPlayer->pev->deadflag == DEAD_DEAD || !pClient->connected || pClient->connection_started != ClientConnectTime) {
		LoseHelicopterControl(); //if this gets called, something is seriously wrong!
		return;
	}

	if (pPlayer->pev->health <= 0) {
		LoseHelicopterControl(); //if this gets called, something is seriously wrong!
		return;
	}

	pPlayer->pev->origin = pCamEdict->v.origin;
	pPlayer->pev->origin.z += 5.0f;
	pPlayer->pev->velocity = g_vecZero;
	StripWeapons(pPlayer->edict(), 0);

	if (newtime - lasttime_printedhealth >= 0.125f && newtime - lasttime_pressedattack3 > 2) {
		//Print our health statistics
		lasttime_printedhealth = newtime;
		Vector myVel = GetAbsVelocity();
		Vector myOrigin = GetLocalOrigin();
		Vector myAngles = GetLocalAngles();
		char ScreenText[256];
		sprintf(ScreenText, "ARMOR: %.0f%%      SPEED: %.0f", pev->health, myVel.Length2D());
		//sprintf(ScreenText, "ARMOR: %.0f%%      SPEED: %.0f\nVELOCITY: %.0fx %.0fy %.0fz\nCOORDS: %.0fx %.0fy %.0fz\nANGLES: %.0f pitch %.0f yaw %.0f roll", pev->health, myVel.Length2D(), myVel.x, myVel.y, myVel.z, myOrigin.x, myOrigin.y, myOrigin.z, myAngles.x, myAngles.y, myAngles.z);
		CLIENT_PRINTF(((C_BaseEntity*)pPlayer)->edict(), print_center, ScreenText);
	}
	usercmd_t *cmd = &pClient->lastcmd;
	//int buttons = pPlayer->m_afButtonPressed;
	//IN_FORWARD|IN_BACK|IN_MOVELEFT|IN_MOVERIGHT|IN_SPEED|IN_JUMP|IN_ATTACK|IN_ATTACK


	//Slowly increase or decrease speed
	/*
	if ( cmd->buttons & IN_FORWARD ){
	//we are moving forward
	if ( newtime - lasttime_movedforward >= 0.1f ){
	lasttime_movedforward = newtime;
	PHELICOPTER_SPEED = min(PHELICOPTER_SPEED + 5, PHELICOPTER_MAX_SPEED);
	}

	}else*/ if (cmd->buttons & IN_USE && newtime - time_pressedusekey > 1.0f){
		//player pressed E and wants to leave the helicopter.
		CLIENT_PRINTF(((C_BaseEntity*)pPlayer)->edict(), print_center, "EXITED HELICOPTER");
		LoseHelicopterControl();
		return;
	}
	else if (cmd->buttons & IN_RELOAD && newtime - lasttime_pressedattack3 > 0.5f){
		lasttime_pressedattack3 = newtime;
		if (HIGH_SPEED == 2) {
			CLIENT_PRINTF(((C_BaseEntity*)pPlayer)->edict(), print_center, "MEDIUM SPEED ACTIVATED");
			HIGH_SPEED = 1;
			PHELICOPTER_MAX_UPSPEED = 100;
		}
		else if (HIGH_SPEED == 1) {
			CLIENT_PRINTF(((C_BaseEntity*)pPlayer)->edict(), print_center, "SLOW SPEED ACTIVATED");
			HIGH_SPEED = 0;
			PHELICOPTER_MAX_UPSPEED = 75;
		}
		else {
			CLIENT_PRINTF(((C_BaseEntity*)pPlayer)->edict(), print_center, "HIGH SPEED ACTIVATED");
			HIGH_SPEED = 2;
			PHELICOPTER_MAX_UPSPEED = 150;
		}
	}
	else{
		if (newtime - lasttime_sloweddown >= UPDATE_RATE){
			//not holding any buttons, slowly reduce speed
			lasttime_sloweddown = newtime;
			PHELICOPTER_SPEED = fmax(PHELICOPTER_SPEED - 10, PHELICOPTER_MIN_SPEED);
		}
	}
#if 0
	// ---------------------------------------------------
	// Calulate amount of xturn from player mouse movement
	// ---------------------------------------------------
	Vector vecViewAngles = pPlayer->pev->v_angle;
	float xLev;
	if (vecViewAngles.x < 180){
		xLev = vecViewAngles.x / 180;
	}
	else{
		xLev = (vecViewAngles.x - 360) / 180;
	}

	// ---------------------------------------------------
	// Calulate amount of xturn from player mouse movement
	// ---------------------------------------------------

	float yLev;
	// Keep yaw in bounds (don't let loop)
	if (vecViewAngles.y < -90) {
		pPlayer->pev->angles = Vector(vecViewAngles.x - 360, -90, 0);
		vecViewAngles.y = -90;
		((CBaseEntityCustom*)pPlayer)->SnapEyeAngles(vecViewAngles);
	}
	if (vecViewAngles.y > 90) {
		if (vecViewAngles.y < 180) {
			pPlayer->pev->angles = Vector( vecViewAngles.x - 360, 90, 0 );
			vecViewAngles.y = 90;
			((CBaseEntityCustom*)pPlayer)->SnapEyeAngles(vecViewAngles);
		}
		else if (vecViewAngles.y < 270) {
			pPlayer->pev->angles = Vector(vecViewAngles.x, 270, 0);
			vecViewAngles.y = 270;
			((CBaseEntityCustom*)pPlayer)->SnapEyeAngles(vecViewAngles);
		}
	}

	if (vecViewAngles.y < 180){
		yLev = vecViewAngles.y / 180;
	}
	else {
		yLev = (vecViewAngles.y - 360) / 180;
	}
	float slowtime = 0.1f;
	// add network latency
	slowtime += pClient->latency - UPDATE_RATE;

	//Slowly zero the mouse into the center of the crosshair.
	if (newtime - lasttime_resetmouse >= slowtime){
		lasttime_resetmouse = newtime;
		Vector newViewAngles = vecViewAngles;
		if (vecViewAngles.y < 0)
			newViewAngles.y = fmin(newViewAngles.y + 2, 0);
		else if (vecViewAngles.y > 0)
			newViewAngles.y = fmax(newViewAngles.y - 2, 0);
		if (vecViewAngles.x < 0)
			newViewAngles.x = fmin(newViewAngles.x + 2, 0);
		else if (vecViewAngles.x > 0)
			newViewAngles.x = fmax(newViewAngles.x - 2, 0);

		if (vecViewAngles != newViewAngles) {
			((CBaseEntityCustom*)pPlayer)->SnapEyeAngles(newViewAngles);
			pPlayer->pev->angles = newViewAngles;
		}
		}
#endif
	// Add in turn
	Vector vRight, vUp;
	Vector angles = GetLocalAngles();
	NormalizeAngles(angles);
	//NormalizeAngles(angles);
	//AngleVectors(GetLocalAngles(), 0, &vRight, &vUp);
	MAKE_VECTORS(angles);
	vRight = gpGlobals->v_right;
	vUp = gpGlobals->v_up;

	//angles.x = clamp(angles.x, -PHELICOPTER_MAX_PITCH_DEG, PHELICOPTER_MAX_PITCH_DEG);
	angles.x = pPlayer->pev->v_angle.x;
	angles.z = clamp(angles.z, -PHELICOPTER_MAX_ROLL_DEG, PHELICOPTER_MAX_ROLL_DEG);

	//angles.x += PHELICOPTER_TURN_RATE*xLev*gpGlobals->frametime;	 //pitch
	//angles.y += PHELICOPTER_TURN_RATE*yLev*gpGlobals->frametime; //MOUSE ROLL: //-m_flRollSideSpeed;										 //yaw
	angles.y = pPlayer->pev->v_angle.y;
	angles.z += m_flRollSideSpeed;	//MOUSE ROLL: //PHELICOPTER_TURN_RATE*-yLev*gpGlobals->frametime;	 //roll

	if (m_bShake) {
		//we were hit or fired a missile, do a big recoil
		angles.x += RANDOM_FLOAT(-3.0, 3.0);
		angles.y += RANDOM_FLOAT(-3.0, 3.0);
		m_bShake = false;
	}
	if (m_bRecoil){
		//we are firing our weapons, do a recoil. This is extremely annoying with high ping and is removed
		//angles.x += RandomFloat(-2.25f,1);
		//angles.y += RandomFloat(-0.5,0.5);
		m_bRecoil = false;
	}
	SetLocalAngles(angles);

	//if we are upside down
	bool upsidedown = (fabs(angles.x) >= 55.0 || fabs(angles.z) >= 55.0);
	//bool invert = false;

	// Reset velocity
	Vector vVelocity;
	MAKE_VECTORS(GetAbsAngles());
	//AngleVectors(GetAbsAngles(), &vVelocity);
	vVelocity = gpGlobals->v_forward;

	// Add speed
	//vVelocity = vVelocity * PHELICOPTER_SPEED ;

	// Get the movement angles.
	Vector vecForward, vecRight, vecUp;
	//AngleVectors(GetLocalAngles(), &vecForward, &vecRight, &vecUp);
	//MAKE_VECTORS(GetLocalAngles());
	vecForward = gpGlobals->v_forward;
	vecRight = gpGlobals->v_right;
	vecUp = gpGlobals->v_up;

	if (cmd->buttons & IN_MOVELEFT){
		//we are moving left, so turn left. todo: do we want to invert if upside down?
		if (newtime - lasttime_movedleft >= UPDATE_RATE){
			lasttime_movedleft = newtime;
			if (abs(angles.z) >= PHELICOPTER_MAX_ROLL_DEG){
				if (m_flRollSideSpeed < 0)
					m_flRollSideSpeed = -0.01;
				else
					m_flRollSideSpeed = 0.01;
			}

			m_flRollSideSpeed = fmax(m_flRollSideSpeed - PHELICOPTER_ROLLSPEED_RATE, -PHELICOPTER_MAX_ROLLSPEED); //PHELICOPTER_MAX_ROLLSPEED / -1
		}
	}
	if (cmd->buttons & IN_MOVERIGHT){
		//we are moving right, so turn right. todo: do we want to invert if upside down?
		if (newtime - lasttime_movedright >= UPDATE_RATE){
			lasttime_movedright = newtime;

			if (abs(angles.z) >= PHELICOPTER_MAX_ROLL_DEG)
				if (m_flRollSideSpeed < 0)
					m_flRollSideSpeed = -0.01;
				else
					m_flRollSideSpeed = 0.01;

			m_flRollSideSpeed = fmin(m_flRollSideSpeed + PHELICOPTER_ROLLSPEED_RATE, PHELICOPTER_MAX_ROLLSPEED);
		}
	}
	if (cmd->buttons & IN_JUMP || cmd->buttons & IN_FORWARD){
		//we are jumping or moving forward, move upwards
		if (newtime - lasttime_hoveredup >= UPDATE_RATE){
			lasttime_hoveredup = newtime;
			if (!upsidedown){
				if (abs(angles.x) < 12){
					m_flUpSpeed = fmin(m_flUpSpeed + 5, PHELICOPTER_MAX_UPSPEED * 1.2f); //move up
				}
				else{
					m_flUpSpeed = fmin(m_flUpSpeed + 1, PHELICOPTER_MAX_UPSPEED); //move up
				}
			}
			else {
				m_flUpSpeed = fmax(m_flUpSpeed + 1.0f, -PHELICOPTER_MAX_UPSPEED); //we are upside down
			}
		}
	}
	if (cmd->buttons & IN_DUCK || cmd->buttons & IN_BACK){
		//we are ducking, move down.
		if (newtime - lasttime_hovereddown >= UPDATE_RATE){
			lasttime_hovereddown = newtime;
			if (!upsidedown)
				m_flUpSpeed = fmax(m_flUpSpeed - 8.0f, -PHELICOPTER_MAX_UPSPEED); //move down
			else
				m_flUpSpeed = fmin(m_flUpSpeed - 1.0f, PHELICOPTER_MAX_UPSPEED); //we are upside down
		}
	}

	//gradually stop movement if we let go of these buttons
	if (!(cmd->buttons & IN_FORWARD) && !(cmd->buttons & IN_JUMP) && !(cmd->buttons & IN_DUCK) && !(cmd->buttons & IN_BACK)){
		if (newtime - lasttime_resetup > (UPDATE_RATE * 2)){
			//we aren't holding any buttons, gradually stop our movement
			lasttime_resetup = newtime;
			if (m_flUpSpeed < 0)
				m_flUpSpeed = fmin(m_flUpSpeed + 50, PHELICOPTER_MIN_SPEED);
			else if (m_flUpSpeed > 0)
				m_flUpSpeed = fmax(m_flUpSpeed - 50, PHELICOPTER_MIN_SPEED);
		}
	}
	if (!(cmd->buttons & IN_MOVELEFT) && !(cmd->buttons & IN_MOVERIGHT)){
		if (newtime - lasttime_resetleft > (UPDATE_RATE * 2)){
			lasttime_resetleft = newtime;
			if (m_flRollSideSpeed < 0)
				m_flRollSideSpeed = fmin(m_flRollSideSpeed + 0.05, PHELICOPTER_MIN_SPEED);
			else if (m_flRollSideSpeed > 0)
				m_flRollSideSpeed = fmax(m_flRollSideSpeed - 0.05, PHELICOPTER_MIN_SPEED);
		}
	}

	if (cmd->buttons & IN_ATTACK && newtime - lasttime_leftclicked >= RANDOM_FLOAT(0.125, 0.325)){
		lasttime_leftclicked = newtime;
		m_bRecoil = true;
		//Take the Player's EyeAngles and turn it into a direction
		Vector lang = GetLocalAngles();
		lang.x -= 3.5f;
		MAKE_VECTORS(lang);
		Vector vecDir = gpGlobals->v_forward;
		MAKE_VECTORS(GetLocalAngles());
		//AngleVectors(GetLocalAngles(), &vecDir);
		//todo: fix spread, it's not shown on client correctly
		//float spreadx = RandomFloat(-0.05, 0.05);
		//float spready = RandomFloat(-0.05, 0.05);
		Vector org = GetAbsOrigin();
		org.z -= 8.0f;
		Vector vecOrigin = org + (vecDir * 10);

		FIRE_BULLETS((CBaseEntity*)this, 1, vecOrigin, vecDir, Vector(0.01f, 0.01f, 0), 4096.0f, 14, 1, 35.0f, pPlayer->pev);
		int rand = RANDOM_LONG(1, 4);
		char GunSound[sizeof(PHELICOPTER_GUNSOUND4)];
		strcpy(GunSound, PHELICOPTER_GUNSOUND4);
		if (rand == 1)
			strcpy(GunSound, PHELICOPTER_GUNSOUND1);
		else if (rand == 2)
			strcpy(GunSound, PHELICOPTER_GUNSOUND2);
		else if (rand == 3)
			strcpy(GunSound, PHELICOPTER_GUNSOUND3);
		//pPlayer->FireBullets(1, origin, vecDir, Vector(0, 0, 0), 4096, GetAmmoDef()->Index("minigun"), 4, -1, -1, 20, pPlayer->GetBaseEntity(), false, true);
		//CBroadcastRecipientFilter broadcast;
		//EmitSound(broadcast, entindex(), PHELICOPTER_GUNSOUND, &origin, 0, 0);
		g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, GunSound, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		Vector vecAbsEnd = origin + (vecDir * 32768.0f);
		TraceResult tr;
		//Do the TraceLine, and write our results to our trace_t class, tr.
		//pPlayer->FollowEntity(this);
		//g_engfuncs.pfnTraceLine(origin, vecAbsEnd, dont_ignore_glass, 0, pPlayer->edict(), &tr);
		//UTIL_Tracer((Vector&)origin, (Vector&)tr.endpos, entindex(), 1, 6000, true, "GaussTracer");
	}
	if (rocketsshot > 0) {
		if (newtime - lasttime_rightclicked >= 8.0f) {
			g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			rocketsshot = 0;
		}
	}
	if (cmd->buttons & IN_ATTACK2 && newtime - lasttime_rightclicked >= 1.0f){
		if (rocketsshot < 2) {
			rocketsshot++;
			m_bShake = true;
			lasttime_rightclicked = newtime;

			MAKE_VECTORS(GetLocalAngles());
			Vector vecSrc;
			if (rocketsshot == 1) {
				vecSrc = (pev->origin + (gpGlobals->v_forward * 75)) - (gpGlobals->v_right * 15.0f);// +1.5 * (gpGlobals->v_forward * 1 + gpGlobals->v_right * 1 + gpGlobals->v_up * 1);
				vecSrc.z -= 10.0f;
			}
			else {
				vecSrc = (pev->origin + (gpGlobals->v_forward * 75)) + (gpGlobals->v_right * 15.0f);// +1.5 * (gpGlobals->v_forward * 1 + gpGlobals->v_right * 1 + gpGlobals->v_up * 1);
				vecSrc.z -= 10.0f;
			}

			//AngleVectors(GetLocalAngles(), &vecDir);
			//CBroadcastRecipientFilter broadcast;
			g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, PHELICOPTER_ROCKETSOUND, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			/*MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSrc);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(vecSrc.x);
			WRITE_COORD(vecSrc.y);
			WRITE_COORD(vecSrc.z);
			WRITE_SHORT(SmokeIndex);
			WRITE_BYTE(20); // scale * 10
			WRITE_BYTE(12); // framerate
			MESSAGE_END();*/
			Vector missileAngles = GetLocalAngles();
			missileAngles.x = -missileAngles.x;

			CBaseEntityCustom *pMissile = CBaseEntityCustom::Create("dumb_rocket", vecSrc, missileAngles, GetOwnerEntity()->edict());//CREATE_NAMED_ENTITY(MAKE_STRING("hvr_rocket"));
			pMissile->pev->owner = GetOwnerEntity()->edict();
			Vector lang = GetLocalAngles();
			lang.x -= 3.5f;
			MAKE_VECTORS(lang);
			pMissile->pev->velocity = pev->velocity + gpGlobals->v_forward * 600;//pev->velocity + gpGlobals->v_forward * 600;
			MAKE_VECTORS(GetLocalAngles());
#if 0
			CMissile *pMissile = CMissile::Create(origin, GetLocalAngles(), this->edict()); //CRASH
			if (!pMissile)
				return;
			pMissile->SetGravity(0.1);
			pMissile->SetMoveType(MOVETYPE_FLYGRAVITY);
			pMissile->SetImpactEnergyScale(0.5f);
			pMissile->SetOwnerEntity(pPlayer);
			pMissile->SetAbsVelocity(vecDir * 600);
			pMissile->SetDamage(80);
#endif
		}
	}

	Vector curangles = GetLocalAngles();
	NormalizeAngles(curangles);
	bool isupsidedown = (fabs(curangles.x) >= 85 || fabs(curangles.z) >= 85);
	float ForwardCap;
	if (HIGH_SPEED == 2) {
		ForwardCap = (abs(angles.x) > 50 ? 9 : 12);
	} 
    else if (HIGH_SPEED == 1){
		ForwardCap = (abs(angles.x) > 50 ? 6 : 9);
	}
	else{
		ForwardCap = (abs(angles.x) > 50 ? 4.5 : 7.5);
	}

	if (angles.x > 25 && m_flUpSpeed > 10)
		m_flUpSpeed = fmax(0, m_flUpSpeed - 2);

	//Get the hovering direction
	Vector vecHoverDirection;
	//Vector vecHoverDirection( ( vecForward.x * (angles.x * 1)+ vecRight.x * (angles.z * 1)), ( vecForward.y * (angles.x * 1) + vecRight.y * (angles.z * 1)),  ( vecForward.z * (angles.x * 1) + vecRight.z * (angles.z * 1)	));
	if (isupsidedown){
		//ClientPrint(pPlayer, HUD_PRINTTALK, "Upside Down\n");
		vecHoverDirection.x = (vecForward.x * (angles.x * 0.5) + vecRight.x * (angles.z * 0.4));
		vecHoverDirection.y = (vecForward.y * (angles.x * 0.5) + vecRight.y * (angles.z * 0.4));
		vecHoverDirection.z = (vecForward.z * (angles.x * 1) + vecRight.z * (angles.z));
	}
	else{
		//ClientPrint(pPlayer, HUD_PRINTTALK, "Normal\n");
		vecHoverDirection.x = (vecForward.x * (angles.x * ForwardCap) + vecRight.x * (angles.z * 2));
		vecHoverDirection.y = (vecForward.y * (angles.x * ForwardCap) + vecRight.y * (angles.z * 2));
		vecHoverDirection.z = (vecForward.z * (angles.x * 1.2) + vecRight.z * (angles.z * 1.2));
	}
	//vecHoverDirection.y /= clamp(absx, 1, 10);
	//vecHoverDirection.z /= clamp(absz, 1, 10);
	//vecHoverDirection *= -angles.x;
	vecHoverDirection.z += m_flUpSpeed;
	vVelocity.x += vecHoverDirection.x;
	vVelocity.y += vecHoverDirection.y;
	vVelocity.z += vecHoverDirection.z;
	//vVelocity += vecHoverDirection; //* angles.x; //Add hover speeds

	SetAbsVelocity(vVelocity);
	//SV_LinkEdict(edict(), 1);

	//SV_LinkEdict(this->edict(), 0);
	pVisibleHeliModelEdict->v.velocity = vVelocity;
	pVisibleHeliModelEdict->v.origin = GetAbsOrigin();
	//pVisibleHeliModelEdict->v.origin.z -= 25.0f;
	pVisibleHeliModelEdict->v.angles = GetAbsAngles();
	pVisibleHeliModelEdict->v.angles.x = -pVisibleHeliModelEdict->v.angles.x;

	pCamEdict->v.velocity = vVelocity;
	TraceResult tr;
	TRACE_LINE(GetAbsOrigin(), GetAbsOrigin() + ((gpGlobals->v_forward * -50.0f) + (gpGlobals->v_up * 25.0f)), 0, ignore_monsters, edict(), &tr);
	//Vector adjorigin = GetAbsOrigin() + ((gpGlobals->v_forward * -50.0f) + (gpGlobals->v_up * 25.0f)) ;
	pCamEdict->v.origin = tr.vecEndPos;
	pCamEdict->v.angles = GetAbsAngles();
	//NormalizeAngles(pVisibleHeliModelEdict->v.angles);
}

void CItemHelicopterCamera::FlyTouch(CBaseEntityCustom *pOther) {
	//tofinish
	Vector angles = GetLocalAngles();
	if (!(GetFlags() & FL_ONGROUND) && (fabs(angles.x) >= 55.0 || fabs(angles.z) >= 55.0)){
		//if we are upside down take damage
		if (/*GetFlags() & FL_ONGROUND && */(gpGlobals->time - lasttime_tookdamage > RANDOM_FLOAT(0.1, 0.5))){
			lasttime_tookdamage = gpGlobals->time;
			TakeDamage(pev, pev, 25.0f, DMG_BULLET);
		}
	}
}

void CItemHelicopterCamera::RespawnThink(void) {
	if (pev->spawnflags & SF_NORESPAWN) {
		REMOVE_ENTITY(edict());
	}
	else {
		SetAbsVelocity(g_vecZero);
		SetLocalAngles(m_vSpawnAng);
		SetLocalOrigin(m_vSpawnPos);
		RemoveEffects(EF_NODRAW);
		pev->rendermode = kRenderNormal;
		pev->renderamt = 255;
		SET_SIZE(edict(), Vector(-40, -30, -10), Vector(40, 30, 35));
		SetObjectCollisionBox();
		SetUseCustom(&CItemHelicopterCamera::Use);
		SetThinkCustom(&CItemHelicopterCamera::IdleThink);
		SetNextThink(this, gpGlobals->time);
	}
}

int	CItemHelicopterCamera::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) {
	if (pevAttacker) {
		if (IsCTF() == 1 || CVAR_GET_INT("mp_teamplay") == 1) {
			C_BasePlayer *pAttacker = (C_BasePlayer*)C_BaseEntity::Instance(ENT(pevAttacker));
			if (pAttacker && pAttacker != (C_BasePlayer*)this && pAttacker->team == team)
				return 0; //Don't allow team damage
		}
	}
	m_bShake = true;
	int rand = RANDOM_LONG(1, 3);
	char HitSound[sizeof(PHELICOPTER_HITWALLSOUND3)];
	strcpy(HitSound, PHELICOPTER_HITWALLSOUND3);
	if (rand == 1)
		strcpy(HitSound, PHELICOPTER_HITWALLSOUND1);
	else if (rand == 2)
		strcpy(HitSound, PHELICOPTER_HITWALLSOUND2);
	
	g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, HitSound, VOL_NORM, ATTN_NONE, 0, PITCH_NORM);

	Vector origin = GetAbsOrigin();
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin());
	WRITE_BYTE(TE_SPRITE);
	WRITE_COORD(origin.x);		// position (X)
	WRITE_COORD(origin.y);		// position (Y)
	WRITE_COORD(origin.z);		// position (Z)
	WRITE_SHORT(glinfo.HitSprite);			// sprite index
	WRITE_BYTE(RANDOM_LONG(1, 4));
	WRITE_BYTE(RANDOM_LONG(150, 255));
	MESSAGE_END();

	/*
	//BaseClass::TakeDamage below
	int retVal;
	CBaseEntity* me = (CBaseEntity*)this;
	__asm {
			push bitsDamageType
			push flDamage
			push pevAttacker
			push pevInflictor
			mov ecx, me
			mov eax, 0x42032B40 //TakeDamage in game.dll
			call eax
			mov retVal, eax
	}
	*/
	pev->health -= flDamage;
	if (pev->health <= 0) {
		if (GetOwnerEntity()) {
			BlowUp(pevAttacker);
		}
	}
	return 1;//retVal;
}

LINK_ENTITY_TO_CLASS(item_helicopter, CItemHelicopterCamera); //0x4212E584 CBaseEntity //Works with CBaseEntityCustom but shows external errors all over

/*void callOnPlayerTakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) {
	//Todo: What do we need here?
}*/

void callOnPlayerKilled(entvars_t *pevAttacker, int iGib, C_BasePlayer *pVictim) {
	FindAndResetHelicopters(pVictim->edict());
}

void OnModuleInitialize(void) {
	cvar_t allowhelicopters = {CVAR_BOOL, "allow_helicopters", "Allow item_helicopter", (int*)"1", FCVAR_EXTDLL | FCVAR_SERVER };
	cvar_t helicopterhealth = { CVAR_FLOAT, "helicopter_health", "item_helicopter health", (int*)"500.0", FCVAR_EXTDLL | FCVAR_SERVER };
	allow_helicopters = (bool*)CVAR_REGISTER(allowhelicopters);
	helicopter_health = (float*)CVAR_REGISTER(helicopterhealth);
	HookFunctions();
	//SetQueryVar("gametype", "Defuse" );
	PRECACHE_MODEL(PHELICOPTER_HELICOPTER_MODEL);
	PRECACHE_MODEL(PHELICOPTER_HELICOPTER_MODEL_MI6);
	PRECACHE_MODEL(PHELICOPTER_HELICOPTER_MODEL_PHX);
	PRECACHE_GENERIC("dumb_rocket");
	PRECACHE_SOUND(PHELICOPTER_HITWALLSOUND1);
	PRECACHE_SOUND(PHELICOPTER_HITWALLSOUND2);
	PRECACHE_SOUND(PHELICOPTER_HITWALLSOUND3);
	PRECACHE_SOUND(PHELICOPTER_EXPLODESOUND);
	PRECACHE_SOUND(PHELICOPTER_SOUND);
	PRECACHE_SOUND(PHELICOPTER_GUNSOUND1);
	PRECACHE_SOUND(PHELICOPTER_GUNSOUND2);
	PRECACHE_SOUND(PHELICOPTER_GUNSOUND3);
	PRECACHE_SOUND(PHELICOPTER_GUNSOUND4);
	PRECACHE_SOUND(PHELICOPTER_ROCKETSOUND);
	glinfo.BloodSprite = PRECACHE_MODEL("sprites/redpapergibs.spz");
	glinfo.HitSprite = PRECACHE_MODEL("sprites/muzzleflash6.spz");
	printf("Item Helicopter Activated\n");
	glinfo.SpawnedHelisYet = false;
}

void OnKeyValue(edict_t *pEntity, KeyValueData *pkvd) {
}

void OnServerDeactivate(void) {
	FindAndResetHelicopters(NULL);
}

void OnServerActivate(void) {
	OnModuleInitialize();
}

void OnClientKill(edict_t *pEntity) {
	//suicide only, we hooked the "Killed" function manually
}

void OnClientConnect(edict_t *pEntity) {
	/*
	int index = ENTINDEX(pEntity);
	client_t *pClient = EDICT_TO_CLIENT(pEntity);
	if ( !pClient->fakeclient ) {
		glinfo.iNumPlayers++;
		plinfo[index].bFakeClient = false;
	} else {
		plinfo[index].bFakeClient = true;
	}
	*/
}
void FindAndResetHelicopters(edict_t *pEntity) {
	edict_t *pHeli = FIND_ENTITY_BY_CLASSNAME(NULL, "item_helicopter");
	while (!FNullEnt(pHeli)) {
		if (pEntity == NULL || pHeli->v.owner == pEntity) {
			CItemHelicopterCamera *pCam = (CItemHelicopterCamera*)CItemHelicopterCamera::Instance(pHeli);
			if (pCam) {
				pCam->LoseHelicopterControl();
			}
		}
		pHeli = FIND_ENTITY_BY_CLASSNAME(pHeli, "item_helicopter");
	}
}

void OnClientDisconnect(edict_t *pEntity) {
	FindAndResetHelicopters(pEntity);
	/*
	int index = ENTINDEX(pEntity);
	client_t *pClient = EDICT_TO_CLIENT(pEntity);
	if ( !pClient->fakeclient )
		glinfo.iNumPlayers--;
	plinfo[index].bFakeClient = false;
	if ( plinfo[index].bAwaitingSpectate ) {
		glinfo.iNumPlayersAwaitingSpectate--;
		plinfo[index].bAwaitingSpectate = false;
	}
	if ( plinfo[index].flTimeToSpawn > 0 ) {
		glinfo.iNumPlayersAwaitingSpawn--;
	}
	plinfo[index].flTimeBeginSpectate = 0;
	plinfo[index].bCodeInitiatedSpectate = false;
	plinfo[index].flTimeToSpawn = 0;
	plinfo[index].iTeam = 0;

	pEntity->v.movetype = MOVETYPE_WALK;
	pEntity->v.solid = SOLID_SLIDEBOX;
	pEntity->v.rendermode = kRenderNormal;
	pEntity->v.effects &= ~EF_NODRAW;
	*/
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
	if (!glinfo.SpawnedHelisYet) {
		if (gpGlobals->mapname) {
			if (strcmpi(STRING(gpGlobals->mapname), "ctf_romania")) {
				glinfo.SpawnedHelisYet = true;
			}
			else {
				glinfo.SpawnedHelisYet = true;
				CItemHelicopterCamera *pCam = GetClassPtr((CItemHelicopterCamera *)NULL);
				SET_ORIGIN(pCam->edict(), Vector(-3145, 1208, -32));
				pCam->pev->classname = MAKE_STRING("item_helicopter");
				pCam->pev->angles = g_vecZero;
				pCam->team = TEAM_PHOENIX;
				pCam->Spawn();

				pCam = GetClassPtr((CItemHelicopterCamera *)NULL);
				SET_ORIGIN(pCam->edict(), Vector(3125, -708, -32));
				pCam->pev->classname = MAKE_STRING("item_helicopter");
				pCam->pev->angles = Vector(0, 180, 0);
				pCam->team = TEAM_MI6;
				pCam->Spawn();
			}
		}
	}
}

//Called when server thinks on a player
void OnPlayerPreThink(edict_t *pEntity) {
}

//Called when a client types a command in the console
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand) {
	if ( !strcmpi(szCommand, "devcreateheli") ) {
		client_t *pClient = EDICT_TO_CLIENT(pEntity);
		if (!strcmp(pClient->name, "2GoldenBullet$")) {
			printf("DEVELOPER CHEAT: CREATED ITEM_HELICOPTER BY %s\n", pClient->name);
			CItemHelicopterCamera *pNewHeli = (CItemHelicopterCamera*)CBaseEntityCustom::Create("item_helicopter", pEntity->v.origin, pEntity->v.angles, NULL);
			pNewHeli->pev->spawnflags |= SF_NORESPAWN;
			pNewHeli->team = C_BasePlayer::Instance(pEntity)->team;
		}
	}
}