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

//TODO: cleanup this garbage code
// use vtables properly and reverse member variables


//EV_LighterSpark
void(*EV_LighterSpark)(event_args_t* event) = (void(*)(event_args_t*))0x4102C020;

inline float UTIL_WeaponTimeBase()
{
    return 0.0f;
}

inline void CBasePlayer_SetAnimation(int anim)
{
}


void __fastcall CLighter_PrimaryAttack(DWORD weapon, int edx, int a2)
{
    if (!*(DWORD*)(weapon + 228) && !*(DWORD*)(weapon + 212))
    {
       // original code
       //return (*(int(__thiscall**)(int, int))(*(DWORD*)weapon + 416))(this, a2);

        //SendWeaponAnim
        (*(void(__thiscall**)(DWORD, int, int))(*(DWORD*)weapon + 400))(weapon, 2, 0);

        const float next_flick_time = UTIL_WeaponTimeBase() + 0.73f;
        *(float*)(weapon + 164) = next_flick_time;
        *(float*)(weapon + 160) = next_flick_time;
        *(float*)(weapon + 168) = UTIL_WeaponTimeBase() + 0.3f;

        CBasePlayer_SetAnimation(5);

        event_args_t event;
        event.bparam1 = 0;
        EV_LighterSpark(&event);

        // fade in from darkness
        //event.bparam1 = 1;
        //EV_LighterSpark(&event);
    }
}

void(__thiscall* g_oCLighter_PrimaryAttackGameDLL)(DWORD, int);
void(__thiscall* CBasePlayer_SetAnimation_GameDLL)(DWORD*, int) = (void(__thiscall*)(DWORD*, int))0x420A6DA0;

void __fastcall CLighter_PrimaryAttackGameDLL(DWORD weapon, int edx, int a2)
{
    if (*((DWORD*)weapon + 57))
        return;
    if (!*((DWORD*)weapon + 53))
    {
        //original code
        //(*(void(__thiscall**)(DWORD, int))(*(DWORD*)weapon + 416))(weapon, a2);
        //return;

         //SendWeaponAnim
        (*(void(__thiscall**)(DWORD, int, int))(*(DWORD*)weapon + 400))(weapon, 2, 0);

        const float next_flick_time = UTIL_WeaponTimeBase() + 0.73f;
        *(float*)(weapon + 164) = next_flick_time;
        *(float*)(weapon + 160) = next_flick_time;
        *(float*)(weapon + 168) = UTIL_WeaponTimeBase() + 0.3f;

        CBasePlayer_SetAnimation_GameDLL(*((DWORD**)weapon + 30), 5);
        return;
    }

    g_oCLighter_PrimaryAttackGameDLL(weapon, a2);
}

void Fix_LighterSpark()
{
    static bool fixed_client = false;
    static bool fixed_server = false;

    if (*g_clientDllHinst && !fixed_client)
    {
        // overwrite client.dll primaryattack
        PlaceJMP((BYTE*)0x4100A9D0, (DWORD)&CLighter_PrimaryAttack, 5);
        fixed_client = true;
    }

    // need to overwrite game.dll primaryattack as well
    if (g_gameDllHinst && !fixed_server)
    {
        // hook primary attack for weapon_lighter in game.dll
        if (!HookFunctionWithMinHook((BYTE*)0x420E03C0, CLighter_PrimaryAttackGameDLL, (void**)&g_oCLighter_PrimaryAttackGameDLL))
            return;

        fixed_server = true;
    }
}