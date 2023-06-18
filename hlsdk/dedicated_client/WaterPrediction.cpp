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
#include <enginefuncs.h>
#include <globalvars.h>
#include <server.h>
#include <nightfire_pointers.h>
#include "nightfire_hooks.h"

#include <pm_defs.h>

#include <studio.h>
#include <StudioModelRenderer.h>

bool nf_hooks::UTIL_CheckForWater(float startx, float starty, float startz, float endx, float endy, float endz, float* dest)
{
	pmtrace_s tr;
	int old_hull = g_Pointers.EV_GetTraceHull(); //gearbox didn't have this
	g_Pointers.g_pCL_EngineFuncs->pEventAPI->EV_SetTraceHull(2);
	g_Pointers.g_pCL_EngineFuncs->pEventAPI->EV_PlayerTrace(&startx, &endx, 0x100000, 0x31, -1, &tr);
	g_Pointers.g_pCL_EngineFuncs->pEventAPI->EV_SetTraceHull(old_hull);// this is the fix for broken hull bounds when in water!!
	if (tr.fraction >= 1.0f || !tr.surface)
		return false;
	if (!tr.surface->parent_brush || !(tr.surface->parent_brush->contents & 0x100000) || tr.plane.normal[2] <= 0.99f)
		return false;

	dest[0] = tr.endpos[0];
	dest[1] = tr.endpos[1];
	dest[2] = tr.endpos[2];
	return true;
}

void Fix_Water_Hull()
{
	//Fixes bug introduced somewhere between alpha demo and public build in UTIL_CheckForWater
	//Hull gets set to point size and then not restored, causing major movement prediction errors, lag and audio glitches whenever this function gets called	
	if (!HookFunctionWithMinHook(g_Pointers.UTIL_CheckForWater, nf_hooks::UTIL_CheckForWater, nullptr))
		return;
}