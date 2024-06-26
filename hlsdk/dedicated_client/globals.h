#pragma once
#include <Windows.h>
#include "nightfire_other.h"
#include "engine_launcher_api.h"
#include "exefuncs.h"
#include "sys_ded.h"

#include <renderd3d.h>

// new protocol versions for nightfire patches will cause proper translated connection failures to old clients
#define PROTOCOL_VERSION 52

extern CreateRenderT CreateRender;
extern CreateRenderPlatformInfoT CreateRenderPlatformInfo;
extern CreateRenderFeaturesT CreateRenderFeatures;
extern CreateRenderMaterialT CreateRenderMaterial;
extern CreateRenderViewportT CreateRenderViewport;
extern CreateRenderVideoTextureT CreateRenderVideoTexture;

extern void* render;
extern DWORD* renderplatforminfo;
extern void* renderfeatures;

extern HWND g_EngWnd;

extern BOOLEAN g_bFullScreen;
extern BOOLEAN g_bDirectToConsole;
extern BOOLEAN border;
extern BOOLEAN g_bNoMetaAudio;

extern BYTE g_AltTabbedAway;

extern RECT g_WindowRect;
extern unsigned g_WindowX;
extern unsigned g_WindowY;
extern unsigned offsetwidth;
extern unsigned offsetheight;

extern engine_api_t nullapi;
extern engine_api_t g_EngineAPI;
extern gui_api_t nullguiapi;
extern gui_api_t g_guiAPI;

extern long g_guiDllHinst;
extern long g_engineDllHinst;
extern long g_platformDllHinst;
extern long g_hRenderDLL;
extern long g_hRawInputDLL;
extern long* g_clientDllHinst;
extern long g_gameDllHinst;
extern long g_MetaAudioDllHinst;
extern HANDLE g_hRawInputThread;
extern HANDLE g_hRawInputEvent;

extern HANDLE hinput;
extern HANDLE houtput;
extern const char* g_pszengine;
extern const char* g_pszplatform;

extern BOOLEAN g_bDedicated;
extern BOOLEAN bFinalTick;

extern HANDLE* g_pMultipleInstanceMutex;

extern unsigned char* s_pMemBase;
extern size_t s_MemSize;
extern size_t s_HunkSize; //64mb nf hunk //0x2000000;  // 32 Mb default heapsize
extern size_t s_HeapSize; //72mb nf heap
extern exefuncs_t s_LauncherAPI;
extern char	console_text[256];
extern int	console_textlen;
extern char s_szCmdLine[2048];

extern BOOL g_mouseParamsValid;
extern unsigned clipboardformat;
extern int g_originalMouseParameters[3];
extern DWORD PlatformID;