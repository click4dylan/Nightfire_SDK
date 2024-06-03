// This program is designed to hook into nightfire's BSP compiler to fix known issues
// Eventually it will be a complete rewrite

#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "hooks.h"
#include "renderer.h"
#include "textures.h"

bool g_MakeNodePortals = false;
node_t* g_Node = nullptr;


__declspec(naked) void SplitFaces_Hook(/*face_t* original_face @<eax>, int split_from_plane @<edi>*/ int depth, face_t** dest_front_Face, face_t** dest_back_face)
{
    __asm
    {
        push [esp + 12]
        push [esp + 12]
        push [esp + 12]
        push edi
        push eax
        call SplitFaces
        add esp, 20
        ret
    }
}

__declspec(naked) void BuildBspTree_r_Hook(int bspdepth, node_t* node, face_t* front_face, bool make_node_portals)
{
    __asm
    {
        jmp BuildBspTree_r
    }
}

__declspec(naked) void FilterFaceIntoTree_r_Hook(int depth,
    node_t* node,
    side_t* brush_side,
    face_t* face,
    bool unknown,
    bool is_in_lighting_stage)
{
    __asm
    {
        jmp FilterFaceIntoTree_r
    }
}

#if 0
void __cdecl GetFlagsForTexture(
    const char* texture,
    leaf_types_t* leaf_type,
    int* render_flags,
    int* brush_flags)
{
    int v4; // ebp
    leaf_types_t v5; // ebx
    unsigned int i; // esi
    special_texture_t* v7; // edi
    int v8; // [esp+10h] [ebp-4h]

    v4 = 0;
    v5 = LEAF_SOLID_AKA_OPAQUE;                              // default leaf type is solid leaf
    v8 = 0;
    i = 0;
    v7 = g_SpecialTextures;
    while (_stricmp(texture, v7->name))
    {
        ++i;
        ++v7;
        if (i >= 15)
            goto LABEL_6;
    }
    v5 = g_SpecialTextures[i].leaf_type;//dword_429DEC[4 * i];
    v4 = g_SpecialTextures[i].renderflags;//dword_429DF0[4 * i];
    v8 = g_SpecialTextures[i].brushflags;// dword_429DF4[4 * i];
LABEL_6:
    if (i == 15 && !_strnicmp(texture, "liquids", 7u))
    {
        v5 = LEAF_EMPTY_AKA_NOT_OPAQUE;
        v4 = 0x50000;
        v8 = CONTENTS_WATER;
    }
    *leaf_type = v5;
    *render_flags = v4;
    *brush_flags = v8;
}
#endif

void PrintSpecialTextures()
{
#if 0
    char tmp[1024];
    leaf_types_t leaftype;
    int renderflags;
    int brushflags;

    for (int i = 0; i < NUM_SPECIAL_TEXTURES; ++i)
    {
        const auto tex = &g_SpecialTextures[i];
        GetFlagsForTexture(tex->name, &leaftype, &renderflags, &brushflags);

        sprintf_s(tmp, "Special Texture: %s\nLeafType: %i\nRenderFlags: %#010x\nBrushFlags: %#010x\n\n", tex->name, leaftype, renderflags, brushflags);
        OutputDebugStringA(tmp);
    }

    const char* test_texture_name = "concrete/floor_a";
    GetFlagsForTexture(test_texture_name, &leaftype, &renderflags, &brushflags);

    sprintf_s(tmp, "Normal Texture: %s\nLeafType: %i\nRenderFlags: %#010x\nBrushFlags: %#010x\n\n", test_texture_name, leaftype, renderflags, brushflags);
    OutputDebugStringA(tmp);
#else

#if 0
    for (int i = 0; i < NUM_SPECIAL_TEXTURES; ++i)
    {
        special_texture_t* tex = &g_SpecialTextures[i];
        char tmp[1024];
        sprintf_s(tmp, "Special Texture: %s\nLeafType: %i\nRenderFlags: %#010x\nBrushFlags: %#010x\n\n", tex->name, tex->leaf_type, tex->renderflags, tex->brushflags);
        OutputDebugStringA(tmp);
    }
#endif
#endif
}

#if 0
void PrintErrorMessages()
{
    for (int i = 0; i < MAX_HLASSUME_ERRORS; ++i)
    {
        print_hlassume_errormsg(i);
    }
}
#endif

#define LAST_IND(x,part_type)    (sizeof(x)/sizeof(part_type) - 1)
#define HIGH_IND(x, part_type)  LAST_IND(x, part_type)
#  define LOW_IND(x,part_type)   0

#define DWORDn(x, n)  (*((DWORD*)&(x)+n))
#define LODWORD(x) DWORDn(x,LOW_IND(x,DWORD))
#define HIDWORD(x) DWORDn(x,HIGH_IND(x,DWORD))

#if 0
int callTextureAxisFromPlane(const plane_t* plane)
{
    DWORD adr = 0x00411390;
    return reinterpret_cast<int(__thiscall*)(const plane_t*)>(adr)(plane);
}
#endif

float RandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

static int sz = 0;
#include <map>
void PrintDebug()
{
   // std::map<int, int> myMap;
    //sz = sizeof(myMap);
    //printf("%i\n", sz);
    //DebugBreak();

#if 0
    for (int Index = 0; Index < 6; ++Index)
    {
        double value[3];

#if 1
        unsigned char* dword_423528 = (unsigned char*)0x423528;
        unsigned char* dword_42352C = (unsigned char*)0x42352C;
        unsigned char* dword_423530 = (unsigned char*)0x423530;
        unsigned char* dword_423534 = (unsigned char*)0x423534;

        LODWORD(value[0]) = *(DWORD*)&dword_423528[24 * Index];
        HIDWORD(value[0]) = *(DWORD*)&dword_42352C[24 * Index];
        LODWORD(value[1]) = *(DWORD*)&dword_423530[24 * Index];
        HIDWORD(value[1]) = *(DWORD*)&dword_423534[24 * Index];
        LODWORD(value[2]) = *(DWORD*)&dword_423534[24 * Index + 4];
        HIDWORD(value[2]) = *(DWORD*)&dword_423534[24 * Index + 8];
#else
        __asm

        {
            mov esi, Index
            lea ebx, value

            LEA EAX, [ESI * 2 + ESI]
            MOV ECX, DWORD PTR DS : [EAX * 8 + 0x423528]

            MOV DWORD PTR DS : [EBX] , ECX
            MOV EDX, DWORD PTR DS : [EAX * 8 + 0x42352C]
            MOV DWORD PTR DS : [EBX + 4] , EDX
            MOV ECX, DWORD PTR DS : [EAX * 8 + 0x423530]
            LEA EAX, [EAX * 8 + 0x423528]
            MOV DWORD PTR DS : [EBX + 8] , ECX
            MOV EDX, DWORD PTR DS : [EAX + 0x0C]
            MOV DWORD PTR DS : [EBX + 0x0C] , EDX
            MOV ECX, DWORD PTR DS : [EAX + 0x10]
            MOV EAX, DWORD PTR DS : [EAX + 0x14]
            MOV DWORD PTR DS : [EBX + 0x10] , ECX
            MOV DWORD PTR DS : [EBX + 0x14] , EAX

        }
#endif
        char tmp[256];
        sprintf(tmp, "value 0 at Index %i is %f\n", Index, value[0]);
        OutputDebugStringA(tmp);
        sprintf(tmp, "value 1 at Index %i is %f\n", Index, value[1]);
        OutputDebugStringA(tmp);
        sprintf(tmp, "value 2 at Index %i is %f\n", Index, value[2]);
        OutputDebugStringA(tmp);
    }

    OutputDebugStringA("\n\nNow using s_baseaxis\n\n");

    for (int Index = 0; Index < 6; ++Index)
    {
        double value[3];
        value[0] = data[Index][0];
        value[1] = data[Index][1];
        value[2] = data[Index][2];
        char tmp[256];
        sprintf(tmp, "value 0 at Index %i is %f\n", Index, value[0]);
        OutputDebugStringA(tmp);
        sprintf(tmp, "value 1 at Index %i is %f\n", Index, value[1]);
        OutputDebugStringA(tmp);
        sprintf(tmp, "value 2 at Index %i is %f\n", Index, value[2]);
        OutputDebugStringA(tmp);
    }
#endif

    //PrintErrorMessages();
}

HANDLE CreateThreadSafe(const LPTHREAD_START_ROUTINE func, const LPVOID lParam)
{
    const HANDLE hThread = CreateThread(nullptr, 0, nullptr, lParam, CREATE_SUSPENDED, nullptr);
    if (!hThread)
    {
        __fastfail(1);
        return 0;
    }

    CONTEXT threadCtx;
    threadCtx.ContextFlags = CONTEXT_INTEGER;
    GetThreadContext(hThread, &threadCtx);
#ifdef _WIN64
    threadCtx.Rax = reinterpret_cast<decltype(threadCtx.Rax)>(func);
#else
    threadCtx.Eax = reinterpret_cast<decltype(threadCtx.Eax)>(func);
    threadCtx.ContextFlags = CONTEXT_INTEGER;
#endif
    SetThreadContext(hThread, &threadCtx);

    if (ResumeThread(hThread) != 1 || ResumeThread(hThread) != NULL)
    {
        __fastfail(1);
        return 0;
    }

    return hThread;
}

bool fixed = false;
void Fix(HMODULE hModule)
{
    PrintDebug();
    PrintSpecialTextures();
    int sz = sizeof(face_t);
    if (fixed)
        return;
    fixed = true;

    // Start d3d renderer
    //const HANDLE hThread = CreateThreadSafe(&StartRendering, hModule);
    //if (hThread)
    //    CloseHandle(hThread);

    PlaceJMP((BYTE*)0x41DD00, (DWORD)&SplitFaces_Hook, 5);
    PlaceJMP((BYTE*)0x41E060, (DWORD)&BuildBspTree_r_Hook, 5);
    PlaceJMP((BYTE*)0x41DBF0, (DWORD)&FilterFaceIntoTree_r_Hook, 5);

    // StripUnnecessaryFacesWithBrushFlag
    DWORD old;
    DWORD adr = 0x41C5E0;
    VirtualProtect((void*)adr, 32, PAGE_EXECUTE_READWRITE, &old);
    *(unsigned char*)(adr) = 0xC3;
    VirtualProtect((void*)adr, 32, old, &old);

    // StripUnnecessaryBackFacesWithBrushFlag
    adr = 0x41CC70;
    VirtualProtect((void*)adr, 32, PAGE_EXECUTE_READWRITE, &old);
    *(unsigned char*)(adr) = 0xC3;
    VirtualProtect((void*)adr, 32, old, &old);
}

#if 0

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Fix(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


#endif