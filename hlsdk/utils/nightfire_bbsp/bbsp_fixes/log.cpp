
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "globals.h"
#include "helpers.h"
#include "log.h"
#include "threads.h"
#include "hlassert.h"

const char* g_Program = "Uninitialized variable ::g_Program";
char            g_Mapname[MAX_PATH] = "Uninitialized variable ::g_Mapname";
developer_level_t g_developer = DEFAULT_DEVELOPER;
bool            g_verbose = DEFAULT_VERBOSE;
bool            g_log = DEFAULT_LOG;

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

void            WriteLog(const char* const message)
{
    Safe_WriteLog(message);

    fprintf(stdout, message);
    fflush(stdout);
}

void Log(const char* const warning, ...)
{
    char            message[MAX_MESSAGE];

    va_list         argptr;

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

void Warning(const char* Format, ...)
{
    char Buffer[2048]; // [esp+8h] [ebp-1000h] BYREF
    char a1[2048]; // [esp+808h] [ebp-800h] BYREF
    va_list ArgList; // [esp+1010h] [ebp+8h] BYREF

    va_start(ArgList, Format);
    _vsnprintf(Buffer, 0x800u, Format, ArgList);
    snprintf(a1, 0x800u, "Warning: %s\n", Buffer);
    WriteLog(a1);
}

void ResetErrorLog()
{
    char FileName[260]; // [esp+0h] [ebp-104h] BYREF

    if (g_log)
    {
        safe_snprintf(FileName, 0x104u, "%s.err", g_Mapname);
        _unlink(FileName);
    }
}

void ResetLog()
{
    char FileName[260]; // [esp+0h] [ebp-104h] BYREF

    if (g_log)
    {
        safe_snprintf(FileName, 260u, "%s.log", g_Mapname);
        _unlink(FileName);
    }
}

void OpenLog(const int clientid)
{
    char FileName[MAX_PATH]; // [esp+0h] [ebp-104h] BYREF

    if (g_log)
    {
        safe_snprintf(FileName, MAX_PATH, "%s.log", g_Mapname);
        CompileLog = fopen(FileName, "a");
        if (!CompileLog)
        {
            fprintf(stderr, "ERROR: Could not open logfile %s", FileName);
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
    int i; // esi

    Log("Command line: ");
    for (i = 0; i < argc; ++i)
    {
        if (strchr(argv[i], 32))
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
    const char* lightingSetting;

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

        lightingSetting = g_lighting ? "on" : "off";
        Log("build lighting bsp  [ %7s ] [ %7s ]\n", lightingSetting, "on");

        Log("max node size       [ %7d ] [ %7d ] (Min %d) (Max %d)\n", g_MaxNodeSize, 1024, 64, 4096);
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
    char            message[2048];
    char            message2[2048];
    va_list         argptr;

    if (count > 0) // make sure it only gets called once
    {
        return;
    }
    count++;

    va_start(argptr, warning);
    vsnprintf(message, 2048, warning, argptr);
    va_end(argptr);

    safe_snprintf(message2, MAX_MESSAGE, "Error: %s\n", message);
    WriteLog(message2);
    LogError(message2);
}

void LogError(const char* const message)
{
    if (g_log && CompileLog)
    {
        char            logfilename[MAX_PATH];
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
    char            message[2048];
    char            message2[2048];
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
    vsnprintf(message, 2048, error, argptr);
    va_end(argptr);

    safe_snprintf(message2, MAX_MESSAGE, "Error: %s\n", message);
    WriteLog(message2);
    LogError(message2);

    fatal = 1;
    CheckFatal();
}

void Fatal(assume_msgs msgid, const char* const warning, ...)
{
    char            message[2048];
    char            message2[2048];

    va_list         argptr;

    va_start(argptr, warning);
    vsnprintf(message, 2048, warning, argptr);
    va_end(argptr);

    safe_snprintf(message2, MAX_MESSAGE, "Error: %s\n", message);
    WriteLog(message2);
    LogError(message2);

    {
        char message[MAX_MESSAGE];
        const MessageTable_t* msg = GetAssume(msgid);

        safe_snprintf(message, MAX_MESSAGE, "%s\nDescription: %s\nHowto Fix: %s\n", msg->title, msg->text, msg->howto);
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
        char            message[MAX_MESSAGE];
        const MessageTable_t* msg = GetAssume(msgid);

        safe_snprintf(message, MAX_MESSAGE, "%s\nDescription: %s\nHowto Fix: %s\n", msg->title, msg->text, msg->howto);
        Error(message);
    }
}