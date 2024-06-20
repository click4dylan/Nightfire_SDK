#pragma once

#include "globals.h"
#include "cmdlib.h"


#define	MAXTOKEN 4096

extern char     g_token[MAXTOKEN];

extern bool     GetToken(bool crossline);
extern void     UnGetToken();
extern bool     TokenAvailable();
extern bool IsScriptNonEmptyAndNotComment();
extern void            LoadScriptFile(const char* const filename);
extern void            ParseFromMemory(char* buffer, const int size);