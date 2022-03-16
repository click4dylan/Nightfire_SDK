#pragma once
#include <Windows.h>
#include <stdio.h>

struct HCOMFILE
{
    int begin;
    int end;
    int handle_index;
    void* gbx_vtable;
    HCOMFILE() : begin(0), end(0), handle_index(-1), gbx_vtable(0) {};
};

typedef struct searchpath_s
{
    char dir[MAX_PATH];
    char pszDir[MAX_PATH];
    BOOLEAN default;
    searchpath_s* next;
} searchpath_t;

enum Loadfilelocation : int
{
    HUNK = 0,
    TEMP = 1,
    HEAP = 2,
    CACHE = 3,
    STACK = 4
};

class LinkedList;

#define NIGHTFIRE_FILESYSTEM_VERSION 1

struct NightfireFileSystem
{
    int version; // our version in case this struct gets changed

    void(*COM_Init)();
    //COM_Init: A0 ? ? ? ? 84 C0 0F 85 B3 00 00 00 66 83 3D ? ? ? ? 01 75 3E
    void(*COM_InitFilesystem)();
    //COM_InitFilesystem: 64 A1 00 00 00 00 6A FF 68 ? ? ? ? 50 64 89 25 00 00 00 00 81 EC 1C 01 00 00
    void(*COM_InitArgv)(int count, char** dest);
    //COM_InitArgv: 55 8B 6C 24 08 56 8B 74 24 10 57 33 C0 33 FF 90
    void(*COM_Shutdown)();
    //COM_Shutdown: A0 ? ? ? ? 83 EC 08 53 33 DB 3A C3 0F 84 89 00 00 00 A1 ? ? ? ?
    void(*COM_FileSeek)(HCOMFILE& file, int position);
    //COM_FileSeek: 83 EC 08 56 8B 74 24 10 8B 4E 0C 85 C9 74 27 8B 15 ? ? ? ? 8B 12 8B 01
    void(*Sys_FileSeek)(int handle_index, int position);
    //Sys_FileSeek: 8B 44 24 08 8B 4C 24 04 8B 14 8D ? ? ? ? 6A 00 50 52 E8 ? ? ? ? 83 C4 0C C3
    int(*COM_FileTell)(HCOMFILE comfile);
    //COM_FileTell: 8B 44 24 0C 50 E8 ? ? ? ? 8B 4C 24 08 83 C4 04 2B C1 C3
    int(*Sys_FileTell)(FILE* file);
    //Sys_FileTell: 8B 44 24 04 8B 0C 85 ? ? ? ? 89 4C 24 04 E9 ? ? ? ?
    int(*COM_ReadFile)(HCOMFILE& comfile, void* dest, int size);
    //COM_ReadFile: 8B 44 24 04 8B 48 0C 83 EC 08 85 C9 74 2D 8B 01 8D 54 24 0C 52
    int (*Sys_FileRead)(int handle_index, void* dest, int count);
    //Sys_FileRead: C3 90 90 90 90 8B 44 24 04 8B 0C 85 ? ? ? ? 8B 54 24 0C 8B 44 24 08 51 52 6A 01 50 + 5
    int (*Sys_FileOpenRead)(char* path, int* handle_index);  //returns handle_index
    //Sys_FileOpenRead: B8 01 00 00 00 57 EB 08 8D A4 24 00 00 00 00 90
    int (*Sys_FileOpenWrite)(char* path); //returns handle_index
    //Sys_FileOpenWrite: 56 B8 01 00 00 00 57 EB 07 8D A4 24 00 00 00 00
    void(*COM_CloseFile)(HCOMFILE comfile);
    //COM_CloseFile: 8B 4C 24 10 85 C9 74 07 8B 01 6A 01 FF 10 C3 8B 4C 24 0C 51 E8 ? ? ? ? 59 C3
    void(*Sys_FileClose)(int handle_index);
    //Sys_FileClose: 56 8B 74 24 08 85 F6 7C 1B
    int(*Sys_FileTime)(char* path);
    //Sys_FileTime: 8B 4C 24 04 83 EC 24 8D 44 24 00 50 51 E8 ? ? ? ? 83 C4 08 83 F8 FF
    bool(*COM_CompareFileTime)(char* path1, char* path2, int* dest);
    //COM_CompareFileTime: 8B 44 24 04 83 EC 10 53 55 8B 6C 24 24 56 57 33 FF 85 C0
    int(*Sys_CompareFileTime)(int* time1, int* time2);
    //Sys_CompareFileTime: 8B 4C 24 04 33 C0 85 C9 74 1B 8B 54 24 08 85 D2 74 13
    void(*COM_WriteFile)(const char* name, void* data, int len);
    //COM_WriteFile(const char*name, void* data, int len): 8B 44 24 04 81 EC 04 01 00 00 50 68 ? ? ? ? 68 ? ? ? ?
    int(*COM_WriteFile_)(HCOMFILE& file, const void* data, int len);
    //COM_WriteFile(HCOMFILE& file, const void* data, int len): 8B 54 24 04 8B 42 08 89 44 24 04 E9
    int(*Sys_FileWrite)(int handle_index, void* data, int count);
    //Sys_FileWrite: C3 90 90 90 90 8B 44 24 04 8B 0C 85 ? ? ? ? 8B 54 24 0C 8B 44 24 08 51 52 6A 01 50 + 0x35
    int(*COM_OpenFile)(const char* name, HCOMFILE* phFile); //returns length
    //COM_OpenFile: 8B 44 24 08 8B 4C 24 04 6A 00 50 51 6A 00 6A 00 E8 ? ? ? ? 83 C4 14 C3
    int(*COM_OpenFileInDir)(const char* path, const char* name, HCOMFILE* phFile); //returns length
    //COM_OpenFileInDir: 8B 44 24 0C 8B 4C 24 08 8B 54 24 04 6A 00 50 51 52 6A 00 E8 ? ? ? ? 83 C4 14 C3
    bool(*COM_FileExists)(const char* path, int* pSize);
    //COM_FileExists: 8B 4C 24 04 83 EC 10 53 56 6A 00 8D 44 24 0C 50 51 6A 00 6A 00 32 DB
    int (*COM_FindFile)(const char* path, HCOMFILE* phFile, FILE** file); //returns length
    //COM_FindFile: 8B 44 24 0C 8B 4C 24 08 8B 54 24 04 50 51 52 6A 00 6A 00 E8 ? ? ? ? 83 C4 14 C3
    int(*COM_FindFileSearch)(searchpath_t** searchpath, const char* path, const char* name, HCOMFILE* phFile, FILE** file); //returns length
    //COM_FindFileSearch: 6A FF 68 ? ? ? ? 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 81 EC 38 01 00 00
    char(*COM_Token)();
    //COM_Token: B8 ? ? ? ? C3 90 90 90 90 90 90 90 90 90 90 A0 6D 25 48 43
    void(*COM_UngetToken)();
    //COM_UngetToken: (+1) 90 C6 05 ? ? ? ? 01 C3
    char* (*COM_ParseFile)(const char* data, char* token, unsigned int token_len);
    //COM_ParseFile: 8B 44 24 04 56 50 E8 ? ? ? ? 8B 4C 24 14 8B 54 24 10 83 C4 04
    char* (*COM_Parse)(const char* data);
    //COM_Parse: A0 ? ? ? ? 53 33 DB 3A C3 8B
    void(*COM_FreeFile)(void* file);
    //COM_FreeFile: 8B 44 24 04 85 C0 74 07 50 FF 15 ? ? ? ? C3 8B 44 24 0C 81 EC 00 04 00 00
    void* (*COM_LoadFileLimit)(char* name, int desired_start, int desired_end, int* end_used, HCOMFILE* comfile);
    //COM_LoadFileLimit: 83 EC 30 53 55 56 57 8B 7C 24 54 8B 47 08
    unsigned char* (*COM_LoadHeapFile)(const char* path, int* length);
    //COM_LoadHeapFile: 8B 44 24 08 53 8B 5C 24 08 50 B8 02 00 00 00 E8 ? ? ? ? 83 C4 04 5B C3
    //unsigned char*(*COM_LoadFile)(const char* path, Loadfilelocation loc, int* length);
    //path on ebx, loc on eax
    //COM_LoadFile: 81 EC 14 01 00 00 55 8B AC 24 1C 01 00 00 85 ED 56 57 8B F0 74 07
    unsigned char* (*COM_LoadHunkFile)(const char* path);
    //COM_LoadHunkFile: 53 8B 5C 24 08 6A 00 33 C0 E8 ? ? ? ? 83 C4 04 5B C3
    unsigned char* (*COM_LoadStackFile)(const char* path, void* buf, int size, int* length);
    //COM_LoadStackFile: 8B 44 24 08 8B 54 24 10 8B 4C 24 0C 53 8B 5C 24 08 A3 ? ? ? ?
    unsigned char* (*COM_LoadTempFile)(const char* path, int* length);
    //COM_LoadTempFile: 8B 44 24 08 53 8B 5C 24 08 50 B8 01 00 00 00 E8 ? ? ? ? 83 C4 04 5B C3
    int(*Hunk_LowMark)();
    //Hunk_LowMark: A1 ? ? ? ? C3 90 90 90 90 90 90 90 90 90 90 56
    void(*Hunk_FreeToLowMark)(int mark);
    //Hunk_FreeToLowMark: 90 56 8B 74 24 08 85 F6 7C 08 3B 35 ? ? ? ? 7E 0E 56 68 ? ? ? ? E8 ? ? ? ? 83 C4 08 89 35 ? ? ? ? + 1
    int(*COM_ExpandFilename)(const char* name, unsigned int size); //AKA GetLocalPath
    //COM_ExpandFilename: 81 EC 04 01 00 00 53 55 56 8B 35 ? ? ? ? 85 F6 57 8B BC 24 18 01 00 00
    void* (*COM_StripExtension)(const char* in, char* out);
    //COM_StripExtension: 53 8B 5C 24 08 56 57 8B 3D ? ? ? ? 53 FF D7 53 8D 34 18
    void* (*COM_DefaultExtension)(char* path, unsigned int pathStringLength, const char* extension);
    //COM_DefaultExtension: 56 8B 74 24 08 56 FF 15 ? ? ? ? 8A 4C 30 FF 80 F9 2F
    void(*COM_FixSlashes)(char* path);
    //COM_FixSlashes: 8B 44 24 04 80 38 00 74 17 8D A4 24 00 00 00 00 80 38 2F 75 03 C6 00 5C
    int(*COM_FOpenFile)(const char* path, FILE** dest);
    //COM_FOpenFile: 8B 44 24 08 8B 4C 24 04 50 6A 00 51 6A 00 6A 00 E8 ? ? ? ? 83 C4 14 C3
    void(*COM_CreatePath)(const char* path);
    //COM_CreatePath: 8B 44 24 04 81 EC 04 01 00 00 56 68 04 01 00 00 50 8D 4C 24 0C 51
    int(*Sys_mkdir)(const char* path);
    //Sys_mkdir: 55 8B EC 53 8B 5D 08 56 57 53
    void(*COM_FileBase)(const char* path, char* dest, unsigned int size);
    //COM_FileBase: 56 57 8B 7C 24 0C 57 FF 15 ? ? ? ? 48 8B C8 85 C9 74 15
    int(*COM_FileSize)(const char* path);
    //COM_FileSize: 8B 4C 24 04 83 EC 10 56 6A 00 8D 44 24 08 50 51 6A 00 6A 00
    int(*COM_GetGameDir)(char* dest, unsigned int size);
    //COM_GetGameDir: 8B 44 24 04 85 C0 74 11 8B 4C 24 08 51
    const char* (*COM_GetWriteableDirectory)();
    //COM_GetWriteableDirectory: B8 ? ? ? ? C3 90 90 90 90 90 90 90 90 90 90 A1
    const char* (*Sys_FindFirst)(const char* path, char* filebase, unsigned int size);
    //Sys_FindFirst: A1 ? ? ? ? 56 33 F6 83 F8 FF
    const char* (*Sys_FindNext)(char* filebase, unsigned int size);
    //Sys_FindNext: A1 ? ? ? ? 56 68 ? ? ? ? 50 33 F6
    void(*Sys_FindClose)();
    //Sys_FindClose: A1 ? ? ? ? 50 FF 15 ? ? ? ? C7 05 ? ? ? ? FF FF FF FF
    void(*COM_Log)(char* pszFile, char* fmt, ...);
    //COM_Log: 8B 44 24 04 81 EC 00 04 00 00 85 C0 75 05
    void(*COM_Munge)(unsigned char* data, int len, int seq);
    //COM_Munge: 8D 4C 30 02 51 E8 ? ? ? ? 83 C4 18 + 6 call
    void(*COM_Munge2)(unsigned char* data, int len, int seq);
    //COM_Munge2: 83 C1 F8 51 83 C2 08 52 E8 ? ? ? ? + 9 call
    void(*COM_Munge2swap4)(unsigned char* data, int len, int seq);
    //COM_Munge2swap4: 50 8D 4C 24 10 6A 04 51 E8 ? ? ? ? + 9 call
    void(*COM_Munge3)(unsigned char* data, int len, int seq);
    //COM_Munge3: 6A 04 51 89 54 24 34 E8 ? ? ? ? 8B 54 24 34 + 8 call
    void(*COM_UnMunge)(unsigned char* data, int len, int seq);
    //COM_UnMunge: 6A 20 55 E8 ? ? ? ? 0F BE 4D 00 + 4 call
    void(*COM_UnMunge2)(unsigned char* data, int len, int seq);
    //COM_UnMunge2: E8 ? ? ? ? 83 C4 0C F7 C5 00 00 00 10 + 1 call
    void(*COM_UnMunge3)(unsigned char* data, int len, int seq);
    //COM_UnMunge3: 83 C9 FF 2B C8 81 E1 FF 00 00 00 51 6A 04 + 0x19 call
    LinkedList* (*COM_ListMapsToLinkedList)(const char* str);
    //COM_ListMapsToLinkedList: 6A ? ? ? ? 0D 43 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 81 EC B4 00 00 00
    void(*COM_ListMaps)(const char* str);
    //COM_ListMaps: 53 56 68 ? ? ? ? E8 ? ? ? ? 68
    bool(*COM_CheckParm)(char* parm);
    //COM_CheckParm: A1 ? ? ? ? 53 56 BE 01 00 00 00 3B C6 57 7E 2D
    void(*COM_AddGameDirectory)(bool packfile, const char* pszBaseDir, const char* pszDir);
    //COM_AddGameDirectory: 81 EC 10 04 00 00 53 8A 9C 24 18 04 00 00 84 DB 55 8B AC 24 24 04 00 00 56
    void(*COM_ChangeGameDir)(const char* path);
    //COM_ChangeGameDir: A1 ? ? ? ? 57 33 FF 85 C0 74 39 53
    unsigned char(*COM_BlockSequenceCRCByte)(char* base, int length, int sequence);
    //COM_BlockSequenceCRCByte: 83 EC 40 56 8B 74 24 50 85 F6 57 7D 0D
    void(*COM_ExplainDisconnection)(bool drop);
    //COM_ExplainDisconnection: A1 ? ? ? ? 83 EC 14 85 C0 56 57 0F
    void* g_Archive;
    //g_Archive: 8B 0D ? ? ? ? 50 C7 84 24 E4 00 00 00 01 + 2

    void Init(unsigned long engine_dll);
};

extern NightfireFileSystem* g_pNightfireFileSystem;