//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include <stdio.h>
#include <windows.h>
#define snprintf _snprintf

#include "dedicated.h"
#include "dll_state.h"
#include "enginecallback.h"
#include "sys_ded.h"
#include <Uxtheme.h>
#include "VTHook.h"
#include "globals.h"
#include "keyboard.h"
#include "video.h"
#include "fixes.h"
#include <NightfireFileSystem.h>
#include "MetaHook.h"
#include <platformdll.h>
#include "pattern_scanner.h"

typedef int (*engine_api_func)( int version, int size, struct engine_api_s *api );
typedef int(*gui_api_func)(int version, int size, struct gui_api_s *api);

int StartRInputThread(DWORD handletoinjecteddll, DWORD openprocessHandle, const char *entrypoint, void *eventa)
{
	//CreateThread(0, 0, )
	g_hRawInputThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)entrypoint, 0, 0, 0);
	if (!g_hRawInputThread)
	/*if (!CreateRemoteThread(
		*(HANDLE *)openprocessHandle,
		0,
		0,
		(LPTHREAD_START_ROUTINE)&entrypoint[handletoinjecteddll - *(DWORD *)(openprocessHandle + 4)],
		0,
		0,
		0))
		*/
	{
		return 0;
	}
	if (WaitForSingleObject(eventa, 0x1964u))
	{
		return 0;
	}
	return 1;
}

void linkRInput()
{
	bool failed = true;
	g_hRawInputDLL = (long)LoadLibraryA("RawInput.dll");
	if (g_hRawInputDLL)
	{
		FARPROC entryPoint = GetProcAddress((HMODULE)g_hRawInputDLL, "entryPoint");
		if (entryPoint)
		{
			HANDLE event = CreateEventA(0, 0, 0, "RInputEvent32");
			if (event)
			{
				if (StartRInputThread((DWORD)g_hRawInputDLL, (DWORD)GetCurrentProcess(), (const char*)entryPoint, event))
				{
					failed = false;
				}
			}
		}
		
		if (failed)
		{
			FreeLibrary((HMODULE)g_hRawInputDLL);
			g_hRawInputDLL = NULL;
		}
	}
}

void unlinkRInput()
{
	if (g_hRawInputThread != INVALID_HANDLE_VALUE)
	{
		DWORD exitcode;
		GetExitCodeThread(g_hRawInputThread, &exitcode);
		if (exitcode == STILL_ACTIVE)
		{
			BOOL succeded = TerminateThread(g_hRawInputThread, EXIT_SUCCESS);
			int test = 1;
		}
		g_hRawInputThread = INVALID_HANDLE_VALUE;
	}

	if (g_hRawInputEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hRawInputEvent);
		g_hRawInputEvent = INVALID_HANDLE_VALUE;
	}

	if (g_hRawInputDLL)
	{
		FreeLibrary((HMODULE)g_hRawInputDLL);
		g_hRawInputDLL = NULL;
	}
}

void linkPlatformDll(const char* name)
{
	g_platformDllHinst = Sys_LoadLibrary((char*)g_pszplatform);

	if (!g_platformDllHinst)
		ErrorMessage(-1, "Was not able to load in the membank \"platform.dll\"");

	g_pNightfirePlatformFuncs->Init(g_platformDllHinst);
}

void linkGUIDll(const char* name)
{
	g_guiDllHinst = Sys_LoadLibrary(name);

	if (!g_guiDllHinst)
		ErrorMessage(-1, "Was not able to load in the GUI \"gui.dll\"");

	gui_api_func pfnGUIapi;

	pfnGUIapi = (gui_api_func)Sys_GetProcAddress(g_guiDllHinst, "GUI_api");
	if (!pfnGUIapi || !pfnGUIapi(ENGINE_LAUNCHER_API_VERSION, sizeof(gui_api_t), &g_guiAPI))
		ErrorMessage(-1, "Could not bind GUI functions from \"engine.dll\"");

	Fix_GUI_GetAction_Crash();
}

/*
==============
Eng_LoadFunctions

Load engine->front end interface, if possible
==============
*/
void linkEngineDll( const char* name )
{
	g_engineDllHinst = Sys_LoadLibrary(name);

	if (!g_engineDllHinst)
		ErrorMessage(-1, "Was not able to load in the engine \"engine.dll\"");

	engine_api_func pfnEngineAPI = ( engine_api_func )Sys_GetProcAddress(g_engineDllHinst, "Sys_EngineAPI"  );
	if ( !pfnEngineAPI || !pfnEngineAPI( ENGINE_LAUNCHER_API_VERSION, sizeof( engine_api_t ), &g_EngineAPI ) ) 
		ErrorMessage(-1, "Could not bind engine functions from \"engine.dll\"");

	g_pNightfireFileSystem->Init(g_engineDllHinst, g_platformDllHinst);

	DWORD loadthisdll = FindMemoryPattern(g_engineDllHinst, "57 53 FF 15 ? ? ? ? 8B F8 85 FF 75 17", false);
	if (loadthisdll)
	{
		g_pGlobalVariables = (void*)*(DWORD*)(loadthisdll + 0x4D);
		g_pEngineFuncs = (struct enginefuncs_s*)*(DWORD*)(loadthisdll + 0x52);
	}
	RunMetaHook();
	Fix_Engine_Bugs();
	Fix_Gamespy();
}

/*
==============
Eng_LoadStubs

Force NULL interface
==============
*/
void Eng_LoadStubs( void )
{
	// No callbacks in dedicated server since engine should always be loaded.
	memset( &g_EngineAPI, 0, sizeof( g_EngineAPI ) );

	g_EngineAPI.version		= ENGINE_LAUNCHER_API_VERSION;
	//engineapi.rendertype	= RENDERTYPE_UNDEFINED;
	g_EngineAPI.size			= sizeof( engine_api_t );
}

void UpdateWindowGlobals(unsigned newwidth, unsigned newheight)
{
	unsigned width = GetSystemMetrics(SM_CXSCREEN);
	unsigned height = GetSystemMetrics(SM_CYSCREEN);

	if ( !newwidth || !newheight)
	{
		newwidth = width;
		newheight = height;
	}
	if (!g_bFullScreen)
	{
		if (width < newwidth)
		{
			newwidth = width;
		}
		if (height < newheight)
		{
			newheight = height;
		}
		g_WindowRect.left = 0;
		g_WindowRect.top = 0;
		g_WindowRect.right = newwidth;
		g_WindowRect.bottom = newheight;
		g_WindowX = (width - newwidth) / 2;
		g_WindowY = (height - newheight) / 2;
	}
	else
	{
		g_WindowX = 0;
		g_WindowY = 0;
		g_WindowRect.left = 0;
		g_WindowRect.top = 0;
		g_WindowRect.right = newwidth;
		g_WindowRect.bottom = newheight;
	}
	offsetheight = 0;
	offsetwidth = 0;
	if (!g_bFullScreen && border)
	{
		//Dylan added, this fudges the window height a bit so that the mouse cursor in Nightfire's GUI is able to freely move all the way up and down
		HTHEME htheme = OpenThemeData(g_EngWnd, L"EDIT");
		int borderwidth = GetThemeSysSize(htheme, SM_CXBORDER);
		int borderheight = GetThemeSysSize(htheme, SM_CYSIZE) + GetThemeSysSize(htheme, SM_CXPADDEDBORDER) * 2;
		offsetheight = -((float)borderheight * 1.778); //16 / 9
	}
}

void FreeRenderer()
{
	if (g_hRenderDLL)
	{
		FreeLibrary((HMODULE)g_hRenderDLL);
		g_hRenderDLL = NULL;
	}
	CreateRender = NULL;
	CreateRenderViewport = NULL;
	CreateRenderPlatformInfo = NULL;
	CreateRenderFeatures = NULL;
	CreateRenderMaterial = NULL;
}

bool Render_Shutdown()
{
	if (Render_Close())
		FreeRenderer();
	return true;
}

void unlinkGUIDll()
{
	if (g_guiDllHinst)
	{
		FreeLibrary((HMODULE)g_guiDllHinst);
		g_guiDllHinst = NULL;
	}
}

void unlinkEngineDll()
{
	if (g_engineDllHinst)
	{
		ShutdownMetaHook();
		FreeLibrary((HMODULE)g_engineDllHinst);
		g_engineDllHinst = NULL;
	}
}

//Written to be binary compatible with original Bond.exe
BOOLEAN LoadRenderDLL(char* mod)
{
	if (!g_hRenderDLL)
	{
		g_hRenderDLL = (long)LoadLibraryA(mod);
		if (!g_hRenderDLL)
			return 0;
		CreateRender = (CreateRenderT)GetProcAddress((HMODULE)g_hRenderDLL, "CreateRender");
		CreateRenderPlatformInfo = (CreateRenderPlatformInfoT)GetProcAddress((HMODULE)g_hRenderDLL, "CreateRenderPlatformInfo");
		CreateRenderFeatures = (CreateRenderFeaturesT)GetProcAddress((HMODULE)g_hRenderDLL, "CreateRenderFeatures");
		CreateRenderMaterial = (CreateRenderMaterialT)GetProcAddress((HMODULE)g_hRenderDLL, "CreateRenderMaterial");
		CreateRenderViewport = (CreateRenderViewportT)GetProcAddress((HMODULE)g_hRenderDLL, "CreateRenderViewport");
		CreateRenderVideoTexture = (CreateRenderVideoTextureT)GetProcAddress((HMODULE)g_hRenderDLL, "CreateRenderVideoTexture");
		if (CreateRender && CreateRenderPlatformInfo && CreateRenderFeatures && CreateRenderMaterial && CreateRenderViewport && CreateRenderVideoTexture)
			return 1;
		FreeRenderer();
	}
	return 0;
}

void MakeLauncherVisible()
{
	if (g_EngineAPI.StoreProfile)
		g_EngineAPI.StoreProfile();
	ShowHideCursor(1);
	ShowWindow(g_EngWnd, SW_HIDE);
	ClipCursor(0);
	g_EngineAPI.IN_DeactivateMouse();
	ClearAllStates();
}

//Written to be binary compatible with original Bond.exe
BOOLEAN Render_Init(char* mod)
{
	register BOOLEAN success = 0;
	success = LoadRenderDLL(mod);
	return success;
}

#define SHIWORD(width)   (*((short*)&(width)+1))

unsigned shitedi = 0;

//sub_401E20
void __cdecl ShowHideCursor(char a1)
{
	signed int v1; // esi@2
	int result; // eax@2
	signed int i; // ecx@2
	signed int v4; // esi@9
	signed int j; // ecx@9

	if (a1)
	{
		++shitedi;
		v1 = -1;
		result = ShowCursor(1);
		for (i = result; result < 0; i = result)
		{
			if (i == v1)
				break;
			result = abs(i);
			if (result > 5)
				break;
			v1 = i;
			result = ShowCursor(1);
		}
		if (i >= 2)
		{
			do
				result = ShowCursor(0);
			while (result >= 2);
		}
	}
	else
	{
		--shitedi;
		v4 = 1;
		result = ShowCursor(0);
		for (j = result; result >= 0; j = result)
		{
			if (j == v4)
				break;
			result = abs(j);
			if (result > 5)
				break;
			v4 = j;
			result = ShowCursor(0);
		}
		if (j <= -2)
		{
			do
				result = ShowCursor(1);
			while (result <= -2);
		}
	}
}

BYTE g_AltTabbedAway = 0; //byte_41BF2C
bool ForceCapture = 0; //Forces game to capture the window regardless of if mouse is in the client area

//sub_401F00
void ActivateGameWindowFromOther()
{
	struct tagRECT Rect;

	ShowWindow(g_EngWnd, SW_SHOWNORMAL);
	UpdateWindow(g_EngWnd);
	ShowHideCursor(0);

	bool ClickedIntoGame = false;

	if (g_bFullScreen)
	{
		ClipCursor(0);
		ReleaseCapture();
		ClickedIntoGame = true;
	}
	else
	{
		//Dylan's crap fix to make window borders allow you to click and drag around
		POINT cursorpos;
		GetCursorPos(&cursorpos);
		GetWindowRect(g_EngWnd, &Rect);
		if (border)
		{
			DWORD Style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
			HTHEME htheme = OpenThemeData(g_EngWnd, L"EDIT");
			int borderwidth = GetThemeSysSize(htheme, SM_CXBORDER);
			int borderheight = GetThemeSysSize(htheme, SM_CYSIZE) + GetThemeSysSize(htheme, SM_CXPADDEDBORDER) * 2;
			//Rect.bottom -= borderheight;
			Rect.top += borderheight;
			//Rect.right -= borderheight / 2;
			Rect.left += borderwidth;
			//AdjustWindowRectEx(&Rect, Style, FALSE, NULL);
		}
		if (ForceCapture || !border || (border && (cursorpos.x > Rect.left && cursorpos.y > Rect.top)))
		{
			//only capture mouse if we are within the box of the game, not the border
			ReleaseCapture();
			SetCapture(g_EngWnd);
			ClipCursor(&Rect);
			ClickedIntoGame = true;
			ForceCapture = false;
		}
		else
		{
			ShowHideCursor(1);
		}
		//Original nightfire code for this entire section
		//ReleaseCapture();
		//SetCapture(NightfireWindow);
		//GetWindowRect(NightfireWindow, &Rect);
		//ClipCursor(&Rect);
	}

	if (ClickedIntoGame)
	{
		g_EngineAPI.IN_ActivateMouse();
		//dword_41B62C(); //empty retn function :/
		g_EngineAPI.Keyboard_ReturnToGame();

		g_EngineAPI.SetMessagePumpDisableMode(0);
		g_EngineAPI.SetPauseState(0);
		g_AltTabbedAway = 0;
	}
}

#define _Key_Event(a, b) g_EngineAPI._Key_Event(a, b)
#define _Key_ClearStates() g_EngineAPI._Key_ClearStates()
#define _IN_ClearStates() g_EngineAPI._IN_ClearStates()
#define _IN_DeactivateMouse() g_EngineAPI._IN_DeactivateMouse()
#define _SetMessagePumpDisableMode(a) g_EngineAPI._SetMessagePumpDisableMode(a)
#define _SetPauseState(a) g_EngineAPI._SetPauseState(a)
#define _Key_CharEvent(a) g_EngineAPI._Key_CharEvent(a)
#define _IN_MouseEvent(a) g_EngineAPI._IN_MouseEvent(a)
#define _S_BlockSound() g_EngineAPI._S_BlockSound()
#define _S_UnblockSound() g_EngineAPI._S_UnblockSound()

//sub_401920
void Render_DoRender()
{
	DWORD v0 = *(DWORD*)(*(DWORD*)((DWORD)render + 4) + 4) + (DWORD)render + 4;
	(*(void(__thiscall**)(int))(*(DWORD*)v0 + 12))(v0);
}

//sub_401940
void Render_DoFlip()
{
	DWORD v0 = *(DWORD*)(*(DWORD*)((DWORD)render + 4) + 4) + (DWORD)render + 4;
	(*(void(__thiscall**)(int))(*(DWORD*)v0 + 16))(v0);
}

DWORD __cdecl GetCDKey(char *dest, unsigned int buffer_size, DWORD *new_length, BYTE *result)
{
	_snprintf(dest, buffer_size, "2712128302580");
	DWORD sz = strlen(dest);
	*new_length = sz;
	*result = 0;
	return sz;
}

int IsValidCD()
{
	return 1;
}

void CDAudio_Play(unsigned __int8 a1, int a2)
{
}
void CDAudio_Pause() { }
void CDAudio_Resume() { }
void CDAudio_Update() { }

//sub_401200
void __cdecl Launcher_PlayMovie(char* movie)
{
	DWORD v1 = (DWORD)g_guiAPI.GUI_Manager();
	DWORD v2 = (*(int(__thiscall**)(int, const char*))(*(DWORD*)v1 + 16))(v1, "gui/Scripts/MoviePlayer.txt");
	DWORD v3 = (DWORD)g_guiAPI.GUI_Manager();
	(*(void(__thiscall**)(int, int, int))(*(DWORD*)v3 + 36))(v3, v2, 1);
	DWORD v4 = (DWORD)g_guiAPI.GUI_Manager();
	(*(void(__thiscall**)(int, int, const char*, const char*))(*(DWORD*)v4 + 52))(v4, v2, "themovie", movie);
}

int __cdecl Launcher_OpenFrontEnd(int a1, int a2, int a3, int a4)
{
	int v4; // eax@2
	int v5; // esi@2
	DWORD* v6; // eax@3
	DWORD* v7; // eax@5
	int v8; // esi@5
	int v9; // ST0C_4@5
	DWORD* v10; // eax@5
	int v11; // ST0C_4@5
	DWORD* v12; // eax@5
	DWORD* v13; // eax@6
	DWORD* v14; // eax@7
	int v15; // eax@8
	int v16; // esi@8
	DWORD* v17; // eax@9
	DWORD* v18; // eax@10
	int v19; // esi@10
	DWORD* v20; // eax@11
	DWORD* v21; // eax@12
	int v22; // esi@12
	int v23; // ST0C_4@12
	DWORD* v24; // eax@12
	int v25; // ST0C_4@12
	DWORD* v26; // eax@12
	DWORD* v27; // eax@13
	int v28; // esi@13
	int v29; // ST0C_4@13
	DWORD* v30; // eax@13
	int v32; // [sp+4h] [bp-8h]@2
	int v33; // [sp+4h] [bp-8h]@5
	int v34; // [sp+4h] [bp-8h]@7
	int v35; // [sp+4h] [bp-8h]@8
	int v36; // [sp+4h] [bp-8h]@10
	char* off_419044 = "gui/Scripts/Mainmenu/Nightfire.txt";
	char* off_419048 = "gui/Scripts/Escapemenu/menu.txt";
	char* off_41904C = "gui/Scripts/Escapemenu/mpmenu.txt";


	if (a1)
	{
		v4 = ((int(__cdecl *)())g_guiAPI.GUI_Manager)();
		v5 = (*(int(__thiscall **)(int, char *))(*(DWORD *)v4 + 20))(v4, off_419044); //[0]
		if (v5)
		{
			v6 = g_guiAPI.GUI_Manager();
			(*(void(__thiscall **)(DWORD*, int, DWORD))(*(DWORD *)v6 + 36))(v6, v5, 0);
		}
		if (a2)
		{
			v7 = g_guiAPI.GUI_Manager();
			v8 = (*(int(__thiscall **)(DWORD*, char *))(*(DWORD *)v7 + 16))(v7, off_41904C);
			v10 = g_guiAPI.GUI_Manager();
			(*(void(__thiscall **)(DWORD*, int, const char *, DWORD))(*(DWORD *)v10 + 48))(v10, v8, "mpmenu_server", a3 != 0);
			v12 = g_guiAPI.GUI_Manager();
			(*(void(__thiscall **)(DWORD*, int, const char *, DWORD))(*(DWORD *)v12 + 48))(
				v12,
				v8,
				"mpmenu_teamplay",
				a4 != 0);
		}
		else
		{
			v13 = g_guiAPI.GUI_Manager();
			v8 = (*(int(__thiscall **)(DWORD*, char *))(*(DWORD *)v13 + 16))(v13, off_419048); //[0]
		}
		v14 = g_guiAPI.GUI_Manager();
		(*(void(__thiscall **)(DWORD*, int, signed int))(*(DWORD *)v14 + 36))(v14, v8, 1);
	}
	else
	{
		v15 = ((int(__cdecl *)())g_guiAPI.GUI_Manager)();
		v16 = (*(int(__thiscall **)(int, char *))(*(DWORD *)v15 + 20))(v15, off_419048); //[0]
		if (v16)
		{
			v17 = g_guiAPI.GUI_Manager();
			(*(void(__thiscall **)(DWORD*, int, DWORD))(*(DWORD *)v17 + 36))(v17, v16, 0);
		}
		v18 = g_guiAPI.GUI_Manager();
		v19 = (*(int(__thiscall **)(DWORD*, char *))(*(DWORD *)v18 + 20))(v18, off_41904C);
		if (v19)
		{
			v20 = g_guiAPI.GUI_Manager();
			(*(void(__thiscall **)(DWORD*, int, DWORD))(*(DWORD *)v20 + 36))(v20, v19, 0);
		}
		v21 = g_guiAPI.GUI_Manager();
		v22 = (*(int(__thiscall **)(DWORD*, char *))(*(DWORD *)v21 + 16))(v21, off_419044); //[0]
		v24 = g_guiAPI.GUI_Manager();
		(*(void(__thiscall **)(DWORD*, int, signed int))(*(DWORD *)v24 + 36))(v24, v22, 1);
		v26 = g_guiAPI.GUI_Manager();
		(*(void(__thiscall **)(DWORD*, int, const char *, DWORD))(*(DWORD *)v26 + 48))(
			v26,
			v22,
			"mm_can_exit",
			g_bDirectToConsole != 0);
	}
	v27 = g_guiAPI.GUI_Manager();
	v28 = (*(int(__thiscall **)(DWORD*, const char *))(*(DWORD *)v27 + 16))(v27, "gui/Scripts/Dialogs/dlg_wait.txt");
	v30 = g_guiAPI.GUI_Manager();
	return (*(int(__thiscall **)(DWORD*, int, signed int))(*(DWORD *)v30 + 36))(v30, v28, 1);
}


//sub_405060
DWORD GetOSPlatform()
{
	DWORD result;
	struct _OSVERSIONINFOA VersionInformation;

	VersionInformation.dwOSVersionInfoSize = 148;
	result = GetVersionExA(&VersionInformation);
	if (result)
	{
		result = VersionInformation.dwPlatformId - 1;
		if (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			PlatformID = 2;
		}
		else
		{
			result = VersionInformation.dwPlatformId - 2;
			if (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT)
				PlatformID = 3;
			else
				PlatformID = 1;
		}
	}
	return result;
}

//sub_4050c0
int getOSVersion()
{
	if (!PlatformID)
		GetOSPlatform();
	return PlatformID;
}

//sub_4011D0
void ClearAllStates()
{
	signed int v0 = 0; // esi@1

	do
		g_EngineAPI.Key_Event(v0++, 0);
	while (v0 < 345);
	g_EngineAPI.Key_ClearStates();
	g_EngineAPI.IN_ClearStates();
}

//sub_401EC0
void DeactivateFromGameWindow()
{
	ClipCursor(0);
	ReleaseCapture();
	g_EngineAPI.IN_DeactivateMouse();
	ShowHideCursor(1);
	if (!g_bRunWhileAltTabbed)
	{
		g_EngineAPI.SetMessagePumpDisableMode(g_bFullScreen); //< setting this to 0 disables networking, we don't want that
		g_EngineAPI.SetPauseState(1);
	}
	g_AltTabbedAway = 1;
}

//sub_401060
void __cdecl AppActivate(BOOLEAN active)
{
	if (active)
	{
		ActivateGameWindowFromOther();
		ClearAllStates();
		ClearAllStates(); //Gearbox, why do you call this twice..
	}
	else
	{
		DeactivateFromGameWindow();
		ClearAllStates();
		ClearAllStates(); //Gearbox, why do you call this twice..
	}
}


//unsigned NO_INPUT = 0; //dword_41BF30
#ifdef ORIGINAL_NF_WINPROC
LONG WINAPI HLEngineWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT v4; // ebx@1
	int result; // eax@2
	int v6; // eax@27
	signed int v7; // eax@50
	signed int v8; // eax@57
	signed int v9; // [sp-14h] [bp-5Ch]@5
	signed int v10; // [sp-8h] [bp-50h]@27
	struct tagPAINTSTRUCT Paint; // [sp+8h] [bp-40h]@15

	v4 = 0;
	if (Msg == 953)
		return 0;
	if (Msg == clipboardformat)
	{
		if (SHIWORD(wParam) <= 0)
		{
			_Key_Event(338, 1);
			v9 = 338;
		}
		else
		{
			_Key_Event(339, 1);
			v9 = 339;
		}
		_Key_Event(v9, 0);
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
	if (Msg <= 0x104)
	{
		if (Msg == 260)
			goto LABEL_66;
		if (Msg <= 0x1C)
		{
			if (Msg == 28)
			{
				AppActivate(wParam == 1);
				return 0;
			}
			switch (Msg)
			{
			case 5u:
				if (wParam != 1)
					return v4;
				MoveWindow(hWnd, 0, -20, 0, 20, 0);
				return 0;
			case 0xFu:
				BeginPaint(hWnd, &Paint);
				EndPaint(hWnd, &Paint);
				return 0;
			case 0x10u:
				if (!NO_INPUT)
					g_EngineAPI.ConsoleBuffer_AddText("quit prompt\n");
				return 0;
			default:
				return DefWindowProcA(hWnd, Msg, wParam, lParam);
			case 3u:
				return v4;
			}
			return v4;
		}
		if (Msg == 256)
		{
		LABEL_66:
			if (!NO_INPUT)
			{
				v10 = 1;
				v6 = sub_401170(lParam);
			LABEL_45:
				_Key_Event(v6, v10);
				return 0;
			}
		}
		else
		{
			if (Msg != 257)
			{
				if (Msg == 258)
				{
					if (wParam >= 0x20 || wParam == 13)
					{
						_Key_CharEvent(wParam);
						return 0;
					}
					return v4;
				}
				return DefWindowProcA(hWnd, Msg, wParam, lParam);
			}
		LABEL_43:
			if (!NO_INPUT)
			{
				v10 = 0;
				v6 = sub_401170(lParam);
				goto LABEL_45;
			}
		}
		return v4;
	}
	if (Msg > 0x205)
	{
		switch (Msg)
		{
		case 0x20Au:
			if (SHIWORD(wParam) <= 0)
			{
				_Key_Event(338, 1);
				_Key_Event(338, 0);
				result = 0;
			}
			else
			{
				_Key_Event(339, 1);
				_Key_Event(339, 0);
				result = 0;
			}
			return result;
		case 0x207u:
		case 0x208u:
			goto LABEL_50;
		case 0x20Bu:
		case 0x20Cu:
			v8 = 0;
			if (wParam & 0x20)
				v8 = 8;
			if (wParam & 0x40)
				v8 |= 0x10u;
			_IN_MouseEvent(v8);
			return 1;
		default:
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		}
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
	if (Msg < 0x204)
	{
		switch (Msg)
		{
		case 0x112u:
			if (wParam == 61760 || wParam == 61536)
				return 0;
			if (!NO_INPUT)
				_S_BlockSound();
			v4 = DefWindowProcA(hWnd, Msg, wParam, lParam);
			if (!NO_INPUT)
			{
				_S_UnblockSound();
				return v4;
			}
			break;
		case 0x105u:
			if ((wParam == 9 || wParam == 27) && ((unsigned int)lParam >> 29) & 1 && ((unsigned int)lParam >> 31) & 1)
			{
				g_AltTabbedAway = 1;
				_IN_DeactivateMouse();
			}
			goto LABEL_43;
		case 0x200u:
		case 0x201u:
		case 0x202u:
			goto LABEL_50;
		default:
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case 0x106u:
			return v4;
		}
		return v4;
	}
LABEL_50:
	v7 = 0;
	if (wParam & 1)
		v7 = 1;
	if (wParam & 2)
		v7 |= 2u;
	if (wParam & 0x10)
		v7 |= 4u;
	_IN_MouseEvent(v7);
	return 0;
}
#else
LONG WINAPI HLEngineWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	int key;
	signed int mouseupstate = 0;
	signed int mousedownstate = 0;

	if (Msg == MM_MCINOTIFY)
		return 0;
	if (Msg == clipboardformat)
	{
		signed int key2;
		if (SHIWORD(wParam) <= 0)
		{
			g_EngineAPI.Key_Event(338, 1);
			key2 = 338;
		}
		else
		{
			g_EngineAPI.Key_Event(339, 1);
			key2 = 339;
		}
		g_EngineAPI.Key_Event(key2, 0);
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
	if (Msg <= WM_SYSKEYDOWN)
	{
		if (Msg <= WM_MOUSEACTIVATE) //Original nightfire is if (Msg <= WM_ACTIVATEAPP)
		{
			switch (Msg)
			{
			case WM_PAINT:
				struct tagPAINTSTRUCT Paint;
				BeginPaint(hWnd, &Paint);
				EndPaint(hWnd, &Paint);
				return 0;
			case WM_MOUSEACTIVATE:
			case WM_ACTIVATEAPP:
				AppActivate(wParam == WA_ACTIVE);
				return 0;
			case WM_SIZE:
				if (wParam != SIZE_MINIMIZED)
					return result;
				MoveWindow(hWnd, 0, -20, 0, 20, 0);
				return 0;
			case WM_CLOSE:
				CLOSEGAME:
				//if (!NO_INPUT)
					g_EngineAPI.ConsoleBuffer_AddText("quit prompt\n");
				return 0;
			case WM_CANCELMODE: //Dylan added
				AppActivate(0);
				return DefWindowProcA(hWnd, Msg, wParam, lParam);
			case WM_MOVE:
				g_EngineAPI.SCR_SetWindowPosition((int)LOWORD(lParam), (int)HIWORD(lParam)); //dylan added
				return result;
			default:
				return DefWindowProcA(hWnd, Msg, wParam, lParam);
			}
			return result;
		}
		switch (Msg)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (wParam == VK_LWIN || wParam == VK_RWIN) //Dylan added, this is the windows keys checks
			{
				AppActivate(0);
				return DefWindowProcA(hWnd, Msg, wParam, lParam);
			}
			//if (!NO_INPUT)
			//{
				key = MapKey(lParam);
				//KEYEVENT:
				g_EngineAPI.Key_Event(key, 1);
				return 0;
			//}
			//return result;
		case WM_KEYUP:
		SENDKEYUP:
			//if (!NO_INPUT)
			//{
				key = MapKey(lParam);
				//goto KEYEVENT;
				g_EngineAPI.Key_Event(key, 0);
				return 0;
			//}
			return result;
		case WM_CHAR:
			if (wParam >= 0x20 || wParam == 13)
			{
				g_EngineAPI.Key_CharEvent(wParam);
				return 0;
			}
			return result;
		case WM_NCACTIVATE: //Dylan added
			AppActivate(wParam == WA_ACTIVE);
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		//default:
			//return DefWindowProcA(hWnd, Msg, wParam, lParam);
		}
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
	if (Msg > WM_RBUTTONUP)
	{
		switch (Msg)
		{
		case  WM_MOUSEWHEEL:
			if (SHIWORD(wParam) <= 0)
			{
				g_EngineAPI.Key_Event(338, 1);
				g_EngineAPI.Key_Event(338, 0);
			}
			else
			{
				g_EngineAPI.Key_Event(339, 1);
				g_EngineAPI.Key_Event(339, 0);
			}
			return 0;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			goto Mouse;
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
			if (wParam & 0x20)
				mouseupstate = 8;
			if (wParam & 0x40)
				mouseupstate |= 0x10u;
			g_EngineAPI.IN_MouseEvent(mouseupstate);
			return 1;
		default:
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		}
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
	if (Msg < WM_RBUTTONDOWN)
	{
		switch (Msg)
		{
		case WM_SYSCOMMAND:
#if 0 //Original nightfire behavior
			if (wParam == SC_SCREENSAVE || wParam == SC_CLOSE)
				return 0;
#else
	  //Fix ability to close game from windows without ending process
			if (wParam == SC_SCREENSAVE)
				return 0;
			if (wParam == SC_CLOSE)
				goto CLOSEGAME;
#endif
			//if (!NO_INPUT)
				g_EngineAPI.S_BlockSound();
			result = DefWindowProcA(hWnd, Msg, wParam, lParam);
			//if (!NO_INPUT)
			//{
				g_EngineAPI.S_UnblockSound();
				return result;
			//}
			break;
		case WM_SYSKEYUP:
			if ((wParam == VK_TAB || wParam == VK_ESCAPE) && ((unsigned int)lParam >> VK_NONCONVERT) & 1 && ((unsigned int)lParam >> VK_MODECHANGE) & 1)
			{
				g_AltTabbedAway = 1;
				g_EngineAPI.IN_DeactivateMouse();
			}
			goto SENDKEYUP;
#if 0
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			goto Mouse;
#else
		case WM_LBUTTONUP:
			//fix clip cursor with bordered window
			if (g_AltTabbedAway && !g_bFullScreen && border)
			{
				ForceCapture = true;
				AppActivate(TRUE);
				return 0;
			}
			goto Mouse;
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
			goto Mouse;
#endif
		default:
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		case WM_SYSCHAR:
			return result;
		}
		return result;
	}
Mouse:
	if (wParam & 1)
		mousedownstate = 1;
	if (wParam & 2)
		mousedownstate |= 2u;
	if (wParam & 0x10)
		mousedownstate |= 4u;
	g_EngineAPI.IN_MouseEvent(mousedownstate);
	return 0;
}
#endif

unsigned dword_41B6FC = 0;
//Created to be binary compatible with original bond.exe
#include "../utils/procinfo/procinfo.h"

char Table[96];
DWORD dword_41B6A0;
DWORD dword_41B6A4;
BYTE byte_41B6A8;
DWORD dword_41B6C8;
BYTE byte_41BF38;
DWORD dword_41B6EC;
DWORD dword_41B6E0;
DWORD dword_41B6DC;

char* currentdir;
char MovieBuffer[1024];
char *NullString = "";
char* GetCDPath()
{
	HANDLE File; 
	char FileName[MAX_PATH];
	WIN32_FIND_DATAA FindFileData;

	if (!g_bDedicated && GetCurrentDirectoryA(1024, MovieBuffer))
	{
		_snprintf(FileName, MAX_PATH, "%s\\bond\\%s", MovieBuffer, "movies\\m1_intro.avi");
		File = FindFirstFileA(FileName, &FindFileData);
		if (File != INVALID_HANDLE_VALUE)
		{
			FindClose(File);
			return MovieBuffer;
		}
		else
		{
			//todo: cd prompt
			return NullString;
		}
	}
	return NullString;
}

LSTATUS __cdecl GetRegionInfo(char* region)
{
	LSTATUS result;
	HKEY phkResult;
	DWORD cbData;
	DWORD Type;
	DWORD dwDisposition;
	char Data[128];

	result = RegCreateKeyExA(
		HKEY_LOCAL_MACHINE,
		"SOFTWARE\\GEARBOX\\NIGHTFIRE",
		0,
		0,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		0,
		&phkResult,
		&dwDisposition);
	if (!result)
	{
		cbData = 16;
		if (!RegQueryValueExA(phkResult, "region", 0, &Type, reinterpret_cast<LPBYTE>(&Data), &cbData) && Type == REG_SZ)
		{
			*(char*)(region) = Data[0];
			*(char*)(region + 1) = Data[1];
			*(char*)(region + 2) = 0;
		}
		result = RegCloseKey(phkResult);
	}
	return result;
}

//static const char* defaultregion = "EN";
//static const char* nullregion = "";

void(*Host_InitializeGameDLL) (void) = (void(*)(void))0x430597D0;