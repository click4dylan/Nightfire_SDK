#pragma once

#define DEFAULT_NUMTHREADS -1

typedef enum
{
    eThreadPriorityLow = -1,
    eThreadPriorityNormal,
    eThreadPriorityHigh
}
q_threadpriority;

extern int      g_numthreads;
extern q_threadpriority g_threadpriority;

#define DEFAULT_THREAD_PRIORITY eThreadPriorityNormal

extern void ThreadSetDefault();
extern void ThreadSetPriority(q_threadpriority type);
extern void ThreadUnlock();
extern void ThreadLock();