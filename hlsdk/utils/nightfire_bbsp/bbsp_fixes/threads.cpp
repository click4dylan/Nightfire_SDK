#include <Windows.h>
#include "globals.h"
#include "threads.h"
#include "log.h"
#include "helpers.h"
#include "cmdlib.h"

int             g_numthreads = DEFAULT_NUMTHREADS;
static CRITICAL_SECTION crit;
static int      enter;
q_threadpriority g_threadpriority = DEFAULT_THREAD_PRIORITY;
q_threadfunction q_entry;

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

q_threadfunction workfunction;

int GetThreadWork()
{
    int             r, f, i;
    double          ct, finish, finish2, finish3;

    ThreadLock();

    if (dispatch == 0)
    {
        oldf = 0;
    }

    if (dispatch > workcount)
    {
        Developer(DEVELOPER_LEVEL_ERROR, "dispatch > workcount!!!\n");
        ThreadUnlock();
        return -1;
    }
    if (dispatch == workcount)
    {
        Developer(DEVELOPER_LEVEL_MESSAGE, "dispatch == workcount, work is complete\n");
        ThreadUnlock();
        return -1;
    }
    if (dispatch < 0)
    {
        Developer(DEVELOPER_LEVEL_ERROR, "negative dispatch!!!\n");
        ThreadUnlock();
        return -1;
    }

    f = THREADTIMES_SIZE * dispatch / workcount;
    if (pacifier)
    {
        printf("\r%6d /%6d", dispatch, workcount);
#ifdef ZHLT_PROGRESSFILE // AJM
        if (g_progressfile)
        {


        }
#endif

        if (f != oldf)
        {
            ct = I_FloatTime();
            /* Fill in current time for threadtimes record */
            for (i = oldf; i <= f; i++)
            {
                if (threadtimes[i] < 1)
                {
                    threadtimes[i] = ct;
                }
            }
            oldf = f;

            if (f > 10)
            {
                finish = (ct - threadtimes[0]) * (THREADTIMES_SIZEf - f) / f;
                finish2 = 10.0 * (ct - threadtimes[f - 10]) * (THREADTIMES_SIZEf - f) / THREADTIMES_SIZEf;
                finish3 = THREADTIMES_SIZEf * (ct - threadtimes[f - 1]) * (THREADTIMES_SIZEf - f) / THREADTIMES_SIZEf;

                if (finish > 1.0)
                {
                    printf("  (%d%%: est. time to completion %ld/%ld/%ld secs)   ", f, (long)(finish), (long)(finish2),
                        (long)(finish3));
#ifdef ZHLT_PROGRESSFILE // AJM
                    if (g_progressfile)
                    {


                    }
#endif
                }
                else
                {
                    printf("  (%d%%: est. time to completion <1 sec)   ", f);

#ifdef ZHLT_PROGRESSFILE // AJM
                    if (g_progressfile)
                    {


                    }
#endif
                }
            }
        }
    }
    else
    {
        if (f != oldf)
        {
            oldf = f;
            switch (f)
            {
            case 10:
            case 20:
            case 30:
            case 40:
            case 50:
            case 60:
            case 70:
            case 80:
            case 90:
            case 100:
                /*
                            case 5:
                            case 15:
                            case 25:
                            case 35:
                            case 45:
                            case 55:
                            case 65:
                            case 75:
                            case 85:
                            case 95:
                */
                printf("%d%%...", f);
            default:
                break;
            }
        }
    }

    r = dispatch;
    dispatch++;

    ThreadUnlock();
    return r;
}

static void ThreadWorkerFunction(int unused)
{
    int             work;

    while ((work = GetThreadWork()) != -1)
    {
        workfunction(work);
    }
}

void RunThreadsOnIndividual(int workcnt, bool showpacifier, q_threadfunction func)
{
    workfunction = func;
    RunThreadsOn(workcnt, showpacifier, ThreadWorkerFunction);
}

void threads_InitCrit()
{
    InitializeCriticalSection(&crit);
    threaded = true;
}

void threads_UninitCrit()
{
    DeleteCriticalSection(&crit);
}

static void* CDECL ThreadEntryStub(void* pParam)
{
    q_entry((int)pParam);
    return NULL;
}

void RunThreadsOn(int workcnt, bool showpacifier, q_threadfunction func)
{
    DWORD           threadid[MAX_THREADS];
    HANDLE          threadhandle[MAX_THREADS];
    int             i;
    double          start, end;

    threadstart = I_FloatTime();
    start = threadstart;
    for (i = 0; i < THREADTIMES_SIZE; i++)
    {
        threadtimes[i] = 0;
    }
    dispatch = 0;
    workcount = workcnt;
    oldf = -1;
    pacifier = showpacifier;
    threaded = true;
    q_entry = func;

    if (workcount < dispatch)
    {
        Developer(DEVELOPER_LEVEL_ERROR, "RunThreadsOn: Workcount(%i) < dispatch(%i)\n", workcount, dispatch);
    }
    hlassume(workcount >= dispatch, assume_BadWorkcount);

    //
    // Create all the threads (suspended)
    //
    threads_InitCrit();
    for (i = 0; i < g_numthreads; i++)
    {
        HANDLE          hThread = CreateThread(NULL,
            0,
            (LPTHREAD_START_ROUTINE)ThreadEntryStub,
            (LPVOID)i,
            CREATE_SUSPENDED,
            &threadid[i]);

        if (hThread != NULL)
        {
            threadhandle[i] = hThread;
        }
        else
        {
            LPVOID          lpMsgBuf;

            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
                (LPTSTR)&lpMsgBuf, 0, NULL);
            // Process any inserts in lpMsgBuf.
            // ...
            // Display the string.
            Developer(DEVELOPER_LEVEL_ERROR, "CreateThread #%d [%08X] failed : %s\n", i, threadhandle[i], lpMsgBuf);
            Fatal(assume_THREAD_ERROR, "Unable to create thread #%d", i);
            // Free the buffer.
            LocalFree(lpMsgBuf);
        }
    }
    CheckFatal();

    // Start all the threads
    for (i = 0; i < g_numthreads; i++)
    {
        if (ResumeThread(threadhandle[i]) == 0xFFFFFFFF)
        {
            LPVOID          lpMsgBuf;

            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
                (LPTSTR)&lpMsgBuf, 0, NULL);
            // Process any inserts in lpMsgBuf.
            // ...
            // Display the string.
            Developer(DEVELOPER_LEVEL_ERROR, "ResumeThread #%d [%08X] failed : %s\n", i, threadhandle[i], lpMsgBuf);
            Fatal(assume_THREAD_ERROR, "Unable to start thread #%d", i);
            // Free the buffer.
            LocalFree(lpMsgBuf);
        }
    }
    CheckFatal();

    // Wait for threads to complete
    for (i = 0; i < g_numthreads; i++)
    {
        Developer(DEVELOPER_LEVEL_MESSAGE, "WaitForSingleObject on thread #%d [%08X]\n", i, threadhandle[i]);
        WaitForSingleObject(threadhandle[i], INFINITE);
    }
    threads_UninitCrit();

    q_entry = NULL;
    threaded = false;
    end = I_FloatTime();
    if (pacifier)
    {
        printf("\r%60s\r", "");
    }
    Log(" (%.2f seconds)\n", end - start);
}