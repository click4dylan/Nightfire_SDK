#include <Windows.h>
#include "console.h"
#include "conproc.h"
#include "globals.h"
#include <string>

char* Sys_ConsoleInput(void)
{
	INPUT_RECORD	recs[1024];
	unsigned long	dummy;
	int				ch;
	unsigned long	numread, numevents;

	while (1) {
		if (!GetNumberOfConsoleInputEvents(hinput, &numevents))
			exit(-1);

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			exit(-1);

		if (numread != 1)
			exit(-1);


		if (recs[0].EventType == KEY_EVENT) {
			if (!recs[0].Event.KeyEvent.bKeyDown) {
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;
				switch (ch) {
				case '\r':
					WriteFile(houtput, "\r\n", 2, &dummy, NULL);
					if (console_textlen) {
						console_text[console_textlen] = 0;
						console_textlen = 0;
						return console_text;
					}
					break;

				case '\b':
					if (console_textlen) {
						console_textlen--;
						WriteFile(houtput, "\b \b", 3, &dummy, NULL);
					}
					break;

				default:
					if (ch >= ' ') {
						if (console_textlen < sizeof(console_text) - 2) {
							WriteFile(houtput, &ch, 1, &dummy, NULL);
							console_text[console_textlen] = ch;
							console_textlen++;
						}
					}

					break;

				}
			}
		}
	}

	return NULL;
}

void WriteStatusText(char* szText) 
{
	char szFullLine[81];
	COORD coord;
	DWORD dwWritten = 0;
	WORD wAttrib[80];

	int i;

	for (i = 0; i < 80; i++) {
		wAttrib[i] = FOREGROUND_RED | FOREGROUND_INTENSITY;
	}

	memset(szFullLine, 0, sizeof(szFullLine));
	_snprintf(szFullLine, sizeof(szFullLine), "%s", szText);

	coord.X = 0;
	coord.Y = 0;

	WriteConsoleOutputAttribute(houtput, wAttrib, 80, coord, &dwWritten);
	WriteConsoleOutputCharacter(houtput, szFullLine, 80, coord, &dwWritten);
}

int CreateConsoleWindow(void)
{
	if (!AllocConsole())
	{
		return 0;
	}

	hinput = GetStdHandle(STD_INPUT_HANDLE);
	houtput = GetStdHandle(STD_OUTPUT_HANDLE);

	InitConProc();

	return 1;
}

void ProcessConsoleInput(void)
{
	char* s;

	if (!g_EngineAPI.ConsoleBuffer_AddText)
		return;

	do {
		s = Sys_ConsoleInput();
		if (s) {
			char szBuf[256];
			_snprintf(szBuf, sizeof(szBuf), "%s\n", s);
			g_EngineAPI.ConsoleBuffer_AddText(szBuf);
		}
	} while (s);
}


void Sys_ConsoleOutput(char* string)
{
	unsigned long dummy;
	char	text[256];

	if (console_textlen) {
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen + 1] = '\r';
		text[console_textlen + 2] = 0;
		WriteFile(houtput, text, console_textlen + 2, &dummy, NULL);
	}

	WriteFile(houtput, string, strlen(string), &dummy, NULL);

	if (console_textlen)
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);

	UpdateStatus(1 /* force */);
}

void DestroyConsoleWindow(void)
{
	FreeConsole();

	// shut down QHOST hooks if necessary
	DeinitConProc();
}

void ProcessCommands(char* cmdline)
{
	const char* v1; // r28
	const char* v2; // r30
	char v3; // r10
	char v4; // r11
	char v5; // r4
	int v6; // r8
	int v7; // r10
	char v8; // r11
	char i; // r9
	char tmp[2052]; // [sp+40h] [-820h] BYREF

	v1 = cmdline;
	v2 = cmdline;
	v3 = *cmdline;
	v4 = *cmdline;
	if (*cmdline)
	{
		while (1)
		{
			if (v3 == '+')
			{
				if (v2 == v1)
					break;
				v5 = *(v2 - 1);
				if (v5 == ' ' || v5 == '\t')
					break;
			}
			if (v4)
				v4 = *++v2;
		LABEL_22:
			v3 = v4;
			if (!v4)
				return;
		}
		memset(tmp, 0, 2048u);
		++v2;
		v6 = 0;
		v7 = 0;
		while (++v6 <= '\n')
		{
			v8 = *v2;
			for (i = *v2; *v2 != ' ' && v8 != '\t' && v8; i = *v2)
			{
				tmp[v7] = i;
				++v2;
				if (++v7 > 2047)
					return;
				v8 = *v2;
			}
			tmp[v7++] = ' ';
			if (v8)
				v8 = *++v2;
			if (v8 == '+' || !v8)
			{
				strcpy(&tmp[v7 - 1], "\n");
				g_EngineAPI.ConsoleBuffer_AddText(tmp);
				v4 = *v2;
				goto LABEL_22;
			}
		}
	}
}