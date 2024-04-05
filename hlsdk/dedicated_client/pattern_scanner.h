#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#define PSAPI_VERSION 1
#include <Psapi.h>
#include "MinHook/MinHook.h"
#include "sys_ded.h" // for ErrorBox


#define LAZY_FIND_MEMORY_PATTERN_ENABLED

#if defined(_DEBUG) && defined(LAZY_FIND_MEMORY_PATTERN_ENABLED)
	extern bool g_bSuppressPatternErrors;
	#define SUPPRESS_PATTERN_ERRORS g_bSuppressPatternErrors = true;
	#define UNSUPRESS_PATTERN_ERRORS g_bSuppressPatternErrors = false;
#else
	#define SUPPRESS_PATTERN_ERRORS
	#define UNSUPRESS_PATTERN_ERRORS
#endif

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

struct pattern_t
{
	void*& dest;
	void* module_handle;
	const std::string strpattern;
	bool double_wide;
	bool dereference = false;
	int addbeforedereference = 0;
	int addatend = 0;
	bool is_relative = false;
	bool crash_if_not_found = false;
	const char* debug_name;

	template <class T, class M>
	pattern_t(T& _dest, M _module_handle, const std::string& pattern, bool _double_wide_pattern, const char* _debug_name = "", bool _crash_if_not_found = false)
		: dest((void*&) _dest), module_handle((void*)_module_handle), strpattern(pattern), double_wide(_double_wide_pattern), debug_name(_debug_name), crash_if_not_found(_crash_if_not_found)
	{
	}
	template <class T, class M>
	pattern_t(T& _dest, M _module_handle, const std::string& pattern, bool _double_wide_pattern, bool _dereference = false, int _addbeforedereference = 0, int _addatend = 0, bool _is_relative = false, const char* _debug_name = "", bool _crash_if_not_found = false)
		: dest((void*&)_dest), module_handle((void*)_module_handle), strpattern(pattern), double_wide(_double_wide_pattern), dereference(_dereference), addbeforedereference(_addbeforedereference), addatend(_addatend), is_relative(_is_relative), debug_name(_debug_name), crash_if_not_found(_crash_if_not_found)
	{
	}
};

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

inline bool FindMemoryPattern(pattern_t& pattern)
{
	uintptr_t start, end;
	GetModuleStartEndPoints(pattern.module_handle, start, end);
	bool result = FindMemoryPattern2(pattern.dest, start, end, pattern.strpattern.c_str(), pattern.strpattern.length(), pattern.double_wide, pattern.dereference, pattern.addbeforedereference, pattern.addatend, pattern.is_relative);
	if (!result && pattern.crash_if_not_found)
	{
		char errormsg[2048];
		sprintf_s(errormsg, "Error: could not find signature \"%s\" at %#010x with pattern \"%s\"", pattern.debug_name, (DWORD)start, pattern.strpattern.c_str());
		ErrorBox(errormsg);
	}

	return result;
}

#ifdef LAZY_FIND_MEMORY_PATTERN_ENABLED
template<class T, class M>
inline bool FindMemoryPattern(T& dest, M ModuleHandle, std::string strpattern, bool double_wide, bool dereference = false, int addbeforedereference = 0, int addatend = 0, bool is_relative = false)
{
	uintptr_t start, end;
	GetModuleStartEndPoints(ModuleHandle, start, end);
	bool result = FindMemoryPattern2(dest, start, end, strpattern.c_str(), strpattern.length(), double_wide, dereference, addbeforedereference, addatend, is_relative);

#if defined(_DEBUG)
	if (!result && !g_bSuppressPatternErrors)
	{
		char errormsg[2048];
		sprintf_s(errormsg, "Error: could not find signature at %#010x with pattern \"%s\"", (DWORD)start, strpattern.c_str());
		MessageBoxA(NULL, errormsg, "Nightfire", MB_OK);
	}
#endif

	return result;
}
#endif

template <class A, class D, class O>
inline bool HookFunctionWithMinHook(A pFunctionAddress, D pDetourAddress, O ppOriginal, const char* szDebugName = " ") {
	char error_msg[512];
	int nCreateHookRet = MH_CreateHook((LPVOID)pFunctionAddress, (LPVOID)pDetourAddress, reinterpret_cast<LPVOID*>(ppOriginal));
	if (nCreateHookRet != MH_OK)
	{
		int nDisableHookRet = MH_DisableHook((LPVOID)pFunctionAddress);
		if (nDisableHookRet != MH_OK)
			return false;
		sprintf_s(error_msg, "Error: function \"%s\" (%#010x) already hooked! Tried to detour to %#010x", szDebugName, (DWORD)pFunctionAddress, (DWORD)pDetourAddress);
		ErrorBox(error_msg);
		return false;
	}
	if (MH_EnableHook((LPVOID)pFunctionAddress) == MH_OK)
		return true;
	sprintf_s(error_msg, "Error: function \"%s\" (%#010x) failed to hook! Tried to detour to %#010x", szDebugName, (DWORD)pFunctionAddress, (DWORD)pDetourAddress);
	ErrorBox(error_msg);
	return false;
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