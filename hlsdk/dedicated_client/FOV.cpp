//TODO

#if 0
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
#include "clientdll_funcs.h"
#include <cvardef.h>

ConsoleVariable* default_fov = nullptr;
void (__thiscall* g_oHud_Init)(void*);

void __fastcall CHud_Init(void* hudptr)
{
	g_oHud_Init(hudptr);
}

void __fastcall __MsgFunc_LockWeapons(BYTE* thisptr, void* edx, char const* arg1, int size, void* buf)
{
	BEGIN_READ(buf, size);
	thisptr[36] = READ_BYTE() != 0; //g_LockWeapons
	int default_weapon = READ_BYTE();
	if (!thisptr[36] && default_weapon != 255)
	{
		int fov = default_fov->getValueFloat(); //76
		if (g_FIELDOFVIEW != fov)
		{
			g_FIELDOFVIEW = fov;
			gViewPort->rocketoverlaypanel->TurnOff();
			gViewPort->crosshairoverlaypanel->setCrosshair(-1);
			gViewPort->crosshairpanel->TurnOn();
			m_flMouseSensitivity = 0.0f;
		}
		CHud__SelectWeapon(&gHUD, default_weapon, 1);
	}
}

void InjectFOVFix()
{
	// register a float default_fov command and set it to notify the server in the userinfo, and save to config.cfg
	default_fov = g_EngineAPI.CreateConsoleVariable(IConsoleVariable(CVAR_FLOAT, "default_fov", "Custom FOV changer", "76", FCVAR_ARCHIVE | FCVAR_USERINFO));
	if (!default_fov)
		return;

	if (!HookFunctionWithMinHook((void*)0x4104AD20, CHud_Init, (void**)&g_oHud_Init))
		return;

	//register default_fov with client.dll
	*(ConsoleVariable**)0x410B6100 = default_fov;

	//stop client.dll from registering default_fov
	DWORD old, old2;
	VirtualProtect((void*)0x4104B27D, 32, PAGE_EXECUTE_READWRITE, &old);
	char* first_adr = (char*)0x4104B27D;
	for (int i = 0; i < 18; ++i)
	{
		first_adr[i] = 0x90;
	}
	char* second_adr = (char*)0x4104B294;
	for (int i = 0; i < 6; ++i)
	{
		second_adr[i] = 0x90;
	}
	VirtualProtect((void*)0x4104B27D, 32, old, &old2);


	uintptr_t SIG552_SECONDARYATTACK_FOV_ZOOMED_ADR = 
	PlaceJMP((BYTE*)(adr - 0x15), (DWORD)&WaterDropRayTrace, 5);
}
#endif