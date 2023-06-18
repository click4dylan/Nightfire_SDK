#include <Windows.h>
#include <vector>
//#include <extdll.h>	
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

//#include <pm_defs.h>
#include "fixes.h"

nf_pointers g_Pointers;

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
		DWORD find_name;
		if (FindMemoryPattern(find_name, g_engineDllHinst, "7E 27 8B F7 8D 45 48 8B D6 90", false))
		{
			// prevent CGBAudio::findName from reusing old slot when AudioAPIGetSoundID did not find any crc matching loaded sound filenames
			PlaceShort((BYTE*)find_name, 0x27EB);

			//0x430813D0
			DWORD constructor;
			if (FindMemoryPattern(constructor, g_engineDllHinst, "68 00 FC 00 00 89 1D", false))
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
		DWORD end_precaching;
		if (FindMemoryPattern(end_precaching, g_engineDllHinst, "C6 41 05 00 A1 ?? ?? ?? ?? 57 33 FF 85 C0", false))
		{
			// prevent CGBAudio::endPrecaching from unloading audio files when servercount changes
			PlaceShort((BYTE*)(end_precaching + 0xE), 0x69EB);
		}

#if 0
		//0x43080190
		DWORD load_sound;
		if (FindMemoryPattern(load_sound, g_engineDllHinst, "81 EC 54 03 00 00 53", false))
		{
			// stop CGBAudio::loadSound from dereferencing nullptr on first argument due to CGBAudio::findName returning nullptr
			//PlaceJMP((BYTE*)load_sound, (DWORD)&loadSound_CrashFix, 5);
			loadSound_JmpBack = load_sound + 6;
		}
#endif
	}
//


void __stdcall RainDropTrace(DWORD ESI, float* vecStart, float* vecEnd, int brushflags, int traceflags, int pSkip, pmtrace_s* tr)
{
	int old_hull = g_Pointers.EV_GetTraceHull(); //gearbox didn't have this
	g_Pointers.g_pCL_EngineFuncs->pEventAPI->EV_SetTraceHull(2);

	g_Pointers.g_pCL_EngineFuncs->pEventAPI->EV_PlayerTrace(vecStart, vecEnd, brushflags, traceflags, pSkip, tr);

	const float groundzpos = tr->endpos[2];
	*(float*)(ESI + 0x9C) = groundzpos + 3.0f;

	g_Pointers.g_pCL_EngineFuncs->pEventAPI->EV_PlayerTrace(vecStart, vecEnd, 0x100000, 0x31, -1, tr);

#if 1
	if (tr->endpos[2] > groundzpos)
		*(float*)(ESI + 0x9C) = tr->endpos[2];
#else
	vec3_t waterpos;
	if (UTIL_CheckForWater_Hook(vecStart[0], vecStart[1], vecStart[2], vecEnd[0], vecEnd[2], vecEnd[2], waterpos))
	{
		//*(float*)(ESI + 0x94) = waterpos[0];
		//*(float*)(ESI + 0x98) = waterpos[1];
		*(float*)(ESI + 0x9C) = waterpos[2];
	}
#endif

	g_Pointers.g_pCL_EngineFuncs->pEventAPI->EV_SetTraceHull(old_hull);
}

__declspec(naked) void WaterDropRayTrace()
{
	__asm
	{
		FSTP DWORD PTR SS : [ESP + 0x2C]
		push ebx
		push ecx
		push -1
		push 2
		push 0
		push edx
		push eax
		push esi
		call RainDropTrace
		pop ebx
		mov al, bl
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 0x88
		retn
	}
}

float splash_z_offset = 0.0f;
void Fix_RainDrop_WaterCollision()
{
	DWORD adr;
	if (!FindMemoryPattern(adr, *g_clientDllHinst, "D9 44 24 7C D8 05 ? ? ? ? 83 C4 18 8A C3 D9", false))
		return;
	PlaceJMP((BYTE*)(adr - 0x15), (DWORD)&WaterDropRayTrace, 5);
	DWORD touch;
	if (!FindMemoryPattern(touch, *g_clientDllHinst, "0F 85 ? ? ? ? D9 44 24 2C 8B 74 24 28", false))
		return;
	//increase frequency of splashes
	DWORD old, old2;
	VirtualProtect((LPVOID)touch, 128, PAGE_EXECUTE_READWRITE, &old);
	*(DWORD*)touch = 0x90909090;
	*(unsigned short*)(touch + 4) = 0x9090;

	//make splashes work on water by removing addition of z offset by 32 units..
	*(float**)(touch + 0x10) = &splash_z_offset;

	VirtualProtect((LPVOID)touch, 128, old, &old2);
}

// Forces various entities on the server to not delete themselves and instead send to the client
// Fixes things such as sprite render fx, model/breakable collision, etc
void Force_ServerSide_Entities_GameDLL()
{
	DWORD adr;
	if (FindMemoryPattern(adr, g_gameDllHinst, "39 98 ? ? 00 00 75 11 39 98 ? ? 00 00 75 09 56", false))
	{
		//force CGenericItem to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 6) = 0xEB;
		PopProtection();
	}
	if (FindMemoryPattern(adr, g_gameDllHinst, "8B 88 ? ? 00 00 85 C9 75 13 8B 88 ? ? 00 00 85 C9", false))
	{
		//force CBreakableItem to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 8) = 0xEB;
		PopProtection();
	}
	if (FindMemoryPattern(adr, g_gameDllHinst, "0F 94 C1 33 D2 85 DB 0F 94 C2 85 CA 74 09", false))
	{
		//force CHangingLantern to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 12) = 0xEB;
		PopProtection();
	}
	if (FindMemoryPattern(adr, g_gameDllHinst, "8B 82 ? ? 00 00 85 C0 75 09 56", false))
	{
		//force CEnvGlow to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 8) = 0xEB;
		PopProtection();
	}
	if (FindMemoryPattern(adr, g_gameDllHinst, "F6 80 ? ? 00 00 04 75 11 39 B8 ? ? 00 00 75 09", false))
	{
		//force CEnvSprite to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 7) = 0xEB;
		PopProtection();
	}
	//FIXME: worldspawn and env_fog are still client side! They don't have a specific call to UTIL_Remove..
}

// returns the allocated entity
void* ClientEntityHandler_Dummy(std::string const& name, std::vector<std::pair<std::string, std::string>>const& keyvalues)
{
	return nullptr;
}

void Force_ServerSide_Entities_ClientDLL()
{
	//TODO: FIXME: check if server we are connected to is running latest patch!!!
	//Otherwise we will be missing clientside entities on old servers!
	DWORD adr;
	if (FindMemoryPattern(adr, *g_clientDllHinst, "8B 0C F5 ? ? ? ? 51 50 FF D3", false))
	{
		// replace all the client side entity create functions with a dummy so the client doesn't create them!!
		int numclientsidedentities = *(unsigned char*)(adr + 0x12);
		CLIENTSIDEENTITY* client_side_ent_table = *(CLIENTSIDEENTITY**)(adr + 3);
		PushProtection(client_side_ent_table);

		// nightfire 1.1 has 7 client sided entities, in addition to these there is "env_fog" and "worldspawn"
		const char* blocked_clientsided_entities[] = { "env_sprite", "env_glow", "item_generic", "item_breakable", "physics_lantern" };

		for (int i = 0; i < numclientsidedentities; ++i)
		{
			CLIENTSIDEENTITY* client_side_ent = &client_side_ent_table[i];
			for (int j = 0; j < ARRAYSIZE(blocked_clientsided_entities); ++j)
			{
				if (!strcmp(client_side_ent->name, blocked_clientsided_entities[j]))
				{
					client_side_ent->ClientEntityHandler = ClientEntityHandler_Dummy;
					break;
				}
			}
		}

		PopProtection();
	}
}

void Fix_GameDLL_Bugs()
{
	static bool already_fixed = false;
	if (already_fixed)
		return;

	g_gameDllHinst = (DWORD)GetModuleHandleA("game.dll");
	if (!g_gameDllHinst)
		return;

	// prevent GiveNamedItem for testing purposes
#if 0
	DWORD adr = FindMemoryPattern((DWORD)g_gameDllHinst, "8B 44 24 04 56 57 8B F9", false);
	if (adr)
	{
		PushProtection(adr, 4);
		*(DWORD*)(adr) = 0x900004C2;
		PopProtection();
	}
#endif

	already_fixed = true;

	Force_ServerSide_Entities_GameDLL();
	Fix_AI_TurnSpeed();
}

void Fix_ClientDLL_Bugs()
{
	g_Pointers.GetImportantEngineOffsets(g_engineDllHinst);
	Fix_Model_Crash();
	Fix_Water_Hull();
	Fix_RainDrop_WaterCollision();
	Force_ServerSide_Entities_ClientDLL();
}

void Fix_RateLimiter()
{
	static const double maxrate = MAX_RATE;
	static const float maxrate2 = MAX_RATE;
	DWORD adr;
	if (!FindMemoryPattern(adr, g_engineDllHinst, "8D 97 D0 4C 00 00 52 FF 15 ? ? ? ? 68", false))
		return;

	// fix client sending incorrect rate command value to server
	DWORD old;
	VirtualProtect((void*)(adr + 0xE), 4, PAGE_EXECUTE_READWRITE, &old);
	*(const char**)(adr + 0xE) = "rate";
	VirtualProtect((void*)(adr + 0xE), 4, old, nullptr);

	DWORD adr2;
	if (!FindMemoryPattern(adr2, g_engineDllHinst, "68 E8 03 00 00 68 20 4E 00 00", false))
		return;

	//fix limits
	VirtualProtect((void*)(adr2 - 256), 256, PAGE_EXECUTE_READWRITE, &old);
	*(unsigned int*)(adr + 0xCB) = MAX_RATE;
	*(unsigned int*)(adr2 + 6) = MAX_RATE;
	*(const float**)(adr2 - 0x18) = &maxrate2;
	*(const char**)(adr2 + 0xB) = "cl_rate:  Maximum %u, Minimum %u\n";
	*(float*)(adr2 - 64) = DEFAULT_RATE;

	VirtualProtect((void*)(adr2 - 256), 256, old, nullptr);
}

void Fix_FpsCap()
{
	DWORD adr;
	if (!FindMemoryPattern(adr, g_engineDllHinst, "DF E0 F6 C4 05 8D 44 24 08 7B 04 8D 44 24 00 8B 08", false))
		return;
	DWORD old;
	VirtualProtect((void*)(adr - 64), 128, PAGE_EXECUTE_READWRITE, &old);
	static const float maxfps = MAX_FPS;
	*(const float**)(adr - 12) = &maxfps; //set fps command comparison limiter
	*(float*)(adr - 4) = MAX_FPS; //set max fps to 1000
	*(float*)(adr - 0x95) = MAX_FPS; // change loading screen fps limit from 30 to 1000
	VirtualProtect((void*)(adr - 64), 256, old, nullptr);
}


float* g_NextCLCmdTime;
DWORD Set_NextCLCmdTimeCPP_Jmpback;
float Set_NextCLCmdTimeCPP(float desired_delta)
{
	const double curtime = *g_Pointers.realtime;
	const double dt = 1.0 / desired_delta;
	double delta = curtime - *g_NextCLCmdTime;
	delta = min(max(delta, 0.0), 0.1);
	double next_time = curtime + dt - delta;
	return (float)next_time;
}
__declspec(naked) void Set_NextCLCmdTime()
{
	__asm
	{
		push ebx
		push ecx
		push[esp + 0x18]
		call Set_NextCLCmdTimeCPP
		add esp, 4
		pop ecx
		pop ebx
		jmp Set_NextCLCmdTimeCPP_Jmpback
	}
}

DWORD* g_Player;
DWORD g_NextSVCmdTimeOffset;
DWORD g_DesiredSVCmdDeltaOffset;
double* host_frametime;
DWORD Set_NextSVCmdTime_Jmpback;
void Set_NextSVCmdTimeCPP()
{
	static ConsoleVariable* fps_max = g_pEngineFuncs->pfnGetConsoleVariableGame("fps_max");
	static ConsoleVariable* sys_ticrate = g_pEngineFuncs->pfnGetConsoleVariableGame("sys_ticrate");
	double minimum_frametime = 1.0 / (g_bDedicated ? (double)sys_ticrate->getValueFloat() : (double)fps_max->getValueFloat());
	minimum_frametime = min(max(minimum_frametime, 1.0 / MAX_FPS), 1.0 / 0.5);
		
	DWORD player = *g_Player;
	const double curtime = *g_Pointers.realtime;
	const double desired_delta = *(double*)(player + g_DesiredSVCmdDeltaOffset);
	const double next_cmdtime = *(double*)(player + g_NextSVCmdTimeOffset);
	double delta = curtime - next_cmdtime;
	delta = min(max(delta, 0.0), minimum_frametime);
	double next_time = curtime + desired_delta - delta;
	//nightfire code below:
	//next_time = curtime + *host_frametime + desired_delta;

	*(double*)(player + g_NextSVCmdTimeOffset) = next_time;
}

__declspec(naked) void Set_NextSVCmdTime()
{
	__asm
	{
		push ebx
		call Set_NextSVCmdTimeCPP
		pop ebx
		jmp Set_NextSVCmdTime_Jmpback
	}
}

void Fix_RateDesync()
{
	DWORD adr;
	if (!FindMemoryPattern(adr, g_engineDllHinst, "DC 05 ? ? ? ? EB 06 DD 05 ? ? ? ? A1 ? ? ? ?", false))
		return;
	g_NextCLCmdTime = *(float**)(adr + 0x15);
	PlaceJMP((BYTE*)(adr - 10), (DWORD)&Set_NextCLCmdTime, 5);
	Set_NextCLCmdTimeCPP_Jmpback = adr + 0x13;

	DWORD adr2;
	if (!FindMemoryPattern(adr2, g_engineDllHinst, "DD 05 ? ? ? ? A1 ? ? ? ? DC 80 ? ? 00 00 DC 05 ? ? ? ? DD 98 ? ? 00 00", false))
		return;
	DWORD adr3;
	if (!FindMemoryPattern(adr3, g_engineDllHinst, "DD 05 ? ? ? ? D9 5C 24 0C D9 44 24 0C D8 0F", false))
		return;
	g_Player = *(DWORD**)(adr2 + 7);
	g_NextSVCmdTimeOffset = *(DWORD*)(adr2 + 0x19);
	g_DesiredSVCmdDeltaOffset = *(DWORD*)(adr2 + 0xD);
	host_frametime = *(double**)(adr3 + 2);
	PlaceJMP((BYTE*)adr2, (DWORD)&Set_NextSVCmdTime, 29);
	Set_NextSVCmdTime_Jmpback = adr2 + 0x17;
}

void Fix_UserInfoString()
{
	DWORD adr;
	if (!FindMemoryPattern(adr, g_engineDllHinst, "A0 ? ? ? ? 84 C0 0F 85 ? ? ? ? 68 00 01 00 00", false))
		return;

	DWORD old;
	VirtualProtect((void*)adr, 256, PAGE_EXECUTE_READWRITE, &old);

	*(const char**)(adr + 0x2C) = "6.00 insecure";
	*(const char**)(adr + 0x31) = "gamever";
}

void(*g_oAlertMessage)(int, char*, ...);
void AlertMessage(int a1, char* Format, ...)
{
	static char szOut[1024] = {0};
	int v2; // eax
	va_list ArgList; // [esp+10h] [ebp+Ch] BYREF
	static ConsoleVariable* dev = nullptr;
	if (!dev) //hack
		g_pEngineFuncs->pfnGetConsoleVariableGame("developer");

	va_start(ArgList, Format);
	if (a1 == 5 && *g_Pointers.svs_maxclients > 1)
	{
		vsnprintf(szOut, 1024, Format, ArgList);
		szOut[1023] = 0;
		g_Pointers.Log_Printf("%s", szOut);
	}
	else if (dev && dev->getValueInt())
	{
		switch (a1)
		{
		case 0:
			strncpy_s(szOut, "NOTE:  ", 1024);
			szOut[1023] = 0;
			break;
		case 1:
			szOut[0] = 0;
			break;
		case 2:
			if (dev->getValueInt() < 2)
				return;
			szOut[0] = 0;
			break;
		case 3:
			strncpy_s(szOut, "WARNING:  ", 1024);
			szOut[1023] = 0;
			break;
		case 4:
			strncpy_s(szOut, "ERROR:  ", 1024);
			szOut[1023] = 0;
			break;
		default:
			break;
		}
		v2 = strlen(szOut);
		vsnprintf(&szOut[v2], 1024 - v2, Format, ArgList); // this is the crash fix..., use vsnprintf
		szOut[1023] = 0;
		g_Pointers.g_pCL_EngineFuncs->Con_Printf("%s", szOut);
	}
}

void Fix_AlertMessage_Crash()
{
	DWORD adr;
	if (!FindMemoryPattern(adr, g_engineDllHinst, "56 8B 74 24 08 83 FE 05 75 37", false))
		return;

	g_Pointers.GetImportantEngineOffsets(g_engineDllHinst);

	if (!HookFunctionWithMinHook((void*)adr, (void*)&AlertMessage, (void**)&g_oAlertMessage))
		return;
}

void(*g_oCon_DPrintf)(const char*, ...);
bool* g_fIsDebugPrint;
void Con_DPrintf(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[4096];
	static ConsoleVariable* dev = g_pEngineFuncs->pfnGetConsoleVariableGame("developer");
	if (dev && dev->getValueInt())
	{
		va_start(argptr, fmt);
		vsnprintf(msg, sizeof(msg), fmt, argptr);
		va_end(argptr);

		msg[sizeof(msg) - 1] = 0;

		*g_fIsDebugPrint = true;
		g_Pointers.g_pCL_EngineFuncs->Con_Printf(msg);
		*g_fIsDebugPrint = false;
	}
}

void Fix_Con_DPrintf_StackSmash_Crash()
{
	g_Pointers.GetImportantEngineOffsets(g_engineDllHinst);
	if (!g_Pointers.g_pCL_EngineFuncs->Con_DPrintf)
		return;

	g_fIsDebugPrint = *(bool**)((DWORD)g_Pointers.g_pCL_EngineFuncs->Con_DPrintf + 0x44);
	
	if (!HookFunctionWithMinHook((void*)g_Pointers.g_pCL_EngineFuncs->Con_DPrintf, (void*)&Con_DPrintf, (void**)&g_oAlertMessage))
		return;
}

void LoadThisDll_Post(const char* gamedll)
{
	g_gameDllHinst = (long)GetModuleHandleA("game.dll");
	if (!g_gameDllHinst)
		return;
	
	Fix_GameDLL_Bugs();
}

DWORD g_oLoadThisDll;
__declspec(naked) void LoadThisDll_Hook()
{
	__asm
	{
		push ebx
		call g_oLoadThisDll
		pop ebx
		push ebx
		call LoadThisDll_Post
		add esp, 4
		retn
	}
}

void Hook_GameDLLLoadLibrary()
{
	DWORD adr;
	if (!FindMemoryPattern(adr, g_engineDllHinst, "57 53 FF 15 ? ? ? ? 8B F8 85 FF 75 17", false))
		return;

	if (!HookFunctionWithMinHook((void*)adr, (void*)&LoadThisDll_Hook, (void**)&g_oLoadThisDll))
		return;
}

void Fix_Engine_Bugs()
{
	Hook_GameDLLLoadLibrary();
	Fix_Sound_Overflow();
	Fix_Netchan();
	Fix_AlertMessage_Crash();
	Fix_Con_DPrintf_StackSmash_Crash();
	Fix_RateLimiter();
	Fix_RateDesync();
	Fix_FpsCap();
	Fix_UserInfoString();
}

void Fix_Gamespy()
{
	DWORD adr;
	while (FindMemoryPattern(adr, g_engineDllHinst, "6D 61 73 74 65 72 2E 67 61 6D 65 73 70 79 2E 63 6F 6D 00", false))
	{
		DWORD old, old2;
		VirtualProtect((void*)adr, 19, PAGE_READWRITE, &old);
		strncpy((char*)adr, "master.openspy.net", 19);
		VirtualProtect((void*)adr, 19, old, &old2);
		FindMemoryPattern(adr, g_engineDllHinst, "6D 61 73 74 65 72 2E 67 61 6D 65 73 70 79 2E 63 6F 6D 00", false);
	}
}

// apply nullptr check from mac version
void Fix_GUI_GetAction_Crash()
{
	if (!g_guiDllHinst)
		return;

	DWORD adr;
	if (!FindMemoryPattern(adr, g_guiDllHinst, "8B 10 53 8B C8 FF 52 08", false))
		return;

	PlaceJMP((BYTE*)adr, (DWORD)&GUI_GetAction_Return, 5);
	GUI_GetAction_JmpBack = adr + 8;
}

void Fix_HighFPSBugs()
{
	usercmd_t test;
}