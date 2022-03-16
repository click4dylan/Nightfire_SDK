#include <NightfireFileSystem.h>
#include "globals.h"
#include "pattern_scanner.h"

NightfireFileSystem g_NightfireFilesystem;
NightfireFileSystem* g_pNightfireFileSystem = &g_NightfireFilesystem;


void NightfireFileSystem::Init(unsigned long engine_dll)
{
	//repeating the cardinal sin of metamod, TODO: create castable definitions for each func type
	version = NIGHTFIRE_FILESYSTEM_VERSION;
	COM_Init = (void(*)())FindMemoryPattern(engine_dll, "A0 ? ? ? ? 84 C0 0F 85 B3 00 00 00 66 83 3D ? ? ? ? 01 75 3E");
	COM_InitFilesystem = (void(*)())FindMemoryPattern(engine_dll, "64 A1 00 00 00 00 6A FF 68 ? ? ? ? 50 64 89 25 00 00 00 00 81 EC 1C 01 00 00");
	COM_InitArgv = (void(*)(int, char**))FindMemoryPattern(engine_dll, "55 8B 6C 24 08 56 8B 74 24 10 57 33 C0 33 FF 90");
	COM_Shutdown = (void(*)())FindMemoryPattern(engine_dll, "A0 ? ? ? ? 83 EC 08 53 33 DB 3A C3 0F 84 89 00 00 00 A1 ? ? ? ?");
	COM_FileSeek = (void(*)(HCOMFILE &, int))FindMemoryPattern(engine_dll, "83 EC 08 56 8B 74 24 10 8B 4E 0C 85 C9 74 27 8B 15 ? ? ? ? 8B 12 8B 01");
	Sys_FileSeek = (void(*)(int, int))FindMemoryPattern(engine_dll, "8B 44 24 08 8B 4C 24 04 8B 14 8D ? ? ? ? 6A 00 50 52 E8 ? ? ? ? 83 C4 0C C3");
	COM_FileTell = (int(*)(HCOMFILE))FindMemoryPattern(engine_dll, "8B 44 24 0C 50 E8 ? ? ? ? 8B 4C 24 08 83 C4 04 2B C1 C3");
	Sys_FileTell = (int(*)(FILE *))FindMemoryPattern(engine_dll, "8B 44 24 04 8B 0C 85 ? ? ? ? 89 4C 24 04 E9 ? ? ? ?");
	COM_ReadFile = (int(*)(HCOMFILE &, void*, int))FindMemoryPattern(engine_dll, "8B 44 24 04 8B 48 0C 83 EC 08 85 C9 74 2D 8B 01 8D 54 24 0C 52");
	Sys_FileRead = (int (*)(int, void*, int))(FindMemoryPattern(engine_dll, "C3 90 90 90 90 8B 44 24 04 8B 0C 85 ? ? ? ? 8B 54 24 0C 8B 44 24 08 51 52 6A 01 50") + 5);
	Sys_FileOpenRead = (int (*)(char*, int*))FindMemoryPattern(engine_dll, "B8 01 00 00 00 57 EB 08 8D A4 24 00 00 00 00 90");
	Sys_FileOpenWrite = (int (*)(char*))FindMemoryPattern(engine_dll, "56 B8 01 00 00 00 57 EB 07 8D A4 24 00 00 00 00");
	COM_CloseFile = (void(*)(HCOMFILE))FindMemoryPattern(engine_dll, "8B 4C 24 10 85 C9 74 07 8B 01 6A 01 FF 10 C3 8B 4C 24 0C 51 E8 ? ? ? ? 59 C3");
	Sys_FileClose = (void(*)(int))FindMemoryPattern(engine_dll, "56 8B 74 24 08 85 F6 7C 1B");
	Sys_FileTime = (int(*)(char*))FindMemoryPattern(engine_dll, "8B 4C 24 04 83 EC 24 8D 44 24 00 50 51 E8 ? ? ? ? 83 C4 08 83 F8 FF");
	COM_CompareFileTime = (bool(*)(char*, char*, int*))FindMemoryPattern(engine_dll, "8B 44 24 04 83 EC 10 53 55 8B 6C 24 24 56 57 33 FF 85 C0");
	Sys_CompareFileTime = (int(*)(int*, int*))FindMemoryPattern(engine_dll, "8B 4C 24 04 33 C0 85 C9 74 1B 8B 54 24 08 85 D2 74 13");
	COM_WriteFile = (void(*)(const char*, void*, int))FindMemoryPattern(engine_dll, "8B 44 24 04 81 EC 04 01 00 00 50 68 ? ? ? ? 68 ? ? ? ?");
	COM_WriteFile_ = (int(*)(HCOMFILE &, const void*, int))FindMemoryPattern(engine_dll, "8B 54 24 04 8B 42 08 89 44 24 04 E9");
	Sys_FileWrite = (int(*)(int, void*, int))(FindMemoryPattern(engine_dll, "C3 90 90 90 90 8B 44 24 04 8B 0C 85 ? ? ? ? 8B 54 24 0C 8B 44 24 08 51 52 6A 01 50") + 0x35);
	COM_OpenFile = (int(*)(const char*, HCOMFILE *))FindMemoryPattern(engine_dll, "8B 44 24 08 8B 4C 24 04 6A 00 50 51 6A 00 6A 00 E8 ? ? ? ? 83 C4 14 C3");
	COM_OpenFileInDir = (int(*)(const char*, const char*, HCOMFILE*))FindMemoryPattern(engine_dll, "8B 44 24 0C 8B 4C 24 08 8B 54 24 04 6A 00 50 51 52 6A 00 E8 ? ? ? ? 83 C4 14 C3");
	COM_FileExists = (bool(*)(const char*, int*))FindMemoryPattern(engine_dll, "8B 4C 24 04 83 EC 10 53 56 6A 00 8D 44 24 0C 50 51 6A 00 6A 00 32 DB");
	COM_FindFile = (int (*)(const char*, HCOMFILE*, FILE**))FindMemoryPattern(engine_dll, "8B 44 24 0C 8B 4C 24 08 8B 54 24 04 50 51 52 6A 00 6A 00 E8 ? ? ? ? 83 C4 14 C3");
	COM_FindFileSearch = (int(*)(searchpath_t **, const char*, const char*, HCOMFILE *, FILE **))FindMemoryPattern(engine_dll, "6A FF 68 ? ? ? ? 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 81 EC 38 01 00 00");
	COM_Token = (char(*)())FindMemoryPattern(engine_dll, "B8 ? ? ? ? C3 90 90 90 90 90 90 90 90 90 90 A0 6D 25 48 43");
	COM_UngetToken = (void(*)())(FindMemoryPattern(engine_dll, "90 C6 05 ? ? ? ? 01 C3") + 1);
	COM_ParseFile = (char* (*)(const char*, char*, unsigned int))FindMemoryPattern(engine_dll, "8B 44 24 04 56 50 E8 ? ? ? ? 8B 4C 24 14 8B 54 24 10 83 C4 04");
	COM_Parse = (char*(*)(const char*))FindMemoryPattern(engine_dll, "A0 ? ? ? ? 53 33 DB 3A C3 8B");
	COM_FreeFile = (void(*)(void*))FindMemoryPattern(engine_dll, "8B 44 24 04 85 C0 74 07 50 FF 15 ? ? ? ? C3 8B 44 24 0C 81 EC 00 04 00 00");
	COM_LoadFileLimit = (void* (*)(char*, int, int, int*, HCOMFILE*))FindMemoryPattern(engine_dll, "83 EC 30 53 55 56 57 8B 7C 24 54 8B 47 08");
	COM_LoadHeapFile = (unsigned char* (*)(const char*, int*))FindMemoryPattern(engine_dll, "8B 44 24 08 53 8B 5C 24 08 50 B8 02 00 00 00 E8 ? ? ? ? 83 C4 04 5B C3");
	COM_LoadHunkFile = (unsigned char* (*)(const char*))FindMemoryPattern(engine_dll, "53 8B 5C 24 08 6A 00 33 C0 E8 ? ? ? ? 83 C4 04 5B C3");
	COM_LoadStackFile = (unsigned char* (*)(const char*, void*, int, int*))FindMemoryPattern(engine_dll, "8B 44 24 08 8B 54 24 10 8B 4C 24 0C 53 8B 5C 24 08 A3 ? ? ? ?");
	COM_LoadTempFile = (unsigned char* (*)(const char*, int*))FindMemoryPattern(engine_dll, "8B 44 24 08 53 8B 5C 24 08 50 B8 01 00 00 00 E8 ? ? ? ? 83 C4 04 5B C3");
	Hunk_LowMark = (int(*)())FindMemoryPattern(engine_dll, "A1 ? ? ? ? C3 90 90 90 90 90 90 90 90 90 90 56");
	Hunk_FreeToLowMark = (void(*)(int))(FindMemoryPattern(engine_dll, "90 56 8B 74 24 08 85 F6 7C 08 3B 35 ? ? ? ? 7E 0E 56 68 ? ? ? ? E8 ? ? ? ? 83 C4 08 89 35 ? ? ? ?") + 1);
	COM_ExpandFilename = (int(*)(const char* name, unsigned int size))FindMemoryPattern(engine_dll, "81 EC 04 01 00 00 53 55 56 8B 35 ? ? ? ? 85 F6 57 8B BC 24 18 01 00 00");
	COM_StripExtension = (void* (*)(const char* in, char* out))FindMemoryPattern(engine_dll, "53 8B 5C 24 08 56 57 8B 3D ? ? ? ? 53 FF D7 53 8D 34 18");
	COM_DefaultExtension = (void* (*)(char* path, unsigned int pathStringLength, const char* extension))FindMemoryPattern(engine_dll, "56 8B 74 24 08 56 FF 15 ? ? ? ? 8A 4C 30 FF 80 F9 2F");
	COM_FixSlashes = (void(*)(char* path))FindMemoryPattern(engine_dll, "8B 44 24 04 80 38 00 74 17 8D A4 24 00 00 00 00 80 38 2F 75 03 C6 00 5C");
	COM_FOpenFile = (int(*)(const char* path, FILE * *dest))FindMemoryPattern(engine_dll, "8B 44 24 08 8B 4C 24 04 50 6A 00 51 6A 00 6A 00 E8 ? ? ? ? 83 C4 14 C3");
	COM_CreatePath = (void(*)(const char* path))FindMemoryPattern(engine_dll, "8B 44 24 04 81 EC 04 01 00 00 56 68 04 01 00 00 50 8D 4C 24 0C 51");
	Sys_mkdir = (int(*)(const char* path))FindMemoryPattern(engine_dll, "55 8B EC 53 8B 5D 08 56 57 53");
	COM_FileBase = (void(*)(const char* path, char* dest, unsigned int size))FindMemoryPattern(engine_dll, "56 57 8B 7C 24 0C 57 FF 15 ? ? ? ? 48 8B C8 85 C9 74 15");
	COM_FileSize = (int(*)(const char* path))FindMemoryPattern(engine_dll, "8B 4C 24 04 83 EC 10 56 6A 00 8D 44 24 08 50 51 6A 00 6A 00");
	COM_GetGameDir = (int(*)(char* dest, unsigned int size))FindMemoryPattern(engine_dll, "8B 44 24 04 85 C0 74 11 8B 4C 24 08 51");
	COM_GetWriteableDirectory = (const char* (*)())FindMemoryPattern(engine_dll, "B8 ? ? ? ? C3 90 90 90 90 90 90 90 90 90 90 A1");
	Sys_FindFirst = (const char* (*)(const char* path, char* filebase, unsigned int size))FindMemoryPattern(engine_dll, " A1 ? ? ? ? 56 33 F6 83 F8 FF");
	Sys_FindNext = (const char* (*)(char* filebase, unsigned int size))FindMemoryPattern(engine_dll, "A1 ? ? ? ? 56 68 ? ? ? ? 50 33 F6");
	Sys_FindClose = (void(*)())FindMemoryPattern(engine_dll, "A1 ? ? ? ? 50 FF 15 ? ? ? ? C7 05 ? ? ? ? FF FF FF FF");
	COM_Log = (void(*)(char* pszFile, char* fmt, ...))FindMemoryPattern(engine_dll, "8B 44 24 04 81 EC 00 04 00 00 85 C0 75 05");
	COM_Munge = (void(*)(unsigned char* data, int len, int seq))RelativeToAbsolute((FindMemoryPattern(engine_dll, "8D 4C 30 02 51 E8 ? ? ? ? 83 C4 18") + 6));
	COM_Munge2 = (void(*)(unsigned char* data, int len, int seq))RelativeToAbsolute((FindMemoryPattern(engine_dll, "83 C1 F8 51 83 C2 08 52 E8 ? ? ? ?") + 9));
	COM_Munge2swap4 = (void(*)(unsigned char* data, int len, int seq))RelativeToAbsolute((FindMemoryPattern(engine_dll, "50 8D 4C 24 10 6A 04 51 E8 ? ? ? ?") + 9));
	COM_Munge3 = (void(*)(unsigned char* data, int len, int seq))RelativeToAbsolute((FindMemoryPattern(engine_dll, "6A 04 51 89 54 24 34 E8 ? ? ? ? 8B 54 24 34") + 8));
	COM_UnMunge = (void(*)(unsigned char* data, int len, int seq))RelativeToAbsolute((FindMemoryPattern(engine_dll, "6A 20 55 E8 ? ? ? ? 0F BE 4D 00") + 4));
	COM_UnMunge2 = (void(*)(unsigned char* data, int len, int seq))RelativeToAbsolute((FindMemoryPattern(engine_dll, "E8 ? ? ? ? 83 C4 0C F7 C5 00 00 00 10") + 1));
	COM_UnMunge3 = (void(*)(unsigned char* data, int len, int seq))RelativeToAbsolute((FindMemoryPattern(engine_dll, "83 C9 FF 2B C8 81 E1 FF 00 00 00 51 6A 04") + 0x19));
	COM_ListMapsToLinkedList = (LinkedList * (*)(const char* str))FindMemoryPattern(engine_dll, "6A ? ? ? ? 0D 43 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 81 EC B4 00 00 00");
	COM_ListMaps = (void(*)(const char* str))FindMemoryPattern(engine_dll, "53 56 68 ? ? ? ? E8 ? ? ? ? 68");
	COM_CheckParm = (bool(*)(char* parm))FindMemoryPattern(engine_dll, "A1 ? ? ? ? 53 56 BE 01 00 00 00 3B C6 57 7E 2D");
	COM_AddGameDirectory = (void(*)(bool packfile, const char* pszBaseDir, const char* pszDir))FindMemoryPattern(engine_dll, "81 EC 10 04 00 00 53 8A 9C 24 18 04 00 00 84 DB 55 8B AC 24 24 04 00 00 56");
	COM_ChangeGameDir = (void(*)(const char* path))FindMemoryPattern(engine_dll, "A1 ? ? ? ? 57 33 FF 85 C0 74 39 53");
	COM_BlockSequenceCRCByte = (unsigned char(*)(char* base, int length, int sequence))FindMemoryPattern(engine_dll, "83 EC 40 56 8B 74 24 50 85 F6 57 7D 0D");
	COM_ExplainDisconnection = (void(*)(bool drop))FindMemoryPattern(engine_dll, "A1 ? ? ? ? 83 EC 14 85 C0 56 57 0F");
	g_Archive = *(void**)(FindMemoryPattern(engine_dll, "8B 0D ? ? ? ? 50 C7 84 24 E4 00 00 00 01") + 2);

#ifdef _DEBUG
	// rudimentary bugcheck, 32 bit only
	for (unsigned long* off = (unsigned long*)this + 1; off != (unsigned long*)this + (sizeof(*this) / sizeof(unsigned long*)); ++off)
	{
		if (*off < 1024)
		{
			printf("ERROR: filesystem pattern missing!\n");
			DebugBreak();
		}
	}

#endif
}