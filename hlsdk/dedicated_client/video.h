#pragma once
#include <Windows.h>

//sub_00401D50
extern BOOLEAN Render_Close(void);
extern BOOLEAN Render_Open(bool cfullscreen, unsigned cwidth, unsigned cheight, unsigned ccolorbits, unsigned crefreshrate, unsigned cantialias, bool cvsync, bool chwinfo);
extern int UnregisterWindowClass(int unknown, HWND window);
extern HWND CreateRenderWindow(unsigned u1, char alwaysontop, unsigned width, unsigned height);
extern HWND CreateRenderWindow(unsigned u1, char alwaysontop, unsigned width, unsigned height);
extern LONG WINAPI HLEngineWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
extern void CreateGUIInterface(void);
extern LSTATUS __cdecl LoadDisplaySettings(unsigned& outwidth, unsigned& outheight, unsigned& outdepth, unsigned& outrefreshrate, unsigned& outantialias, unsigned& outvsync);
extern void SaveVideoSettingsToRegistry(void);
extern int ResChangeHandler();