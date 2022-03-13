#include "RequiredHooks.h"

globalhooks_s globalhooks_t;

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

DLL_GLOBAL const Vector g_vecZero = Vector(0, 0, 0);//{ 0, 0, 0 };

BYTE *retJMP;

BYTE* getHookFuncJMPAddr(DWORD fnName) {
	for (int i = 0; i<sizeof(funcHooks) / sizeof(HookCallInfo); i++) {
		if (funcHooks[i].retFunc == fnName) {
			return (BYTE*)funcHooks[i].retJMPAddr;
		}
	}
	printf("Error: One or more of the provided function addresses weren't found\n");
	return 0;
}

//faster method of the above
BYTE* getHookFuncJMPAddr(ADD_HOOK_TYPES hooktype) {
	return (BYTE*)funcHooks[hooktype].retJMPAddr;
}

/*
void HookFunctions() {
	//Hooks all functions
	for (int i = 0; i<sizeof(funcHooks) / sizeof(HookCallInfo); i++) {
		PlaceJMP((BYTE*)funcHooks[i].address, funcHooks[i].retFunc, funcHooks[i].len);
	}
}
*/
edict_t* callOnFindEntityInSphere(edict_t *pEdictStartSearchAfter, const Vector &org, float rad) {
	int e = pEdictStartSearchAfter ? NUM_FOR_EDICT(pEdictStartSearchAfter) : 0;
	//int blah = (int)&g_psv->num_edicts;
	////44A7B99C
	for (int i = e + 1; i < g_psv->num_edicts; i++)
	{
		edict_t* ent = &g_psv->edicts[i];
		if (ent->free || !ent->v.classname)
			continue;

		if (i <= g_psvs->maxclients && !g_psvs->clients[i - 1].active)
			continue;

		float distSquared = 0.0;
		for (int j = 0; j < 3 && distSquared <= (rad * rad); j++)
		{
			float eorg;
			if (org[j] >= ent->v.absmin[j])
				eorg = (org[j] <= ent->v.absmax[j]) ? 0.0f : org[j] - ent->v.absmax[j];
			else
				eorg = org[j] - ent->v.absmin[j];
			distSquared = eorg * eorg + distSquared;
		}

		if (distSquared <= ((rad * rad))) {
			C_BaseEntityCustom *pBaseEnt = C_BaseEntityCustom::Instance(ent);
			if (pBaseEnt)
				return ent;
			else {
				printf("ERROR: FindEntityInSphere (required_hooks.cpp): Found invalid entity %s!\n", ent->v.classname ? STRING(ent->v.classname) : "");
			}
		}
	}

	return &g_psv->edicts[0]; //NULL
}
const Vector* tempVector = 0;
edict_t *tempEdict;
__declspec(naked) edict_t* hookOnFindEntityInSphere(edict_t *pEdictStartSearchAfter, const Vector &org, float rad) {
	__asm {
		jmp callOnFindEntityInSphere
	}
}

__declspec(naked) void hookOnHudInit() {
	__asm {
		push ebp //create stack frame
		mov ebp, esp
		sub esp, 0x14
		mov dword ptr ss : [esp + 4], eax
		mov dword ptr ss : [esp + 8], ecx
		mov dword ptr ss : [esp + 0xC], edx
		mov dword ptr ss : [esp + 0x10], edi
		mov edi, dword ptr ds : [edi + 0x4]   //EDICT_NUM
		mov edi, dword ptr ds : [edi + 0x204] //EDICT_NUM
		mov dword ptr ss:[esp], edi
		call dword ptr ds:[callOnHudInit]
	}
	//callOnHudInit((edict_t*)tempRegister);
	retJMP = getHookFuncJMPAddr(ADD_HOOK_TYPES::H_HUDINIT);
	__asm {
		mov eax, dword ptr ss : [esp + 4]
		mov ecx, dword ptr ss : [esp + 8]
		mov edx, dword ptr ss : [esp + 0xC]
		mov edi, dword ptr ss : [esp + 0x10]
		mov esp, ebp
		pop ebp

		MOV ECX, DWORD PTR DS : [0x4217CA9C]
		jmp retJMP //jump to original code
	}
}

void callOnHudInit(edict_t *pEntity) {
	for (int i = 1; i <= globalhooks_t.hudinit_t->numhooks; i++) {
		((void(*)(void))globalhooks_t.hudinit_t->hooks[i])(); //call the original function
	}
}

void* killedcall = (void*)0x4200BF80;
__declspec(naked) void hookOnPlayerKilled() {
	__asm {
		//semi optimized
		/*
		sub esp, 0x1C
		mov eax, dword ptr ss:[esp + 0x20]
		mov dword ptr ss:[esp], EAX //Arg1
		mov eax, dword ptr ss:[esp + 0x24]
		mov dword ptr ss:[esp + 4], EAX //Arg2
		mov dword ptr ss:[esp + 8], ECX //Arg3
		mov dword ptr ss:[esp + 0xC], ECX
		mov dword ptr ss:[esp + 0x10], EBX
		mov dword ptr ss:[esps+ 0x14], ESI
		call dword ptr ds:[callOnPlayerKilled]
		*/

		//optimized
		/*
		sub esp, 0x18
		mov eax, dword ptr ss:[esp + 0x1C]
		mov dword ptr ss:[esp], eax //Arg1
		mov eax, dword ptr ss:[esp + 0x20]
		mov dword ptr ss:[esp + 4], eax //Arg2
		mov dword ptr ss:[esp + 8], ecx //Arg3, also Save
		mov dword ptr ss:[esp + 0xC], ebx //Save
		mov dword ptr ss:[esp + 0x10], esi //Save
		call callOnPlayerKilled
		*/
		sub esp, 0x18
		  mov eax, dword ptr ss:[esp + 0x1C]
		  mov dword ptr ss:[esp + 8], ecx //Arg3, also Save
		  mov dword ptr ss:[esp], eax //Arg1
		  mov dword ptr ss:[esp + 0x10], esi //Save
		  mov eax, dword ptr ss:[esp + 0x20]
		  mov dword ptr ss:[esp + 0xC], ebx //Save
		  mov dword ptr ss:[esp + 4], eax //Arg2
		  call callOnPlayerKilled
		
		//add esp, 0x18
		//shit code below
		//mov tempECX, ecx
		//mov tempEBX, ebx
		//mov tempESI, esi
		//mov eax, dword ptr ss:[esp + 4]
		//mov tempRegister, eax
		//mov eax, dword ptr ss:[esp + 8]
		//mov tempRegister2, eax
		//mov tempRegister3, ecx
	}
	retJMP = getHookFuncJMPAddr(ADD_HOOK_TYPES::H_PLAYERKILLED);//getHookFuncJMPAddr((DWORD)hookOnPlayerKilled);
	//callOnPlayerKilled((entvars_t *)tempRegister, (int)tempRegister2, (C_BasePlayer *)tempRegister3);

	__asm {
		mov ecx, dword ptr ss : [esp + 8]
		mov ebx, dword ptr ss : [esp + 0xC]
		mov esi, dword ptr ss : [esp + 0x10]
		add esp, 0x18
		//mov ecx, tempECX
		//mov ebx, tempEBX
		//mov esi, tempESI
		//restore intercepted code below
		PUSH EBX
		PUSH ESI
		MOV ESI, ECX
		call killedcall

		jmp retJMP
	}
}

void callOnPlayerKilled(entvars_t *pevAttacker, int iGib, C_BasePlayer *pVictim) {
	for (int i = 1; i <= globalhooks_t.playerkilled_t->numhooks; i++) {
		((void(*)(entvars_t *, int, C_BasePlayer *))globalhooks_t.playerkilled_t->hooks[i])(pevAttacker, iGib, pVictim); //call the function
	}
}

//extensiondll_t g_rgextdll[50];

__declspec(naked) void hookOnGetEntityInit() {
	__asm {
		mov tempEBX, ebx
		mov tempESI, esi
		mov tempEDI, edi
		mov tempECX, ecx
		mov tempEDX, edx
		push dword ptr ss:[esp + 4]
		call callOnGetEntityInit
		add esp, 4
	}
	//callOnGetEntityInit((LPCSTR)tempRegister);
	__asm {
		test eax, eax
			je CustomEntityNotFound
			//Custom entity found! Just return so the game doesn't think it's unknown
			mov ebx, tempEBX
			mov esi, tempESI
			mov edi, tempEDI
			mov ecx, tempECX
			mov edx, tempEDX
			retn
	}
CustomEntityNotFound:
	retJMP = getHookFuncJMPAddr(ADD_HOOK_TYPES::H_ENTITYINIT);
	__asm {
		mov ecx, tempECX
		mov ebx, tempEBX
		mov esi, tempESI
		mov edi, tempEDI
		mov edx, tempEDX
		//restore intercepted code
		MOV EAX,DWORD PTR DS:[0x44B7B1B8]
		jmp retJMP
	}
}

FARPROC callOnGetEntityInit(LPCSTR lpProcName) {
	FARPROC pDispatch;
	extensiondll_t *g_rgextdll = (extensiondll_t*)0x44B7A7A8;
	int i;
	for (i = 0; i < *(int*)0x44B7B1B8; i++)
	{
		pDispatch = GetProcAddress((HMODULE)g_rgextdll[i].lDLLHandle, lpProcName);
		if (pDispatch)
		{
			return pDispatch;
		}
	}

	FARPROC entityinit = NULL;

	for (int i = 1; i <= globalhooks_t.entityinit_t->numhooks; i++) {
		entityinit = ((FARPROC(*)(LPCSTR))globalhooks_t.entityinit_t->hooks[i])(lpProcName);
		if (entityinit)
			break;
	}

	return entityinit;
}

float tempFloatX, tempFloatY, tempFloatZ;
__declspec(naked) void hookOnSpawnBlood() {
	__asm jmp callOnSpawnBlood
#if 0
	__asm {
		sub esp, 0x18
		push dword ptr ss : [esp + 0x2C]
		push dword ptr ss : [esp + 0x2C]
		push dword ptr ss : [esp + 0x2C]
		push dword ptr ss : [esp + 0x2C]
		push dword ptr ss : [esp + 0x2C]
		call callOnSpawnBlood
		add esp, 0x2C
		retn
	}
#endif
		//shit code below
#if 0
		mov eax, dword ptr ss : [esp + 4]
		mov tempFloatX, eax
		mov ecx, dword ptr ss : [esp + 8]
		mov tempFloatY, ecx
		mov eax, dword ptr ss : [esp + 0xc]
		mov tempFloatZ, eax
		mov ecx, dword ptr ss : [esp + 0x10]
		mov tempECX, ecx
		mov eax, dword ptr ss : [esp + 0x14]
		mov tempFloat, eax
			
	}
	callOnSpawnBlood(tempFloatX, tempFloatY, tempFloatZ, tempECX, tempFloat);
	__asm{
		retn
	}
#endif
}

void callOnSpawnBlood(float X, float Y, float Z, int bloodColor, float flDamage) {
	for (int i = 1; i <= globalhooks_t.spawnblood_t->numhooks; i++) {
		((void(*)(float, float, float, int, float))globalhooks_t.spawnblood_t->hooks[i])(X, Y, Z, bloodColor, flDamage); //call the function
	}
}


float tempFloat1 = 0;
float tempFloat2 = 0;
float tempFloat3 = 0;
float tempFloat4 = 0;
float tempFloat5 = 0;
float tempFloat6 = 0;
__declspec (naked) void hookOnFireBullets(void) {
	//New code
	__asm {
		sub esp, 0x11c
			mov dword ptr ss : [esp], eax
			mov dword ptr ss : [esp + 4], ebx
			mov dword ptr ss : [esp + 8], ecx
			mov dword ptr ss : [esp + 0xC], edx
			mov ebx, dword ptr ds : [ecx + 4]
			mov ebx, dword ptr ds : [ebx + 0x204]
			mov dword ptr ds : [tempRegister], ebx //edict

			mov eax, dword ptr ss : [esp + 0x128]
			mov dword ptr ds : [tempFloat1], eax

			mov eax, dword ptr ss : [esp + 0x12c]
			mov dword ptr ds : [tempFloat2], eax

			mov eax, dword ptr ss : [esp + 0x130]
			mov dword ptr ds : [tempFloat3], eax

			mov eax, dword ptr ss : [esp + 0x134]
			mov dword ptr ds : [tempFloat4], eax

			mov eax, dword ptr ss : [esp + 0x138]
			mov dword ptr ds : [tempFloat5], eax

			mov eax, dword ptr ss : [esp + 0x13C]
			mov dword ptr ds : [tempFloat6], eax
	}
	callOnFireBullets((edict_t*)tempRegister, Vector(tempFloat1, tempFloat2, tempFloat3), Vector(tempFloat4, tempFloat6, 0), tempFloat5);
	retJMP = getHookFuncJMPAddr(ADD_HOOK_TYPES::H_FIREBULLETS);
	__asm {
		mov eax, dword ptr ss : [esp]
			mov ebx, dword ptr ss : [esp + 4]
			mov ecx, dword ptr ss : [esp + 8]
			mov edx, dword ptr ss : [esp + 0xC]
			jmp retJMP
	}
}

void callOnFireBullets(edict_t *edict, Vector vecSrc, Vector vecShootDir, float dist) {
	for (int i = 1; i <= globalhooks_t.firebullets_t->numhooks; i++) {
		((void(*)(edict_t *, Vector, Vector, float))globalhooks_t.firebullets_t->hooks[i])(edict, vecSrc, vecShootDir, dist); //call the function
	}
}

__declspec(naked) void hookOnBasePlayerSpawn() {
	__asm {
		push ebx
			push ecx
			push esi
			mov tempRegister, ecx
	}

	retJMP = getHookFuncJMPAddr(ADD_HOOK_TYPES::H_BASEPLAYERSPAWN);
	callOnBasePlayerSpawn((C_BasePlayer*)tempRegister);

	__asm {
		pop esi
			pop ecx
			pop ebx
			test eax, eax
			jne ContinueSpawn
			retn //Don't allow a spawn

		ContinueSpawn :
		//restore intercepted code
		sub esp, 0x24
			push ebx
			push ebp

			jmp retJMP
	}
}

BOOL callOnBasePlayerSpawn(C_BasePlayer *pPlayer) {
	for (int i = 1; i <= globalhooks_t.baseplayerspawn_t->numhooks; i++) {
		if (((BOOL(*)(C_BasePlayer *))globalhooks_t.baseplayerspawn_t->hooks[i])(pPlayer) == FALSE) //call the function
			return FALSE; //The function that tells us we should not spawn takes priority over every other one
	}
	return TRUE;
}

BOOL __stdcall hookOnFPlayerCanRespawn(C_BasePlayer *pPlayer) {
	for (int i = 1; i <= globalhooks_t.fplayercanrespawn_t->numhooks; i++) {
		if (((BOOL(*)(C_BasePlayer *))globalhooks_t.fplayercanrespawn_t->hooks[i])(pPlayer) == FALSE) //call the function
			return FALSE; //The function that tells us we should not spawn takes priority over every other one
	}
	return TRUE;

	/*
	__asm {
		mov tempEAX, eax
			mov eax, dword ptr ss : [esp + 4]
			mov tempRegister, eax
	}
	callOnFPlayerCanRespawn((C_BasePlayer*)tempRegister);
	__asm {
		RETN 4
	}
	*/
}

__declspec(naked) void hookOnPlayerTakeDamage() {
	__asm {
		mov tempECX, ecx
			mov tempEBX, ebx
			mov tempESI, esi
			mov eax, dword ptr ss : [esp + 4]
			mov tempRegister, eax
			mov eax, dword ptr ss : [esp + 8]
			mov tempRegister2, eax
			mov eax, dword ptr ss : [esp + 0xC]
			mov tempFloat, eax
			mov eax, dword ptr ss : [esp + 0x10]
			mov tempRegister3, eax
	}

	retJMP = getHookFuncJMPAddr(ADD_HOOK_TYPES::H_BASEPLAYERTAKEDAMAGE);
	callOnPlayerTakeDamage((entvars_t *)tempRegister, (entvars_t *)tempRegister2, tempFloat, (int)tempRegister3);

	__asm {
		mov ecx, tempECX
			mov eax, tempEAX
			mov esi, tempESI
			//restore intercepted code below
			SUB ESP, 0x10
			PUSH EBX
			MOV EBX, DWORD PTR SS : [ESP + 0x24]

			jmp retJMP
	}
}

void callOnPlayerTakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) {
	for (int i = 1; i <= globalhooks_t.playertakedamage_t->numhooks; i++) {
		((void(*)(entvars_t *, entvars_t *, float, int))globalhooks_t.playertakedamage_t->hooks[i])(pevInflictor, pevAttacker, flDamage, bitsDamageType); //call the function
	}
}

void hookOnCL_PM_Init(struct playermove_s *ppmove)
{

}

void hookOnCL_PM_Move(struct playermove_s *ppmove, int server)
{

}


//typedef void(__cdecl *ADD_HOOK_FUNC)(void*, int);

/*UTIL_DLLEXPORT*/C_DLLEXPORT BOOLEAN __cdecl ADD_HOOK(void* address, ADD_HOOK_TYPES hooktype) {
#if 0
	//hook and allocate everything at once. it's better to allocate only what's needed instead so don't use this unless necessary
	HookFunctions();
	if (!globalhooks_t.entityinit_t) {
		for (hook_t** offset = (hook_t**)&globalhooks_t; (intptr_t)offset < (intptr_t)&globalhooks_t + sizeof(globalhooks_t); offset = (hook_t**)((intptr_t)offset + sizeof(hook_t*))) {
			//hook_t** tableoffset = (hook_t**)offset;
			*offset = (hook_t*)malloc(sizeof(hook_t));
			//(*tableoffset)->numhooks = 0;
			memset(*offset, 0, sizeof(hook_t));
		}
	}
#endif

	hook_t** offset = (hook_t**)((intptr_t)&globalhooks_t + (hooktype * sizeof(hook_t*)));
	hook_t* hookadr = (hook_t*)(*offset);
	if (hookadr == NULL) {
		//if this hook hasn't been allocated yet, then intercept it and then allocate our struct
		//int funchookoffset = hooktype == 0 ? hooktype : (hooktype / sizeof(hook_t*)) - 1;

		if (hooktype < 0 || hooktype >(sizeof(funcHooks) / sizeof(HookCallInfo))) {
			printf("ERROR: ADD_HOOK: invalid hooktype %i!\n", hooktype);
			return FALSE;
		}

		PlaceJMP((BYTE*)funcHooks[hooktype].address, funcHooks[hooktype].retFunc, funcHooks[hooktype].len);
		*offset = (hook_t*)malloc(sizeof(hook_t));
		memset(*offset, 0, sizeof(hook_t));
		hookadr = (hook_t*)(*offset);
	}
	hookadr->numhooks++;
	if (hookadr->numhooks > MAX_HOOKS) {
		printf("ERROR: required_dll_hooks_amxx.dll: too many hooks for hooktype %i, address: %i!\n", hooktype, (int)address);
		return FALSE;
	}
	hookadr->hooks[hookadr->numhooks] = address;
	return TRUE;
}

void OnModuleInitialize(void) {
	//ADD_DLL_HOOK(&OnServerDeactivate, ADD_HOOK_TYPES::H_FINDENTITYINSPHERE);
}

void OnServerDeactivate(void) {
	/*
	for (hook_t** offset = (hook_t**)&globalhooks_t; (intptr_t)offset < (intptr_t)&globalhooks_t + sizeof(globalhooks_t); offset = (hook_t**)((intptr_t)offset + sizeof(hook_t*))) {
		hook_t* hookadr = (hook_t*)(*offset);
		if (hookadr)
		{
			free(*offset);
			*offset = NULL;
		}
	}
	*/
}

void OnServerActivate(void) {
}

void OnClientConnect(edict_t *pEntity) {
}

void OnClientDisconnect(edict_t *pEntity) {
}