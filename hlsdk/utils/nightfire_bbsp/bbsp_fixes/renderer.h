#ifndef MYDIRECTXDLL_H
#define MYDIRECTXDLL_H

#include <windows.h>

//extern "C" __declspec(dllexport) void StartRendering();
extern DWORD WINAPI StartRendering(LPVOID lParam);
extern "C" __declspec(dllexport) void StopRendering();

#endif // MYDIRECTXDLL_H