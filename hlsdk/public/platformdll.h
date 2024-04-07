#pragma once

struct GbxResult
{
	unsigned long result[2];
};

struct MembankUsageType_Class
{
	DWORD pad[4]; //unknown size
};

typedef GbxResult*(__stdcall *MembankInitialize_t) (GbxResult& result, size_t heap, const char* out_of_memory_report_file);
typedef GbxResult*(__thiscall *MembankSetEnsureStatus_t) (GbxResult& result, GbxResult& result_same, int enable);
typedef GbxResult*(__stdcall *MembankShutdown_t) (GbxResult& result);
typedef void(__thiscall *GbxResultDestructor_t) (GbxResult& source);
typedef int(__thiscall *MembankUsageType_t) (MembankUsageType_Class* c, int type);
typedef void(__thiscall *MembankUsageTypeDestructor_t) (MembankUsageType_Class* c);
typedef void* (__stdcall *mallocx_t)(size_t bytes);
typedef void (__stdcall *freex_t)(void* mem);
typedef int (__stdcall *strcmpx_t)(char const* str1, char const* str2);

#define NIGHTFIRE_PLATFORM_FUNCS_VERSION 2

class NightfirePlatformFuncs
{
public:
	int version; // in case we change it
	unsigned long hPlatformDll;

	MembankInitialize_t MembankInitialize;
	MembankSetEnsureStatus_t MembankSetEnsureStatus;
	MembankShutdown_t MembankShutdown;
	GbxResultDestructor_t GbxResultDestructor;
	MembankUsageType_t MembankUsageType;
	MembankUsageTypeDestructor_t MembankUsageTypeDestructor;
	mallocx_t mallocx;
	freex_t freex;
	strcmpx_t strcmpx;

	NightfirePlatformFuncs() : version(0), hPlatformDll(0) {};
	void Init(unsigned long platform_module);
};

extern NightfirePlatformFuncs *g_pNightfirePlatformFuncs;