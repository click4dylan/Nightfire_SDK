#pragma once

struct d3dRenderMaterial;
class ConsoleVariable;
struct IConsoleFunction;
class RenderTextureAPI;
class RenderAPI;

struct IConsoleFunction;

typedef void*(*CreateRenderT)(void);
typedef DWORD* (*CreateRenderPlatformInfoT)(const void* a1);
typedef void*(*CreateRenderFeaturesT)(void* render);
typedef d3dRenderMaterial* (*CreateRenderMaterialT)(RenderAPI* api, const void* srcbuf, int srcbufsize);

typedef void* (*CreateRenderViewportT)(const void* one);
typedef int (*CreateRenderVideoTextureT)(void);
typedef void (*Render_DoRenderT)(void);
typedef void (*Render_DoFlipT)(void);
typedef void (*AppActivateT)(BOOLEAN active);
typedef void (*CDAudio_PlayT)(unsigned __int8 a1, int a2);
typedef void (*CDAudio_PauseT)();
typedef void (*CDAudio_ResumeT)();
typedef void (*CDAudio_UpdateT)();
typedef void (*ErrorMessageT)(int nLevel, const char* pszErrorMessage);
typedef void (*Sys_PrintfT)(char * fmt, ...);
typedef int (*IsValidCDT)(void);
typedef DWORD(*GetCDKeyT)(char* dest, unsigned int buffer_size, DWORD* new_length, BYTE* result);
typedef void (*ChangeGameDirectoryT)(const char* a1);
typedef void (*Launcher_PlayMovieT)(char* movie);
typedef void (*Launcher_OpenFrontEndT)(bool to_escape_menu, bool multiplayer, bool is_server, bool is_teamplay);
typedef int (*getOSVersionT)();
typedef DWORD* (*GUI_ManagerT)();