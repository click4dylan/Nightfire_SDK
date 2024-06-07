#pragma once
#include <Windows.h>
#include "bytepatcher.h"
#include "winding.h"
#include "globals.h"

// class to allow calls to original bbsp functions for debugging/testing
// THIS IS NOT FEASIBLE AND IS CURRENTLY UNUSED
class BBSP_Original
{
    uintptr_t baseadr;
    Winding tempwinding;

public:
    BBSP_Original()
    {
        baseadr = (uintptr_t)LoadLibrary("bbsp_original.exe");
        if (!baseadr)
        {
            MessageBox(NULL, "ERROR: couldn't find or load bbsp_original.exe", "bbsp", MB_OK);
            exit(EXIT_FAILURE);
            return;
        }

        Winding_Duplicate_Relocate();
        New_Relocate();
        Free_Relocate();
    }

    ~BBSP_Original()
    {
        if (!baseadr)
            return;
       
        bp.RemoveAllBytePatches();
        FreeLibrary((HMODULE)baseadr);
    }

private:
    static void* __cdecl New_Wrapper(unsigned size) { return malloc(size); }
    static void __cdecl Free_Wrapper(void* adr) { free(adr); }

    void New_Relocate()
    {
        uintptr_t jmp = baseadr + 0x5BE0;
        bp.BytePatch(baseadr + 0x5BE0, absolute_to_relative(jmp, &New_Wrapper));
    }

    void Free_Relocate()
    {
        uintptr_t jmp = baseadr + 0x5BE5;
        bp.BytePatch(baseadr + 0x5BE5, absolute_to_relative(jmp, &Free_Wrapper));
    }

    void Winding_Duplicate_Relocate()
    {
        uintptr_t function = baseadr + 0x128B0;
        bp.BytePatch(function + 2, &g_numWindings);
        bp.BytePatch(function + 0x1E, &g_numWindings);
        bp.CopyVTablePointer(function + 9, tempwinding);
    }

    void Winding_Constructor_Relocate()
    {
        uintptr_t function = baseadr + 0x12850;
        bp.BytePatch(function + 3, &g_numWindings);
        bp.CopyVTablePointer(function + 0xF, tempwinding);
        bp.BytePatch(function + 0x13, &g_numWindings);
    }
};

//inline BBSP_Original bbsp_og;