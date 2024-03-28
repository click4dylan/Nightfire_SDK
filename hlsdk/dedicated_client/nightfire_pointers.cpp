#include "pattern_scanner.h"
#include "MinHook/MinHook.h"
#include <event_api.h>
#include "MetaHook.h"
#include <usercmd.h>
#include <cvardef.h>
#include <clientsideentity.h>
#include <NightfireFileSystem.h>
//#include <r_studioint.h>

#include <vector_types.h>
//#include <eiface.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <platformdll.h>
#include <enginefuncs.h>
#include <globalvars.h>
#include <server.h>
#include <nightfire_pointers.h>
#include "nightfire_hooks.h"
#include "pattern_scanner.h"

#include <pm_defs.h>

void nf_pointers::GetImportantEngineOffsets(long enginedll)
{
	if (!enginedll)
		return;

	if (!FindMemoryPattern(psvs, enginedll, "E8 ? ? ? ? A0 ? ? ? ? 84 C0 74 0E 68", false, true, 6, 0, false))
		return;

	if (!EV_PlayerTrace)
		FindMemoryPattern(EV_PlayerTrace, enginedll, "8B 44 24 14 8B 4C 24 10 8B 54 24 0C 83 EC 48", false);

	if (!EV_SetTraceHull)
		if (!FindMemoryPattern(EV_SetTraceHull, enginedll, "8B 44 24 04 8B 0D ? ? ? ? 89 81 ? 00 00 00 C3", false))
			return;
	
	if (!g_pCL_EngineFuncs)
	{
		DWORD adr;
		FindMemoryPattern(adr, enginedll, "68 ? ? ? ? FF 15 ? ? ? ? 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 10", false);
		if (adr)
			g_pCL_EngineFuncs = *(cl_enginefuncs_s**)((DWORD)adr + 1);
	}

	if (!g_PlayerMove)
		g_PlayerMove = *(playermove_t***)((DWORD)EV_SetTraceHull + 6);

	if (!svs_maxclients)
	{
		DWORD adr;
		FindMemoryPattern(adr, enginedll, "83 3D ? ? ? ? 01 7E 2E 8B 4C 24 0C 8D 44 24 10 50", false);
		if (adr)
			svs_maxclients = *(int**)(adr + 2);
	}

	if (!Log_Printf)
		FindMemoryPattern(Log_Printf, enginedll, "A0 ? ? ? ? 81 EC 04 08 00 00 84 C0", false);

	if (!ENG_GetPlayerInfo)
		FindMemoryPattern(ENG_GetPlayerInfo, enginedll, "8B 44 24 04 69 C0 B4 01 00 00 05 ? ? ? ? C3 55", false);

	if (!Host_IsSinglePlayerGame)
		FindMemoryPattern(Host_IsSinglePlayerGame, enginedll, "A0 ? ? ? ? 84 C0 8B 0D ? ? ? ? 74 06", false);

	if (!R_ChangeMPSkin)
		FindMemoryPattern(R_ChangeMPSkin, enginedll, "A1 ? ? ? ? 8B 08 8B 04 ? ? ? ? ? 85 C0", false);

	if (!g_rCurrentEntity)
	{
		DWORD adr;
		FindMemoryPattern(adr, enginedll, "A1 ? ? ? ? 55 8B 6C 24 08 57 8B FD 69 FF", false);
		if (adr)
		{
			g_rCurrentEntity = *(struct cl_entity_s***)((DWORD)adr + 1);
			DM_PlayerState = (char*)*(DWORD*)((DWORD)adr + 0x15);
		}
	}

	if (!g_psv)
	{
		DWORD adr;
		FindMemoryPattern(adr, enginedll, "68 ? ? ? ? FF 15 ? ? ? ? 8B 3D ? ? ? ? 6A 40", false);
		if (adr)
			g_psv = *(server_t***)(adr + 1);
	}

	if (!R_StudioSetupPlayerModel)
		FindMemoryPattern(R_StudioSetupPlayerModel, enginedll, "A1 ? ? ? ? 55 8B 6C 24 08 57 8B FD 69 FF", false);
}

void nf_pointers::GetImportantClientOffsets(long clientdll)
{
	if (!clientdll)
		return;

	if (!UTIL_CheckForWater)
		FindMemoryPattern(UTIL_CheckForWater, clientdll, "A1 ? ? ? ? 83 EC 48 6A 02", false);

	if (!g_ppStudioAPI)
	{
		if (FindMemoryPattern(g_ppStudioAPI, clientdll, "B9 ? ? ? ? E8 ? ? ? ? B8 01 00 00 00 5D C3", false))
			g_ppStudioAPI = (class CStudioModelRenderer**)((uintptr_t)g_ppStudioAPI + 1);
	}
	if (g_ppStudioAPI)
		g_pStudioAPI = *g_ppStudioAPI;
}

int nf_pointers::EV_GetTraceHull()
{
	return (*g_PlayerMove)->usehull;
}