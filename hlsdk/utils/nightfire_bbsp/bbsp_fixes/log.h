#pragma once
#include "messages.h"

typedef enum
{
    DEVELOPER_LEVEL_ALWAYS,
    DEVELOPER_LEVEL_ERROR,
    DEVELOPER_LEVEL_WARNING,
    DEVELOPER_LEVEL_MESSAGE,
    DEVELOPER_LEVEL_FLUFF,
    DEVELOPER_LEVEL_SPAM,
    DEVELOPER_LEVEL_MEGASPAM
}
developer_level_t;

extern const char* g_Program;
extern char     g_Mapname[_MAX_PATH];

#define DEFAULT_DEVELOPER   DEVELOPER_LEVEL_ALWAYS
#define DEFAULT_VERBOSE     false
#define DEFAULT_LOG         true

extern developer_level_t g_developer;
extern bool          g_verbose;
extern bool          g_log;
extern unsigned long g_clientid;                           // Client id of this program
extern unsigned long g_nextclientid;                       // Client id of next client to spawn from this server

extern void Safe_WriteLog(const char* const message);
extern void WriteLog(const char* const message);
extern void Log(const char* const message, ...);
extern void LogTimeElapsed(float elapsed_time);
extern void Warning(const char* Format, ...);
extern void ResetErrorLog();
extern void ResetLog();
extern void OpenLog(const int clientid);
extern void CloseLog();
extern void LogEnd();
extern void LogStart(int argc, const char** argv);
extern void LogArgs(int argc, const char** argv);
extern void Banner();
extern void Settings();
extern void DisplayDeveloperLevel();
extern void PrintAllocationData(const char* a1);
extern int GlobUsage(const char* const szItem, const int itemstorage, const int maxstorage);
extern void Verbose(const char* const warning, ...);
extern void Developer(developer_level_t  level, const char* warning, ...);
extern void ResetPrintOnce();
extern void PrintOnce(const char* const warning, ...);
extern void LogError(const char* const message);
extern void Error(const char* const error, ...);
extern void Fatal(assume_msgs msgid, const char* const warning, ...);
extern void CheckFatal();
extern void hlassume(bool exp, assume_msgs msgid);