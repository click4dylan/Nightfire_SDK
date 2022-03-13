#pragma once
#include <Windows.h>
#include <string>

__inline void PlaceJMP(BYTE* bt_DetourAddress, DWORD dw_FunctionAddress, DWORD dw_Size)
{
	BYTE* dt = (BYTE*)bt_DetourAddress;
	DWORD fn = (DWORD)dw_FunctionAddress;

	DWORD dw_OldProtection, dw_Distance;
	VirtualProtect(dt, dw_Size, PAGE_EXECUTE_READWRITE, &dw_OldProtection);
	dw_Distance = (DWORD)(fn - (DWORD)dt) - 5;
	*dt = 0xE9;
	*(DWORD*)(dt + 0x1) = dw_Distance;
	for (unsigned int i = 0x5; i < dw_Size; i++) *(dt + i) = 0x90;
	VirtualProtect(dt, dw_Size, dw_OldProtection, NULL);
	return;
}

inline void PlaceShort(BYTE* bt_DetourAddress, unsigned short data)
{
	DWORD dw_OldProtection, dw_Old2;
	VirtualProtect(bt_DetourAddress, 2, PAGE_EXECUTE_READWRITE, &dw_OldProtection);
	*(unsigned short*)bt_DetourAddress = data;
	VirtualProtect(bt_DetourAddress, 2, dw_OldProtection, &dw_Old2);
}

inline void PlaceInt(BYTE* bt_DetourAddress, unsigned int data)
{
	DWORD dw_OldProtection, dw_Old2;
	VirtualProtect(bt_DetourAddress, 4, PAGE_EXECUTE_READWRITE, &dw_OldProtection);
	*(unsigned int*)bt_DetourAddress = data;
	VirtualProtect(bt_DetourAddress, 4, dw_OldProtection, &dw_Old2);
}

extern uintptr_t FindMemoryPattern(uintptr_t start, uintptr_t end, const char* strpattern, size_t length, bool double_wide = true);

extern uintptr_t FindMemoryPattern(DWORD ModuleHandle, std::string strpattern, bool double_wide = true);