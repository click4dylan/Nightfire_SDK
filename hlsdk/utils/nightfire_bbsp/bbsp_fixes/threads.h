#pragma once

#define DEFAULT_NUMTHREADS -1
#define	MAX_THREADS	64

typedef enum
{
    eThreadPriorityLow = -1,
    eThreadPriorityNormal,
    eThreadPriorityHigh
}
q_threadpriority;

typedef void    (*q_threadfunction) (int);

extern int      g_numthreads;
extern q_threadpriority g_threadpriority;

#define DEFAULT_THREAD_PRIORITY eThreadPriorityNormal

extern void ThreadSetDefault();
extern void ThreadSetPriority(q_threadpriority type);
extern void ThreadUnlock();
extern void ThreadLock();

extern int GetThreadWork();
extern void     RunThreadsOnIndividual(int workcnt, bool showpacifier, q_threadfunction);
extern void     RunThreadsOn(int workcnt, bool showpacifier, q_threadfunction);

#define NamedRunThreadsOn(n,p,f) { Log("%s\n", #f ":"); RunThreadsOn(n,p,f); }
#define NamedRunThreadsOnIndividual(n,p,f) { Log("%s\n", #f ":"); RunThreadsOnIndividual(n,p,f); }