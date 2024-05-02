//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// dedicated.h
#ifndef INC_DEDICATEDH
#define INC_DEDICATEDH

#include <Windows.h>
#include "exefuncs.h"
#include "nightfire_other.h"

char *CheckParm		( const char *psz, char **ppszValue = (char **)0 );
void __cdecl AppActivate(BOOLEAN active);
void DeactivateFromGameWindow();
void ClearAllStates();
void ActivateGameWindowFromOther();
void __cdecl ShowHideCursor(char enable);
extern long g_hRenderDLL;
extern void* render;
extern DWORD* renderplatforminfo;
extern void* renderfeatures;
extern CreateRenderMaterialT CreateRenderMaterial;
extern CreateRenderViewportT CreateRenderViewport;
extern CreateRenderVideoTextureT CreateRenderVideoTexture;

extern exefuncs_t s_LauncherAPI;
extern BOOLEAN border;
extern BOOLEAN g_bDedicated;

void Render_DoRender();
void Render_DoFlip();
DWORD __cdecl GetCDKey(char* dest, unsigned int buffer_size, DWORD* new_length, BYTE* result);
int IsValidCD();
void CDAudio_Play(unsigned __int8 a1, int a2);
void CDAudio_Pause();
void CDAudio_Resume();
void CDAudio_Update();
void __cdecl Launcher_PlayMovie(char* movie);
void __cdecl Launcher_OpenFrontEnd(bool to_escape_menu, bool multiplayer, bool is_server, bool is_teamplay);
DWORD GetOSPlatform();
int getOSVersion();
extern BYTE g_AltTabbedAway;
extern HWND g_EngWnd;
#include "engine_launcher_api.h"
extern gui_api_t g_guiAPI;
extern unsigned int* KeyboardLayout;
extern unsigned int* KeyboardLayouts[8];
extern BOOLEAN bFinalTick;
extern void SaveVideoSettingsToRegistry(void);
extern BOOLEAN Render_Close(void);
extern bool Render_Shutdown(void);
extern void unlinkGUIDll(void);
extern void unlinkEngineDll(void);
BOOLEAN Render_Open(bool cfullscreen, unsigned cwidth, unsigned cheight, unsigned ccolorbits, unsigned crefreshrate, unsigned cantialias, bool cvsync, bool chwinfo);
extern long g_guiDllHinst;

#endif
