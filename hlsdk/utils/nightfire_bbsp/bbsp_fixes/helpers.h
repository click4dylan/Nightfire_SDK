#pragma once
#include "globals.h"
#include <utility> // for std::forward
#include <stdio.h>
#include "log.h"

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

#if 0
inline void* Verbose_func = (void*)0x419C80;
inline __declspec(naked) void Verbose(const char* Format, ...)
{
    __asm jmp Verbose_func
}
#endif

inline bool safe_snprintf(char* dest, size_t size, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = _vsnprintf(dest, size, format, args);
    va_end(args);

    if (result != size)
        return true;

    dest[size - 1] = '\0';

    return false;
}

inline bool safe_strncpy(char* dest, const char* format, size_t size)
{
    return safe_snprintf(dest, size, "%s", format);
}

#if 0
template<typename... Args>
inline void Error2(const char* format, Args&&... args)
{
    reinterpret_cast<void(*)(const char*, ...)>(0x41A310)(format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void Error(const char* format, Args&&... args)
{
    reinterpret_cast<void(*)(const char*, ...)>(0x41A390)(format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void Fatal(BBSP_ERROR_CODES code, const char* format, Args&&... args)
{
    reinterpret_cast<void(*)(BBSP_ERROR_CODES err, const char*, ...)>(0x420990)(code, format, std::forward<Args>(args)...);
}

inline void CheckFatal()
{
    reinterpret_cast<void(*)()>(0x419B80)();
}

inline void MakeNodePortal(node_t* node)
{
    reinterpret_cast<void(__thiscall*)(node_t*)>(0x41D7A0)(node);
}

inline void SplitNodePortals(node_t* node)
{
    reinterpret_cast<void(__cdecl*)(node_t*)>(0x41D5E0)(node);
}

inline int ChoosePlaneFromList(node_t* node, face_t* face)
{
    return reinterpret_cast<int(__cdecl*)(node_t*, face_t*)>(0x41C7F0)(node, face);
}
#endif

#if 0
//MAX_HLASSUME_ERRORS = 46
inline const char** hlassume_errormsg(int err_index)
{
    if (err_index <= 0)
        err_index = 0;
    return (const char**)(12 * err_index + 0x4235B8);
}

inline void print_hlassume_errormsg(int err_index)
{
    const char** arr = hlassume_errormsg(err_index);
    if (arr && *arr && arr[1] && arr[2])
    {
        char tmp[2048];
        sprintf_s(tmp, "Error %i = %s\n", err_index, arr[0]);
        OutputDebugStringA(tmp);
    }
}

inline void hlassume(bool check, BBSP_ERROR_CODES error_index)
{
    const char** err; // eax
    char Format[2048]; // [esp+0h] [ebp-800h] BYREF

    if (!check)
    {
        err = hlassume_errormsg(error_index);
        snprintf(
            Format,
            2048u,
            "%s\nDescription: %s\nHowto Fix: %s\n",
            *err,
            err[1],
            err[2]);
        Error(Format);
    }
}
#endif


inline void FlipSlashes(char* result)
{
    if (*result)
    {
        do
        {
            if (*result == '\\' || *result == '/')
                *result = '\\';
        } while (*++result);
    }
}

inline void StripExtension(char* str)
{
    unsigned int i; // eax
    char v2; // cl

    for (i = strlen(str); i; --i)
    {
        if (str[i] == '.')
            break;
        v2 = str[i - 1];
        if (v2 == '/')
            return;
    }
    if (i)
        str[i] = 0;
}

extern bool safe_strncat(char* dest, const char* src, size_t count);