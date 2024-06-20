/// ********* WIN32 **********
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>
#include "cmdlib.h"
#include "messages.h"
#include "log.h"
#include "hlassert.h"
#include "blockmem.h"

// =====================================================================================
//  AllocBlock
// =====================================================================================
void* AllocBlock(const unsigned long size)
{
    void* pointer;
    HANDLE          h;

    if (!size)
    {
        Warning("Attempting to allocate 0 bytes");
    }

    h = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, size);

    if (h)
    {
        pointer = GlobalLock(h);
    }
    else
    {
        return NULL;
    }

    return pointer;
}

// =====================================================================================
//  FreeBlock
// =====================================================================================
bool            FreeBlock(void* pointer)
{
    HANDLE          h;

    if (!pointer)
    {
        Warning("Freeing a null pointer");
    }

    h = GlobalHandle(pointer);

    if (h)
    {
        GlobalUnlock(h);
        GlobalFree(h);
        return true;
    }
    else
    {
        Warning("Could not translate pointer into handle");
        return false;
    }
}

#ifdef CHECK_HEAP
// =====================================================================================
//  HeapCheck
// =====================================================================================
void            HeapCheck()
{
    if (_heapchk() != _HEAPOK)
        hlassert(false);
}
#endif

// =====================================================================================
//  AllocBlock
// =====================================================================================
// HeapAlloc/HeapFree is thread safe by default
void* Alloc(const unsigned long size)
{
    HeapCheck();
    return calloc(1, size);
}

// =====================================================================================
//  AllocBlock
// =====================================================================================
bool            Free(void* pointer)
{
    HeapCheck();
    free(pointer);
    return true;
}