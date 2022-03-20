#include <Windows.h>
#include "globals.h"
#include "pattern_scanner.h"
#include "MinHook/MinHook.h"

typedef float vec_t;
typedef float vec2_t[2];
typedef float vec3_t[3];
#include <eiface.h>
#include <com_model.h>
#include <pmtrace.h>

DWORD GUI_GetAction_JmpBack;
__declspec(naked) void GUI_GetAction_Return()
{
	__asm
	{
		test eax, eax
		jz isnullptr
		mov edx, [eax]
		push ebx
		mov ecx, eax
		call [edx + 8]
		isnullptr:
		jmp GUI_GetAction_JmpBack
	}
}

// this fix is not ideal as audio files will still leak if they are changed during runtime,
// but this will allow us to have more than 768 sounds, and stops the CGBAudio::m_registerSoundFX array being overflowed all the time
// also fixes crash when reconnecting to servers multiple times
//
	DWORD loadSound_JmpBack;
	__declspec(naked) void loadSound_CrashFix()
	{
		__asm
		{
			mov eax, [esp + 4]
			test eax, eax
			jz isnullptr
			sub esp, 0x354
			jmp loadSound_JmpBack
			isnullptr :
			xor eax, eax
				ret 8
		}
	}
	void Fix_Sound_Overflow()
	{
#if 0
		//0x4307EFF6
		DWORD find_name = FindMemoryPattern(g_engineDllHinst, "7E 27 8B F7 8D 45 48 8B D6 90", false);
		if (find_name)
		{
			// prevent CGBAudio::findName from reusing old slot when AudioAPIGetSoundID did not find any crc matching loaded sound filenames
			PlaceShort((BYTE*)find_name, 0x27EB);

			//0x430813D0
			DWORD constructor = FindMemoryPattern(g_engineDllHinst, "68 00 FC 00 00 89 1D", false);
			if (constructor)
			{
				// increase max audio files limit in CGBAudio::findName from 768 to 4096
				PlaceInt((BYTE*)(find_name + 0x2B), 4096);
				PlaceInt((BYTE*)(find_name + 0x32), 4096);

				// increase allocation size for audio files in CGBAudio::CGBAudio()
				// note: nightfire alpha demo struct size is 80 bytes
				PlaceInt((BYTE*)(constructor + 1), 4096 * 84); //alloc
				PlaceInt((BYTE*)(constructor + 0x14), 4096 * 84); //memset
			}

		}
#endif

		//0x4307ED40
		DWORD end_precaching = FindMemoryPattern(g_engineDllHinst, "C6 41 05 00 A1 ?? ?? ?? ?? 57 33 FF 85 C0", false);
		if (end_precaching)
		{
			// prevent CGBAudio::endPrecaching from unloading audio files when servercount changes
			PlaceShort((BYTE*)(end_precaching + 0xE), 0x69EB);
		}

#if 0
		//0x43080190
		DWORD load_sound = FindMemoryPattern(g_engineDllHinst, "81 EC 54 03 00 00 53", false);
		if (load_sound)
		{
			// stop CGBAudio::loadSound from dereferencing nullptr on first argument due to CGBAudio::findName returning nullptr
			//PlaceJMP((BYTE*)load_sound, (DWORD)&loadSound_CrashFix, 5);
			loadSound_JmpBack = load_sound + 6;
		}
#endif
	}
//

void(*EV_SetTraceHull)(int hull);
DWORD g_PlayerMove, offset;
int EV_GetTraceHull()
{
	return *(int*)(*(DWORD*)g_PlayerMove + offset);
}
void(*EV_PlayerTrace) (float* start, float* end, int brushFlags, int traceFlags, int ignore_pe, struct pmtrace_s* tr);
bool(*g_oUTIL_CheckForWater)(float, float, float, float, float, float, vec3_t&);

bool UTIL_CheckForWater(float startx, float starty, float startz, float endx, float endy, float endz, vec3_t& dest)
{

}

bool UTIL_CheckForWater_Hook(float startx, float starty, float startz, float endx, float endy, float endz, vec3_t& dest)
{
	pmtrace_s tr;
	int old_hull = EV_GetTraceHull(); //gearbox didn't have this
	EV_SetTraceHull(2);
	EV_PlayerTrace(&startx, &endx, 0x100000, 0x31, -1, &tr);
	EV_SetTraceHull(old_hull);// this is the fix for broken hull bounds when in water!!
	if (tr.fraction >= 1.0f || !tr.surface)
		return false;
	if (!tr.surface->parent_brush || !(tr.surface->parent_brush->contents & 0x100000) || tr.plane.normal[2] <= 0.99f)
		return false;

	//nightfire default behavior is to just set dest to tr.endpos here and return true
	dest[0] = tr.endpos[0];
	dest[1] = tr.endpos[1];
	dest[2] = tr.endpos[2];
	return true;
	// 
	//TODO: replicate old amx hook behavior

#if 0
	// fix endpos so that rain collides with water as intended, etc
	vec3_t ground;
	ground[0] = tr.endpos[0];
	ground[1] = tr.endpos[1];
	ground[2] = tr.endpos[2];
	startz += 3.0f;
	EV_PlayerTrace(&startx, &endx, 0x100000, 0x31, -1, &tr);
	if (tr.endpos[2] > ground[2])
	{
		dest[0] = tr.endpos[0];
		dest[1] = tr.endpos[1];
		dest[2] = tr.endpos[2] + 6.0f;
		return true;
	}

	dest[0] = ground[0];
	dest[1] = ground[1];
	dest[2] = ground[2];
	return true;
#endif
}

void Fix_Water_Hull()
{
	//Fixes bug introduced somewhere between alpha demo and public build in UTIL_CheckForWater
	//Hull gets set to point size and then not restored, causing major movement prediction errors, lag and audio glitches whenever this function gets called
	HMODULE clientdll = GetModuleHandleA("client.dll");
	DWORD adr = FindMemoryPattern(*g_clientDllHinst, "A1 ? ? ? ? 83 EC 48 6A 02", false);
	EV_PlayerTrace = (void(*)(float*, float*, int, int, int, struct pmtrace_s*))FindMemoryPattern(g_engineDllHinst, "8B 44 24 14 8B 4C 24 10 8B 54 24 0C 83 EC 48", false);
	EV_SetTraceHull = (void(*)(int))FindMemoryPattern(g_engineDllHinst, "8B 44 24 04 8B 0D ? ? ? ? 89 81 ? 00 00 00 C3", false);
	g_PlayerMove = *(DWORD*)((DWORD)EV_SetTraceHull + 6);
	offset = *(DWORD*)((DWORD)EV_SetTraceHull + 0xC);
	if (!HookFunctionWithMinHook((void*)adr, UTIL_CheckForWater_Hook, (void**)&g_oUTIL_CheckForWater))
		return;
}

void Fix_Engine_Bugs()
{
	Fix_Sound_Overflow();
}

void Fix_Gamespy()
{
	DWORD adr = FindMemoryPattern(g_engineDllHinst, "6D 61 73 74 65 72 2E 67 61 6D 65 73 70 79 2E 63 6F 6D 00", false);
	while (adr)
	{
		DWORD old, old2;
		VirtualProtect((void*)adr, 19, PAGE_READWRITE, &old);
		strncpy((char*)adr, "master.openspy.net", 19);
		VirtualProtect((void*)adr, 19, old, &old2);
		adr = FindMemoryPattern(g_engineDllHinst, "6D 61 73 74 65 72 2E 67 61 6D 65 73 70 79 2E 63 6F 6D 00", false);
	}
}

// apply nullptr check from mac version
void Fix_GUI_GetAction_Crash()
{
	if (!g_guiDllHinst)
		return;

	DWORD adr = FindMemoryPattern(g_guiDllHinst, "8B 10 53 8B C8 FF 52 08", false);
	if (!adr)
		return;

	PlaceJMP((BYTE*)adr, (DWORD)&GUI_GetAction_Return, 5);
	GUI_GetAction_JmpBack = adr + 8;
}