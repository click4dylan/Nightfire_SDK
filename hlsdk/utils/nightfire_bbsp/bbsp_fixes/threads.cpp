#include <Windows.h>
#include "globals.h"
#include "threads.h"
#include "log.h"
#include "helpers.h"

int             g_numthreads = DEFAULT_NUMTHREADS;
static CRITICAL_SECTION crit;
static int      enter;
q_threadpriority g_threadpriority = DEFAULT_THREAD_PRIORITY;

#define THREADTIMES_SIZE 100
#define THREADTIMES_SIZEf (float)(THREADTIMES_SIZE)

static int      dispatch = 0;
static int      workcount = 0;
static int      oldf = 0;
static bool     pacifier = false;
static bool     threaded = false;
static double   threadstart = 0;
static double   threadtimes[THREADTIMES_SIZE];

void ThreadSetDefault()
{
    _SYSTEM_INFO SystemInfo; // [esp+0h] [ebp-24h] BYREF

    if (g_numthreads == -1)
    {
        GetSystemInfo(&SystemInfo);
        g_numthreads = SystemInfo.dwNumberOfProcessors;
        if ((int)SystemInfo.dwNumberOfProcessors < 1 || (int)SystemInfo.dwNumberOfProcessors > 32)
            g_numthreads = 1;
    }
}

void ThreadSetPriority(q_threadpriority type)
{
    int             val;

    g_threadpriority = type;

    switch (g_threadpriority)
    {
    case eThreadPriorityLow:
        val = IDLE_PRIORITY_CLASS;
        break;

    case eThreadPriorityHigh:
        val = HIGH_PRIORITY_CLASS;
        break;

    case eThreadPriorityNormal:
    default:
        val = NORMAL_PRIORITY_CLASS;
        break;
    }

    SetPriorityClass(GetCurrentProcess(), val);
}

void ThreadLock()
{
    if (!threaded)
    {
        return;
    }
    EnterCriticalSection(&crit);
    if (enter)
    {
        Warning("Recursive ThreadLock\n");
    }
    enter++;
}

void ThreadUnlock()
{
    if (!threaded)
    {
        return;
    }
    if (!enter)
    {
        Error("ThreadUnlock without lock\n");
    }
    enter--;
    LeaveCriticalSection(&crit);
}