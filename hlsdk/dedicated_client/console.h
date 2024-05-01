#pragma once

extern char* Sys_ConsoleInput(void);
extern void Sys_ConsoleOutput(char* string);
extern void WriteStatusText(char* szText);
extern int CreateConsoleWindow(void);
extern void ProcessConsoleInput(void);
extern void DestroyConsoleWindow(void);
extern void UpdateStatus(int force);
extern void ProcessCommands(const char* cmdline);