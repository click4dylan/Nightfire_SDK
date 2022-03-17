#include "MetaHook.h"
#include "pattern_scanner.h"
#include "globals.h"
#include "MinHook/MinHook.h"
#include <NightfireFileSystem.h>
#include <platformdll.h>

void(*g_oClientDLL_Init)() = nullptr;
cl_exportfuncs_t* g_pExportFuncs;
void* g_pClientDLL_Init = nullptr;
cl_exportfuncs_s g_oExportFuncs;
cl_enginefuncs_s g_oEngineFuncs;
typedef void(*StartMetaAudioFn)(unsigned long hEngineDLL, NightfirePlatformFuncs* platform, NightfireFileSystem* filesystem, cl_exportfuncs_t* pExportFunc, cl_enginefunc_t* pEngineFuncs);
typedef void(*ShutdownMetaAudioFn)();
StartMetaAudioFn g_oStartMetaAudio = NULL;
ShutdownMetaAudioFn g_oShutdownMetaAudio = NULL;
long g_MetaAudioDllHinst = NULL;

//client.dll override
int CLIENT_Initialize(cl_enginefuncs_s* enginefuncs)
{
	if (enginefuncs->version != 11 || enginefuncs->size != sizeof(cl_enginefuncs_s))
		return g_oExportFuncs.CLIENT_Initialize(enginefuncs);

	//store original enginefuncs
	g_oEngineFuncs = *enginefuncs;

	// call original func
	int result = g_oExportFuncs.CLIENT_Initialize(enginefuncs);

	if (result)
	{
		g_MetaAudioDllHinst = (long)LoadLibraryA("MetaAudio.dll");
		if (g_MetaAudioDllHinst)
		{
			g_oStartMetaAudio = (StartMetaAudioFn)GetProcAddress((HMODULE)g_MetaAudioDllHinst, "StartMetaAudio");
			g_oShutdownMetaAudio = (ShutdownMetaAudioFn)GetProcAddress((HMODULE)g_MetaAudioDllHinst, "ShutdownMetaAudio");
			if (g_oStartMetaAudio && g_oShutdownMetaAudio)
				g_oStartMetaAudio(g_engineDllHinst, g_pNightfirePlatformFuncs, g_pNightfireFileSystem, g_pExportFuncs, &g_oEngineFuncs);
		}
	}

	return result;
}

//client.dll override
int CLIENT_Shutdown()
{
	int result = g_oExportFuncs.CLIENT_Shutdown();

	if (g_MetaAudioDllHinst && g_oStartMetaAudio && g_oShutdownMetaAudio)
	{
		g_oShutdownMetaAudio();
		FreeLibrary((HMODULE)g_MetaAudioDllHinst);
		g_MetaAudioDllHinst = NULL;
	}

	return result;
}

//client.dll export
int HUD_ClientAPI(int iVersion, int size, cl_exportfuncs_t* pClientFuncs)
{
	int(*oHUD_ClientAPI)(int, int, cl_exportfuncs_t*) = (int(*)(int, int, cl_exportfuncs_t*))GetProcAddress((HMODULE)*g_clientDllHinst, "HUD_ClientAPI");
	
	int result = oHUD_ClientAPI(iVersion, size, pClientFuncs);
	if (result == 1 && iVersion == 5 && size == sizeof(cl_exportfuncs_t) /*200*/)
	{
		// store original exportfuncs
		g_oExportFuncs = *pClientFuncs;

		//hook CLIENT_Initialize with our own
		pClientFuncs->CLIENT_Initialize = &CLIENT_Initialize;
		pClientFuncs->CLIENT_Shutdown = &CLIENT_Shutdown;
	}

	return result;
}

//engine.dll hook
void ClientDLL_Init()
{
	g_pExportFuncs = *(cl_exportfuncs_t**)((DWORD)g_pClientDLL_Init + 0x90);
	g_clientDllHinst = (long*)(*(DWORD*)((DWORD)g_pClientDLL_Init + 0x60));

	// force game to call our HUD_ClientAPI instead of client.dll's
	DWORD old, old2;
	unsigned char* init_assembly = (unsigned char*)((DWORD)g_pClientDLL_Init + 0x84);
	VirtualProtect((void*)init_assembly, 12, PAGE_EXECUTE_READWRITE, &old);
	unsigned char data[12] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	*(DWORD*)&data[1] = (DWORD)&HUD_ClientAPI;
	for (int i = 0; i < 12; ++i)
		init_assembly[i] = data[i];
	VirtualProtect((void*)init_assembly, 12, old, &old2);

	g_oClientDLL_Init();
}

void RunMetaHook()
{
	MH_Initialize();
	g_pClientDLL_Init = (void*)FindMemoryPattern(g_engineDllHinst, "A0 ? ? ? ? 81 EC 04 01 00 00 84 C0 0F 85 D3", false);
	if (!g_pClientDLL_Init)
		return;

	if (!HookFunctionWithMinHook(g_pClientDLL_Init, ClientDLL_Init, (void**)&g_oClientDLL_Init))
		return;
}

void ShutdownMetaHook()
{
	MH_DisableHook(0);
	MH_Uninitialize();
}