//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( SYS_DEDH )
#define SYS_DEDH
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif

#if defined _MSC_VER && _MSC_VER >= 1400
	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE
	#endif

	#pragma warning(disable: 4996) // deprecated functions
#endif

//Nightfire Platform.dll functions

//static int storage, storage2;

#include <windows.h>
extern BOOLEAN g_bDedicated;

typedef void (*SleepType)(int);
extern void InitTime();
void CreateEngineInterface(void);
double Sys_FloatTime();
unsigned long Sys_MSTime();
long Sys_LoadLibrary( const char *lib );
__forceinline bool FilterTime( double dt );
void Host_AccumulateTime (double time);
float GetTickInterval();
void Sys_FreeLibrary( long library );
void *Sys_GetProcAddress( long library, const char *name );
void Sys_Printf(char *fmt, ...);
extern __declspec(noreturn) void ErrorMessage( int level, const char *msg );
extern __declspec(noreturn) void ErrorBox(const char* msg, const char* filename);
extern void linkEngineDll(const char* name);
extern void linkGUIDll(const char* name);
extern char* GetCDPath();
extern LSTATUS __cdecl GetRegionInfo(char* region);
extern void UpdateWindowGlobals(unsigned newwidth, unsigned newheight);
extern BOOLEAN Render_Init(char* mod);
extern void HookFuncs(bool ENABLE_NVIDIA_FIX);
extern void linkRInput();
extern void unlinkRInput();
extern void linkPlatformDll(const char* library);
#endif // SYS_DEDH
