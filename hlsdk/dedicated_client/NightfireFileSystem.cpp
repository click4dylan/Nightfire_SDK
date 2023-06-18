#include <NightfireFileSystem.h>
#include "globals.h"
#include "pattern_scanner.h"

NightfireFileSystem g_NightfireFilesystem;
NightfireFileSystem* g_pNightfireFileSystem = &g_NightfireFilesystem;


void NightfireFileSystem::Init(unsigned long engine_dll, unsigned long platform_dll)
{
	//repeating the cardinal sin of metamod, TODO: create castable definitions for each func type
	size = sizeof(NightfireFileSystem);
	version = NIGHTFIRE_FILESYSTEM_VERSION;
	FindMemoryPattern(COM_Init, engine_dll, "A0 ? ? ? ? 84 C0 0F 85 B3 00 00 00 66 83 3D ? ? ? ? 01 75 3E", false);
	FindMemoryPattern(COM_InitFilesystem, engine_dll, "64 A1 00 00 00 00 6A FF 68 ? ? ? ? 50 64 89 25 00 00 00 00 81 EC 1C 01 00 00", false);
	FindMemoryPattern(COM_InitArgv, engine_dll, "55 8B 6C 24 08 56 8B 74 24 10 57 33 C0 33 FF 90", false);
	FindMemoryPattern(COM_Shutdown, engine_dll, "A0 ? ? ? ? 83 EC 08 53 33 DB 3A C3 0F 84 89 00 00 00 A1 ? ? ? ?", false);
	FindMemoryPattern(COM_FileSeek_, engine_dll, "83 EC 08 56 8B 74 24 10 8B 4E 0C 85 C9 74 27 8B 15 ? ? ? ? 8B 12 8B 01", false);
	FindMemoryPattern(Sys_FileSeek, engine_dll, "8B 44 24 08 8B 4C 24 04 8B 14 8D ? ? ? ? 6A 00 50 52 E8 ? ? ? ? 83 C4 0C C3", false);
	FindMemoryPattern(COM_FileTell_, engine_dll, "8B 44 24 0C 50 E8 ? ? ? ? 8B 4C 24 08 83 C4 04 2B C1 C3", false);
	FindMemoryPattern(Sys_FileTell, engine_dll, "8B 44 24 04 8B 0C 85 ? ? ? ? 89 4C 24 04 E9 ? ? ? ?", false);
	FindMemoryPattern(COM_ReadFile, engine_dll, "8B 44 24 04 8B 48 0C 83 EC 08 85 C9 74 2D 8B 01 8D 54 24 0C 52", false);
	FindMemoryPattern(Sys_FileRead, engine_dll, "C3 90 90 90 90 8B 44 24 04 8B 0C 85 ? ? ? ? 8B 54 24 0C 8B 44 24 08 51 52 6A 01 50", false, false, 5, 0, false);
	FindMemoryPattern(Sys_FileOpenRead, engine_dll, "B8 01 00 00 00 57 EB 08 8D A4 24 00 00 00 00 90", false);
	FindMemoryPattern(Sys_FileOpenWrite, engine_dll, "56 B8 01 00 00 00 57 EB 07 8D A4 24 00 00 00 00", false);
	FindMemoryPattern(COM_CloseFile, engine_dll, "8B 4C 24 10 85 C9 74 07 8B 01 6A 01 FF 10 C3 8B 4C 24 0C 51 E8 ? ? ? ? 59 C3", false);
	FindMemoryPattern(Sys_FileClose, engine_dll, "56 8B 74 24 08 85 F6 7C 1B", false);
	FindMemoryPattern(Sys_FileTime, engine_dll, "8B 4C 24 04 83 EC 24 8D 44 24 00 50 51 E8 ? ? ? ? 83 C4 08 83 F8 FF", false);
	FindMemoryPattern(COM_CompareFileTime, engine_dll, "8B 44 24 04 83 EC 10 53 55 8B 6C 24 24 56 57 33 FF 85 C0", false);
	FindMemoryPattern(Sys_CompareFileTime, engine_dll, "8B 4C 24 04 33 C0 85 C9 74 1B 8B 54 24 08 85 D2 74 13", false);
	FindMemoryPattern(COM_WriteFile, engine_dll, "8B 44 24 04 81 EC 04 01 00 00 50 68 ? ? ? ? 68 ? ? ? ?", false);
	FindMemoryPattern(COM_WriteFile_, engine_dll, "8B 54 24 04 8B 42 08 89 44 24 04 E9", false);
	FindMemoryPattern(Sys_FileWrite, engine_dll, "C3 90 90 90 90 8B 44 24 04 8B 0C 85 ? ? ? ? 8B 54 24 0C 8B 44 24 08 51 52 6A 01 50", false, false, 0, 0x35, false);
	FindMemoryPattern(COM_OpenFile, engine_dll, "8B 44 24 08 8B 4C 24 04 6A 00 50 51 6A 00 6A 00 E8 ? ? ? ? 83 C4 14 C3", false);
	FindMemoryPattern(COM_OpenFileInDir, engine_dll, "8B 44 24 0C 8B 4C 24 08 8B 54 24 04 6A 00 50 51 52 6A 00 E8 ? ? ? ? 83 C4 14 C3", false);
	FindMemoryPattern(COM_FileExists, engine_dll, "8B 4C 24 04 83 EC 10 53 56 6A 00 8D 44 24 0C 50 51 6A 00 6A 00 32 DB", false);
	FindMemoryPattern(COM_FindFile, engine_dll, "8B 44 24 0C 8B 4C 24 08 8B 54 24 04 50 51 52 6A 00 6A 00 E8 ? ? ? ? 83 C4 14 C3", false);
	FindMemoryPattern(COM_FindFileSearch, engine_dll, "6A FF 68 ? ? ? ? 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 81 EC 38 01 00 00", false);
	FindMemoryPattern(COM_Token, engine_dll, "B8 ? ? ? ? C3 90 90 90 90 90 90 90 90 90 90 A0 6D 25 48 43", false);
	FindMemoryPattern(COM_UngetToken, engine_dll, "90 C6 05 ? ? ? ? 01 C3", false, false, 0, 1);
	FindMemoryPattern(COM_ParseFile, engine_dll, "8B 44 24 04 56 50 E8 ? ? ? ? 8B 4C 24 14 8B 54 24 10 83 C4 04", false);
	FindMemoryPattern(COM_Parse, engine_dll, "A0 ? ? ? ? 53 33 DB 3A C3 8B", false);
	FindMemoryPattern(COM_FreeFile, engine_dll, "8B 44 24 04 85 C0 74 07 50 FF 15 ? ? ? ? C3 8B 44 24 0C 81 EC 00 04 00 00", false);
	FindMemoryPattern(COM_LoadFile_, engine_dll, "81 EC 14 01 00 00 55 8B AC ? ? ? ? ? 85 ED 56 57", false);
	FindMemoryPattern(COM_LoadFileLimit, engine_dll, "83 EC 30 53 55 56 57 8B 7C 24 54 8B 47 08", false);
	FindMemoryPattern(COM_LoadHeapFile, engine_dll, "8B 44 24 08 53 8B 5C 24 08 50 B8 02 00 00 00 E8 ? ? ? ? 83 C4 04 5B C3", false);
	FindMemoryPattern(COM_LoadHunkFile, engine_dll, "53 8B 5C 24 08 6A 00 33 C0 E8 ? ? ? ? 83 C4 04 5B C3", false);
	FindMemoryPattern(COM_LoadStackFile, engine_dll, "8B 44 24 08 8B 54 24 10 8B 4C 24 0C 53 8B 5C 24 08 A3 ? ? ? ?", false);
	FindMemoryPattern(COM_LoadTempFile, engine_dll, "8B 44 24 08 53 8B 5C 24 08 50 B8 01 00 00 00 E8 ? ? ? ? 83 C4 04 5B C3", false);
	FindMemoryPattern(Hunk_LowMark, engine_dll, "A1 ? ? ? ? C3 90 90 90 90 90 90 90 90 90 90 56", false);
	FindMemoryPattern(Hunk_FreeToLowMark, engine_dll, "A1 ? ? ? ? 50 E8 ? ? ? ? 83 C4 04 C7 05", false, false, 0, 7, true);
	FindMemoryPattern(COM_ExpandFilename, engine_dll, "81 EC 04 01 00 00 53 55 56 8B 35 ? ? ? ? 85 F6 57 8B BC 24 18 01 00 00", false);
	FindMemoryPattern(COM_StripExtension, engine_dll, "53 8B 5C 24 08 56 57 8B 3D ? ? ? ? 53 FF D7 53 8D 34 18", false);
	FindMemoryPattern(COM_DefaultExtension, engine_dll, "56 8B 74 24 08 56 FF 15 ? ? ? ? 8A 4C 30 FF 80 F9 2F", false);
	FindMemoryPattern(COM_FixSlashes, engine_dll, "8B 44 24 04 80 38 00 74 17 8D A4 24 00 00 00 00 80 38 2F 75 03 C6 00 5C", false);
	FindMemoryPattern(COM_FOpenFile, engine_dll, "8B 44 24 08 8B 4C 24 04 50 6A 00 51 6A 00 6A 00 E8 ? ? ? ? 83 C4 14 C3", false);
	FindMemoryPattern(COM_CreatePath, engine_dll, "8B 44 24 04 81 EC 04 01 00 00 56 68 04 01 00 00 50 8D 4C 24 0C 51", false);
	FindMemoryPattern(Sys_mkdir, engine_dll, "55 8B EC 53 8B 5D 08 56 57 53", false);
	FindMemoryPattern(COM_FileBase, engine_dll, "56 57 8B 7C 24 0C 57 FF 15 ? ? ? ? 48 8B C8 85 C9 74 15", false);
	FindMemoryPattern(COM_FileSize_, engine_dll, "8B 4C 24 04 83 EC 10 56 6A 00 8D 44 24 08 50 51 6A 00 6A 00", false);
	FindMemoryPattern(COM_GetGameDir, engine_dll, "8B 44 24 04 85 C0 74 11 8B 4C 24 08 51", false);
	FindMemoryPattern(COM_GetWriteableDirectory, engine_dll, "B8 ? ? ? ? C3 90 90 90 90 90 90 90 90 90 90 A1", false);
	FindMemoryPattern(Sys_FindFirst, engine_dll, " A1 ? ? ? ? 56 33 F6 83 F8 FF", false);
	FindMemoryPattern(Sys_FindNext, engine_dll, "A1 ? ? ? ? 56 68 ? ? ? ? 50 33 F6", false);
	FindMemoryPattern(Sys_FindClose, engine_dll, "A1 ? ? ? ? 50 FF 15 ? ? ? ? C7 05 ? ? ? ? FF FF FF FF", false);
	FindMemoryPattern(COM_Log, engine_dll, "8B 44 24 04 81 EC 00 04 00 00 85 C0 75 05", false);
	FindMemoryPattern(COM_Munge, engine_dll, "8D 4C 30 02 51 E8 ? ? ? ? 83 C4 18", false, false, 0, 6, true);
	FindMemoryPattern(COM_Munge2, engine_dll, "83 C1 F8 51 83 C2 08 52 E8 ? ? ? ?", false, false, 0, 9, true);
	FindMemoryPattern(COM_Munge2swap4, engine_dll, "50 8D 4C 24 10 6A 04 51 E8 ? ? ? ?", false, false, 0, 9, true);
	FindMemoryPattern(COM_Munge3, engine_dll, "6A 04 51 89 54 24 34 E8 ? ? ? ? 8B 54 24 34", false, false, 0, 8, true);
	FindMemoryPattern(COM_UnMunge, engine_dll, "6A 20 55 E8 ? ? ? ? 0F BE 4D 00", false, false, 0, 4, true);
	FindMemoryPattern(COM_UnMunge2, engine_dll, "E8 ? ? ? ? 83 C4 0C F7 C5 00 00 00 10", false, false, 0, 1, true);
	FindMemoryPattern(COM_UnMunge3, engine_dll, "83 C9 FF 2B C8 81 E1 FF 00 00 00 51 6A 04", false, false, 0, 0x19, true);
	FindMemoryPattern(COM_ListMapsToLinkedList, engine_dll, "6A ? ? ? ? 0D 43 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 81 EC B4 00 00 00", false);
	FindMemoryPattern(COM_ListMaps, engine_dll, "53 56 68 ? ? ? ? E8 ? ? ? ? 68", false);
	FindMemoryPattern(COM_CheckParm, engine_dll, "A1 ? ? ? ? 53 56 BE 01 00 00 00 3B C6 57 7E 2D", false);
	FindMemoryPattern(COM_AddGameDirectory, engine_dll, "81 EC 10 04 00 00 53 8A 9C 24 18 04 00 00 84 DB 55 8B AC 24 24 04 00 00 56", false);
	FindMemoryPattern(COM_ChangeGameDir, engine_dll, "A1 ? ? ? ? 57 33 FF 85 C0 74 39 53", false);
	FindMemoryPattern(COM_BlockSequenceCRCByte, engine_dll, "83 EC 40 56 8B 74 24 50 85 F6 57 7D 0D", false);
	FindMemoryPattern(COM_ExplainDisconnection, engine_dll, "A1 ? ? ? ? 83 EC 14 85 C0 56 57 0F", false);
	FindMemoryPattern(g_Archive, engine_dll, "8B 0D ? ? ? ? 50 C7 84 24 E4 00 00 00 01", false, true, 2, 0, false);
	g_DiskFileHandles = *(FILE***)((unsigned long)Sys_FileSeek + 0xB);
	FindMemoryPattern(gbx_fseek, engine_dll, "6A 0C 68 ? ? ? ? E8 ? ? ? ? FF 75 08 E8 ? ? ? ? 59 83 65 FC 00 FF 75 10", false);
	FindMemoryPattern(gbx_ftell, engine_dll, "6A 0C 68 ? ? ? ? E8 ? ? ? ? FF 75 08 E8 ? ? ? ? 59 83 65 FC 00 FF 75 08", false);
	FindMemoryPattern(com_filesize, engine_dll, "A1 ? ? ? ? 50 53 8D 8C 24 4C 01 00 00 55 51", false);
	SEEK_FROM_CUR = (int*)GetProcAddress((HMODULE)platform_dll, "?SEEK_FROM_CUR@File@@2HB");
	SEEK_FROM_END = (int*)GetProcAddress((HMODULE)platform_dll, "?SEEK_FROM_END@File@@2HB");
	SEEK_FROM_START = (int*)GetProcAddress((HMODULE)platform_dll, "?SEEK_FROM_START@File@@2HB");
	loadcache = *(cache_user_t***)((DWORD)COM_LoadFile_ + 0xC2);
	FindMemoryPattern(Cache_Alloc, engine_dll, "53 8B 5C 24 08 83 3B 00 55 8B 6C 24 14 56 57", false);
	FindMemoryPattern(Cache_Free, engine_dll, "53 56 57 8B 7C 24 10 8B 07 33 DB 3B C3", false);
	FindMemoryPattern(Cache_Check, engine_dll, "57 8B 7C 24 08 8B 07 85 C0 75 02 5F C3", false);
#ifdef _DEBUG
	// rudimentary bugcheck, 32 bit only
	for (unsigned long* off = (unsigned long*)this + 2; off != (unsigned long*)this + (sizeof(*this) / sizeof(unsigned long*)); ++off)
	{
		if (*off < 1024)
		{
			printf("ERROR: filesystem pattern missing!\n");
			//DebugBreak();
		}
	}

#endif
}