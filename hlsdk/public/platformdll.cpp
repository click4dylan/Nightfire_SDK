#include <Windows.h>
#include "platformdll.h"

NightfirePlatformFuncs g_NightfirePlatformFuncs;
NightfirePlatformFuncs* g_pNightfirePlatformFuncs = &g_NightfirePlatformFuncs;

MembankInitialize_t MembankInitialize;
MembankSetEnsureStatus_t MembankSetEnsureStatus;
MembankShutdown_t MembankShutdown;
GbxResultDestructor_t GbxResultDestructor;
MembankUsageType_t MembankUsageType;
MembankUsageTypeDestructor_t MembankUsageTypeDestructor;
mallocx_t mallocx;
freex_t freex;

void NightfirePlatformFuncs::Init(unsigned long platform_module)
{
	version = NIGHTFIRE_PLATFORM_FUNCS_VERSION;
	MembankInitialize = (MembankInitialize_t)GetProcAddress((HMODULE)platform_module, "?MembankInitialize@@YG?AVGbxResult@@HPBD@Z");
	MembankSetEnsureStatus = (MembankSetEnsureStatus_t)GetProcAddress((HMODULE)platform_module, "?MembankSetEnsureStatus@@YG?AVGbxResult@@W4MemEnsureStatusState@Membank@@@Z");
	MembankShutdown = (MembankShutdown_t)GetProcAddress((HMODULE)platform_module, "?MembankShutdown@@YG?AVGbxResult@@XZ");
	GbxResultDestructor = (GbxResultDestructor_t)GetProcAddress((HMODULE)platform_module, "??1GbxResult@@QAE@XZ");
	MembankUsageType = (MembankUsageType_t)GetProcAddress((HMODULE)platform_module, "??0MembankUsageType@@QAE@W4MemPlacementHint@Membank@@@Z");
	MembankUsageTypeDestructor = (MembankUsageTypeDestructor_t)GetProcAddress((HMODULE)platform_module, "??1MembankUsageType@@QAE@XZ");
	mallocx = (mallocx_t)GetProcAddress((HMODULE)platform_module, "?mallocx@@YGPAXH@Z");
	freex = (freex_t)GetProcAddress((HMODULE)platform_module, "?freex@@YGXPAX@Z");
	strcmpx = (strcmpx_t)GetProcAddress((HMODULE)platform_module, "?strcmpx@@YGHPBD0@Z");
}