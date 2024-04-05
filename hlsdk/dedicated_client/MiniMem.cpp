//#include <CVector.h>
//#include "CString.h"
//#include "entity.h"
//#include <gpglobals.h>
//#include <const.h>
//#include <util.h>
//#include <cbase.h>
#include "globals.h"
#include "pattern_scanner.h"
#include "MinHook/MinHook.h"
#include <event_api.h>
#include "MetaHook.h"
#include <usercmd.h>
#include <cvardef.h>
#include <clientsideentity.h>
#include <NightfireFileSystem.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <platformdll.h>
//#include <enginefuncs.h>
#include <globalvars.h>
#include <server.h>
#include <nightfire_pointers.h>
#include "nightfire_hooks.h"

#include <pm_defs.h>

#include <studio.h>
#include <StudioModelRenderer.h>

void*(__thiscall* g_oMiniMem_newBlock)(void*);

void* __fastcall MiniMem_newBlock(void* minimem)
{
	void* block = g_oMiniMem_newBlock(minimem);
	if (!block)
		g_Pointers.g_pCL_EngineFuncs->Con_NPrintf(0, "MiniMem is out of memory, too many particles");
	return block;
}

void Fix_MiniMem_OutOfMemoryMissingNotification()
{
	//MiniMem running out of memory used to have an error notification in the alpha build of the game, which was removed.. this restores that

	DWORD adr;
	if (!FindMemoryPattern(adr, *g_clientDllHinst, "E8 ? ? ? ? 8B F0 89 74 24 10 33 FF 3B F7 89", false, false, 0, 1, true))
		return;

	if (!HookFunctionWithMinHook((void*)adr, MiniMem_newBlock, (void**)&g_oMiniMem_newBlock))
		return;
}