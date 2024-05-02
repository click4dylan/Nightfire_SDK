#include "pattern_scanner.h"
#include "MinHook/MinHook.h"
#include <event_api.h>
#include "MetaHook.h"
#include <usercmd.h>
#include <cvardef.h>
#include <clientsideentity.h>
#include <NightfireFileSystem.h>
//#include <r_studioint.h>

//#include <vector_types.h>
//#include <eiface.h>
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
#include "pattern_scanner.h"
#include <r_studioint.h>

#include <pm_defs.h>

void nf_pointers::GetImportantEngineOffsets(long enginedll)
{
	if (!enginedll)
		return;

	if (!gpGlobals && !FindMemoryPattern(pattern_t(gpGlobals, enginedll, "68 ? ? ? ? 68 ? ? ? ? FF D0 A1 ? ? ? ? 83 F8 32 75 16 68", false, true, 1, 0, false, "gpGlobals", true)))
		return;

	if (!g_pEngineFuncs && (nf_hooks::LoadThisDLL_FunctionAddress || FindMemoryPattern(nf_hooks::LoadThisDLL_FunctionAddress, enginedll, "57 53 FF 15 ? ? ? ? 8B F8 85 FF 75 17", false)))
	{
		//gpGlobals = (globalvars_t*)*(DWORD*)(nf_hooks::LoadThisDLL_FunctionAddress + 0x4D);
		g_pEngineFuncs = (enginefuncs_s*)*(DWORD*)(nf_hooks::LoadThisDLL_FunctionAddress + 0x52);
	}

	if (!psvs && !FindMemoryPattern(psvs, enginedll, "E8 ? ? ? ? A0 ? ? ? ? 84 C0 74 0E 68", false, true, 6, 0, false))
		return;

	if (!EV_PlayerTrace)
		FindMemoryPattern(EV_PlayerTrace, enginedll, "8B 44 24 14 8B 4C 24 10 8B 54 24 0C 83 EC 48", false);

	if (!EV_SetTraceHull)
		if (!FindMemoryPattern(EV_SetTraceHull, enginedll, "8B 44 24 04 8B 0D ? ? ? ? 89 81 ? 00 00 00 C3", false))
			return;
	
	if (!g_pCL_EngineFuncs)
	{
		DWORD adr;
		FindMemoryPattern(adr, enginedll, "68 ? ? ? ? FF 15 ? ? ? ? 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 10", false);
		if (adr)
			g_pCL_EngineFuncs = *(cl_enginefuncs_s**)((DWORD)adr + 1);
	}

	if (!g_PlayerMove)
		g_PlayerMove = *(playermove_t***)((DWORD)EV_SetTraceHull + 6);

	if (!svs_maxclients)
	{
		DWORD adr;
		FindMemoryPattern(adr, enginedll, "83 3D ? ? ? ? 01 7E 2E 8B 4C 24 0C 8D 44 24 10 50", false);
		if (adr)
			svs_maxclients = *(int**)(adr + 2);
	}

	if (!Log_Printf)
		FindMemoryPattern(Log_Printf, enginedll, "A0 ? ? ? ? 81 EC 04 08 00 00 84 C0", false);

	if (!ENG_GetPlayerInfo)
		FindMemoryPattern(ENG_GetPlayerInfo, enginedll, "8B 44 24 04 69 C0 B4 01 00 00 05 ? ? ? ? C3 55", false);

	if (!Host_IsSinglePlayerGame)
		FindMemoryPattern(Host_IsSinglePlayerGame, enginedll, "A0 ? ? ? ? 84 C0 8B 0D ? ? ? ? 74 06", false);

	if (!R_ChangeMPSkin)
		FindMemoryPattern(R_ChangeMPSkin, enginedll, "A1 ? ? ? ? 8B 08 8B 04 ? ? ? ? ? 85 C0", false);

	if (!g_rCurrentEntity)
	{
		DWORD adr;
		FindMemoryPattern(adr, enginedll, "A1 ? ? ? ? 55 8B 6C 24 08 57 8B FD 69 FF", false);
		if (adr)
		{
			g_rCurrentEntity = *(struct cl_entity_s***)((DWORD)adr + 1);
			DM_PlayerState = (char*)*(DWORD*)((DWORD)adr + 0x15);
		}
	}

	if (!g_psv)
	{
		DWORD adr;
		FindMemoryPattern(adr, enginedll, "68 ? ? ? ? FF 15 ? ? ? ? 8B 3D ? ? ? ? 6A 40", false);
		if (adr)
			g_psv = *(server_t**)(adr + 1);
	}

	// get studio model api from the engine
	if (!g_pStudioModelAPI)
	{
		DWORD adr;
		if (FindMemoryPattern(adr, enginedll, "68 ? ? ? ? 68 ? ? ? ? 6A 01 FF D0 83 C4 0C 85 C0 75 12", false, true, 1))
		{
			g_pStudioModelAPI = (engine_studio_api_s*)adr;
			g_StudioModelAPI = g_pStudioModelAPI;
		}
	}


	if (!R_StudioSetupPlayerModel)
		FindMemoryPattern(R_StudioSetupPlayerModel, enginedll, "A1 ? ? ? ? 55 8B 6C 24 08 57 8B FD 69 FF", false);

	if (!CL_InitTempEntModels)
	{
		FindMemoryPattern(pattern_t(CL_InitTempEntModels, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, "CL_initTempEntModels", true));
		FindMemoryPattern(pattern_t(cl_sprite_dot, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0x18, 0, false, "cl_sprite_dot", true));
		FindMemoryPattern(pattern_t(cl_sprite_lightning, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0x2B, 0, false, "cl_sprite_lightning", true));
		FindMemoryPattern(pattern_t(cl_sprite_glow, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0x3E, 0, false, "cl_sprite_glow", true));
		FindMemoryPattern(pattern_t(cl_sprite_muzzleflash, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0x51, 0, false, "cl_sprite_muzzleflash", true));
		FindMemoryPattern(pattern_t(cl_sprite_muzzleflash2, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0x64, 0, false, "cl_sprite_muzzleflash2", true));
		FindMemoryPattern(pattern_t(cl_sprite_muzzleflash3, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0x7A, 0, false, "cl_sprite_muzzleflash3", true));
		FindMemoryPattern(pattern_t(cl_sprite_muzzleflash4, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0x8D, 0, false, "cl_sprite_muzzleflash4", true));
		FindMemoryPattern(pattern_t(cl_sprite_muzzleflash5, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0xA0, 0, false, "cl_sprite_muzzleflash5", true));
		FindMemoryPattern(pattern_t(cl_sprite_muzzleflash6, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0xB3, 0, false, "cl_sprite_muzzleflash6", true));
		FindMemoryPattern(pattern_t(cl_sprite_muzzleflash7, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0xC6, 0, false, "cl_sprite_muzzleflash7", true));
		FindMemoryPattern(pattern_t(cl_sprite_muzzleflash8, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0xD9, 0, false, "cl_sprite_muzzleflash8", true));
		FindMemoryPattern(pattern_t(cl_sprite_muzzleflash9, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0xEF, 0, false, "cl_sprite_muzzleflash9", true));
		FindMemoryPattern(pattern_t(cl_sprite_ricochet, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0x102, 0, false, "cl_sprite_ricochet", true));
		FindMemoryPattern(pattern_t(cl_sprite_shell, enginedll, "6A 00 6A 00 68 ? ? ? ? E8 ? ? ? ? 6A 00", false, true, 0x10F, 0, false, "cl_sprite_shell", true));
		FindMemoryPattern(pattern_t(CL_ParseTextMessage, enginedll, "51 56 57 E8 ? ? ? ? 8B F8", false, "CL_ParseTextMessage", true));
		FindMemoryPattern(pattern_t(R_DefaultSprite, enginedll, "51 DD 05 ? ? ? ? 57", false, "R_DefaultSprite", true));
		FindMemoryPattern(pattern_t(S_StartDynamicSound, enginedll, "8B 0D ? ? ? ? 85 C9 74 2D 8B 54 24 20 52 8B 54 24 20 8B 01 52 8B 54 24 20 52 8B 54 24 20 52 8B 54 24 20 52 8B 54 24 20 52 8B 54 24 20 52 8B 54 24 20 52 FF 50 0C C3", false, "S_StartDynamicSound", true));
		FindMemoryPattern(pattern_t(CL_ParseTEnt, enginedll, "55 8B EC 83 E4 F8 83 EC 58", false, "CL_ParseTEnt", true));
		FindMemoryPattern(pattern_t(cl_sfx_r_exp1, enginedll, "55 8B EC 83 E4 F8 83 EC 58", false, true, 0xA62, 0, false, "cl_sfx_r_exp1", true));
		FindMemoryPattern(pattern_t(cl_sfx_r_exp2, enginedll, "55 8B EC 83 E4 F8 83 EC 58", false, true, 0xA07, 0, false, "cl_sfx_r_exp2", true));
		FindMemoryPattern(pattern_t(cl_sfx_r_exp3, enginedll, "55 8B EC 83 E4 F8 83 EC 58", false, true, 0xA34, 0, false, "cl_sfx_r_exp3", true));
		FindMemoryPattern(pattern_t(cl_maxents, enginedll, "3B 3D ? ? ? ? 7C 0E 57 68", false, true, 2, 0, false));
		FindMemoryPattern(pattern_t(Sys_Error, enginedll, "E8 ? ? ? ? 8B 5C 24 34", false, "Sys_Error", true));
		FindMemoryPattern(pattern_t(MSG_StartBitWriting, enginedll, "E8 ? ? ? ? 0F B7 84 24 ? ? ? ?", false, false, 0, 1, true, "MSG_StartBitWriting", true));
		FindMemoryPattern(pattern_t(MSG_IsBitWriting, enginedll, "8B 0D ? ? ? ? 33 C0 85 C9 0F 95 C0 C3 90 90 A1 ? ? ? ? 85 C0", false, "MSG_IsBitWriting", true));
		FindMemoryPattern(pattern_t(MSG_CurrentBit, enginedll, "E8 ? ? ? ? 6A 08 89 44 24 20 ", false, false, 0, 1, true, "MSG_CurrentBit", true));
		FindMemoryPattern(pattern_t(MSG_IsBitReading, enginedll, "8B 0D ? ? ? ? 33 C0 85 C9 0F 95 C0 C3 90 90 A1 ? ? ? ? 33 C9", false, "MSG_IsBitReading", true));
		FindMemoryPattern(pattern_t(MSG_StartBitReading, enginedll, "E8 ? ? ? ? 6A 09 E8 ? ? ? ?", false, false, 0, 1, true, "MSG_StartBitReading", true));
		FindMemoryPattern(pattern_t(MSG_EndBitReading, enginedll, "E8 ? ? ? ? D9 45 40", false, false, 0, 1, true, "MSG_EndBitReading", true)); //fixme: check this
		FindMemoryPattern(pattern_t(MSG_ReadOneBit, enginedll, "E8 ? ? ? ? 8B F0 8B 44 24 08", false, false, 0, 1, true, "MSG_ReadOneBit", true));
		FindMemoryPattern(pattern_t(MSG_ReadBits, enginedll, "E8 ? ? ? ? 8B 37 ", false, false, 0, 1, true, "MSG_ReadBits", true));
		FindMemoryPattern(pattern_t(MSG_PeekBits, enginedll, "8B 44 24 04 83 EC 18", false, "MSG_PeekBits", true));
		FindMemoryPattern(pattern_t(MSG_ReadSBits, enginedll, "E8 ? ? ? ? 0F BF D0", false, false, 0, 1, true, "MSG_ReadSBits", true));
		FindMemoryPattern(pattern_t(MSG_ReadBitString, enginedll, "56 6A 08 C6 05 ? ? ? ? ?", false, "MSG_ReadBitString", true));
		FindMemoryPattern(pattern_t(MSG_ReadBitData, enginedll, "E8 ? ? ? ? 83 C4 10 6A 20", false, false, 0, 1, true, "MSG_ReadBitData", true));
		FindMemoryPattern(pattern_t(MSG_ReadBitCoord, enginedll, "51 6A 20 E8 ? ? ? ?", false, "MSG_ReadBitCoord", true));
		FindMemoryPattern(pattern_t(MSG_ReadBitVec3Coord, enginedll, "56 6A 20 E8 ? ? ? ? 8B 74 24 0C ", false, "MSG_ReadBitVec3Coord", true));
		FindMemoryPattern(pattern_t(MSG_BeginReading, enginedll, "E8 ? ? ? ? 85 F6 74 2A", false, false, 0, 1, true, "MSG_BeginReading", true));
		FindMemoryPattern(pattern_t(MSG_ReadChar, enginedll, "E8 ? ? ? ? 89 44 24 10 DB 44 24 10 46", false, false, 0, 1, true, "MSG_ReadChar", true));
		FindMemoryPattern(pattern_t(MSG_ReadByte, enginedll, "E8 ? ? ? ? 0F B6 C8", false, false, 0, 1, true, "MSG_ReadByte", true));
		FindMemoryPattern(pattern_t(MSG_ReadShort, enginedll, "E8 ? ? ? ? 83 FE 0D", false, false, 0, 1, true, "MSG_ReadShort", true));
		FindMemoryPattern(pattern_t(MSG_ReadLong, enginedll, "E8 ? ? ? ? 89 46 50", false, false, 0, 1, true, "MSG_ReadLong", true));
		FindMemoryPattern(pattern_t(MSG_ReadFloat, enginedll, "E8 ? ? ? ? DD 1D ? ? ? ? 83 3D ? ? ? ? ?", false, false, 0, 1, true, "MSG_ReadFloat", true));
		FindMemoryPattern(pattern_t(MSG_ReadBuf, enginedll, "E8 ? ? ? ? 83 C4 08 6A 20", false, false, 0, 1, true, "MSG_ReadBuf", true));
		FindMemoryPattern(pattern_t(MSG_ReadString, enginedll, "E8 ? ? ? ? 6A 28", false, false, 0, 1, true, "MSG_ReadString", true));
		FindMemoryPattern(pattern_t(MSG_ReadStringLine, enginedll, "E8 ? ? ? ? 8D 4C 24 18 8B F0", false, false, 0, 1, true, "MSG_ReadStringLine", true));
		FindMemoryPattern(pattern_t(MSG_ReadAngle, enginedll, "E8 ? ? ? ? D9 5C 24 54 89 74 24 58", false, false, 0, 1, true, "MSG_ReadAngle", true));
		FindMemoryPattern(pattern_t(MSG_ReadHiresAngle, enginedll, "E8 ? ? ? ? D9 5C 24 00 D9 05 ? ? ? ?", false, false, 0, 1, true, "MSG_ReadHiresAngle", true));
		FindMemoryPattern(pattern_t(MSG_WriteByte, enginedll, "E8 ? ? ? ? 8B 87 ? ? ? ? 50 E8 ? ? ? ? 50", false, false, 0, 1, true, "MSG_WriteByte", true));
		FindMemoryPattern(pattern_t(MSG_WriteShort, enginedll, "E8 ? ? ? ? 6A 2D", false, false, 0, 1, true, "MSG_WriteShort", true));
		FindMemoryPattern(pattern_t(MSG_WriteWord, enginedll, "8B 44 24 04 6A 02 50 E8 ? ? ? ? 8B 4C 24 10 88 08 C1 F9 08 83 C4 08 88 48 01 C3 90 90 90 90 8B 44 24 04 6A 04", false, "MSG_WriteWord", true));
		FindMemoryPattern(pattern_t(MSG_WriteLong, enginedll, "E8 ? ? ? ? 6A 39", false, false, 0, 1, true, "MSG_WriteLong", true));
		FindMemoryPattern(pattern_t(MSG_WriteFloat, enginedll, "E8 ? ? ? ? 8A 15 ? ? ? ?", false, false, 0, 1, true, "MSG_WriteFloat", true));
		FindMemoryPattern(pattern_t(MSG_WriteString, enginedll, "E8 ? ? ? ? 8D 44 24 44", false, false, 0, 1, true, "MSG_WriteString", true));
		FindMemoryPattern(pattern_t(MSG_WriteBuf, enginedll, "E8 ? ? ? ? D9 43 28", false, false, 0, 1, true, "MSG_WriteBuf", true));
		FindMemoryPattern(pattern_t(MSG_WriteAngle, enginedll, "E8 ? ? ? ? 83 C4 10 83 C7 04", false, false, 0, 1, true, "MSG_WriteAngle", true));
		FindMemoryPattern(pattern_t(MSG_WriteHiresAngle, enginedll, "D9 44 24 08 53", false, "MSG_WriteHiresAngle", true));
		FindMemoryPattern(pattern_t(MSG_WriteOneBit, enginedll, "8B 0D ? ? ? ? 83 F9 08 7C 25", false, "MSG_WriteOneBit", true));
		FindMemoryPattern(pattern_t(MSG_EndBitWriting, enginedll, "A1 ? ? ? ? F6 40 04 02", false, "MSG_EndBitWriting", true));
		FindMemoryPattern(pattern_t(MSG_WriteBits, enginedll, "E8 ? ? ? ? 8B 76 08", false, false, 0, 1, true, "MSG_WriteBits", true));
		FindMemoryPattern(pattern_t(MSG_WriteSBits, enginedll, "E8 ? ? ? ? 83 C4 08 EB 69 ", false, false, 0, 1, true, "MSG_WriteSBits", true));
		FindMemoryPattern(pattern_t(MSG_WriteBitString, enginedll, "56 8B 74 24 08 8A 06", false, "MSG_WriteBitString", true));
		FindMemoryPattern(pattern_t(MSG_WriteBitData, enginedll, "56 8B 74 24 08 57 8B 7C 24 10 85 FF", false, "MSG_WriteBitData", true));
		FindMemoryPattern(pattern_t(MSG_WriteBitAngle, enginedll, "8B 4C 24 08 83 F9 20", false, "MSG_WriteBitAngle", true));
		FindMemoryPattern(pattern_t(MSG_ReadBitAngle, enginedll, "E8 ? ? ? ? 8B 46 24", false, false, 0, 1, true, "MSG_ReadBitAngle", true));
		FindMemoryPattern(pattern_t(MSG_WriteBitCoord, enginedll, "8B 44 24 04 6A 20", false, "MSG_WriteBitCoord", true));
		FindMemoryPattern(pattern_t(MSG_WriteBitVec3Coord, enginedll, "56 8B 74 24 08 8B 06 6A 20", false, "MSG_WriteBitVec3Coord", true));
		FindMemoryPattern(pattern_t(MSG_ReadCoord, enginedll, "E8 ? ? ? ? D9 5E 04", false, false, 0, 1, true, "MSG_ReadCoord", true));
		FindMemoryPattern(pattern_t(MSG_WriteCoord, enginedll, "E8 ? ? ? ? 8B 0F 51", false, false, 0, 1, true, "MSG_WriteCoord", true));
		FindMemoryPattern(pattern_t(MSG_ReadVec3Coord, enginedll, "51 A1 ? ? ? ? 8B 0D ? ? ? ? 8A 14 08 88 54 24 00 8A 54 08 01 88 54 24 01 8A 54 08 02 8A 4C 08 03 88 54 24 02 88 4C 24 03 8B 54 24 00 56", false, "MSG_ReadVec3Coord", true));
		FindMemoryPattern(pattern_t(MSG_WriteVec3Coord, enginedll, "53 56 8B 74 24 10 8B 06", false, "MSG_WriteVec3Coord", true));
		FindMemoryPattern(pattern_t(MSG_ReadUsercmd, enginedll, "E8 ? ? ? ? 8B C6 83 C4 08", false, false, 0, 1, true, "MSG_ReadUsercmd", true));
		FindMemoryPattern(pattern_t(MSG_WriteUsercmd, enginedll, "E8 ? ? ? ? 83 C4 0C 4F", false, false, 0, 1, true, "MSG_WriteUsercmd", true));

		// interfaces to game.dll
		FindMemoryPattern(pattern_t(g_EntityInterface, enginedll, "68 ? ? ? ? 89 74 24 18", false, true, 1, 0, false, "gEntityInterface", true));
		FindMemoryPattern(pattern_t(g_NewDLLFunctions, enginedll, "68 ? ? ? ? C7 44 24 18", false, true, 1, 0, false, "gNewDLLFunctions", true));
	}
}

void nf_pointers::GetImportantClientOffsets(long clientdll)
{
	if (!clientdll)
		return;

	if (!UTIL_CheckForWater)
		FindMemoryPattern(UTIL_CheckForWater, clientdll, "A1 ? ? ? ? 83 EC 48 6A 02", false);

	if (!g_ppStudioAPI)
	{
		if (FindMemoryPattern(g_ppStudioAPI, clientdll, "B9 ? ? ? ? E8 ? ? ? ? B8 01 00 00 00 5D C3", false))
			g_ppStudioAPI = (class CStudioModelRenderer**)((uintptr_t)g_ppStudioAPI + 1);
	}
	if (g_ppStudioAPI)
		g_pStudioAPI = *g_ppStudioAPI;
}

int nf_pointers::EV_GetTraceHull()
{
	return (*g_PlayerMove)->usehull;
}