//#include <CVector.h>
//#include "CString.h"
//#include "entity.h"
//#include <gpglobals.h>
//#include <const.h>
//#include <util.h>
//#include <cbase.h>
#include "globals.h"
#include "pattern_scanner.h"
#include "MinHook/MinHook.h"
#include <event_api.h>
#include "MetaHook.h"
#include <usercmd.h>
#include <cvardef.h>
#include <clientsideentity.h>
#include <NightfireFileSystem.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <platformdll.h>
//#include <enginefuncs.h>
#include <globalvars.h>
#include <server.h>
#include <nightfire_pointers.h>
#include "nightfire_hooks.h"

#include <pm_defs.h>

#include <studio.h>
#include <StudioModelRenderer.h>

float edict_yaw_times[4096] = { FLT_MIN };
void(__fastcall *g_oAI_ChangeYaw)(void*, void*, int);
// TODO: move this to AMX and do a proper fix for all entity types
void __fastcall AI_ChangeYaw(void* ent, void* edx, int speed)
{
	//hack... can't access these tables without header madness
	typedef int(*pIndexOfEdictFn)(struct edict_s*);
	typedef edict_s* (*pFindEntityByVarsFn)(struct entvars_s*);
	//#define pentityofentindex 0x4306FC90
	//pIndexOfEdictFn pIndexOfEdict = (pIndexOfEdictFn)0x4306FC40;//*(DWORD*)((DWORD)g_pEngineFuncs + 72);
	//pFindEntityByVarsFn pFindEntityByVars = (pFindEntityByVarsFn)0x4306FD00; //*(DWORD*)((DWORD)g_pEngineFuncs + 20);
	struct entvars_s* entvars = *(entvars_s**)((DWORD)ent + 4);
	float* globalvars2 = (float*)0x4217A168;
	float frametime = globalvars2[1];

	edict_t* ed = (edict_t*)g_pEngineFuncs->pfnFindEntityByVars(entvars);
	int entindex = g_pEngineFuncs->pfnIndexOfEdict(g_pEngineFuncs->pfnFindEntityByVars(entvars));
	if (entindex < 0 || entindex >= ARRAYSIZE(edict_yaw_times))
		return g_oAI_ChangeYaw(ent, edx, speed);

	auto time = Sys_FloatTime();
	if (time < edict_yaw_times[entindex])
		return;

	globalvars2[1] = time - edict_yaw_times[entindex];

	g_oAI_ChangeYaw(ent, edx, speed);

	globalvars2[1] = frametime;
	edict_yaw_times[entindex] = time + 0.03125f;
}

void Fix_AI_TurnSpeed()
{
	static bool already_fixed = false;
	if (!already_fixed)
	{
		DWORD gamedll = (DWORD)GetModuleHandleA("game.dll");
		if (!gamedll)
			return;

		already_fixed = true;

		DWORD adr;
		if (!FindMemoryPattern(pattern_t(adr, gamedll, "90 90 90 90 90 90 83 EC 0C 56 8B F1 8B 46 04 8B 48 54 51", false, "AI_ChangeYaw", true)))
			return;

		adr += 6;

		if (!HookFunctionWithMinHook((void*)adr, AI_ChangeYaw, (void**)&g_oAI_ChangeYaw))
			return;
	}
}