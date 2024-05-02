#include <Windows.h>
#include <Uxtheme.h>
#include "video.h"
#include <string>
#include "globals.h"
#include "resource.h"


//sub_00401D50
BOOLEAN Render_Close(void)
{
	if (render)
	{
		if (!renderplatforminfo)
			goto LABEL_6;
		(*(void(__thiscall*)(void*, DWORD))(*(DWORD*)(*(DWORD*)render + 24)))(render, (DWORD)renderplatforminfo);
	}
	if (renderplatforminfo)
	{
		(*(void(__thiscall*)(DWORD*))(*(DWORD*)(*(DWORD*)renderplatforminfo + 16)))(renderplatforminfo);
		renderplatforminfo = 0;
	}
LABEL_6:
	if (renderfeatures)
	{
		(*(void(__thiscall*)(void*))(*(DWORD*)(*(DWORD*)renderfeatures + 104)))(renderfeatures);
		renderfeatures = 0;
	}
	if (render)
	{
		(*(void(__thiscall*)(void*))(*(DWORD*)(*(DWORD*)render + 124)))(render);
		render = 0;
	}
	return 1;
}

int ResChangeHandler()
{
	int result = 0;
	DWORD features = (DWORD)CreateRenderFeatures(render);
	if (features)
	{
		DWORD featuresdereferenced = *(DWORD*)features;
		((void(__thiscall*)(void*, DWORD))(*(DWORD*)(*(DWORD*)render + 0x0c)))(render, features);
		DWORD newwidth = ((DWORD(__thiscall*)(DWORD))(*(DWORD*)(featuresdereferenced + 0x18)))(features);
		DWORD newheight = ((DWORD(__thiscall*)(DWORD))(*(DWORD*)(featuresdereferenced + 0x20)))(features);
		UpdateWindowGlobals(newwidth, newheight);

		//FIXME: why was this commented
		BOOLEAN notactive = ((BOOLEAN(__thiscall*)(DWORD))(*(DWORD*)(featuresdereferenced + 0x50)))(features);
		if (!notactive)
		{
			MoveWindow(g_EngWnd, g_WindowX, g_WindowY, g_WindowRect.right - g_WindowRect.left, g_WindowRect.bottom - g_WindowRect.top, TRUE);
			//MoveWindow(NightfireWindow, realwidth - offsetwidth, realheight - offsetheight, realwidth, realheight, TRUE);
			UpdateWindow(g_EngWnd);
			if (!g_AltTabbedAway)
			{
				ReleaseCapture();
				SetCapture(g_EngWnd);
				RECT rect;
				GetWindowRect(g_EngWnd, &rect);
				ClipCursor(&rect);
			}
			return ((int(__thiscall*)(DWORD))(*(DWORD*)(featuresdereferenced + 0x68)))(features);
		}

		ClipCursor(NULL);
		ReleaseCapture();
		result = ((int(__thiscall*)(DWORD))(*(DWORD*)(featuresdereferenced + 0x68)))(features);
	}
	return result;
}

BOOLEAN Render_Open(bool cfullscreen, unsigned cwidth, unsigned cheight, unsigned ccolorbits, unsigned crefreshrate, unsigned cantialias, bool cvsync, bool chwinfo)
{
	int ccantialias; // esi@3
	int ccrefreshrate; // ebx@3
	signed int ccheight; // edi@3
	signed int ccwidth; // ebp@3
	int width; // eax@20
	int height; // eax@20
	int color; // eax@20
	int refresh; // eax@20
	int zbuf; // eax@20
	int stencil; // eax@20
	int antialiasing; // eax@20
	char result; // al@20
	char debugstr[1024]; // [sp+40h] [bp-400h]@7

	if (g_hRenderDLL
		&& (render = CreateRender(),
			renderfeatures = CreateRenderFeatures(render),
			renderplatforminfo = CreateRenderPlatformInfo(render),
			(**(void(__thiscall***)(DWORD, HWND(__cdecl*)(unsigned, char, unsigned, unsigned)))renderplatforminfo)(
				(DWORD)renderplatforminfo,
				CreateRenderWindow),
			(*(void(__thiscall**)(DWORD, int(__cdecl*)(int, HWND)))(*(DWORD*)renderplatforminfo + 4))((DWORD)renderplatforminfo, UnregisterWindowClass),
			(*(void(__thiscall**)(DWORD, int(*)()))(*(DWORD*)renderplatforminfo + 12))((DWORD)renderplatforminfo, ResChangeHandler),
			(unsigned __int8)(*(int(__thiscall**)(DWORD, DWORD))(*(DWORD*)render + 4))((DWORD)render, (DWORD)renderplatforminfo)))
	{
		ccantialias = cantialias;
		ccrefreshrate = crefreshrate;
		ccheight = cheight;
		ccwidth = cwidth;

		while (1)
		{
			(*(void(__thiscall**)(DWORD, int))(*(DWORD*)renderfeatures + 84))((DWORD)renderfeatures, cfullscreen);
			(*(void(__thiscall**)(DWORD, int))(*(DWORD*)renderfeatures + 52))((DWORD)renderfeatures, cvsync);
			(*(void(__thiscall**)(DWORD, DWORD))(*(DWORD*)renderfeatures + 68))((DWORD)renderfeatures, 0);
			(*(void(__thiscall**)(DWORD, signed int))(*(DWORD*)renderfeatures + 28))((DWORD)renderfeatures, ccwidth);
			(*(void(__thiscall**)(DWORD, signed int))(*(DWORD*)renderfeatures + 36))((DWORD)renderfeatures, ccheight);
			(*(void(__thiscall**)(DWORD, signed int))(*(DWORD*)renderfeatures + 60))((DWORD)renderfeatures, ccolorbits);
			(*(void(__thiscall**)(DWORD, int))(*(DWORD*)renderfeatures + 44))((DWORD)renderfeatures, ccrefreshrate);
			(*(void(__thiscall**)(DWORD, int))(*(DWORD*)renderfeatures + 92))((DWORD)renderfeatures, ccantialias);
			(*(void(__thiscall**)(DWORD, int))(*(DWORD*)renderfeatures + 100))((DWORD)renderfeatures, chwinfo);
			(*(void(__thiscall**)(DWORD, void*))(*(DWORD*)render + 8))((DWORD)render, renderfeatures);

			if ((unsigned __int8)(*(int(__thiscall**)(DWORD, DWORD*))(*(DWORD*)render + 16))((DWORD)render, renderplatforminfo))
				break;

			_snprintf(
				debugstr,
				1024,
				"Failed to openRender with %dx%dx%d@%d (%d Z) (%d stencil) (%d multisamples) (%s)\n",
				ccwidth,
				ccheight,
				ccolorbits,
				ccrefreshrate,
				24,
				0,
				ccantialias,
				cfullscreen ? "Fullscreen" : "Windowed");
			OutputDebugStringA(debugstr);

			if (ccantialias)
			{
				ccantialias = 0;
			}
			else if (ccrefreshrate)
			{
				ccrefreshrate = 0;
			}
			else if (cvsync)
			{
				if (ccwidth != 640 || ccheight != 480)
				{
					ccwidth = 640;
					ccheight = 480;
				}
				else if (cfullscreen)
				{
					if (ccolorbits == 16)
						goto LABEL_22;
					ccolorbits = 16;
				}
				else
				{
					cfullscreen = 1;
				}
			}
			else
			{
				cvsync = 1;
			}
		}

		(*(void(__thiscall**)(DWORD, void*))(*(DWORD*)render + 12))((DWORD)render, renderfeatures);
		width = (*(int(__thiscall**)(DWORD))(*(DWORD*)renderfeatures + 88))((DWORD)renderfeatures);
		height = (*(int(__thiscall**)(DWORD))(*(DWORD*)renderfeatures + 72))((DWORD)renderfeatures);
		color = (*(int(__thiscall**)(DWORD))(*(DWORD*)renderfeatures + 64))((DWORD)renderfeatures);
		refresh = (*(int(__thiscall**)(DWORD))(*(DWORD*)renderfeatures + 40))((DWORD)renderfeatures);
		zbuf = (*(int(__thiscall**)(DWORD))(*(DWORD*)renderfeatures + 56))((DWORD)renderfeatures);
		stencil = (*(int(__thiscall**)(DWORD))(*(DWORD*)renderfeatures + 32))((DWORD)renderfeatures);
		antialiasing = (*(int(__thiscall**)(DWORD))(*(DWORD*)renderfeatures + 24))((DWORD)renderfeatures);

		_snprintf(debugstr, 1024, "Resolution is %dx%dx%d@%d (%d Z) (%d stencil) (%d multisamples)\n", width, height, color, refresh, zbuf, stencil, antialiasing);
		OutputDebugStringA(debugstr);

		(**(void(__thiscall***)(DWORD, DWORD))(*(DWORD*)(*(DWORD*)((DWORD)render + 4) + 4) + (DWORD)render + 4))(
			*(DWORD*)(*(DWORD*)((DWORD)render + 4) + 4) + (DWORD)render + 4,
			6);
		result = 1;
	}
	else
	{
		ccantialias = cantialias;
		ccrefreshrate = crefreshrate;
		ccheight = cheight;
		ccwidth = cwidth;
	LABEL_22:
		if (renderplatforminfo)
		{
			(*(void(__thiscall**)(DWORD))(*(DWORD*)renderplatforminfo + 16))((DWORD)renderplatforminfo);
			renderplatforminfo = 0;
		}
		if (renderfeatures)
		{
			(*(void(__thiscall**)(DWORD))(*(DWORD*)renderfeatures + 104))((DWORD)renderfeatures);
			renderfeatures = 0;
		}
		if (render)
		{
			(*(void(__thiscall**)(DWORD))(*(DWORD*)render + 124))((DWORD)render);
			render = 0;
		}
		if (ccantialias)
			result = Render_Open(cfullscreen, ccwidth, ccheight, ccolorbits, ccrefreshrate, 0, cvsync, chwinfo);
		else
			result = 0;
	}
	return result;
}

int UnregisterWindowClass(int unknown, HWND window)
{
	int success;
	HMODULE hHandle = GetModuleHandleA(0);
	if (window && !DestroyWindow(window))
	{
		MessageBoxA(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONEXCLAMATION | MB_DEFBUTTON1 | MB_APPLMODAL);
	}
	success = UnregisterClassA("NightfireClass", hHandle);
	if (!success)
	{
		MessageBoxA(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONEXCLAMATION | MB_DEFBUTTON1 | MB_APPLMODAL);
	}
	g_EngWnd = NULL;
	return success;
}

HWND CreateRenderWindow(unsigned u1, char alwaysontop, unsigned width, unsigned height)
{
	HANDLE hHandle = GetModuleHandleA(0);
	HICON icon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_HALFLIFE));
	HCURSOR cursor = LoadCursorA(NULL, IDC_ARROW);
	HGDIOBJ blackbrush = GetStockObject(BLACK_BRUSH);
	WNDCLASSA windowclass;

	windowclass.lpszMenuName = NULL;
	windowclass.lpszClassName = "NightfireClass";
	windowclass.cbClsExtra = NULL;
	windowclass.cbWndExtra = NULL;
	windowclass.hbrBackground = (HBRUSH)blackbrush;
	windowclass.hCursor = cursor;
	windowclass.hInstance = (HINSTANCE)hHandle;
	windowclass.lpfnWndProc = HLEngineWindowProc;
	windowclass.hIcon = icon;
	windowclass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

	RECT wndRect;
	wndRect.left = 0;
	wndRect.top = 0;
	wndRect.bottom = width;
	wndRect.right = height;

	ATOM nfclass = RegisterClassA(&windowclass);
	if (!nfclass)
		ErrorMessage(1, "Failed to Register The Window Class.");

	ShowCursor(FALSE);
	BOOLEAN topmost = 0;
	int alwaysontop2 = alwaysontop & 0x000000FF;
	if (alwaysontop2)
		topmost = 1;
	DWORD dwExStyle = topmost * WS_EX_TOPMOST + WS_EX_APPWINDOW;
	HWND window;
	if (!g_bFullScreen && border)
	{
		HTHEME htheme = OpenThemeData(g_EngWnd, L"EDIT");
		int borderwidth = GetThemeSysSize(htheme, SM_CXBORDER);
		int borderheight = GetThemeSysSize(htheme, SM_CYSIZE) + GetThemeSysSize(htheme, SM_CXPADDEDBORDER) * 2;
		//wndRect.top += borderheight;
		//wndRect.left += borderwidth;
		//wndRect.right -= borderwidth;
		//wndRect.bottom -= borderheight;
		//dwExStyle = WS_POPUP | WS_CLIPSIBLINGS;
		//dwExStyle |= WS_OVERLAPPEDWINDOW;
		//dwExStyle &= ~WS_THICKFRAME;
		//dwExStyle &= ~WS_MAXIMIZEBOX;

		//wndRect.bottom += borderheight;
		wndRect.top += borderheight;// *2;
		//wndRect.right -= borderheight / 2;
		wndRect.left += borderheight;// / 2;

		dwExStyle = NULL;

		DWORD Style = (topmost * WS_EX_TOPMOST) | WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
		AdjustWindowRectEx(&wndRect, Style, FALSE, dwExStyle);

		//wndRect.bottom += 128;
		//window = CreateWindow("NightfireClass", "Nightfire", dwExStyle,
			//0, 0, wndRect.bottom - wndRect.top, wndRect.right - wndRect.left, NULL, NULL, (HINSTANCE)hHandle, NULL);

		window = CreateWindowExA(dwExStyle, "NightfireClass", "Nightfire", Style /*| WS_CLIPCHILDREN*/, 0, 0, wndRect.bottom - wndRect.top, wndRect.right - wndRect.left, NULL, NULL, (HINSTANCE)hHandle, NULL);
	}
	else
	{
		AdjustWindowRectEx(&wndRect, WS_POPUP, FALSE, dwExStyle);
		window = CreateWindowExA(dwExStyle, "NightfireClass", "Nightfire", WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, wndRect.bottom - wndRect.top, wndRect.right - wndRect.left, NULL, NULL, (HINSTANCE)hHandle, NULL);
	}

	if (!window)
	{
		int err = GetLastError();
		ErrorMessage(1, "Window Creation Error.");
		UnregisterWindowClass((int)window, window);
	}
	ShowWindow(window, SW_SHOW);
	SetForegroundWindow(window);
	SetFocus(window);
	g_EngWnd = window;
	return window;
}

void CreateGUIInterface(void)
{
	if (!g_bDedicated)
	{
		gui_imports _gui_imports;
		unsigned sizeofstruct = sizeof(_gui_imports);
		memset(&_gui_imports, 0, sizeofstruct); //struct size is 64 bytes
		_gui_imports.CreateRenderMaterial = g_EngineAPI.CreateRenderMaterial;
		_gui_imports.render = render;
		_gui_imports.renderfeatures = renderfeatures;
		_gui_imports.UnloadTexture_Launcher = g_EngineAPI.UnloadTexture_Launcher;
		_gui_imports.CreateRenderVideoTexture = (unsigned)CreateRenderVideoTexture;
		_gui_imports.LoadTexture_Launcher = g_EngineAPI.LoadTexture_Launcher;
		_gui_imports.ConsoleBuffer_InsertText = g_EngineAPI.ConsoleBuffer_InsertText;
		_gui_imports.Con_Printf = g_EngineAPI.Con_Printf;
		_gui_imports.ConsoleBuffer_AddText = g_EngineAPI.ConsoleBuffer_AddText;
		_gui_imports.COM_LoadHeapFile = g_EngineAPI.COM_LoadHeapFile;
		_gui_imports.GetConsoleVariable = g_EngineAPI.GetConsoleVariable;
		_gui_imports.COM_LoadTempFile = g_EngineAPI.COM_LoadTempFile;
		_gui_imports.COM_FreeFile = g_EngineAPI.COM_FreeFile;
		_gui_imports.COM_ExpandFilename = g_EngineAPI.COM_ExpandFilename;
		_gui_imports.version = 1;
		_gui_imports.iSizeOfStruct = sizeofstruct;


		g_guiAPI.GUI_Init(&_gui_imports);
		s_LauncherAPI.GUI_Manager = g_guiAPI.GUI_Manager;
	}
}

//sub_404000
LSTATUS __cdecl LoadDisplaySettings(unsigned& outwidth, unsigned& outheight, unsigned& outdepth, unsigned& outrefreshrate, unsigned& outantialias, unsigned& outvsync)
{
	LSTATUS result;
	DWORD cbData;
	DWORD Type;
	HKEY phkResult;
	DWORD dwDisposition;

	result = RegCreateKeyExA(
		HKEY_CURRENT_USER,
		"Software\\Gearbox Software\\Nightfire",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&phkResult,
		&dwDisposition);
	if (!result)
	{
		cbData = 4;
		if (RegQueryValueExA(phkResult, "DisplayWidth", 0, &Type, (byte*)&outwidth, &cbData) || Type != REG_DWORD)
			outwidth = 0;
		cbData = 4;
		if (RegQueryValueExA(phkResult, "DisplayHeight", 0, &Type, (byte*)&outheight, &cbData) || Type != REG_DWORD)
			outheight = 0;
		cbData = 4;
		if (RegQueryValueExA(phkResult, "DisplayDepth", 0, &Type, (byte*)&outdepth, &cbData) || Type != REG_DWORD)
			outdepth = 0;
		cbData = 4;
		if (RegQueryValueExA(phkResult, "DisplayRefreshRate", 0, &Type, (byte*)&outrefreshrate, &cbData) || Type != REG_DWORD)
			outrefreshrate = 0;
		cbData = 4;
		if (RegQueryValueExA(phkResult, "DisplayMultisampleCount", 0, &Type, (byte*)&outantialias, &cbData) || Type != REG_DWORD)
			outantialias = 0;
		cbData = 4;
		if (RegQueryValueExA(phkResult, "DisplayVSYNC", 0, &Type, (byte*)&outvsync, &cbData) || Type != REG_DWORD)
			outvsync = 1;
		result = RegCloseKey(phkResult);
	}
	return result;
}

//sub_404600
void SaveVideoSettingsToRegistry(void)
{
	unsigned features, features2; // eax@2
	HKEY phkResult; // [sp+4h] [bp-20h]@3
	unsigned width; // [sp+8h] [bp-1Ch]@3
	unsigned height; // [sp+Ch] [bp-18h]@3
	unsigned colordepth; // [sp+10h] [bp-14h]@3
	unsigned refreshrate; // [sp+14h] [bp-10h]@3
	unsigned antialias; // [sp+18h] [bp-Ch]@3
	unsigned __int8 vsync = 0; // [sp+1Ch] [bp-8h]@3
	DWORD dwDisposition; // [sp+20h] [bp-4h]@3

	if (render)
	{
		features = (DWORD)CreateRenderFeatures(render);
		DWORD result = (LSTATUS)((BYTE(__thiscall*)(void*, DWORD))(*(DWORD*)(*(DWORD*)render + 0x0c)))(render, features);
		if ((BYTE)result)
		{
			features2 = *(DWORD*)features;
			width = (*(int(__thiscall*)(int))(*(DWORD*)(features2 + 24)))(features);
			height = (*(int(__thiscall*)(int))(*(DWORD*)(features2 + 32)))(features);
			colordepth = (*(int(__thiscall*)(int))(*(DWORD*)(features2 + 56)))(features);
			refreshrate = (*(int(__thiscall*)(int))(*(DWORD*)(features2 + 40)))(features);
			antialias = (*(int(__thiscall*)(int))(*(DWORD*)(features2 + 88)))(features);
			vsync = (unsigned __int8)(*(int(__thiscall*)(int))(*(DWORD*)(features2 + 48)))(features);

			result = RegCreateKeyExA(
				HKEY_CURRENT_USER,
				"Software\\Gearbox Software\\Nightfire",
				0,
				0,
				NULL,
				KEY_ALL_ACCESS,
				NULL,
				&phkResult,
				&dwDisposition);
			if (!result)
			{
				if (antialias > 500 && colordepth > 500)
				{
					//Dylan's shit fix to fix people's messed up registry keys from bad revisions of this function
					width = 800;
					height = 600;
					colordepth = 24;
					refreshrate = 0;
					antialias = 0;
					vsync = 0;
				}
				RegSetValueExA(phkResult, "DisplayWidth", 0, REG_DWORD, (const BYTE*)&width, 4u);
				RegSetValueExA(phkResult, "DisplayHeight", 0, REG_DWORD, (const BYTE*)&height, 4u);
				RegSetValueExA(phkResult, "DisplayDepth", 0, REG_DWORD, (const BYTE*)&colordepth, 4u);
				RegSetValueExA(phkResult, "DisplayRefreshRate", 0, REG_DWORD, (const BYTE*)&refreshrate, 4u);
				RegSetValueExA(phkResult, "DisplayMultisampleCount", 0, REG_DWORD, (const BYTE*)&antialias, 4u);
				RegSetValueExA(phkResult, "DisplayVSYNC", 0, REG_DWORD, (const BYTE*)&vsync, 4u);
				result = RegCloseKey(phkResult);
			}
		}
		if (features)//if (features2)
		{
			result = (*(int(__thiscall*)(int))(*(DWORD*)(features2 + 104)))(features);
		}
	}
}