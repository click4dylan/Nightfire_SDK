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

// note: TODO: FIXME: nightfire has a bug in this function. 
// the only way to have command line options work properly is to follow this order:
// all -  keys with no value go first (-dedicated)
// all - keys with a value (-port 26015) go second
// all + keys with or without value (+map ctf_romania) go last
void __cdecl ProcessCommands(const char* input)
{
	char buffer[2048];

	const char* current = input;
	if (*input)
	{
		while (1)
		{
			if (*current == '+')
			{
				if (current == input)
					break;
				char v3 = *(current - 1);
				if (v3 == ' ' || v3 == '\t')
					break;
			}
			if (*current)
			{
				++current;
			LABEL_20:
				if (*current)
					continue;
			}
			return;
		}
		memset(buffer, 0, sizeof(buffer));
		++current;
		int i = 0;
		for (int command_number = 0; command_number < 10; ++command_number)
		{
			for (char current_char = *current; *current != ' '; current_char = *current)
			{
				if (current_char == '\t' || !current_char)
					break;
				++current;
				buffer[i++] = current_char;
				if (i >= 2048)
					return;
			}

			buffer[i++] = ' ';
			if (*current != '\0')
				++current;
			if (*current == '+' || !*current)
			{
				buffer[i - 1] = '\n';
				buffer[i] = 0;
				g_EngineAPI.ConsoleBuffer_AddText(buffer);
				goto LABEL_20;
			}
		}
	}
}