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

void linkPlatformDll(const char* name);


extern MembankInitialize_t MembankInitialize;
extern MembankSetEnsureStatus_t MembankSetEnsureStatus;
extern MembankShutdown_t MembankShutdown;
extern GbxResultDestructor_t GbxResultDestructor;
extern MembankUsageType_t MembankUsageType;
extern MembankUsageTypeDestructor_t MembankUsageTypeDestructor;
extern mallocx_t mallocx;
extern freex_t freex;