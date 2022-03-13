#include <Windows.h>
#include "platformdll.h"
#include "globals.h"

MembankInitialize_t MembankInitialize;
MembankSetEnsureStatus_t MembankSetEnsureStatus;
MembankShutdown_t MembankShutdown;
GbxResultDestructor_t GbxResultDestructor;
MembankUsageType_t MembankUsageType;
MembankUsageTypeDestructor_t MembankUsageTypeDestructor;
mallocx_t mallocx;
freex_t freex;

void linkPlatformDll(const char* name)
{
	g_platformDllHinst = Sys_LoadLibrary((char*)g_pszplatform);

	if (!g_platformDllHinst)
		ErrorMessage(-1, "Was not able to load in the membank \"platform.dll\"");

	MembankInitialize = (MembankInitialize_t)GetProcAddress((HMODULE)g_platformDllHinst, "?MembankInitialize@@YG?AVGbxResult@@HPBD@Z");
	MembankSetEnsureStatus = (MembankSetEnsureStatus_t)GetProcAddress((HMODULE)g_platformDllHinst, "?MembankSetEnsureStatus@@YG?AVGbxResult@@W4MemEnsureStatusState@Membank@@@Z");
	MembankShutdown = (MembankShutdown_t)GetProcAddress((HMODULE)g_platformDllHinst, "?MembankShutdown@@YG?AVGbxResult@@XZ");
	GbxResultDestructor = (GbxResultDestructor_t)GetProcAddress((HMODULE)g_platformDllHinst, "??1GbxResult@@QAE@XZ");
	MembankUsageType = (MembankUsageType_t)GetProcAddress((HMODULE)g_platformDllHinst, "??0MembankUsageType@@QAE@W4MemPlacementHint@Membank@@@Z");
	MembankUsageTypeDestructor = (MembankUsageTypeDestructor_t)GetProcAddress((HMODULE)g_platformDllHinst, "??1MembankUsageType@@QAE@XZ");
	mallocx = (mallocx_t)GetProcAddress((HMODULE)g_platformDllHinst, "?mallocx@@YGPAXH@Z");
	freex = (freex_t)GetProcAddress((HMODULE)g_platformDllHinst, "?freex@@YGXPAX@Z");
}