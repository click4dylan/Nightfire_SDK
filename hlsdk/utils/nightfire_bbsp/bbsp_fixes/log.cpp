
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "globals.h"
#include "helpers.h"
#include "log.h"
#include "threads.h"
#include "hlassert.h"
#include "filelib.h"

const char* g_Program = "Uninitialized variable ::g_Program";
char            g_Mapname[MAX_PATH] = "Uninitialized variable ::g_Mapname";
developer_level_t g_developer = DEFAULT_DEVELOPER;
bool            g_verbose = DEFAULT_VERBOSE;
bool            g_log = DEFAULT_LOG;

#define MAX_ERROR   2048
#define MAX_WARNING 2048
#define MAX_MESSAGE 2048

unsigned long   g_clientid = 0;
unsigned long   g_nextclientid = 0;

static FILE* CompileLog = NULL;
static bool     fatal = false;

static void seconds_to_hhmm(unsigned int elapsed_time, unsigned& days, unsigned& hours, unsigned& minutes, unsigned& seconds)
{
    seconds = elapsed_time % 60;
    elapsed_time /= 60;

    minutes = elapsed_time % 60;
    elapsed_time /= 60;

    hours = elapsed_time % 24;
    elapsed_time /= 24;

    days = elapsed_time;
}

void Safe_WriteLog(const char* const message)
{
    const char* c;

    if (!CompileLog)
        return;

    c = &message[0];

    while (1)
    {
        if (!*c)
            return; // end of string

        if (*c == '\n')
            fputc('\r', CompileLog);

        fputc(*c, CompileLog);

        c++;
    }
}

void WriteLog(const char* const message)
{
    Safe_WriteLog(message);

    fprintf(stdout, message);
    fflush(stdout);
}

void Log(const char* const warning, ...)
{
    char message[MAX_MESSAGE];

    va_list argptr;

    va_start(argptr, warning);
    vsnprintf(message, MAX_MESSAGE, warning, argptr);
    va_end(argptr);

    WriteLog(message);
}

void LogTimeElapsed(float elapsed_time)
{
    unsigned days = 0;
    unsigned hours = 0;
    unsigned minutes = 0;
    unsigned seconds = 0;

    seconds_to_hhmm(elapsed_time, days, hours, minutes, seconds);

    if (days)
    {
        Log("%.2f seconds elapsed [%ud %uh %um %us]\n", elapsed_time, days, hours, minutes, seconds);
    }
    else if (hours)
    {
        Log("%.2f seconds elapsed [%uh %um %us]\n", elapsed_time, hours, minutes, seconds);
    }
    else if (minutes)
    {
        Log("%.2f seconds elapsed [%um %us]\n", elapsed_time, minutes, seconds);
    }
    else
    {
        Log("%.2f seconds elapsed\n", elapsed_time);
    }
}

void Warning(const char* warning, ...)
{
    char message[MAX_WARNING];
    char message2[MAX_MESSAGE];
    va_list argptr;

    va_start(argptr, warning);
    _vsnprintf(message, MAX_MESSAGE, warning, argptr);
    va_end(argptr);

    snprintf(message2, MAX_MESSAGE, "Warning: %s\n", message);
    WriteLog(message2);
}

void ResetErrorLog()
{
    char logfilename[MAX_PATH];

    if (g_log)
    {
        safe_snprintf(logfilename, MAX_PATH, "%s.err", g_Mapname);
        _unlink(logfilename);
    }
}

void ResetLog()
{
    char logfilename[MAX_PATH];

    if (g_log)
    {
        safe_snprintf(logfilename, MAX_PATH, "%s.log", g_Mapname);
        _unlink(logfilename);
    }
}

void OpenLog(const int clientid)
{
    char logfilename[MAX_PATH];

    if (g_log)
    {
        safe_snprintf(logfilename, MAX_PATH, "%s.log", g_Mapname);
        CompileLog = fopen(logfilename, "a");
        if (!CompileLog)
        {
            fprintf(stderr, "ERROR: Could not open logfile %s", logfilename);
            fflush(stderr);
        }
    }
}

void LogEnd()
{
    Log("\n-----   END   %s -----\n\n\n\n", g_Program);
}

void CloseLog()
{
    if (g_log)
    {
        if (CompileLog)
        {
            LogEnd();
            fflush(CompileLog);
            fclose(CompileLog);
            CompileLog = 0;
        }
    }
}

void DisplayDeveloperLevel()
{
    char Dest[2048]; // [esp+8h] [ebp-800h] BYREF

    safe_strncpy(Dest, "Developer messages enabled : [", 0x800u);
    if (g_developer >= 6)
        safe_strncat(Dest, "MegaSpam ", 0x800u);
    if (g_developer >= 5)
        safe_strncat(Dest, "Spam ", 0x800u);
    if (g_developer >= 4)
        safe_strncat(Dest, "Fluff ", 0x800u);
    if (g_developer >= 3)
        safe_strncat(Dest, "Message ", 0x800u);
    if (g_developer >= 2)
        safe_strncat(Dest, "Warning ", 0x800u);
    if (g_developer >= 1)
        safe_strncat(Dest, "Error", 0x800u);
    if (g_developer)
    {
        safe_strncat(Dest, "]\n", 0x800u);
        Log(Dest);
    }
}

void LogArgs(int argc, const char** argv)
{
    int i;

    Log("Command line: ");
    for (i = 0; i < argc; ++i)
    {
        if (strchr(argv[i], ' '))
            Log("\"%s\"", argv[i]);
        else
            Log("%s ", argv[i]);
    }
    Log("\n");
}

void Banner()
{
    Log("%s v5.0.1 rel (%s)\n", g_Program, "Jul 30 2002");
    Log("Submit detailed bug reports to (zoner@gearboxsoftware.com)\n");
}

void LogStart(int argc, const char** argv)
{
    Banner();
    Log("-----  BEGIN  %s -----\n", g_Program);
    LogArgs(argc, argv);
    DisplayDeveloperLevel();
}

void Settings()
{
    const char* verboseSetting;
    const char* logSetting;
    const char* chartSetting;
    const char* estimateSetting;
    const char* prioritySetting;
    const char* waterSetting;
    const char* onlyentsSetting;
    const char* noclipSetting;
    const char* nofillSetting;
    const char* notjuncSetting;
    const char* nosubdivSetting;
    const char* nomergeSetting;
    const char* lightingSetting;
    const char* nostripSetting;

    if (g_info)
    {
        Log("\n-= Current %s Settings =-\n", g_Program);
        Log("Name               |  Setting  |  Default\n");
        Log("-------------------|-----------|-------------------------\n");
        Log("threads             [ %7d ] [  Varies ]\n", g_numthreads);

        verboseSetting = g_verbose ? "on" : "off";
        Log("verbose             [ %7s ] [ %7s ]\n", verboseSetting, "off");

        logSetting = g_log ? "on" : "off";
        Log("log                 [ %7s ] [ %7s ]\n", logSetting, "on");

        Log("developer           [ %7d ] [ %7d ]\n", g_developer, 0);

        chartSetting = g_chart ? "on" : "off";
        Log("chart               [ %7s ] [ %7s ]\n", chartSetting, "off");

        estimateSetting = g_estimate ? "on" : "off";
        Log("estimate            [ %7s ] [ %7s ]\n", estimateSetting, "off");

        if (g_threadpriority == -1)
        {
            prioritySetting = "Low";
        }
        else if (g_threadpriority == 1)
        {
            prioritySetting = "High";
        }
        else
        {
            prioritySetting = "Normal";
        }
        Log("priority            [ %7s ] [ %7s ]\n", prioritySetting, "Normal");

        waterSetting = g_water ? "off" : "on";
        Log("nowater             [ %7s ] [ %7s ]\n", waterSetting, "off");

        onlyentsSetting = g_onlyents ? "on" : "off";
        Log("onlyents            [ %7s ] [ %7s ]\n", onlyentsSetting, "off");

        noclipSetting = g_noclip ? "on" : "off";
        Log("noclip              [ %7s ] [ %7s ]\n", noclipSetting, "off");

        nofillSetting = g_nofill ? "on" : "off";
        Log("nofill              [ %7s ] [ %7s ]\n", nofillSetting, "off");

        notjuncSetting = g_notjunc ? "on" : "off";
        Log("notjunc             [ %7s ] [ %7s ]\n", notjuncSetting, "off");

#ifdef SUBDIVIDE
        nosubdivSetting = g_nosubdiv ? "on" : "off";
        Log("nosubdiv            [ %7s ] [ %7s ]\n", nosubdivSetting, "off");

        if (!g_nosubdiv)
            Log("subdivide size      [ %7d ] [ %7d ] (Min %d) (Max %d)\n", g_subdivide_size, 256, 1, 65536);
#endif

#ifdef MERGE
        nomergeSetting = g_nomerge ? "on" : "off";
        Log("nomerge             [ %7s ] [ %7s ]\n", nomergeSetting, "off");
#endif

#ifdef STRIP
        nostripSetting = g_nostrip ? "on" : "off";
        Log("nostrip             [ %7s ] [ %7s ]\n", nostripSetting, "off");
#endif

        lightingSetting = g_lighting ? "on" : "off";
        Log("build lighting bsp  [ %7s ] [ %7s ]\n", lightingSetting, "on");

#ifdef UNCLAMP_MAX_NODE_SIZE
        Log("max node size       [ %7d ] [ %7d ] (Min %d) (Max %d)\n", g_MaxNodeSize, 1024, 64, 65536);
#else
        Log("max node size       [ %7d ] [ %7d ] (Min %d) (Max %d)\n", g_MaxNodeSize, 1024, 64, 4096);
#endif

#ifdef VARIABLE_LIGHTING_MAX_NODE_SIZE
#ifdef UNCLAMP_MAX_NODE_SIZE
        Log("max L node size     [ %7d ] [ %7d ] (Min %d) (Max %d)\n", g_LightingMaxNodeSize, 512, 64, 65536);
#else
        Log("max L node size     [ %7d ] [ %7d ] (Min %d) (Max %d)\n", g_LightingMaxNodeSize, 512, 64, 4096);
#endif
#endif
        Log("base lightmap scale [ %7d ] [ %7d ]\n", g_blscale, 16);
        Log("inc lightmap scale  [ %7d ] [ %7d ]\n", g_ilscale, 16);
        Log("\n\n");
    }
}

void PrintAllocationData(const char* a1)
{
    Developer(DEVELOPER_LEVEL_MESSAGE, "%s", a1);
    Developer(DEVELOPER_LEVEL_MESSAGE, "Allocation status:\n");
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num maps\n", g_numMapsAlloced);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num ents\n", g_numEnts);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num epairs\n", g_numEPairs);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num portals\n", g_NumPortals);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num faces\n", g_numFaces);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num nodes\n", g_numNodesAllocated);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num brushes\n", g_numBrushes);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num brushsides\n", g_numBrushSides);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num windings\n", g_numWindings);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num markfaces\n", g_numNodesWithMarkFaces);
    Developer(DEVELOPER_LEVEL_MESSAGE, " %5d num markbrushes\n", g_numNodesWithMarkBrushes);
    Developer(DEVELOPER_LEVEL_MESSAGE, "\n");
}

int GlobUsage(const char* const szItem, const int itemstorage, const int maxstorage)
{
    float           percentage = maxstorage ? itemstorage * 100.0 / maxstorage : 0.0;

    Log("%-12s     [variable]    %7i/%-7i  (%4.1f%%)\n", szItem, itemstorage, maxstorage, percentage);

    return itemstorage;
}

void Verbose(const char* const warning, ...)
{
    if (g_verbose)
    {
        char            message[MAX_MESSAGE];

        va_list         argptr;

        va_start(argptr, warning);
        vsnprintf(message, MAX_MESSAGE, warning, argptr);
        va_end(argptr);

        WriteLog(message);
    }
}

void Developer(developer_level_t  level, const char* warning, ...)
{
    if (level <= g_developer)
    {
        char            message[MAX_MESSAGE];

        va_list         argptr;

        va_start(argptr, warning);
        vsnprintf(message, MAX_MESSAGE, warning, argptr);
        va_end(argptr);

#if 1
        WriteLog(message);
#else
        DWORD WriteLog = 0x419C30;
        __asm
        {
            lea esi, message
            call WriteLog
        }
#endif
    }
}

int count = 0;

void ResetPrintOnce()
{
    count = 0;
}

void PrintOnce(const char* const warning, ...)
{
    char            message[MAX_WARNING];
    char            message2[MAX_WARNING];
    va_list         argptr;

    if (count > 0) // make sure it only gets called once
    {
        return;
    }
    count++;

    va_start(argptr, warning);
    vsnprintf(message, MAX_WARNING, warning, argptr);
    va_end(argptr);

    safe_snprintf(message2, MAX_WARNING, "Error: %s\n", message);
    WriteLog(message2);
    LogError(message2);
}

void LogError(const char* const message)
{
    if (g_log && CompileLog)
    {
        char logfilename[MAX_PATH];
        FILE* ErrorLog = NULL;

        safe_snprintf(logfilename, MAX_PATH, "%s.err", g_Mapname);
        ErrorLog = fopen(logfilename, "a");

        if (ErrorLog)
        {
            fprintf(ErrorLog, "%s: %s\n", g_Program, message);
            fflush(ErrorLog);
            fclose(ErrorLog);
        }
        else
        {
            fprintf(stderr, "ERROR: Could not open error logfile %s", logfilename);
            fflush(stderr);
        }
    }
}

void Error(const char* const error, ...)
{
    char            message[MAX_ERROR];
    char            message2[MAX_ERROR];
    va_list         argptr;

    char* wantint3 = getenv("WANTINT3");
    if (wantint3)
    {
        if (atoi(wantint3))
        {
            __asm
            {
                int 3;
            }
        }
    }

    va_start(argptr, error);
    vsnprintf(message, MAX_ERROR, error, argptr);
    va_end(argptr);

    safe_snprintf(message2, MAX_MESSAGE, "Error: %s\n", message);
    WriteLog(message2);
    LogError(message2);

    fatal = 1;
    CheckFatal();
}

void Fatal(assume_msgs msgid, const char* const warning, ...)
{
    char message[MAX_WARNING];
    char message2[MAX_WARNING];

    va_list argptr;

    va_start(argptr, warning);
    vsnprintf(message, MAX_WARNING, warning, argptr);
    va_end(argptr);

    safe_snprintf(message2, MAX_WARNING, "Error: %s\n", message);
    WriteLog(message2);
    LogError(message2);

    {
        char message[MAX_WARNING];
        const MessageTable_t* msg = GetAssume(msgid);

        safe_snprintf(message, MAX_WARNING, "%s\nDescription: %s\nHowto Fix: %s\n", msg->title, msg->text, msg->howto);
        PrintOnce(message);
    }

    fatal = 1;
}

void CheckFatal()
{
    if (fatal)
    {
        hlassert(false);
        exit(1);
    }
}

void hlassume(bool exp, assume_msgs msgid)
{
    if (!exp)
    {
        char message[MAX_MESSAGE];
        const MessageTable_t* msg = GetAssume(msgid);

        safe_snprintf(message, MAX_MESSAGE, "%s\nDescription: %s\nHowto Fix: %s\n", msg->title, msg->text, msg->howto);
        Error(message);
    }
}

void CheckForErrorLog()
{
    if (g_log)
    {
        char            logfilename[_MAX_PATH];

        safe_snprintf(logfilename, _MAX_PATH, "%s.err", g_Mapname);
        if (q_exists(logfilename))
        {
            Log(">> There was a problem compiling the map.\n"
                ">> Check the file %s.log for the cause.\n",
                g_Mapname);
            exit(1);
        }
    }
}