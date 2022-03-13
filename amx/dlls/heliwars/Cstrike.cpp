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
#include "DylansHooks.h"

DLL_GLOBAL const Vector g_vecZero = Vector(0,0,0);

#define HOOK_ObjectCaps 5//47 = weapon touch 0 = spawn 5 = objectcaps
void *pOriginalFunction = NULL;
__declspec (naked) int ButtonObjectCaps() {
	__asm {
		push ecx
		mov tempRegister, ecx
	}
	callOnButtonObjectCaps((C_BaseEntity*)tempRegister);
	__asm {
		cmp eax, -1
		je NotBomb
			//IsBomb
			pop ecx
			retn
	}
		NotBomb:
		__asm {
			pop ecx
			mov eax, pOriginalFunction;
			call eax
			retn
		}
}

int callOnButtonObjectCaps( C_BaseEntity *pEntity ) {
	if ( glinfo.pBomb ) {
		if ( C_BaseEntity::Instance(glinfo.pBomb) == pEntity ) {
			//Allows players to hold E on the bomb instead of tapping
			return FCAP_CONTINUOUS_USE;
		}
	}
	return -1;
}

void HookButtonObjectCaps(void) {
	if ( pOriginalFunction == NULL ) {
		//get classname of the entity you want to hook
		edict_t *pEdict = CREATE_ENTITY();
		CALL_GAME_ENTITY(PLID, "func_button", &pEdict->v.classname);
		if(pEdict->pvPrivateData == NULL) {
			REMOVE_ENTITY(pEdict);
			return;
		}
		//get this pointer
		void *pthis = pEdict->pvPrivateData;
		//get vtable
		void **vtbl = *(void ***)pthis;
		REMOVE_ENTITY(pEdict);
		if(vtbl == NULL)
			return;
    
		int **ivtbl = (int **)vtbl;
		//get the original function
		pOriginalFunction = (void *)ivtbl[HOOK_ObjectCaps];
    
		//patch original function to our function
		DWORD oldflags;
		if(VirtualProtect(&ivtbl[HOOK_ObjectCaps], sizeof(int *), PAGE_READWRITE, &oldflags)) {
			ivtbl[HOOK_ObjectCaps] = (int *)ButtonObjectCaps;
		}
	}
}



BYTE *retJMP;
#if 0
__declspec(naked) void hookOnHudInit() {
	__asm {
		push ebp //create stack frame
		mov ebp, esp
		sub esp, 0x10
		mov dword ptr ss:[esp], eax
		mov dword ptr ss:[esp + 4], ecx
		mov dword ptr ss:[esp + 8], edx
		mov dword ptr ds:[esp + 0xC], edi

		mov edi, dword ptr ds:[edi + 0x4]   //EDICT_NUM
		mov edi, dword ptr ds:[edi + 0x204] //EDICT_NUM
		mov tempRegister, edi
	}
	callOnHudInit((edict_t*)tempRegister);
	retJMP = getHookFuncJMPAddr((DWORD)hookOnHudInit);
	 __asm {
		mov eax, dword ptr ss:[esp]
		mov ecx, dword ptr ss:[esp + 4]
		mov edx, dword ptr ss:[esp + 8]
		mov edi, dword ptr ss:[esp + 0xC]
		mov esp, ebp
		pop ebp

		MOV ECX,DWORD PTR DS:[0x4217CA9C]
		jmp retJMP //jump to original code
	 }
}

void callOnHudInit(edict_t *pEntity) {
	printf("hudinit\n");
}


/*
__declspec(naked) void hookOnPlayerTakeDamage() {
	__asm {
		mov tempECX, ecx
		mov tempEBX, ebx
		mov tempESI, esi
		mov eax, dword ptr ss:[esp + 4]
		mov tempRegister, eax
		mov eax, dword ptr ss:[esp + 8]
		mov tempRegister2, eax
		mov eax, dword ptr ss:[esp + 0xC]
		mov tempFloat, eax
		mov eax, dword ptr ss:[esp + 0x10]
		mov tempRegister3, eax
	}

	retJMP = getHookFuncJMPAddr((DWORD)hookOnPlayerTakeDamage);
	callOnPlayerTakeDamage((entvars_t2 *)tempRegister, (entvars_t2 *)tempRegister2, tempFloat, (int)tempRegister3);

	__asm {
		mov ecx, tempECX
		mov eax, tempEAX
		mov esi, tempESI
		//restore intercepted code below
		SUB ESP, 0x10
		PUSH EBX
		MOV EBX,DWORD PTR SS:[ESP+0x24]

		jmp retJMP
	}
}
*/

__declspec(naked) void hookOnBasePlayerSpawn() {
	__asm {
		push ebx
		push ecx
		push esi
		mov tempRegister, ecx
	}

	retJMP = getHookFuncJMPAddr((DWORD)hookOnBasePlayerSpawn);
	callOnBasePlayerSpawn((C_BasePlayer*)tempRegister);

	__asm {
		pop esi
		pop ecx
		pop ebx
		test eax, eax
		jne ContinueSpawn
		retn //Don't allow a spawn

		ContinueSpawn:
		//restore intercepted code
		sub esp, 0x24
		push ebx
		push ebp

		jmp retJMP
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
	callOnPlayerKilled((entvars_t2 *)tempRegister, (int)tempRegister2, (C_BasePlayer *)tempRegister3);

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

__declspec(naked) BOOL __stdcall hookOnFPlayerCanRespawn(C_BasePlayer *pPlayer) {
	__asm {
		mov tempEAX, eax
		mov eax, dword ptr ss:[esp + 4]
		mov tempRegister, eax
	}
	callOnFPlayerCanRespawn((C_BasePlayer*)tempRegister);
	__asm {
		RETN 4
	}
}

__declspec(naked) void hookOnPlayerSpawnCTF() {
	__asm {
		mov dword ptr ds:[tempECX], ecx
		mov dword ptr ds:[tempEAX], eax
		mov dword ptr ds:[tempESI], esi
		mov dword ptr ds:[tempEDX], edx
		mov tempRegister, ESI
	}
	retJMP = getHookFuncJMPAddr((DWORD)hookOnPlayerSpawnCTF);
	callOnClientSpawnCTF((C_BasePlayer*)tempRegister);

	__asm {
		//add esp, 4
		mov ecx, dword ptr ds:[tempECX]
		mov eax, dword ptr ds:[tempEAX]
		mov esi, dword ptr ds:[tempESI]
		mov edx, dword ptr ds:[tempEDX]
		//The code we replaced below
		pop edi
		test eax, eax
		mov eax, dword ptr ds:[esi+4]
		jmp retJMP
	}
}

__declspec(naked) void hookOnPlayerSpawnDM() {
	__asm {
		mov dword ptr ds:[tempECX], ecx
		mov dword ptr ds:[tempEAX], eax
		mov dword ptr ds:[tempESI], esi
		mov dword ptr ds:[tempEDX], edx
		mov tempRegister, ESI
	}
	retJMP = getHookFuncJMPAddr((DWORD)hookOnPlayerSpawnDM);
	callOnClientSpawnDM((C_BasePlayer*)tempRegister);

	__asm {
		mov ecx, dword ptr ds:[tempECX]
		mov eax, dword ptr ds:[tempEAX]
		mov esi, dword ptr ds:[tempESI]
		mov edx, dword ptr ds:[tempEDX]
		//The code we replaced below
		test eax, eax
		mov eax, dword ptr ds:[esi+4]
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

#endif

FARPROC callOnGetEntityInit(LPCSTR lpProcName) {
	if ( !strcmp(lpProcName, "func_bomb_target") ) {
        HMODULE hMod = GetModuleHandle("Bond_cstrike_amxx.dll");
		if ( hMod ) {
			//FARPROC test = GetProcAddress(hMod, "bomb_target");
			//return test;
			FARPROC test2 = GetProcAddress(hMod, lpProcName);
			return test2;
		}
	}
	return NULL;
}

#if 0
HookCallInfo funcHooks[] = {
	{(BYTE*)ADR_PLAYERSPAWNCTF, (DWORD)hookOnPlayerSpawnCTF, 6, (BYTE*)ADR_PLAYERSPAWNCTF+0x6},
	{(BYTE*)ADR_PLAYERSPAWNDM, (DWORD)hookOnPlayerSpawnDM, 5, (BYTE*)ADR_PLAYERSPAWNDM+0x5},
	{(BYTE*)ADR_FPLAYERCANRESPAWN, (DWORD)hookOnFPlayerCanRespawn, 6, (BYTE*)ADR_FPLAYERCANRESPAWN+0x5},
	//{(BYTE*)ADR_PLAYERTAKEDAMAGE, (DWORD)hookOnPlayerTakeDamage, 6, (BYTE*)ADR_PLAYERTAKEDAMAGE+0x6}, //not needed right now
	{(BYTE*)ADR_PLAYERKILLED, (DWORD)hookOnPlayerKilled, 6, (BYTE*)ADR_PLAYERKILLED+0x6},
	{(BYTE*)ADR_GETENTITYINIT, (DWORD)hookOnGetEntityInit, 5, (BYTE*)ADR_GETENTITYINIT+0x5}, //comment me
	{(BYTE*)ADR_BASEPLAYERSPAWN, (DWORD)hookOnBasePlayerSpawn, 5, (BYTE*)ADR_BASEPLAYERSPAWN+0x5},
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
#endif
// END OF HOOKS


class CBombTarget : public CBaseEntityCustom {//, C_BaseEntity {
	//CBombTarget::CBombTarget(); //Constructor
	//CBombTarget::~CBombTarget(); //Destructor
public:
	virtual int		ObjectCaps(void) { return FCAP_CONTINUOUS_USE; }
	void Precache(void);
	//virtual void Think(void);
	void Spawn(void);// { printf("SPAWNED BOMBTARGET!\nSPAWNED BOMBTARGET!\nSPAWNED BOMBTARGET!\nSPAWNED BOMBTARGET!\nSPAWNED BOMBTARGET!\n"); }
	void Use(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value);
	void TestThink(void);
};

//void CBombTarget::Think(void) {
	//SetThinkCustom(&CBombTarget::TestThink);
	//void(__thiscall CBombTarget::* pFunc)(void) = &CBombTarget::TestThink;
	//void* pPtr = (void*&)pFunc;
	//SET_THINK2((C_BaseEntity*)this, pPtr);
	//this->pev->nextthink = gpGlobals->time + 0.5f;
//}

void CBombTarget::Spawn(void) {
	Precache();
	printf("SPAWNED BOMBTARGET!\n");
	SET_MODEL(this->edict(), "models/player.mdl");
	//SetUseCustom(&CBombTarget::Use);//C_BaseEntity::Instance(this->edict()), &CBombTarget::Use);
	//SET_THINK2((C_BaseEntity*)this, (void*)TestThink);
	void(__thiscall CBombTarget::* pFunc)(void) = &CBombTarget::TestThink;
	void* pPtr = (void*&)pFunc;
	SET_THINK2((C_BaseEntity*)this, pPtr);
	//SET_THINK2((C_BaseEntity*)this, *this->TestThink);
	//SetThinkCustom(&CBombTarget::TestThink);
	this->pev->nextthink = gpGlobals->time;
}

void CBombTarget::Precache(void) {
	PRECACHE_MODEL("models/player.mdl");
}

void CBombTarget::Use(CBaseEntityCustom *pActivator, CBaseEntityCustom *pCaller, USE_TYPE useType, float value) {
}

void CBombTarget::TestThink(void) {
	Vector ang = this->edict()->v.angles;
	ang.y += 1;
	ang.x -= 1;
	ang.z += 10;
	this->edict()->v.angles = ang;
	//SV_LinkEdict(this->edict(), 1);
	this->pev->nextthink = gpGlobals->time + 0.033f;
}

LINK_ENTITY_TO_CLASS(func_bomb_target, CBombTarget); //0x4212E584 CBaseEntity //Works with CBaseEntityCustom but shows external errors all over

/*void callOnPlayerTakeDamage(entvars_t2 *pevInflictor, entvars_t2 *pevAttacker, float flDamage, int bitsDamageType) {
	//Todo: What do we need here?
}*/

void callOnPlayerKilled(entvars_t2 *pevAttacker, int iGib, C_BasePlayer *pVictim) {
	if ( glinfo.bWarmupComplete ) {
		C_BasePlayer *pAttacker = C_BasePlayer::Instance(pevAttacker->pContainingEntity);
		if ( pAttacker && pAttacker->IsPlayer() && !pevAttacker->team == pVictim->team && pAttacker != pVictim) {
			//Give the attacker some points for killing this player
			SetCTFScore(pAttacker, GetCTFScore(pAttacker) + 1);
			//Give money to attacker
		}
		int index = ENTINDEX(pVictim->edict());
		if (!IsPlayerSpectating(pVictim->edict()) && plinfo[index].bCodeInitiatedSpectate != true) {
			int RealTeam = pVictim->team;
			if ( pVictim->team == TEAM_MI6 && plinfo[index].iTeam == TEAM_PHOENIX )
				RealTeam = TEAM_PHOENIX; //Player changed teams, just ignore their current team for now
			else if ( pVictim->team == TEAM_PHOENIX && plinfo[index].iTeam == TEAM_MI6 )
				RealTeam = TEAM_MI6; //Player changed teams, just ignore their current team for now
			plinfo[index].iTeam = pVictim->team; //Now store the new current team in our database

			if ( RealTeam == TEAM_MI6 ) {
				glinfo.iNumPlayersAliveMI6--;
			} else {
				glinfo.iNumPlayersAlivePhoenix--;
				if ( glinfo.clBombCarrier ) {
					client_t *pBombCarrier = (client_t*)glinfo.clBombCarrier;
					C_BasePlayer *pCarrier = C_BasePlayer::Instance(pBombCarrier->edict);
					if ( pCarrier == pVictim ) {
						DropBomb();
					}
				}
			}
			plinfo[index].bCodeInitiatedSpectate = true;
		}
	}
}

BOOL callOnFPlayerCanRespawn (C_BasePlayer* pPlayer) {
	//returns if player is allowed to respawn
	if ( glinfo.bAllowPlayerSpawn || plinfo[ENTINDEX(pPlayer->edict())].bCodeInitiatedSpectate )
		return TRUE;
	return FALSE;
}

void callOnClientSpawnCTF(C_BasePlayer *pPlayer) {
	//No longer used right now
}

void callOnClientSpawnDM(C_BasePlayer *pPlayer) {
	//No longer used right now
}

BOOL callOnBasePlayerSpawn(C_BasePlayer *pPlayer) {
	//Had to hook this function to prevent player from spawning when connecting to the server

	if ( glinfo.bRoundPreStartThink ) {
		//Fade the screen during the pre-round time
		UTIL_ScreenFade(NULL, 245, 240, 205, 100, 0, glinfo.flRoundPreStartTimeEnd - gpGlobals->time, 0);
	}

	int index = ENTINDEX(pPlayer->edict());
	plinfo[index].iTeam = pPlayer->team;

	if ( plinfo[index].bCodeInitiatedSpectate && CVAR_GET_BOOL("allow_spectators") ) {
		//Player tried to respawn while dead, put them in spectator mode
		if ( !plinfo[index].bAwaitingSpectate ) {
			edict_t *pEntity = pPlayer->edict();
			int index = ENTINDEX(pEntity);
			glinfo.iNumPlayersAwaitingSpectate++;
			plinfo[index].bAwaitingSpectate = true;
			plinfo[index].flTimeBeginSpectate = gpGlobals->time + 1.0f;
			pEntity->v.effects |= EF_NODRAW;
			pEntity->v.flags |= FL_FROZEN;
			pEntity->v.takedamage = DAMAGE_NO;
			return TRUE;
		}
	}

	if ( callOnFPlayerCanRespawn(pPlayer) == TRUE ) {
		//Player is allowed to spawn
		if ( glinfo.bWarmupComplete ) {
			if ( pPlayer->team == TEAM_MI6 )
				glinfo.iNumPlayersAliveMI6++;
			else if ( pPlayer->team == TEAM_PHOENIX )
				glinfo.iNumPlayersAlivePhoenix++;
			else {
				glinfo.bForcePlayerAliveCheck = true;
				SetCStrikeNextThink(0.25f);
				printf("WARNING: Couldn't find a team for player %s (team was %i), forcing a recheck!\n", EDICT_TO_CLIENT(pPlayer->edict())->name, pPlayer->team);
			}
		}
		SetCTFCaptures(TEAM_MI6, GetCTFCaptures(TEAM_MI6)); //Done to update the flag status, it's broken otherwise for some reason
		SetCTFCaptures(TEAM_PHOENIX, GetCTFCaptures(TEAM_PHOENIX)); //Done to update the flag status, it's broken otherwise for some reason
		return TRUE;
	} else if ( CVAR_GET_BOOL("allow_spectators") ) {
		//Player joined the server during the round, make them enter spectator mode
		edict_t *pEntity = pPlayer->edict();
		int index = ENTINDEX(pEntity);
		glinfo.iNumPlayersAwaitingSpectate++;
		plinfo[index].bCodeInitiatedSpectate = true;
		plinfo[index].bAwaitingSpectate = true;
		plinfo[index].flTimeBeginSpectate = gpGlobals->time + 1.0f;
		pEntity->v.effects |= EF_NODRAW;
		pEntity->v.flags |= FL_FROZEN;
		pEntity->v.takedamage = DAMAGE_NO;
		return TRUE;
	}
	return FALSE;
}


int* mp_startmoney = nullptr;
float* mp_roundtime = nullptr;
float* mp_freezetime = nullptr;
float* mp_warmuptime = nullptr;
int* mp_maxrounds = nullptr;
float* mp_round_restart_delay = nullptr;
float* mp_c4timer = nullptr;

void Cmd_EndRound() {
	glinfo.flRoundTimeEnd = 0;
}

void Cmd_ResetGame() {
	DeleteBomb();
	ResetRoundSystem();
	CreateBomb();
}

#define C4_PICKUP_SOUND "weapons/tripmine_holster.wav"
#define C4_PLANT_START_SOUND "gadgets/qdeck_tap1.wav"
#define C4_DEFUSE_START_SOUND "Island/tinker_mix.wav"
#define C4_DROP_SOUND "items/weapondrop1.wav"
#define C4_EXPLODE_SOUND "explosions/chopper_explode.wav"
#define C4_BEEP_SOUND "weapons/laser_trip_on.wav"
#define C4_WARNING_SOUND "weapons/tripmine_proximity.wav"

void OnModuleInitialize(void) {
	if (!mp_startmoney)
	{
		cvar_t_small startmoney = { CVAR_INT, "mp_startmoney", "Money start", "800", FCVAR_EXTDLL | FCVAR_SERVER };
		cvar_t_small roundtime = { CVAR_FLOAT, "mp_roundtime", "Round Time in minutes", "5.0", FCVAR_EXTDLL | FCVAR_SERVER };
		cvar_t_small freezetime = { CVAR_FLOAT, "mp_freezetime", "Freeze Time", "7.0", FCVAR_EXTDLL | FCVAR_SERVER };
		cvar_t_small warmuptime = { CVAR_FLOAT, "mp_warmuptime", "Warmup Time", /*"240.0"*/"60.0", FCVAR_EXTDLL | FCVAR_SERVER };
		cvar_t_small maxrounds = { CVAR_INT, "mp_maxrounds", "Max Rounds", "31", FCVAR_EXTDLL | FCVAR_SERVER };
		cvar_t_small round_restart_delay = { CVAR_FLOAT, "mp_round_restart_delay", "Delay after a round ends before a new one begins", "7.0", FCVAR_EXTDLL | FCVAR_SERVER };
		cvar_t_small c4timer = { CVAR_FLOAT, "mp_c4timer", "Time in seconds before C4 explodes", "45.0", FCVAR_EXTDLL | FCVAR_SERVER };
		mp_startmoney = (int*)CVAR_REGISTER(startmoney);
		mp_roundtime = (float*)CVAR_REGISTER(roundtime);
		mp_freezetime = (float*)CVAR_REGISTER(freezetime);
		mp_warmuptime = (float*)CVAR_REGISTER(warmuptime);
		mp_maxrounds = (int*)CVAR_REGISTER(maxrounds);
		/*
		int test;
		__asm {
		mov eax, mp_maxrounds
		mov eax, dword ptr ds:[eax + 0x14]
		mov test, eax
		}*/
		mp_round_restart_delay = (float*)CVAR_REGISTER(round_restart_delay);
		mp_c4timer = (float*)CVAR_REGISTER(c4timer);

		REG_SVR_COMMAND("de_endround", Cmd_EndRound);
		REG_SVR_COMMAND("de_resetgame", Cmd_ResetGame);
	}
	glinfo.flTimeToCheckBombSites = gpGlobals->time + 3;
	PRECACHE_MODEL("models/c4_explosives.mdl");
	PRECACHE_SOUND("defusal/moveout.wav");
	PRECACHE_SOUND("defusal/bombdef.wav");
	PRECACHE_SOUND("defusal/bombpl.wav");
	PRECACHE_SOUND("defusal/rounddraw.wav");
	PRECACHE_SOUND("ctf/mi6_ctf.wav");
	PRECACHE_SOUND("ctf/phoenix_ctf.wav");
	PRECACHE_SOUND(C4_PICKUP_SOUND);
	PRECACHE_SOUND(C4_PLANT_START_SOUND);
	PRECACHE_SOUND(C4_DEFUSE_START_SOUND);
	PRECACHE_SOUND(C4_DROP_SOUND);
	PRECACHE_SOUND(C4_EXPLODE_SOUND);
	PRECACHE_SOUND(C4_BEEP_SOUND);
	PRECACHE_SOUND(C4_WARNING_SOUND);
	glinfo.g_sModelIndexFireball = PRECACHE_MODEL ("sprites/zerogxplode.spz");// Bomb Explosion Sprite
	SetQueryVar("gametype", "Defuse" );

	static bool AlreadyHooked = false;

	if (!AlreadyHooked)
	{
		ADD_DLL_HOOK(&callOnGetEntityInit, ADD_HOOK_TYPES::H_ENTITYINIT);
		ADD_DLL_HOOK(&callOnPlayerKilled, ADD_HOOK_TYPES::H_PLAYERKILLED);
		ADD_DLL_HOOK(&callOnBasePlayerSpawn, ADD_HOOK_TYPES::H_BASEPLAYERSPAWN);
		ADD_DLL_HOOK(&callOnFPlayerCanRespawn, ADD_HOOK_TYPES::H_FPLAYERCANRESPAWN);
		AlreadyHooked = true;
	}

	printf("Bomb Defusal Initialized\n");
}

void OnKeyValue(edict_t *pEntity, KeyValueData *pkvd) {
}

void OnServerDeactivate(void) {
	ResetRoundSystem();
}

void OnServerActivate(void) {
	ResetRoundSystem();
	OnModuleInitialize();
}

void OnClientKill(edict_t *pEntity) {
	//suicide only, we hooked the "Killed" function manually
}

void OnClientConnect(edict_t *pEntity) {
	int index = ENTINDEX(pEntity);
	client_t *pClient = EDICT_TO_CLIENT(pEntity);
	if ( !pClient->fakeclient ) {
		glinfo.iNumPlayers++;
		plinfo[index].bFakeClient = false;
	} else {
		plinfo[index].bFakeClient = true;
	}
	plinfo[index].iMoney = CVAR_GETDIRECT(mp_startmoney);
}

void OnClientDisconnect(edict_t *pEntity) {
	int index = ENTINDEX(pEntity);
	client_t *pClient = EDICT_TO_CLIENT(pEntity);
	if ( !pClient->fakeclient )
		glinfo.iNumPlayers--;
	plinfo[index].bFakeClient = false;
	plinfo[index].iMoney = 0;
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

	if ( glinfo.bWarmupComplete ) {
		C_BasePlayer *pPlayer = C_BasePlayer::Instance(pEntity);
		if ( pPlayer->IsAlive() && !IsPlayerSpectating(pEntity) ) { //FIX THIS FIXME
			if ( GetPlayerTeam(pPlayer) == 1 )
				glinfo.iNumPlayersAliveMI6--;
			else
				glinfo.iNumPlayersAlivePhoenix--;
		}
		if ( glinfo.clBombCarrier ) {
			client_t *pBombCarrier = (client_t*)glinfo.clBombCarrier;
			if ( pBombCarrier->edict == pEntity ) {
				//Bomb carrier disconnected, drop the bomb
				DropBomb();
			}
		}
	}

	pEntity->v.movetype = MOVETYPE_WALK;
	pEntity->v.solid = SOLID_SLIDEBOX;
	pEntity->v.rendermode = kRenderNormal;
	pEntity->v.effects &= ~EF_NODRAW;

	if ( glinfo.iNumPlayers == 0 ) {
		ResetRoundSystem();
	}
}

//Called when client's user info string changes
void CStrike_HandleClientUserInfOChanged( edict_t *pEntity, char *infobuffer ) {
	//int teamchoice = atoi(INFOKEY_VALUE(infobuffer, "teamchoice"));
}


//Called when a server runs a frame
void OnStartFrame(void) {
	if ( IsTimeToThink() ) {
		SetCStrikeNextThink(0.1f);

		if ( !CheckedForBombSites() ) {
			if ( !IsTimeToCheckForBombSites() )
				return;
			FindBombSites();
			CreateBomb();
		}

		if ( glinfo.iNumPlayers == 0 ) {
			//Server is empty, don't do anything
			//ResetRoundSystem();
			SetCStrikeNextThink(1.0f);
			return;
		}

		DrawClock();

		if ( !glinfo.bWarmupComplete ) {
			//We are in the warmup period
			//Todo: Tidy this up a bit
			//glinfo.bAllowPlayerSpawn = true;

			if ( glinfo.iNumPlayers == 1 ) {
				//Warmup is indefinite if there is only 1 player
				return;
			}

			if ( !glinfo.bWarmupStarted ) {
				//Start the warmup
				glinfo.bWarmupStarted = true;
				glinfo.flWarmupTimeEnd = gpGlobals->time + CVAR_GETDIRECT(mp_warmuptime);
			}

			//Warmup Think
			glinfo.flClockSeconds = max(0, glinfo.flWarmupTimeEnd - gpGlobals->time);

			if ( gpGlobals->time < glinfo.flWarmupTimeEnd ) {
				//Warmup is not complete.
				return;
			}

			//Warmup is complete
			OnWarmupComplete();
		}

		if ( gpGlobals->time >= glinfo.flNextTimeForceAliveCheck ) {
			//Temporary workaround for spectate kill bugs and other bugs
			glinfo.flNextTimeForceAliveCheck = gpGlobals->time + 4.0f;
			glinfo.bForcePlayerAliveCheck = true;
		}

		if ( glinfo.bForcePlayerAliveCheck ) {
			//There was a player that spawned with a NULL team, validate the player alive count.
			//This is very hacky, but I don't know of any other easy way to fix it other than maybe hooking ChangePlayerTeamCTF
			glinfo.bForcePlayerAliveCheck = false;
			glinfo.iNumPlayersAliveMI6 = 0;
			glinfo.iNumPlayersAlivePhoenix = 0;
			for ( int i = 0; i < gpGlobals->maxClients; i++ ) {
				client_t *pClient = GetClientPointerFromIndex(i);
				if ( pClient->active || pClient->connected || pClient->spawned ) {
					C_BasePlayer *pPlayer = C_BasePlayer::Instance(pClient->edict);
					if ( pPlayer && pPlayer->IsAlive() && !IsPlayerSpectating(pClient->edict) ) {
						plinfo[ENTINDEX(pClient->edict)].iTeam = pPlayer->team;
						if ( pPlayer->team == TEAM_MI6 )
							glinfo.iNumPlayersAliveMI6++;
						else if ( pPlayer->team == TEAM_PHOENIX )
							glinfo.iNumPlayersAlivePhoenix++;
						else
							glinfo.bForcePlayerAliveCheck = true; //Still an invalid team, recheck again
					}
				}
			}
		}

		if ( glinfo.iNumPlayersAwaitingSpectate > 0 || glinfo.iNumPlayersAwaitingSpawn > 0 ) {
			for ( int i = 1; i <= gpGlobals->maxClients; i++ ) {
				if ( plinfo[i].bAwaitingSpectate ) {
					//If there are players that died and are waiting to become spectators, see if it's time to make them one
					edict_t *pEntity = GetClientPointerFromIndex(i - 1)->edict;
					StripWeapons(pEntity, 1);
					if ( gpGlobals->time >= plinfo[i].flTimeBeginSpectate ) {
						if ( pEntity->v.deadflag == DEAD_NO ) {
							if ( gpGlobals->time - plinfo[i].flTimeBeginSpectate > 2 ) {
								//hacky way to force strip weapons after player spawns
								plinfo[i].flTimeBeginSpectate = 0;
								plinfo[i].bAwaitingSpectate = false;
								glinfo.iNumPlayersAwaitingSpectate--;
							} else {
								CLIENT_COMMAND(GetClientPointerFromIndex(i - 1)->edict, "codebeginspectate\n");
								StripWeapons(pEntity, 1);
							}
						}
					}
				} else if ( plinfo[i].flTimeToSpawn > 0 && gpGlobals->time >= plinfo[i].flTimeToSpawn ) {
					//If there are players that are waiting to spawn because they were in spectator mode at round start, spawn them asap
					glinfo.iNumPlayersAwaitingSpawn--;
					plinfo[i].flTimeToSpawn = 0;
					edict_t *pEntity = GetClientPointerFromIndex(i - 1)->edict;
					C_BasePlayer::Instance(pEntity)->Spawn(); //Spawn player immediately
					pEntity->v.rendermode = kRenderNormal;
					pEntity->v.effects &= ~EF_NODRAW;
				}
			}
		}

		if ( glinfo.bGameEndThink ) {
			//We are in the intermission after all rounds have ended and the game ends
			GameEndThink();
			return;
		}

		if ( glinfo.bRoundPreStartThink ) {
			//We are in the waiting period just before a new round is about to start
			RoundPreStartThink();
		}
		
		if ( glinfo.bRoundEndThink ) {
			//We are in the waiting period after a round has ended, and before the players respawn and start the pre-round waiting period
			RoundEndThink();
			return;
		}

		if ( glinfo.bRoundThink ) {
			//We are in the middle of a round
			RoundThink();
		}
		
	}
}

//Called when server thinks on a player
void OnPlayerPreThink(edict_t *pEntity) {
	//C_BasePlayer *test = C_BasePlayer::Instance(pEntity);
	//int real = (int)test + 0x2C;
	//test->FireBullets(NULL, Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0), 0, 1, 1, 1, NULL);
	//test->Respawn();

	if ( IsPlayerSpectating(pEntity) ) {
		if ( glinfo.clBombCarrier ) {
			client_t *pBombCarrier = (client_t*)glinfo.clBombCarrier;
			if ( pBombCarrier->edict == pEntity ) {
				//Bomb carrier is a spectator, drop the bomb
				DropBomb();
			}
		}
	}
	else if (glinfo.clBombCarrier && pEntity == ((client_t*)glinfo.clBombCarrier)->edict) {
		client_t *pClient = GET_CLIENT_POINTER(ENTINDEX(pEntity) - 1);
		if (pClient->lastcmd.impulse & 205) {
			DropBomb();
		}
	}
}

//Called once as soon as the warmup is complete
void OnWarmupComplete(void) {
	glinfo.bWarmupComplete = true;
	glinfo.bWarmupStarted = false;

	RoundPreStart();
}


//Called while the bomb is not already planted
void BombPlantThink(void) {
	//Todo: Simplify this function by hooking the use function on the bomb site itself

	if ( !glinfo.bBombExplodeThink ) {
		//Bomb is not already planted
		if ( glinfo.clBombCarrier ) {
			//Bomb has a player carrying it
			client_t *pCarrier = (client_t*)glinfo.clBombCarrier;
			if ( pCarrier->edict->v.button & IN_USE ) {
				//Bomb carrier is holding the use key
				bool IsPlanting = false;

				//Bomb Site A Check
				if ( glinfo.pBombSiteA ) {

					//Try finding out if the player is pressing the use key on Bomb Site A

					edict_t* pSearchEnt = NULL;
					while (1) {
						if ( glinfo.bBombStartedPlanting && glinfo.iBombSitePlantingAt != 0 )
							break; //We are planting at a different bomb site, get out of here!

						pSearchEnt = FIND_ENTITY_IN_SPHERE( pSearchEnt, glinfo.pBombSiteA->v.origin, glinfo.flBombSiteARadius );
						if ( FNullEnt(pSearchEnt) ) {
							//No entities are in the bomb site radius or ran out of entities to check
							if ( glinfo.bBombStartedPlanting ) {
								//Player was planting at the bomb site but is no longer in the bomb site radius
								glinfo.bBombStartedPlanting = false;
								glinfo.flBombPlantTime = 0;
								glinfo.iBombSitePlantingAt = -1;
							}
							break;
						} else {
							if ( pSearchEnt == pCarrier->edict ) {
								//Bomb Carrier is in the plant zone radius
								IsPlanting = true;
								if ( !glinfo.bBombStartedPlanting ) {
									TraceResult tr;
									TRACE_LINE(glinfo.pBombSiteA->v.origin, pCarrier->edict->v.origin + pCarrier->edict->v.view_ofs, 0, dont_ignore_monsters, glinfo.pBombSiteA, &tr);
									//Trace a line from the bomb site to the bomb carrier to see if there's any obstruction between them
									if ( tr.pHit == pCarrier->edict ) {
										//No obstruction, player is free to plant
										//Player just started planting
										glinfo.iBombSitePlantingAt = 0;
										OnBombStartPlanting(glinfo.pBombSiteA);
										BombPlantUseThink(glinfo.pBombSiteA);
									}
								} else {
									//Player is still planting
									BombPlantUseThink(glinfo.pBombSiteA);
								}
								break;
							}
						}
					}
				}


				//Bomb Site B Check
				if ( !IsPlanting && glinfo.pBombSiteB ) {
					//Try finding out if the player is pressing the use key on Bomb Site B
					edict_t* pSearchEnt = NULL;
					while (1) {
						if ( glinfo.bBombStartedPlanting && glinfo.iBombSitePlantingAt != 1 )
							break; //We are planting at a different bomb site, get out of here!

						pSearchEnt = FIND_ENTITY_IN_SPHERE( pSearchEnt, glinfo.pBombSiteB->v.origin, glinfo.flBombSiteBRadius );
						if ( FNullEnt(pSearchEnt) ) {
							//No entities are in the bomb site radius or ran out of entities to check
							if ( glinfo.bBombStartedPlanting ) {
								//Player was planting at the bomb site but is no longer in the bomb site radius
								glinfo.bBombStartedPlanting = false;
								glinfo.flBombPlantTime = 0;
								glinfo.iBombSitePlantingAt = -1;
							}
							break;
						} else {
							if ( pSearchEnt == pCarrier->edict ) {
								//Bomb Carrier is in the plant zone radius
								if ( !glinfo.bBombStartedPlanting ) {
									TraceResult tr;
									TRACE_LINE(glinfo.pBombSiteB->v.origin, pCarrier->edict->v.origin + pCarrier->edict->v.view_ofs, 0, dont_ignore_monsters, glinfo.pBombSiteB, &tr);
									//Trace a line from the bomb site to the bomb carrier to see if there's any obstruction between them
									if ( tr.pHit == pCarrier->edict ) {
										//No obstruction, player is free to plant
										//Player just started planting
										glinfo.iBombSitePlantingAt = 1;
										OnBombStartPlanting(glinfo.pBombSiteB);
										BombPlantUseThink(glinfo.pBombSiteB);
									}
								} else {
									//Player is still planting
									BombPlantUseThink(glinfo.pBombSiteB);
								}
								break;
							}
						}
					}
				}

			//Above = pressing use key
			} else {
				//Bomb Carrier is not pressing the Use Key
				if ( glinfo.bBombStartedPlanting ) {
					//Player was planting but no longer is pressing the Use Key
					glinfo.bBombStartedPlanting = false;
					glinfo.flBombPlantTime = 0;
					glinfo.iBombSitePlantingAt = -1;
				}
			}


		} else {
			//No Bomb Carrier, don't do anything
			if ( glinfo.bBombStartedPlanting ) {
				//Player was planting but no longer has the bomb
				glinfo.bBombStartedPlanting = false;
				glinfo.flBombPlantTime = 0;
				glinfo.iBombSitePlantingAt = -1;
			}
		}
	}
}

//Called while the player is pressing the use key on a bomb site to plant the bomb
void BombPlantUseThink(edict_t *pBombSite) {
	if ( gpGlobals->time >= glinfo.flBombPlantTime ) {
		//Bomb was successfully planted
		glinfo.iBombSitePlantingAt = -1;
		OnBombPlant(pBombSite);
	} else {
		client_t *clPlanter = (client_t*)glinfo.clBombCarrier;
		char progress[36];
		float timeremaining = glinfo.flBombPlantTime - gpGlobals->time;
		if ( timeremaining < 0 )
			timeremaining = 0;
		sprintf(progress, "BOMB PLANT TIME REMAINING: %.0f", timeremaining );
		MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "HudText", NULL), 0, clPlanter->edict);
				WRITE_STRING( progress );
		MESSAGE_END();
	}
}

//Called as soon as the bomb is starting to be planted (as soon as they push e on the bombsite)
void OnBombStartPlanting(edict_t *pBombSite) {
	client_t *pCarrier = (client_t*)glinfo.clBombCarrier;
	glinfo.flPrevPlanterMaxSpeed = pCarrier->edict->v.maxspeed;
	g_engfuncs.pfnSetClientMaxspeed(pCarrier->edict, 0);

	glinfo.bBombStartedPlanting = true;
	glinfo.flBombPlantTime = gpGlobals->time + 3;
	g_engfuncs.pfnEmitSound(pBombSite, CHAN_STATIC, C4_PLANT_START_SOUND, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
}

//Called as soon as the bomb is planted
void OnBombPlant(edict_t *pBombSite) {
	glinfo.flBombPlantTime = 0;
	glinfo.bBombExplodeThink = true;
	glinfo.flBombExplodeTime = gpGlobals->time + CVAR_GETDIRECT(mp_c4timer);
	glinfo.flRoundTimeEnd = glinfo.flBombExplodeTime; //Todo: is this right?
	glinfo.clBombPlanter = glinfo.clBombCarrier;
	glinfo.clBombCarrier = NULL;
	glinfo.flNextBeepSound = 0; //todo, put this in a proper location
	glinfo.bPlayedWarningSound = false; //todo, put this in a proper location
	client_t *pPlanter = (client_t*)glinfo.clBombPlanter;
	C_BasePlayer *pPlayer = C_BasePlayer::Instance(pPlanter->edict);
	g_engfuncs.pfnSetClientMaxspeed(pPlanter->edict, (glinfo.flPrevPlanterMaxSpeed ? glinfo.flPrevPlanterMaxSpeed : CVAR_GET_FLOAT("sv_maxspeed")) );
	SetCTFScore(pPlayer, pPlayer->ctf_score + 1);
	MAKE_VECTORS( pPlayer->edict()->v.v_angle );
	glinfo.pBomb->v.origin = pPlayer->edict()->v.origin + pPlayer->edict()->v.view_ofs + (gpGlobals->v_forward * (pPlayer->edict()->v.button & IN_DUCK ? 1 : 50));
	//glinfo.pBomb->v.effects &= ~EF_NODRAW;
	glinfo.pBomb->v.rendermode = kRenderNormal;
	glinfo.pBomb->v.body = 1;
	glinfo.pBomb->v.angles.x = 0;
	glinfo.pBomb->v.angles.y = pPlanter->edict->v.angles.y + 180;
	DROP_TO_FLOOR(glinfo.pBomb);
	EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, "defusal/bombpl.wav", VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
	//void* usefunc = GET_USE(C_BaseEntity::Instance(glinfo.pBomb));
	SET_USE(C_BaseEntity::Instance(glinfo.pBomb), BombUse );
}

//Called while the bomb is planted
void BombExplodeThink(void) {
	C_BaseEntity *pBombEnt = C_BaseEntity::Instance(glinfo.pBomb);
	if ( glinfo.bBombDefused ) {
		glinfo.bBombExplodeThink = false;
		glinfo.pBomb->v.body = 0;
	} else { 

		if ( !glinfo.bPlayedWarningSound && gpGlobals->time >= glinfo.flNextBeepSound ) {
			//Play the beep sound
			glinfo.flNextBeepSound = gpGlobals->time + 0.25f;
			g_engfuncs.pfnEmitSound(glinfo.pBomb, CHAN_STATIC, C4_BEEP_SOUND, VOL_NORM, 0.5f, 0, PITCH_NORM);
		}

		if ( !glinfo.bPlayedWarningSound && glinfo.flBombExplodeTime - gpGlobals->time <= 1.5 ) {
			//Play the warning sound just before the bomb is going to explode
			glinfo.bPlayedWarningSound = true;
			EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, C4_WARNING_SOUND, VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
		}

		if ( gpGlobals->time >= glinfo.flBombExplodeTime ) {
			//Bomb passed the timer, now explode
			OnBombExplode();
		}
	}
}

//Called as soon as the bomb explodes
void OnBombExplode(void) {
	Vector explodeorigin = glinfo.pBomb->v.origin;
	explodeorigin.z += 20;
	Create_TE_EXPLOSION((edict_t*)NULL, explodeorigin, glinfo.g_sModelIndexFireball, 40, 15, TE_EXPLFLAG_NONE);
	EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, C4_EXPLODE_SOUND, VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
	glinfo.bBombExplodeThink = false;
	glinfo.bBombExploded = true;
	glinfo.flBombExplodeTime = 0;
	glinfo.pBomb->v.body = 0;
	//glinfo.pBomb->v.effects |= EF_NODRAW;
	glinfo.pBomb->v.rendermode = kRenderTransColor;

	for ( int i = 0; i < gpGlobals->maxClients; i++ ) {
		client_t *pClient = GetClientPointerFromIndex(i);
		if ( pClient->active && pClient->connected && pClient->spawned && pClient->edict->v.health > 0 && !IsPlayerSpectating(pClient->edict) ) {
			float distancefrombomb = abs(pClient->edict->v.origin.Length2D() - glinfo.pBomb->v.origin.Length2D());
			if ( distancefrombomb < 1000 ) {
				//Give damage to players near the bomb
				C_BaseEntity *pPlayer = C_BaseEntity::Instance(pClient->edict);
				C_BaseEntity *pBombEnt = C_BaseEntity::Instance(glinfo.pBomb);
				pPlayer->TakeDamage(pBombEnt->pev, pBombEnt->pev, 75000 / distancefrombomb, DMG_BLAST);
			}
		}
	}
	SetCStrikeNextThink(1.5f);
}

//Give the bomb to a random person
void GiveBomb(void) {
	//glinfo.pBomb->v.effects |= EF_NODRAW;
	glinfo.pBomb->v.rendermode = kRenderTransColor;

	glinfo.pBomb->v.body = 0;
	char validplayers[32]; //indexes of all valid players
	int count = 0;

	for ( int i = 0; i < gpGlobals->maxClients; i ++ ) {
		client_t *pClient = GetClientPointerFromIndex(i);
		if ( pClient->active && pClient->connected && pClient->spawned ) {
			C_BasePlayer *pPlayer = C_BasePlayer::Instance(pClient->edict);
			if ( pPlayer->team == TEAM_PHOENIX && !IsPlayerSpectating(pClient->edict) ) { //Give bomb to Phoenix team only and only if they are not spectating
				validplayers[count] = i;
				count++;
			}
		}
	}
	if ( count ) {
		int randnumber = rand() % count;
		glinfo.clBombCarrier = (int*)GetClientPointerFromIndex(validplayers[randnumber]);
		NotifyBombCarrier();
	}
}


//Called if the bomb is currently being defused
void __stdcall BombThink ( void ) {
	if ( !glinfo.bBombDefused ) {
		C_BaseEntity *pBombEnt = C_BaseEntity::Instance(glinfo.pBomb);
		if ( gpGlobals->time - glinfo.flLastDefuseTime >= 1.0f ) {
			//Player stopped defusing
			client_t *pDefuser = (client_t*)glinfo.clBombDefuser;
			if ( pDefuser && pDefuser->active && pDefuser->connected ) {
				//Allow player to move again
				if ( glinfo.flPrevDefuserMaxSpeed )
					g_engfuncs.pfnSetClientMaxspeed(pDefuser->edict, glinfo.flPrevDefuserMaxSpeed);
				else
					g_engfuncs.pfnSetClientMaxspeed(pDefuser->edict, CVAR_GET_FLOAT("sv_maxspeed"));
			}
			glinfo.flBombDefuseTime = 0;
			glinfo.flLastDefuseTime = 0;
			glinfo.clBombDefuser = NULL;
			SET_THINK(pBombEnt, NULL);
		} else {
			SET_NEXT_THINK(pBombEnt, gpGlobals->time + 0.1f);
		}
	}
}

//Called when someone presses the use key on the bomb
void __stdcall BombUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) {
	client_t *pClient = EDICT_TO_CLIENT(pActivator->edict());
	if ( pClient && pClient->active && pClient->connected ) {
		C_BasePlayer *pPlayer = (C_BasePlayer*)pActivator;
		C_BaseEntity *pBombEnt = C_BaseEntity::Instance(glinfo.pBomb);
		if ( pPlayer->team == TEAM_MI6 && !IsPlayerSpectating(pClient->edict)) {
			if ( !glinfo.clBombDefuser ) {
				//First defusal attempt
				glinfo.flPrevDefuserMaxSpeed = pPlayer->edict()->v.maxspeed;
				g_engfuncs.pfnSetClientMaxspeed(pPlayer->edict(), 0); //Disallow movement during defuse
				glinfo.clBombDefuser = (int*)EDICT_TO_CLIENT(pPlayer->edict());
				glinfo.flBombDefuseTime = gpGlobals->time + 10; //Todo: if player has defuse kit, it's 5 seconds
				glinfo.flLastDefuseTime = gpGlobals->time;
				SET_THINK(pBombEnt, BombThink);
				SET_NEXT_THINK(pBombEnt, gpGlobals->time);
				g_engfuncs.pfnEmitSound(glinfo.pBomb, CHAN_STATIC, C4_DEFUSE_START_SOUND, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			} else {
				if ( (int*)EDICT_TO_CLIENT(pPlayer->edict()) == glinfo.clBombDefuser ) {
					//Player using bomb is the same as the defuser, continue defusing
					glinfo.flLastDefuseTime = gpGlobals->time;
					char progress[36];
					float timeremaining = glinfo.flBombDefuseTime - gpGlobals->time;
					if ( timeremaining < 0 )
						timeremaining = 0;
					sprintf(progress, "BOMB DEFUSE TIME REMAINING: %.0f", timeremaining );
					MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "HudText", NULL), 0, pClient->edict);
							WRITE_STRING( progress );
					MESSAGE_END();
					if ( gpGlobals->time >= glinfo.flBombDefuseTime ) {
						//Bomb was defused, end the round
						glinfo.flBombDefuseTime = 0;
						glinfo.flLastDefuseTime = 0;
						glinfo.flRoundTimeEnd = gpGlobals->time;
						SET_THINK(pBombEnt, SUB_DoNothing);
						SET_USE(pBombEnt, NULL);
						glinfo.bBombDefused = true;
						glinfo.pBomb->v.body = 0;
						EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, "defusal/bombdef.wav", VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
						SetCStrikeNextThink(1.5f);
						//Give money for defusing? Score?
					} else if ( gpGlobals->time >= glinfo.flRoundTimeEnd ) {
						//Not enough time to defuse, round ended
						glinfo.flBombDefuseTime = 0;
						glinfo.flLastDefuseTime = 0;
						glinfo.clBombDefuser = NULL;
						//glinfo.pBomb->v.effects |= EF_NODRAW;
						glinfo.pBomb->v.rendermode = kRenderTransColor;
						SET_USE(pBombEnt, NULL);
						SET_THINK(pBombEnt, NULL);
					}
				}
			}
		}
	}
}

//Called when someone touches the bomb
void __stdcall BombTouch(C_BaseEntity *pOther) {
	if ( !glinfo.clBombCarrier ) {
		C_BasePlayer *pPlayer = (C_BasePlayer*)pOther;
		if ( pPlayer->team == TEAM_PHOENIX ) {
			//Phoenix player touched the bomb, now pick it up!
			g_engfuncs.pfnEmitSound(glinfo.pBomb, CHAN_STATIC, C4_PICKUP_SOUND, VOL_NORM, ATTN_IDLE, 0, PITCH_NORM);
			DeleteBomb(); //Deleting and recreating the bomb fixes numerous problems that I gave up trying to fix involving SET_USE and SET_THINK
			CreateBomb();
			//SET_TOUCH(C_BaseEntity::Instance(glinfo.pBomb), NULL);
			//glinfo.pBomb->v.effects |= EF_NODRAW;
			//glinfo.pBomb->v.rendermode = kRenderTransColor;
			glinfo.clBombCarrier = (int*)EDICT_TO_CLIENT(pOther->pev->pContainingEntity);
		}
	}
}

//Spawns the bomb
//SpawnBomb
void CreateBomb(void) {
	PRECACHE_MODEL("models/c4_explosives.mdl");
	glinfo.pBomb = CREATE_NAMED_ENTITY(MAKE_STRING("func_button"));
	C_BaseEntity *pBombEnt = C_BaseEntity::Instance(glinfo.pBomb);
	glinfo.pBomb->v.spawnflags = 1;
	pBombEnt->Spawn();
	SET_USE(C_BaseEntity::Instance(glinfo.pBomb), NULL);
	glinfo.pBomb->v.targetname = MAKE_STRING("C4 BOMB");
	SET_MODEL(glinfo.pBomb, "models/c4_explosives.mdl");
	//glinfo.pBomb->v.effects |= EF_NODRAW;
	glinfo.pBomb->v.rendermode = kRenderTransColor;
	glinfo.pBomb->v.body = 0;
	glinfo.pBomb->v.solid = SOLID_TRIGGER;
	glinfo.pBomb->v.movetype = MOVETYPE_TOSS;
	//glinfo.pBomb->v.angles = Vector(90, 0, 0);
	Vector mins = Vector(-5.590000 * 5, -4.100000 * 5, -0.060000);
	Vector maxs = Vector(5.460000 * 5, 7.970000 * 5, 3.610000 * 8);
	UTIL_SetSize(glinfo.pBomb, mins, maxs);
	//glinfo.pBomb->v.sequence = 1;
}

//Deletes the bomb from the world
void DeleteBomb(void) {
	if ( glinfo.pBomb ) {
		REMOVE_ENTITY(glinfo.pBomb);
		glinfo.pBomb = NULL;
	}
}

//Called when the bomb will be set to touchable
void __stdcall DropBombEndThink(void) {
	if ( gpGlobals->time < glinfo.flRoundTimeEnd ) {
		C_BaseEntity *pBombEnt = C_BaseEntity::Instance(glinfo.pBomb);
		SET_TOUCH(pBombEnt, BombTouch);
		SET_THINK(pBombEnt, NULL);
	}
}

//Drops the bomb where the carrier is looking
void DropBomb(void) {
	client_t *pBombCarrier = (client_t*)glinfo.clBombCarrier;
	if ( pBombCarrier ) {
		Vector eyeangles = ( pBombCarrier->edict->v.v_angle );
		MAKE_VECTORS( eyeangles );
		Vector vecAiming = gpGlobals->v_forward;
		//glinfo.pBomb->v.effects &= ~EF_NODRAW;
		glinfo.pBomb->v.rendermode = kRenderNormal;
		glinfo.pBomb->v.body = 0;
		glinfo.pBomb->v.origin = pBombCarrier->edict->v.origin + pBombCarrier->edict->v.view_ofs;
		glinfo.pBomb->v.velocity = vecAiming * 500 + pBombCarrier->edict->v.velocity;
		glinfo.pBomb->v.angles.y = pBombCarrier->edict->v.angles.y + 180;
		glinfo.clBombCarrier = NULL;
		C_BaseEntity *pBombEnt = C_BaseEntity::Instance(glinfo.pBomb);
		SET_THINK(pBombEnt, DropBombEndThink);
		SET_NEXT_THINK(pBombEnt->pev, gpGlobals->time + 1);
		g_engfuncs.pfnEmitSound(glinfo.pBomb, CHAN_STATIC, C4_DROP_SOUND, VOL_NORM, ATTN_IDLE, 0, PITCH_NORM);
	} else {
		SERVER_PRINT("ERROR: Tried to drop the bomb with no carrier!\n");
		SERVER_PRINT("ERROR: Tried to drop the bomb with no carrier!\n");
		SERVER_PRINT("ERROR: Tried to drop the bomb with no carrier!\n");
		SERVER_PRINT("ERROR: Tried to drop the bomb with no carrier!\n");
	}
}

//Tells the bomb carrier that they are holding the bomb
void NotifyBombCarrier(void) {
	client_t *pClient = (client_t*)glinfo.clBombCarrier;
	UTIL_HudText(pClient->edict, "YOU ARE CARRYING THE BOMB! PRESS E ON A BOMB SITE TO PLANT");
	//glinfo.pBomb->v.effects &= ~EF_NODRAW;
	//glinfo.pBomb->v.movetype = MOVETYPE_FOLLOW;
	//glinfo.pBomb->v.aiment = pClient->edict;
	//glinfo.pBomb->v.solid = SOLID_TRIGGER;
}

//Freezes players
void FreezePlayers(void) {
	for ( int i = 1; i <= gpGlobals->maxClients; i++ ) {
		client_t *pClient = GetClientPointerFromIndex(i - 1);
		if ( pClient->active && pClient->connected ) {
			C_BasePlayer *pPlayer = C_BasePlayer::Instance(pClient->edict);
			pPlayer->pev->flags |= FL_FROZEN;
		}
	}
}

//Unfreezes players
void UnFreezePlayers(void) {
	for ( int i = 1; i <= gpGlobals->maxClients; i++ ) {
		client_t *pClient = GetClientPointerFromIndex(i - 1);
		if ( pClient->active && pClient->connected ) {
			C_BasePlayer *pPlayer = C_BasePlayer::Instance(pClient->edict);
			pPlayer->pev->flags &= ~FL_FROZEN;
		}
	}
}

//Respawns all the players in the server
void RespawnPlayers(void) {
	glinfo.iNumPlayersAliveMI6 = 0;
	glinfo.iNumPlayersAlivePhoenix = 0;
	for ( int i = 0; i < gpGlobals->maxClients; i++ ) {
		client_t *pClient = GetClientPointerFromIndex(i);
		if ( pClient->active && pClient->connected && pClient->spawned ) {
			C_BasePlayer *pPlayer = C_BasePlayer::Instance(pClient->edict);
			int index = ENTINDEX(pClient->edict);
			if ( !IsPlayerSpectating(pClient->edict) || (IsPlayerSpectating(pClient->edict) && plinfo[index].bCodeInitiatedSpectate ) ) {
				//Respawn if the player is not spectating, or if the player was told to spectate by the game mode
				plinfo[index].bCodeInitiatedSpectate = false;
				plinfo[index].bAwaitingSpectate = false;
				plinfo[index].flTimeBeginSpectate = 0;
				pClient->edict->v.effects &= ~EF_NODRAW;
				pClient->edict->v.flags &= ~FL_FROZEN;
				pClient->edict->v.takedamage = DAMAGE_AIM;
				if ( IsPlayerSpectating(pClient->edict ) ) {
					//Tell client to exit spectate, then spawn them later
					CLIENT_COMMAND(pClient->edict, "codeexitspectate\n"); //disable spectate
					pClient->edict->v.movetype = MOVETYPE_WALK;
					pClient->edict->v.gravity = 1.0f;
					pClient->edict->v.solid = SOLID_SLIDEBOX;
					pClient->edict->v.rendermode = kRenderNormal;
					pClient->edict->v.renderamt = 0;
					pClient->edict->v.renderfx = kRenderFxNone;
					MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "ToggleHud", NULL), 0, pClient->edict);
						WRITE_BYTE(1);
					MESSAGE_END();
					glinfo.iNumPlayersAwaitingSpawn++;
					plinfo[index].flTimeToSpawn = gpGlobals->time + 1.0f;
				} else {
					//Spawn player immediately if they are not spectating
						//glinfo.pBomb->v.effects |= EF_NODRAW;
					pClient->edict->v.rendermode = kRenderNormal;
					pClient->edict->v.effects &= ~EF_NODRAW;
					pPlayer->Spawn();
				}
			}
		}
	}
}

//Draws the game statistics in the top left of the screen
void DrawClock(void) {
	if ( gpGlobals->time >= glinfo.flNextTimeDrawClock ) {
		glinfo.flNextTimeDrawClock = gpGlobals->time + 0.25;
		//Draw Clock
		int minutes = ((int)floor(glinfo.flClockSeconds) / 60) % 60;
		int seconds = (int)floor(glinfo.flClockSeconds) % 60;
		char str[64];
		char str2[256];
		sprintf(str, "State: %s", !glinfo.bWarmupComplete ? "Warmup" : glinfo.bRoundEndThink ? "Post Round" : glinfo.bRoundPreStartThink ? "Pre Round" : glinfo.bGameEndThink ? "Post Game" : "Round In Progress" );
		client_t *pClient = (client_t*)glinfo.clBombCarrier;
		sprintf(str2, ( seconds < 10 ) ? "%s\nRound: %d\nTime Left: %d:0%d\nBomb Carrier: %s\n" : "%s\nRound: %d\nTime Left: %d:%d\nBomb Carrier: %s\n", str, glinfo.iRoundNumber, minutes, seconds, pClient ? pClient->name : "None");
		Bond_TextMsg((edict_t*)NULL, 1, str2, 0, 0, 0, 0);
		//printf("MI6 %i PHX %i\n", glinfo.iNumPlayersAliveMI6, glinfo.iNumPlayersAlivePhoenix );
	}
}

//Called during the time before the game will end
void GameEndThink(void) {
	float flTimeSinceGameEnd = gpGlobals->time - glinfo.flGameTimeEnd;
	float enddelay = CVAR_GETDIRECT(mp_round_restart_delay);
	glinfo.flClockSeconds = max(0, enddelay - flTimeSinceGameEnd);
	if ( flTimeSinceGameEnd >= enddelay ) {
		//Game is finished
		glinfo.bGameEndThink = false;
		glinfo.flGameTimeEnd = 0;
		glinfo.flNextThink = 0;
		glinfo.flNextTimeDrawClock = 0;
		GameEnd();
	}
}

//Called when the game will end
void GameEnd(void) {
	//Game just ended, called once
	//ToDo: Game End stuff
	OnServerDeactivate();
}

//Called during the period after a round has ended and before a new round is about to begin
void RoundEndThink(void) {
	float flTimeSinceRoundEnd = gpGlobals->time - glinfo.flRoundTimeEnd;
	float restartdelay = CVAR_GETDIRECT(mp_round_restart_delay);
	glinfo.flClockSeconds = max(0, restartdelay - flTimeSinceRoundEnd);
	if ( flTimeSinceRoundEnd >= restartdelay ) {
		//Round end finished, now start a new one
		glinfo.bRoundEndThink = false;
		glinfo.flRoundTimeEnd = 0;
		RoundPreStart();
	}
}

//Called as soon as the round ends
void RoundEnd(void) {
	//Round just ended, called once
	glinfo.bBombExplodeThink = false;
	if ( glinfo.bBombExploded ) {
		//Bomb exploded
		glinfo.iRoundsWonPhoenix++;
		client_t *pPlanter = (client_t*)glinfo.clBombPlanter;
		if ( pPlanter && pPlanter->connected ) {
			C_BasePlayer *pPlayer = C_BasePlayer::Instance(pPlanter->edict);
			SetCTFScore(pPlayer, pPlayer->ctf_score + 10);
		}
		SetCTFCaptures(TEAM_PHOENIX, GetCTFCaptures(TEAM_PHOENIX) + 1);
		EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, "ctf/phoenix_ctf.wav", VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
		//Give money
	} else if ( glinfo.bBombDefused ) {
		//Bomb was defused
		glinfo.iRoundsWonMI6++;
		client_t *pDefuser = (client_t*)glinfo.clBombDefuser;
		if ( pDefuser && pDefuser->connected ) {
			C_BasePlayer *pPlayer = C_BasePlayer::Instance(pDefuser->edict);
			SetCTFScore(pPlayer, pPlayer->ctf_score + 10);
		}
		SetCTFCaptures(TEAM_MI6, GetCTFCaptures(TEAM_MI6) + 1);
		EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, "ctf/mi6_ctf.wav", VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
		//Give money
	} else if ( glinfo.iNumPlayersAliveMI6 == 0 && glinfo.iNumPlayersAlivePhoenix == 0 ) {
		//Round Draw
		SetCTFCaptures(TEAM_MI6, GetCTFCaptures(TEAM_MI6) + 1);
		SetCTFCaptures(TEAM_PHOENIX, GetCTFCaptures(TEAM_PHOENIX) + 1);
		EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, "defusal/rounddraw.wav", VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
		//Give money
	} else {
		if ( glinfo.iNumPlayersAliveMI6 == 0 ) {
			//Phoenix wins
			//Give money to Phoenix
			SetCTFCaptures(TEAM_PHOENIX, GetCTFCaptures(TEAM_PHOENIX) + 1);
			EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, "ctf/phoenix_ctf.wav", VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
		} else {
			//MI6 Wins
			if ( glinfo.clBombPlanter ) {
				//Phoenix managed to plant the bomb, give them $800
			} 
			//Give Money to MI6
			SetCTFCaptures(TEAM_MI6, GetCTFCaptures(TEAM_MI6) + 1);
			EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, "ctf/mi6_ctf.wav", VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
		}
	}

	glinfo.iRoundNumber++;

	if ( glinfo.iRoundNumber > CVAR_GETDIRECT(mp_maxrounds) - 1 ) { // > because we start from 1
		glinfo.flGameTimeEnd = gpGlobals->time;
		glinfo.bGameEndThink = true;
		glinfo.flNextThink = 0;
		glinfo.flNextTimeDrawClock = 0;
	} else {
		glinfo.bRoundEndThink = true;
		glinfo.flNextThink = 0;
		glinfo.flNextTimeDrawClock = 0;
	}
	glinfo.clBombPlanter = NULL;
	glinfo.clBombDefuser = NULL;
	glinfo.bBombExploded = false;
	glinfo.bBombDefused = false;
}


//Called when the Pre-Round time begins
void RoundPreStart(void) {
	DeleteBomb(); //Deleting and recreating the bomb fixes numerous problems that I gave up trying to fix involving SET_USE and SET_THINK
	CreateBomb();
	glinfo.bAllowPlayerSpawn = true;
	glinfo.flNextThink = 0;
	glinfo.flNextTimeDrawClock = 0;
	glinfo.bRoundPreStartThink = true;
	glinfo.flRoundPreStartTimeEnd = gpGlobals->time + CVAR_GETDIRECT(mp_freezetime);
	glinfo.iBombSitePlantingAt = -1;
	RespawnPlayers();
	edict_t *pWeaponBox = FIND_ENTITY_BY_CLASSNAME(NULL, "weaponbag");
	while (!FNullEnt(pWeaponBox)) {
		REMOVE_ENTITY(pWeaponBox);
		pWeaponBox = FIND_ENTITY_BY_CLASSNAME(NULL, "weaponbag");
	}
	for (int i = 0; i < gpGlobals->maxClients; i++) {
		client_t *pClient = GET_CLIENT_POINTER(i);
		if (pClient->active && pClient->connected && pClient->spawned) {
			StripWeapons(pClient->edict, 0);
			edict_t *pPlayerEquip = FIND_ENTITY_BY_CLASSNAME(NULL, "game_player_equip");
			while (!FNullEnt(pPlayerEquip)) {
				C_BaseEntity *pEquip = C_BaseEntity::Instance(pPlayerEquip);
				pEquip->Touch((C_BaseEntity*)C_BasePlayer::Instance(pClient->edict));
				pPlayerEquip = FIND_ENTITY_BY_CLASSNAME(pPlayerEquip, "game_player_equip");
			}
		}
	}
	GiveBomb();
}

//Called in the time period before a round begins
void RoundPreStartThink(void) {
	glinfo.flClockSeconds = max(0, glinfo.flRoundPreStartTimeEnd - gpGlobals->time);
	FreezePlayers();
	if ( gpGlobals->time >= glinfo.flRoundPreStartTimeEnd ) {
		//Check to see if there is an empty team, if there is, reset the game
		int iNumPlayersMI6 = 0;
		int iNumPlayersPhoenix = 0;
		for ( int i = 0; i < gpGlobals->maxClients; i++ ) {
			client_t *pClient = GetClientPointerFromIndex(i);
			if ( pClient->active || pClient->connected || pClient->spawned ) {
				C_BasePlayer *pPlayer = C_BasePlayer::Instance(pClient->edict);
				if ( pPlayer ) {
					if ( pPlayer->team == TEAM_MI6 )
						iNumPlayersMI6++;
					else if ( pPlayer->team == TEAM_PHOENIX )
						iNumPlayersPhoenix++;
					else
						printf("Error: Found unknown player team in RoundPreStartThink(void)! Team: %i\n", pPlayer->team);
				}
			}
		}
		if ( iNumPlayersMI6 == 0 || iNumPlayersPhoenix == 0 ) {
			SayTextAll("No players on one team, resetting game mode");
			UnFreezePlayers();
			ResetRoundSystem();
		} else {
			//Validated all checks, start the round!
			glinfo.flNextThink = 0;
			glinfo.flNextTimeDrawClock = 0;
			glinfo.bRoundPreStartThink = false;
			glinfo.flRoundPreStartTimeEnd = 0;
			RoundStart();
		}
	}
}

//Called as soon as the round starts
void RoundStart(void) {
	UnFreezePlayers();
	if ( glinfo.iNumPlayersAliveMI6 == 0 || glinfo.iNumPlayersAlivePhoenix == 0 ) {
		//all players of one team left, reset the game
		ResetRoundSystem();
	} else { 
		//Start the round
		glinfo.flRoundTimeEnd = gpGlobals->time + (CVAR_GETDIRECT(mp_roundtime) * 60);
		glinfo.bRoundThink = true;
		glinfo.bAllowPlayerSpawn = false;
		EMIT_AMBIENT_SOUND(glinfo.pBomb, glinfo.pBomb->v.origin, "defusal/moveout.wav", VOL_NORM, ATTN_NONE, 0, PITCH_NORM);
	}
}

//Called during the round
void RoundThink(void) {
	BombPlantThink();

	if ( !glinfo.clBombCarrier && glinfo.pBomb->v.rendermode == kRenderTransColor ) {
		GiveBomb();
		glinfo.pBomb->v.rendermode = kRenderNormal; //Crappy fix for bomb disappearing. Todo: figure out why it does it
	}

	if ( glinfo.iNumPlayersAliveMI6 == 0 ) {
		//All MI6 is dead, the round will now end
		glinfo.flRoundTimeEnd = gpGlobals->time;
	} else {
		if ( glinfo.bBombExplodeThink ) {
			//Bomb is planted, think now
			BombExplodeThink();
		} else {
			if ( glinfo.iNumPlayersAlivePhoenix == 0 ) {
				//All Phoenix is dead and there is no bomb planted, the round will now end
				glinfo.flRoundTimeEnd = gpGlobals->time;
			}
		}
	}

	glinfo.flClockSeconds = max(0, glinfo.flRoundTimeEnd - gpGlobals->time);
	if ( gpGlobals->time >= glinfo.flRoundTimeEnd ) {
		glinfo.flNextThink = 0;
		glinfo.flNextTimeDrawClock = 0;
		glinfo.bRoundThink = false;
		RoundEnd();
	}
}

//Called when a client types a command in the console
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand) {
	if ( !strcmpi(szCommand, "dropbomb") ) {
		client_t *pBombCarrier = (client_t*)glinfo.clBombCarrier;
		if ( pBombCarrier && pBombCarrier->edict == pEntity ) {
			if ( !glinfo.bRoundPreStartThink ) //Bomb can only be dropped during a round
				if ( !glinfo.bBombStartedPlanting ) //Bomb cannot be dropped while it is being planted
				DropBomb();
		}
	}
}