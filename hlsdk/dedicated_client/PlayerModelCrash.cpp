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
#include <r_studioint.h>

mstudioanim_t* (__fastcall* g_oStudioGetAnim)(CStudioModelRenderer*, void*, model_t*, mstudioseqdesc_t*);
mstudioanim_t* __fastcall CStudioModelRenderer_StudioGetAnim(CStudioModelRenderer* me, void* edx, model_t* m_pSubModel, mstudioseqdesc_t* pseqdesc)
{
	auto model = &me->m_pCurrentEntity->model;
	mstudioseqgroup_t* pseqgroup;
	cache_user_t* paSequences;
	auto localplayer = g_Pointers.g_pCL_EngineFuncs->GetLocalPlayer();
	studiohdr_t* studiomodel = (studiohdr_t*)localplayer->model->cache;

	if (localplayer)
		g_Pointers.g_pCL_EngineFuncs->Con_Printf("player model adr  %#010x\n", localplayer->model);
	g_Pointers.g_pCL_EngineFuncs->Con_Printf("render model adr %#010x\n", (DWORD)model);
	g_Pointers.g_pCL_EngineFuncs->Con_Printf("RenderModel name %s, ptr %#010x, adr %#010x\n", me->m_pRenderModel->name, me->m_pRenderModel, (DWORD)&me->m_pRenderModel);
	g_Pointers.g_pCL_EngineFuncs->Con_Printf("StudioHeader name %s, ptr %#010x, adr %#010x\n", me->m_pStudioHeader->name, me->m_pStudioHeader, (DWORD)&me->m_pStudioHeader);
	pseqgroup = (mstudioseqgroup_t*)((byte*)me->m_pStudioHeader + me->m_pStudioHeader->seqgroupindex) + pseqdesc->seqgroup;

	//if (!strstr(me->m_pRenderModel->name, me->m_pStudioHeader->name))
	//	return (mstudioanim_t*)((byte*)me->m_pStudioHeader + pseqdesc->animindex);

	if (pseqdesc->seqgroup == 0)
	{
		return (mstudioanim_t*)((byte*)me->m_pStudioHeader + pseqdesc->animindex);
	}

	paSequences = (cache_user_t*)m_pSubModel->submodels;

	if (paSequences == NULL)
	{
		paSequences = (cache_user_t*)g_pNightfirePlatformFuncs->mallocx(16 * sizeof(cache_user_t)); // UNDONE: leak!
		memset(paSequences, 0, 16 * sizeof(cache_user_t));
		m_pSubModel->submodels = (dmodel_t*)paSequences;
	}
	if (!g_pNightfireFileSystem->Cache_Check((struct cache_user_s*)&(paSequences[pseqdesc->seqgroup])))
	{
		if (g_Pointers.g_pCL_EngineFuncs)
			g_Pointers.g_pCL_EngineFuncs->Con_DPrintf("loading %s\n", pseqgroup->name);
		g_pNightfireFileSystem->COM_LoadCacheFile(pseqgroup->name, (struct cache_user_s*)&paSequences[pseqdesc->seqgroup]);
	}
	return (mstudioanim_t*)((byte*)paSequences[pseqdesc->seqgroup].data + pseqdesc->animindex);
}

DWORD invalid_sequence_jmpback;
DWORD valid_sequence_jmpback;
bool __cdecl StudioSequenceGetAnimPosOutOfBoundsCheck(CStudioModelRenderer* renderer, model_t* model)
{
	renderer->m_pStudioHeader = (studiohdr_t*)g_Pointers.g_pStudioModelAPI->Mod_Extradata(model);

	return renderer->m_pCurrentEntity->curstate.sequence < renderer->m_pStudioHeader->numseq;
}

__declspec(naked) void StudioGetAnimPos_Hook()
{
	__asm
	{
		JE invalid_seq

		push eax
		push esi
		push ebx

		push eax //model_t*
		push esi //CStudioModelRenderer*
		call StudioSequenceGetAnimPosOutOfBoundsCheck
		add esp, 8
		test al, al

		pop ebx
		pop esi
		pop eax


		je invalid_seq
		jmp valid_sequence_jmpback

		invalid_seq :
		jmp invalid_sequence_jmpback
	}
}


bool __cdecl StudioGaitSequenceBonesOutOfBoundsCheck(CStudioModelRenderer* renderer, player_info_s* info)
{
	if (info->gaitsequence == 0)
		return true;

	studiohdr_t* hdr = renderer->m_pStudioHeader;
	if (info->gaitsequence >= hdr->numseq)
	{
		info->gaitsequence = 0;
		return false;
	}

	return true;
}

DWORD invalid_gaitsequence_jmpback;
DWORD valid_gaitsequence_jmpback;
__declspec(naked) void StudioSetupBones_Hook()
{
	__asm
	{
		push eax //1
		push edx
		push edi
		push ebx //2
		push esi //3
		push ecx //4
		push eax // m_PlayerInfo
		push esi // thisptr
		call StudioGaitSequenceBonesOutOfBoundsCheck
		add esp, 8
		pop ecx //4
		pop esi //3
		pop ebx //2
		pop edi
		pop edx
		test al, al
		je invalid
		pop eax //1
		mov eax, [eax + 0x17C]
		jmp valid_gaitsequence_jmpback

		invalid :
		pop eax //1
			jmp invalid_gaitsequence_jmpback
	}
}

//studioapi_SetupPlayerModel
model_s* nf_hooks::R_StudioSetupPlayerModel(int playerindex)
{
	player_info_s* info = g_Pointers.ENG_GetPlayerInfo(playerindex);
	static ConsoleVariable* developer = g_pEngineFuncs->pfnGetConsoleVariableGame("developer");
	DM_PlayerState_s* state = (DM_PlayerState_s*)g_Pointers.DM_PlayerState;
	cl_entity_s* current_entity = (*g_Pointers.g_rCurrentEntity);

	if (current_entity)
	{
		if ((developer->getValueInt() || !g_Pointers.Host_IsSinglePlayerGame()) && info->name[0])
		{
			if (strcmp(state->modelname, info->model) || !state->model || state->model->isloaded != 1)
			{
				strncpy(state->modelname, info->model, 260);
				state->modelname[MAX_PATH - 1] = 0;

#if 1
				strncpy(state->modelpath, "models/player/", 260);
				strncat(state->modelpath, info->model, 260);
				state->modelpath[MAX_PATH - 1] = 0;
				strncat(state->modelpath, "/", 260);
				state->modelpath[MAX_PATH - 1] = 0;
				strncat(state->modelpath, info->model, 260);
				state->modelpath[MAX_PATH - 1] = 0;
				strncat(state->modelpath, ".mdl", 260);
				state->modelpath[MAX_PATH - 1] = 0;

				if (g_pNightfireFileSystem->COM_FileExists(state->modelpath, nullptr))
					state->model = g_Pointers.g_pCL_EngineFuncs->Mod_ForName(state->modelpath, false, true);
				else
				{
					strncpy(state->modelpath, "models/", 260);
					strncat(state->modelpath, info->model, 260);
					strncat(state->modelpath, ".mdl", 260);
					state->modelpath[MAX_PATH - 1] = 0;
					if (g_pNightfireFileSystem->COM_FileExists(state->modelpath, nullptr))
						state->model = g_Pointers.g_pCL_EngineFuncs->Mod_ForName(state->modelpath, false, true);
					else
					{
						strncpy(state->modelpath, "models/water/", 260);
						strncat(state->modelpath, info->model, 260);
						state->modelpath[MAX_PATH - 1] = 0;
						strncat(state->modelpath, ".mdl", 260);
						state->modelpath[MAX_PATH - 1] = 0;
						if (g_pNightfireFileSystem->COM_FileExists(state->modelpath, nullptr))
							state->model = g_Pointers.g_pCL_EngineFuncs->Mod_ForName(state->modelpath, false, true);
						else
						{
							strncpy(state->modelpath, "models/sky/", 260);
							strncat(state->modelpath, info->model, 260);
							state->modelpath[MAX_PATH - 1] = 0;
							strncat(state->modelpath, ".mdl", 260);
							state->modelpath[MAX_PATH - 1] = 0;
							if (g_pNightfireFileSystem->COM_FileExists(state->modelpath, nullptr))
								state->model = g_Pointers.g_pCL_EngineFuncs->Mod_ForName(state->modelpath, false, true);
							else
								state->model = nullptr;
						}
					}
				}
#else
				//gearbox code
				strncpy(state->modelpath, "models/player/", 260);
				state->modelpath[MAX_PATH - 1] = 0;
				strncat(state->modelpath, info->model, 260);
				state->modelpath[MAX_PATH - 1] = 0;
				strncat(state->modelpath, "/", 260);
				state->modelpath[MAX_PATH - 1] = 0;
				strncat(state->modelpath, info->model, 260);
				state->modelpath[MAX_PATH - 1] = 0;
				strncat(state->modelpath, ".mdl", 260);
				state->modelpath[MAX_PATH - 1] = 0;

				state->model = g_pCL_EngineFuncs->Mod_ForName(state->modelpath, false, true);
#endif

				if (!state->model)
					state->model = current_entity->model;

				g_Pointers.R_ChangeMPSkin();
			}
		}
		else
		{
			state->modelname[0] = 0;
			if (state->model != current_entity->model)
			{
				state->model = current_entity->model;
				g_Pointers.R_ChangeMPSkin();
			}
		}
	}
	return state->model;
}

void Fix_Model_Crash()
{
	DWORD adr;
	//if (!FindMemoryPattern(adr, *g_clientDllHinst, "56 8B 74 24 0C 8B 86 ? 00 00 00 85 C0 57 75 19", false))
	//	return;
	//if (!HookFunctionWithMinHook(adr, CStudioModelRenderer_StudioGetAnim, (void**)&g_oStudioGetAnim))
	//	return;
	//if (!FindMemoryPattern(g_Pointers.R_StudioSetupPlayerModel, g_engineDllHinst, "A1 ? ? ? ? 55 8B 6C 24 08 57 8B FD 69 FF", false))
	//	return;
	if (!g_Pointers.R_StudioSetupPlayerModel)
		return;
	if (!HookFunctionWithMinHook(g_Pointers.R_StudioSetupPlayerModel, nf_hooks::R_StudioSetupPlayerModel, nullptr))
		return;
	if (FindMemoryPattern(adr, *g_clientDllHinst, "8B 80 ? ? ? ? 85 C0 0F ? ? ? ? ? 8B ? ? ? ? ? 69 ? ? ? ? ? 03", false))
	{
		invalid_gaitsequence_jmpback = adr + 0xDC;
		valid_gaitsequence_jmpback = adr + 6;
		PlaceJMP((BYTE*)adr, (DWORD)&StudioSetupBones_Hook, 5);
	}
	if (FindMemoryPattern(adr, *g_clientDllHinst, "50 FF 15 ? ? ? ? 89 46 ? 88 5E", false))
	{
		valid_sequence_jmpback = adr;// +0xA;
		invalid_sequence_jmpback = adr + 0x1E3;
		PlaceJMP((BYTE*)(adr - 6), (DWORD)&StudioGetAnimPos_Hook, 6);
	}
}