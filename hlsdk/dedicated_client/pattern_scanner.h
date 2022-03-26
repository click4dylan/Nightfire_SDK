#pragma once
#include <Windows.h>
#include <string>
#include "MinHook/MinHook.h"

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

extern uintptr_t FindMemoryPattern(DWORD ModuleHandle, std::string strpattern, bool double_wide = false);

inline bool HookFunctionWithMinHook(LPVOID pFunctionAddress, LPVOID pDetourAddress, LPVOID* ppOriginal, const char* szDebugName = " ") {

	int nCreateHookRet = MH_CreateHook(pFunctionAddress, pDetourAddress, reinterpret_cast<LPVOID*>(ppOriginal));
	if (nCreateHookRet != MH_OK)
	{
		int nDisableHookRet = MH_DisableHook(pFunctionAddress);
		if (nDisableHookRet != MH_OK)
			return false;
		return false;
	}
	return MH_EnableHook(pFunctionAddress) == MH_OK;
}

inline uintptr_t RelativeToAbsolute(uintptr_t address)
{
	return (address + 4 + *reinterpret_cast<unsigned int*>(address));
}

extern DWORD old_protection;
extern void* protection_address;
extern DWORD protection_length;
template<class T> inline void PushProtection(T address, uintptr_t length = 4096)
{
	protection_address = (void*)address;
	protection_length = length;
	VirtualProtect((void*)address, length, PAGE_EXECUTE_READWRITE, &old_protection);
}
inline void PopProtection()
{
	VirtualProtect(protection_address, protection_length, old_protection, nullptr);
}