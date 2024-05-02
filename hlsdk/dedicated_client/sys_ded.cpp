//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <windows.h> 
#include <Uxtheme.h>
#include <io.h>
//#define snprintf _snprintf
//#define vsnprintf _vsnprintf

//#define FRAMERATE // define me to have hlds print out what it thinks the framerate is

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "sys_ded.h"
#include "conproc.h"
#include "dedicated.h"
#include "exefuncs.h"

#include "dll_state.h"
#include "enginecallback.h"
#include "../utils/procinfo/procinfo.h"
#include "../public/platformdll.h"
#include "globals.h"
#include "console.h"
#include "keyboard.h"
#include "video.h"
#include "timer.h"
#include "MinHook/MinHook.h"
#include "fixes.h"
#include "MetaHook.h"

extern void MakeLauncherVisible();


void *Sys_GetProcAddress( long library, const char *name ) 
{
	return ( void * )GetProcAddress( (HMODULE)library, name );
}

long Sys_LoadLibrary( const char *lib ) 
{
	void *hDll = NULL;
	hDll = ::LoadLibrary( lib );
	return (long)hDll;
}

void Sys_FreeLibrary( long library ) 
{
	if ( !library )
		return;
	::FreeLibrary( (HMODULE)library );
}

int Sys_GetExecutableName( char *out ) 
{
	if ( !::GetModuleFileName( ( HINSTANCE )GetModuleHandle( NULL ), out, 256 ) )
		return 0;
	return 1;
}

__declspec(noreturn) void ErrorBox(const char* msg, const char* filename)
{
	char tmp[2052];
	strncpy_s(tmp, msg, 2048);
	if (filename)
	{
		auto f3 = fopen(filename, "rt");
		if (f3)
		{
			int v4 = fileno(f3);
			int file_length = filelength(v4);
			if (file_length)
			{
				file_length = min(file_length, 2047);
				memset(tmp, 0, 2048);
				fread(tmp, file_length, 1, f3);
			}
			fclose(f3);
		}
	}
	MessageBoxA(0, tmp, "ERROR", 48);
	exit(-1);
}

__declspec(noreturn) void ErrorMessage( int level, const char *msg )
{
	DeactivateFromGameWindow();
	MakeLauncherVisible();
	if (g_mouseParamsValid)
		SystemParametersInfoA(4, 0, &g_originalMouseParameters, 0);
	MessageBoxA(0, msg, "Error", MB_ICONEXCLAMATION);
	exit(1);
}

void UpdateStatus( int force )
{
	static double tLast = 0.0;
	double	tCurrent;
	char	szPrompt[256];
	int		n, spec, nMax;
	char	szMap[32];
	float	fps, inKB, outKB;

	if ( !g_EngineAPI.Host_GetHostInfo )
		return;

	tCurrent = (float)( timeGetTime() / 1000.0f );

	//engineapi.Host_GetHostInfo( &fps, &n, &spec, &nMax, szMap );
	g_EngineAPI.Host_GetHostInfo( &fps, &n, &spec, &nMax, szMap, sizeof(szMap), &inKB, &outKB );

	if ( !force ) 
	{
		if ( ( tCurrent - tLast ) < 0.5f )
			return;
	}
	//__asm mov eax, n
	//__asm mov dword ptr ds:[0x44b82e6c], eax //Set numclients to an address the Nightfire dedicated server patch uses to sleep when there's no players
	tLast = tCurrent;

	//snprintf( szPrompt, sizeof(szPrompt), "%.1f fps %2i(%2i spec)/%2i on %16s", (float)fps, n, spec, nMax, szMap);
	_snprintf( szPrompt, sizeof(szPrompt), "%.1f fps %2i/%2i on %16s in: %.1fK/s out: %.1fK/s", (float)fps, n, nMax, szMap, inKB, outKB);

	WriteStatusText( szPrompt );
}

/*
==============
Sys_Printf

Engine is printing to console
==============
*/
void Sys_Printf(char *fmt, ...) 
{
	// Dump text to debugging console.
	va_list argptr;
	char szText[1024];

	va_start (argptr, fmt);
	vsnprintf (szText, sizeof(szText), fmt, argptr);
	va_end (argptr);

	// Get Current text and append it.
	Sys_ConsoleOutput( szText );
}

void DED_Sys_Printf(char* fmt, ...)
{
	// Dump text to debugging console.
	va_list argptr;
	char szText[1024];

	va_start(argptr, fmt);
	vsnprintf(szText, sizeof(szText), fmt, argptr);
	va_end(argptr);

	// Get Current text and append it.
	Sys_ConsoleOutput(szText);
}

/*
==============
CheckParm

Search for psz in command line to .exe, if **ppszValue is set, then the pointer is
 directed at the NEXT argument in the command line
==============
*/
char *CheckParm(const char *psz, char **ppszValue) {
	int i;
	static char sz[128];
	char *pret;

	if (!s_szCmdLine)
		return NULL;

	pret = strstr(s_szCmdLine, psz );

	// should we return a pointer to the value?
	if (pret && ppszValue) {
		char *p1 = pret;
		*ppszValue = NULL;

		while ( *p1 && (*p1 != 32))
			p1++;

		if (p1 != 0) {
			char *p2 = ++p1;

			for ( i = 0; i < 128; i++ ) {
				if ( !*p2 || (*p2 == 32))
					break;
				sz[i] = *p2++;
			}

			sz[i] = 0;
			*ppszValue = &sz[0];		
		}	
	}

	return pret;
}

void ChangeGameDirectory(const char* a1) 
{ 
}

void CreateEngineInterface( void ) 
{
	memset( &s_LauncherAPI, 0, sizeof( s_LauncherAPI ) );
	
	// Function pointers used by dedicated server
	s_LauncherAPI.version = 1;
	s_LauncherAPI.isDedicated = g_bDedicated;
	s_LauncherAPI.iSizeOfStruct = sizeof( s_LauncherAPI );
	s_LauncherAPI.AppActivate = AppActivate;
	s_LauncherAPI.ChangeGameDirectory = ChangeGameDirectory;

#ifndef SWDS
	if (!g_bDedicated)
	{
		s_LauncherAPI.render = render;
		s_LauncherAPI.renderfeatures = renderfeatures;
		s_LauncherAPI.renderplatforminfo = renderplatforminfo;
		s_LauncherAPI.CreateRenderMaterial = CreateRenderMaterial;
		s_LauncherAPI.CreateRenderViewport = CreateRenderViewport;
		s_LauncherAPI.Render_DoRender = Render_DoRender;
		s_LauncherAPI.Render_DoFlip = Render_DoFlip;
		s_LauncherAPI.Sys_Printf = DED_Sys_Printf; 
		s_LauncherAPI.GetCDKey = GetCDKey;
		s_LauncherAPI.IsValidCD = IsValidCD;
		s_LauncherAPI.CDAudio_Play = CDAudio_Play;
		s_LauncherAPI.CDAudio_Pause = CDAudio_Pause;
		s_LauncherAPI.CDAudio_Resume = CDAudio_Resume;
		s_LauncherAPI.CDAudio_Update = CDAudio_Update;
		s_LauncherAPI.ErrorMessage = ErrorMessage;
		s_LauncherAPI.Launcher_PlayMovie = Launcher_PlayMovie;
		s_LauncherAPI.Launcher_OpenFrontEnd = Launcher_OpenFrontEnd;
		s_LauncherAPI.getOSVersion = getOSVersion;
	}
	else
#endif
	{
		s_LauncherAPI.Sys_Printf = Sys_Printf;
		s_LauncherAPI.ErrorMessage = ErrorMessage;
	}
}

DWORD WINAPI test(__in LPVOID lpParameter)
{
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	for (;;)
	{
		//const char* CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
		//write(1, CLEAR_SCREEN_ANSI, 12);
		ConsoleVariable* master = reinterpret_cast<ConsoleVariable* (*)(const char*)>(0x43045AC0)("snd_mastervolume");
		ConsoleVariable* music = reinterpret_cast<ConsoleVariable * (*)(const char*)>(0x43045AC0)("snd_musicvolume");
		static float lastmaster = -1.0f;
		static float lastmusic = -1.0f;
		if (master && music)
		{
			float newmaster = master->getValue();
			float newmusic = music->getValue();
			if (newmaster != lastmaster)
				printf("new master: %f\n", newmaster);
			if (newmusic != lastmusic)
				printf("new music: %f\n", newmusic);
			lastmaster = newmaster;
			lastmusic = newmusic;
		}
	}
}

HINSTANCE g_hDLLInstance;
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) 
{
	unsigned width = 800;
	unsigned height = 600;
	unsigned depth = 32;
	BOOLEAN nosound = 0;
	unsigned aa = 0;
	unsigned vsync = 1;
	unsigned hwinfo = 0;
	unsigned refreshrate = 0;
	const char* cdpath;
	bool nomenu = 0;
	int		iret = -1;
	g_hDLLInstance = hInstance;
	// Store off command line for argument searching
	strncpy_s(s_szCmdLine, lpszCmdLine, 2048);
	MH_Initialize();

	if (!g_bDedicated)
	{
		char* p;
		char region[3];
		strcpy(region, Languages[0]);
		GetRegionInfo(region);
		if (strlen(region) == 0)
			strcpy(region, Languages[0]);

		if (CheckParm("-layout", &p) && p)
		{
			strcpy(region, p);
			//Missing call here, but we aren't using the same Gearbox code anyway so this should work.
			if (!SetKeyboardLayout(region))
			{
				strcpy(region, Languages[0]);
			}
		}

		SetKeyboardLayout(region);
	}

	if (CheckParm("-debug"))
		Sleep(15000);

	char* p;

	if ((CheckParm("-heap", &p)) && p)
	{
		int heap = atoi(p);
		if (heap > 0)
			s_HeapSize = heap; //Heap in MB
	}
	
	/*
	if ((CheckParm("-heapsize", &p)) && p)
	{
		int hunk = atoi(p);
		if (hunk > 0)
			s_HunkSize = hunk * 1024; //Bits to Bytes
	}
	*/

	if ((CheckParm("-hunk", &p)) && p)
	{
		int hunk = atoi(p);
		if (hunk > 0)
			s_HunkSize = hunk;// * (1024 * 1024); //MB to Bytes
	}

	if (CheckParm("-runwhilealttabbed"))
		g_bRunWhileAltTabbed = 1;

	if (CheckParm("-dedicated"))
		g_bDedicated = 1;

	if (CheckParm("-nometaaudio"))
		g_bNoMetaAudio = 1;

	if (CheckParm("-fullscreen"))
		g_bFullScreen = 1;

	if (CheckParm("-windowed"))
		g_bFullScreen = 0;

	if (CheckParm("-window"))
		g_bFullScreen = 0;

	if (CheckParm("-toconsole") || CheckParm("-novid"))
	{
		g_bDirectToConsole = 1;
		nomenu = 1;
	}

	if (CheckParm("-border"))
		border = 1;

	if (CheckParm("-noborder") || CheckParm("-borderless"))
		border = 0;

	linkPlatformDll("platform.dll");

	GbxResult result = { 0,0 };
	g_pNightfirePlatformFuncs->MembankInitialize(result, s_HeapSize << 20, nullptr);
	g_pNightfirePlatformFuncs->GbxResultDestructor(result);
	g_pNightfirePlatformFuncs->MembankSetEnsureStatus(result, result, 1);
	g_pNightfirePlatformFuncs->GbxResultDestructor(result);
	CoInitialize(NULL);

	if (!g_bDedicated)
	{
		// prevent multiple instances
#ifdef _DEBUG
		if (!CheckParm("-allowmultipleinstances"))
		{
#endif
			g_pMultipleInstanceMutex = new HANDLE;
			*g_pMultipleInstanceMutex = CreateMutexA(0, true, "{7B9C6EAB-E1DD-4FE5-AE85-57A7CCDD608A}");

			if (GetLastError() == ERROR_ALREADY_EXISTS || !*g_pMultipleInstanceMutex)
			{
				CloseHandle(*g_pMultipleInstanceMutex);
				exit(EXIT_FAILURE);
			}
#ifdef _DEBUG
		}
#endif
	}

	clipboardformat = RegisterWindowMessageA("MSWHEEL_ROLLMSG");
	g_mouseParamsValid = SystemParametersInfoA(SPI_GETMOUSE, NULL, &g_originalMouseParameters, NULL);

	if (!CheckParm("-safe") && !g_bDedicated)
		LoadDisplaySettings(width, height, depth, refreshrate, aa, vsync);

	if (CheckParm("-nosound"))
		nosound = 1;

	if (CheckParm("-width", &p) && p)
		width = atoi(p);

	if (CheckParm("-height", &p) && p)
		height = atoi(p);

	if (CheckParm("-depth", &p) && p)
		depth = atoi(p);

	if (CheckParm("-aa", &p) && p)
		aa = atoi(p);

	if (CheckParm("-vsync", &p) && p)
		vsync = atoi(p);

	if (CheckParm("-hwinfo"))
		hwinfo = 1;

	if (CheckParm("+connect") || CheckParm("+map"))
		nomenu = 1;

	if (!width)
		width = 800;

	if (!height)
		height = 600;

	if (!depth)
		depth = 32;

	cdpath = GetCDPath();
	UpdateWindowGlobals(width, height);
	Init_Timer();
	InitTime();
	timeBeginPeriod(1); //Set timer resolution
	
	linkEngineDll("engine.dll");

	if (!g_bDedicated)
		linkGUIDll("gui.dll");

	g_EngineAPI.GameSetState(DLL_ACTIVE);
	g_EngineAPI.IN_ActivateMouse();

	bool ENABLE_NVIDIA_FIX = false;

	if (g_bDedicated)
	{
		CreateConsoleWindow();
		SetConsoleTitle(TEXT("Nightfire Dedicated Server"));
	}
	else
	{
		//if (MessageBox(NULL, "Do you wish to enable the NVIDIA graphics corruption fix? If your game crashes after loading a level, disable compatibility mode on Bond2.exe or click 'No' on this message. We are still trying to come up with a workaround for this NVIDIA driver bug. Sorry for any inconvenience.", "Nightfire", MB_YESNO) == IDYES)
			//ENABLE_NVIDIA_FIX = true;

		if (!Render_Init("RenderD3D.dll"))
			ErrorBox("Failed to load RenderD3D.dll", 0);

		if (!Render_Open(g_bFullScreen, g_WindowRect.right - g_WindowRect.left, g_WindowRect.bottom - g_WindowRect.top, depth, refreshrate, aa, vsync, hwinfo))
			ErrorBox("Failed to initialize Direct3D\n\nPlease ensure DirectX 8.1 or newer is installed, and video drivers are up to date.", "d3df.txt");

		if (!g_bFullScreen)
		{
			MoveWindow(g_EngWnd, g_WindowX, g_WindowY, (g_WindowRect.right - g_WindowRect.left) - offsetwidth, (g_WindowRect.bottom - g_WindowRect.top) - offsetheight, TRUE);
			ReleaseCapture();
			SetCapture(g_EngWnd);
			RECT prect;
			GetWindowRect(g_EngWnd, &prect);
			if (border)
			{
				HTHEME htheme = OpenThemeData(g_EngWnd, L"EDIT");
				int borderwidth = GetThemeSysSize(htheme, SM_CXBORDER);
				int borderheight = GetThemeSysSize(htheme, SM_CYSIZE) + GetThemeSysSize(htheme, SM_CXPADDEDBORDER) * 2;
				prect.top += borderheight;
				prect.left += borderwidth;
				prect.right -= borderwidth;
				prect.bottom -= borderheight;
			}
			ClipCursor(&prect);
		}

		ClearAllStates();
		g_EngineAPI.GameSetBackground(false);
	}

	//Allocate the hunk
	MembankUsageType_Class blah;
	memset(&blah, 0, sizeof(MembankUsageType_Class));
	g_pNightfirePlatformFuncs->MembankUsageType(&blah, 1);
	s_MemSize = s_HunkSize << 20;
	s_pMemBase = (unsigned char*)g_pNightfirePlatformFuncs->mallocx(s_HunkSize << 20);
	g_pNightfirePlatformFuncs->MembankUsageTypeDestructor(&blah);

	CreateEngineInterface();
	CreateGUIInterface();

	// dylan test stuff
	//CreateThread(0, 0, test, 0, 0, 0);

	g_EngineAPI.GameInit(s_szCmdLine, s_pMemBase, s_MemSize, &s_LauncherAPI, &g_EngWnd, false, nosound, cdpath);

	g_EngineAPI.SCR_SetWindowPosition(g_WindowX, g_WindowY);
	g_EngineAPI.SetStartupMode(1);
	g_EngineAPI.ForceReloadProfile();

	if (!g_bDedicated)
	{
		ShowWindow(g_EngWnd, SW_SHOW);
		g_EngineAPI.IN_ActivateMouse();
		ShowCursor(FALSE);

		if (!nomenu)
		{
			DWORD* result = g_guiAPI.GUI_Manager();
			DWORD* result2 = ((DWORD * (__thiscall*)(DWORD*, const char*))(*(DWORD*)((*(DWORD*)(DWORD)result) + 0x10)))(result, "gui/Scripts/intro.txt");
			result = g_guiAPI.GUI_Manager();
			((void(__thiscall*)(DWORD*, DWORD*, int))(*(DWORD*)((*(DWORD*)(DWORD)result) + 0x24)))(result, result2, 1);
		}
	}

	ProcessCommands(lpszCmdLine);
	g_EngineAPI.SetStartupMode(0);

	double time = Shell_Sys_FloatTime();

#ifndef SWDS
	if (g_bDedicated)
#endif
	{
		int iState = DLL_ACTIVE;
		int iState2 = DLL_ACTIVE;
		for (;;)
		{
			int dummy_state;
			g_EngineAPI.GameSetBackground(0);
			double new_time = Shell_Sys_FloatTime();
			double delta_time = new_time - time;
			time = new_time;
			iState = g_EngineAPI.Host_Frame((float)delta_time, DLL_ACTIVE, &iState2);
			g_EngineAPI.GetEngineState();
			Fix_GameDLL_Bugs();

			if (iState2 == DLL_TRANS || iState == DLL_CLOSE)
				break;

			// half life console stuff
			MSG msg;
			int bDone = 0;
			while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if (!::GetMessage(&msg, NULL, 0, 0))
				{
					bDone = 1;
					break;
				}
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			if (bDone)
				break;
			ProcessConsoleInput();
			UpdateStatus(0  /* don't force */);
		}
	}
#ifndef SWDS
	else
	{
		if (CheckParm("-rinput"))
			linkRInput();
		HookFuncs(ENABLE_NVIDIA_FIX);
		Enable_Dlights_by_default();
		int iState = DLL_ACTIVE;
		int iState2 = DLL_ACTIVE;
		for (;;)
		{
			double new_time = Shell_Sys_FloatTime();
			double delta_time = new_time - time;
			time = new_time;
			int dummy_state = DLL_ACTIVE;
			g_EngineAPI.GameSetBackground(0);
			Fix_GameDLL_Bugs();

			if (g_AltTabbedAway && !g_bRunWhileAltTabbed)
			{
				Sleep(1);
				iState = g_EngineAPI.Host_Frame((float)delta_time, DLL_ACTIVE, &dummy_state);
				DWORD der1 = *(DWORD*)((DWORD)render + 4);
				DWORD der2 = *(DWORD*)(der1 + 4);
				DWORD adr = (DWORD)render + der2 + 4;
				DWORD der3 = *(DWORD*)adr;
				((void(__thiscall*)(DWORD))* (DWORD*)(der3 + 0x10))(adr);

				PostFrame();
			}
			else
			{
				POINT cursorpos;
				GetCursorPos(&cursorpos);
				ScreenToClient(g_EngWnd, &cursorpos);
				DWORD* result = g_guiAPI.GUI_Manager();
				((void(__thiscall*)(DWORD*, DWORD, DWORD))* (DWORD*)(*result + 0x40))(result, (DWORD)cursorpos.x, (DWORD)cursorpos.y);
				iState = g_EngineAPI.Host_Frame((float)delta_time, DLL_ACTIVE, &iState2);
				g_EngineAPI.GetEngineState();

				PostFrame();
			}

			if (iState2 == DLL_TRANS || iState == DLL_CLOSE)
				break;
		}

		unlinkRInput();
	}
#endif

	timeEndPeriod(1);
	
	g_EngineAPI.GameSetState(DLL_PAUSED);
	DeactivateFromGameWindow();

#ifndef SWDS
	if (!g_bDedicated)
		g_EngineAPI.IN_DeactivateMouse();
#endif

	ClearAllStates();

	g_EngineAPI.GameSetState(DLL_CLOSE);

#ifndef SWDS
	if (!g_bDedicated)
		g_guiAPI.GUI_Shutdown();
#endif

	g_EngineAPI.Host_Shutdown();
	DestroyConsoleWindow();
	g_pNightfirePlatformFuncs->freex(s_pMemBase);
	s_pMemBase = NULL;
	s_MemSize = 0;

#ifndef SWDS
	if (!g_bDedicated)
	{
		SaveVideoSettingsToRegistry();
		Render_Close();
		Render_Shutdown();
		unlinkGUIDll(); //original game had if (!dedicated) on this line...
	}
#endif

	unlinkEngineDll();

	if (g_pMultipleInstanceMutex)
	{
		CloseHandle(*g_pMultipleInstanceMutex);
		delete g_pMultipleInstanceMutex;
	}
	g_pMultipleInstanceMutex = nullptr;

	g_pNightfirePlatformFuncs->MembankShutdown(result); //Gearbox Function
	g_pNightfirePlatformFuncs->GbxResultDestructor(result);
	FreeLibrary((HMODULE)g_platformDllHinst);

	return 0;
}

