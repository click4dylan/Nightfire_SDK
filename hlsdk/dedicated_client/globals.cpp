#include "globals.h"

CreateRenderT CreateRender;
CreateRenderPlatformInfoT CreateRenderPlatformInfo;
CreateRenderFeaturesT CreateRenderFeatures;
CreateRenderMaterialT CreateRenderMaterial;
CreateRenderViewportT CreateRenderViewport;
CreateRenderVideoTextureT CreateRenderVideoTexture;

void* render;
DWORD* renderplatforminfo;
void* renderfeatures;

HWND g_EngWnd = nullptr;

BOOLEAN g_bFullScreen = 1;
BOOLEAN border = 0;
BOOLEAN g_bDirectToConsole = 0;

RECT g_WindowRect = { 0, 0 };
unsigned g_WindowX = 0;
unsigned g_WindowY = 0;
unsigned offsetwidth = 0;
unsigned offsetheight = 0;

static engine_api_t nullapi;
engine_api_t g_EngineAPI = nullapi;
static gui_api_t nullguiapi;
gui_api_t g_guiAPI = nullguiapi;

long g_guiDllHinst = NULL;
long g_engineDllHinst = NULL;
long g_platformDllHinst = NULL;
long g_hRenderDLL = NULL;
long g_hRawInputDLL = NULL;
long* g_clientDllHinst = NULL;
HANDLE g_hRawInputThread = INVALID_HANDLE_VALUE;
HANDLE g_hRawInputEvent = INVALID_HANDLE_VALUE;

HANDLE hinput;
HANDLE houtput;
const char* g_pszengine = "engine.dll";
const char* g_pszplatform = "platform.dll";

#ifdef SWDS
BOOLEAN g_bDedicated = 1;
#else
BOOLEAN g_bDedicated = 0;
#endif
BOOLEAN bFinalTick = 0;

HANDLE* g_pMultipleInstanceMutex = nullptr;

// System Memory & Size
unsigned char* s_pMemBase = NULL;
size_t s_MemSize = 0;
size_t				s_HunkSize = 20; //64mb nf hunk //0x2000000;  // 32 Mb default heapsize
size_t				s_HeapSize = 128; //72mb nf heap

exefuncs_t s_LauncherAPI;

char console_text[256];
int	console_textlen;

char s_szCmdLine[2048];

BOOL g_mouseParamsValid = 0;
unsigned clipboardformat = 0;
int g_originalMouseParameters[3];

DWORD PlatformID; //Platform ID dword_41C3D0