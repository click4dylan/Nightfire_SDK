#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#define PSAPI_VERSION 1
#include <Psapi.h>
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

template <class T>
inline void PlaceShort(T bt_DetourAddress, unsigned short data)
{
	DWORD dw_OldProtection, dw_Old2;
	VirtualProtect((void*)bt_DetourAddress, 2, PAGE_EXECUTE_READWRITE, &dw_OldProtection);
	*(unsigned short*)bt_DetourAddress = data;
	VirtualProtect((void*)bt_DetourAddress, 2, dw_OldProtection, &dw_Old2);
}

template <class T>
inline void PlaceInt(T bt_DetourAddress, unsigned int data)
{
	DWORD dw_OldProtection, dw_Old2;
	VirtualProtect((void*)bt_DetourAddress, 4, PAGE_EXECUTE_READWRITE, &dw_OldProtection);
	*(unsigned int*)bt_DetourAddress = data;
	VirtualProtect((void*)bt_DetourAddress, 4, dw_OldProtection, &dw_Old2);
}

template <class T>
inline void GetModuleStartEndPoints(T ModuleHandle, uintptr_t& start, uintptr_t& end) {
	MODULEINFO dllinfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)ModuleHandle, &dllinfo, sizeof(MODULEINFO));
	start = (uintptr_t)ModuleHandle;
	end = (uintptr_t)ModuleHandle + dllinfo.SizeOfImage;
}

template <class T, class A>
inline T RelativeToAbsolute(A address)
{
	return (T)((uintptr_t)address + 4 + *reinterpret_cast<unsigned int*>((uintptr_t)address));
}

template<class T>
inline bool FindMemoryPattern2(T& dest, uintptr_t start, uintptr_t end, const char* strpattern, size_t length, bool double_wide, bool dereference = false, int addbeforedereference = 0, int addatend = 0, bool is_relative = false) {
	uintptr_t	   adrafterfirstmatch = 0;
	size_t		   indextofind = 0;
	size_t		   numhexvalues = 0;
	std::unique_ptr<unsigned char[]>  hexvalues(new unsigned char[length + 1]);
	std::unique_ptr<bool[]>			  shouldskip(new bool[length + 1]);

	if (double_wide) { //DOUBLE SPACES AND QUESTION MARKS, THIS IS FASTER TO RUN
		for (size_t i = 0; i < length - 1; i += 2) {
			//Get the ascii version of the hex values out of the pattern
			char ascii[4];
			*(short*)ascii = *(short*)&strpattern[i];

			//Filter out spaces
			if (ascii[0] != ' ') {
				//Filter out wildcards
				if (ascii[0] == '?') {
					shouldskip[numhexvalues] = true;
				}
				else {
					//Convert ascii to hex
					ascii[2] = NULL; //add null terminator
					hexvalues[numhexvalues] = (unsigned char)std::stoul(ascii, nullptr, 16);
					shouldskip[numhexvalues] = false;
				}
				numhexvalues++;
			}
		}
	}
	else {
		for (size_t i = 0, maxlength = length - 1; i < maxlength; i++) {
			//Get the ascii version of the hex values out of the pattern
			char ascii[4];
			*(short*)ascii = *(short*)&strpattern[i];

			//Filter out spaces
			if (ascii[0] != ' ') {
				//Filter out wildcards
				if (ascii[0] == '?') {
					shouldskip[numhexvalues] = true;
				}
				else {
					//Convert ascii to hex
					ascii[2] = NULL; //add null terminator
					hexvalues[numhexvalues] = (unsigned char)std::stoul(ascii, nullptr, 16);
					shouldskip[numhexvalues] = false;
				}
				i++;
				numhexvalues++;
			}
		}
	}

	//Search for the hex signature in memory	
	for (uintptr_t adr = start; adr < end; adr++)
	{
		if (shouldskip[indextofind] || *(char*)adr == hexvalues[indextofind] || *(unsigned char*)adr == hexvalues[indextofind]) {
			if (indextofind++ == 0)
				adrafterfirstmatch = adr + 1;

			if (indextofind >= numhexvalues)
			{
				//FOUND PATTERN!
				DWORD result = (adr - (numhexvalues - 1));
				if (dereference)
					result = *(DWORD*)(result + addbeforedereference);
				result += addatend;
				if (is_relative)
					result = RelativeToAbsolute<DWORD>(result);
				dest = (T)result;
				return true;
			}

		}
		else if (adrafterfirstmatch) {
			adr = adrafterfirstmatch;
			indextofind = 0;
			adrafterfirstmatch = 0;
		}
	}
	dest = (T)NULL; //NOT FOUND!
	return false;
}


//template<class T, class M>
//inline bool FindMemoryPattern(T& dest, M ModuleHandle, const char* strpattern, const size_t length, bool double_wide, bool dereference = false, int addbeforedereference = 0, int addatend = 0)
//{
//	uintptr_t start, end;
//	GetModuleStartEndPoints(ModuleHandle, start, end);
//	return FindMemoryPattern2(dest, start, end, strpattern, length, double_wide, dereference, addbeforedereference, addatend);
//}

//template<class T, class M>
//inline T FindMemoryPattern(M ModuleHandle, const char* strpattern, bool double_wide, bool dereference = false, int addbeforedereference = 0, int addatend = 0)
//{
//	return FindMemoryPattern<T>(ModuleHandle, strpattern, strlen(strpattern), double_wide, dereference, addbeforedereference, addatend);
//}

template<class T, class M>
inline bool FindMemoryPattern(T& dest, M ModuleHandle, std::string strpattern, bool double_wide, bool dereference = false, int addbeforedereference = 0, int addatend = 0, bool is_relative = false)
{
	uintptr_t start, end;
	GetModuleStartEndPoints(ModuleHandle, start, end);
	return FindMemoryPattern2(dest, start, end, strpattern.c_str(), strpattern.length(), double_wide, dereference, addbeforedereference, addatend, is_relative);
}

template <class A, class D, class O>
inline bool HookFunctionWithMinHook(A pFunctionAddress, D pDetourAddress, O ppOriginal, const char* szDebugName = " ") {

	int nCreateHookRet = MH_CreateHook((LPVOID)pFunctionAddress, (LPVOID)pDetourAddress, reinterpret_cast<LPVOID*>(ppOriginal));
	if (nCreateHookRet != MH_OK)
	{
		int nDisableHookRet = MH_DisableHook((LPVOID)pFunctionAddress);
		if (nDisableHookRet != MH_OK)
			return false;
		return false;
	}
	return MH_EnableHook((LPVOID)pFunctionAddress) == MH_OK;
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