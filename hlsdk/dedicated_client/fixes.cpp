#include <Windows.h>
#include <vector>
//#include <extdll.h>	
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
//#include <r_studioint.h>

typedef float vec_t;
typedef float vec2_t[2];
typedef float vec3_t[3];
//#include <eiface.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <platformdll.h>
#include <enginefuncs.h>
#include <globalvars.h>

struct cl_entity_s** g_rCurrentEntity = nullptr;
int* svs_maxclients = 0;
void(*EV_SetTraceHull)(int hull) = 0;
DWORD g_PlayerMove = 0;
DWORD offset = 0;
int EV_GetTraceHull()
{
	return *(int*)(*(DWORD*)g_PlayerMove + offset);
}
void(*EV_PlayerTrace) (float* start, float* end, int brushFlags, int traceFlags, int ignore_pe, struct pmtrace_s* tr) = nullptr;
void(*g_oLog_Printf)(const char*, ...) = nullptr;
player_info_s* (*ENG_GetPlayerInfo)(int index) = nullptr;
BOOL(*Host_IsSinglePlayerGame)() = nullptr;
void(*R_ChangeMPSkin)() = nullptr;
int(*g_oR_StudioSetupPlayerModel)(int);
extern model_s* R_StudioSetupPlayerModel(int playerindex);
char* DM_PlayerState = 0;

void GetImportantEngineOffsets()
{
	if (!g_engineDllHinst)
		return;

	if (!EV_PlayerTrace)
		EV_PlayerTrace = (void(*)(float*, float*, int, int, int, struct pmtrace_s*))FindMemoryPattern(g_engineDllHinst, "8B 44 24 14 8B 4C 24 10 8B 54 24 0C 83 EC 48", false);
	if (!EV_SetTraceHull)
		EV_SetTraceHull = (void(*)(int))FindMemoryPattern(g_engineDllHinst, "8B 44 24 04 8B 0D ? ? ? ? 89 81 ? 00 00 00 C3", false);
	if (!EV_SetTraceHull)
		return;
	if (!g_pCL_EngineFuncs)
	{
		g_pCL_EngineFuncs = (cl_enginefuncs_s*)FindMemoryPattern(g_engineDllHinst, "68 ? ? ? ? FF 15 ? ? ? ? 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 10", false);
		if (g_pCL_EngineFuncs)
			g_pCL_EngineFuncs = *(cl_enginefuncs_s**)((DWORD)g_pCL_EngineFuncs + 1);
	}
	if (!g_PlayerMove)
		g_PlayerMove = *(DWORD*)((DWORD)EV_SetTraceHull + 6);
	if (!offset)
		offset = *(DWORD*)((DWORD)EV_SetTraceHull + 0xC);
	if (!svs_maxclients)
	{
		svs_maxclients = (int*)FindMemoryPattern(g_engineDllHinst, "83 3D ? ? ? ? 01 7E 2E 8B 4C 24 0C 8D 44 24 10 50", false);
		if (svs_maxclients)
			svs_maxclients = *(int**)((DWORD)svs_maxclients + 2);
	}
	if (!g_oLog_Printf)
		g_oLog_Printf = (void(*)(const char*, ...))FindMemoryPattern(g_engineDllHinst, "A0 ? ? ? ? 81 EC 04 08 00 00 84 C0", false);
	if (!ENG_GetPlayerInfo)
		ENG_GetPlayerInfo = (player_info_s * (*)(int))FindMemoryPattern(g_engineDllHinst, "8B 44 24 04 69 C0 B4 01 00 00 05 ? ? ? ? C3 55", false);
	if (!Host_IsSinglePlayerGame)
		Host_IsSinglePlayerGame = (BOOL(*)())FindMemoryPattern(g_engineDllHinst, "A0 ? ? ? ? 84 C0 8B 0D ? ? ? ? 74 06", false);
	if (!R_ChangeMPSkin)
		R_ChangeMPSkin = (void(*)())FindMemoryPattern(g_engineDllHinst, "A1 ? ? ? ? 8B 08 8B 04 ? ? ? ? ? 85 C0", false);
	if (!g_rCurrentEntity)
	{
		g_rCurrentEntity = (struct cl_entity_s**)FindMemoryPattern(g_engineDllHinst, "A1 ? ? ? ? 55 8B 6C 24 08 57 8B FD 69 FF", false);
		DM_PlayerState = (char*)*(DWORD*)((DWORD)g_rCurrentEntity + 0x15);
		if (g_rCurrentEntity)
			g_rCurrentEntity = *(struct cl_entity_s***)((DWORD)g_rCurrentEntity + 1);
	}

}

void GetImportantClientOffsets()
{
	if (!g_clientDllHinst)
		return;
	DWORD studiorenderapi = FindMemoryPattern((DWORD)*g_clientDllHinst, "B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? B8 01 00 00 00 5D C3", false);
	if (studiorenderapi)
		g_pStudioAPI = *(class CStudioModelRenderer**)FindMemoryPattern((DWORD)*g_clientDllHinst, "B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? B8 01 00 00 00 5D C3", false);
}

DWORD GUI_GetAction_JmpBack;
__declspec(naked) void GUI_GetAction_Return()
{
	__asm
	{
		test eax, eax
		jz isnullptr
		mov edx, [eax]
		push ebx
		mov ecx, eax
		call [edx + 8]
		isnullptr:
		jmp GUI_GetAction_JmpBack
	}
}

// this fix is not ideal as audio files will still leak if they are changed during runtime,
// but this will allow us to have more than 768 sounds, and stops the CGBAudio::m_registerSoundFX array being overflowed all the time
// also fixes crash when reconnecting to servers multiple times
//
	DWORD loadSound_JmpBack;
	__declspec(naked) void loadSound_CrashFix()
	{
		__asm
		{
			mov eax, [esp + 4]
			test eax, eax
			jz isnullptr
			sub esp, 0x354
			jmp loadSound_JmpBack
			isnullptr :
			xor eax, eax
				ret 8
		}
	}
	void Fix_Sound_Overflow()
	{
#if 0
		//0x4307EFF6
		DWORD find_name = FindMemoryPattern(g_engineDllHinst, "7E 27 8B F7 8D 45 48 8B D6 90", false);
		if (find_name)
		{
			// prevent CGBAudio::findName from reusing old slot when AudioAPIGetSoundID did not find any crc matching loaded sound filenames
			PlaceShort((BYTE*)find_name, 0x27EB);

			//0x430813D0
			DWORD constructor = FindMemoryPattern(g_engineDllHinst, "68 00 FC 00 00 89 1D", false);
			if (constructor)
			{
				// increase max audio files limit in CGBAudio::findName from 768 to 4096
				PlaceInt((BYTE*)(find_name + 0x2B), 4096);
				PlaceInt((BYTE*)(find_name + 0x32), 4096);

				// increase allocation size for audio files in CGBAudio::CGBAudio()
				// note: nightfire alpha demo struct size is 80 bytes
				PlaceInt((BYTE*)(constructor + 1), 4096 * 84); //alloc
				PlaceInt((BYTE*)(constructor + 0x14), 4096 * 84); //memset
			}

		}
#endif

		//0x4307ED40
		DWORD end_precaching = FindMemoryPattern(g_engineDllHinst, "C6 41 05 00 A1 ?? ?? ?? ?? 57 33 FF 85 C0", false);
		if (end_precaching)
		{
			// prevent CGBAudio::endPrecaching from unloading audio files when servercount changes
			PlaceShort((BYTE*)(end_precaching + 0xE), 0x69EB);
		}

#if 0
		//0x43080190
		DWORD load_sound = FindMemoryPattern(g_engineDllHinst, "81 EC 54 03 00 00 53", false);
		if (load_sound)
		{
			// stop CGBAudio::loadSound from dereferencing nullptr on first argument due to CGBAudio::findName returning nullptr
			//PlaceJMP((BYTE*)load_sound, (DWORD)&loadSound_CrashFix, 5);
			loadSound_JmpBack = load_sound + 6;
		}
#endif
	}
//

bool(*g_oUTIL_CheckForWater)(float, float, float, float, float, float, float*);

bool UTIL_CheckForWater_Hook(float startx, float starty, float startz, float endx, float endy, float endz, float* dest)
{
	pmtrace_s tr;
	int old_hull = EV_GetTraceHull(); //gearbox didn't have this
	EV_SetTraceHull(2);
	EV_PlayerTrace(&startx, &endx, 0x100000, 0x31, -1, &tr);
	EV_SetTraceHull(old_hull);// this is the fix for broken hull bounds when in water!!
	if (tr.fraction >= 1.0f || !tr.surface)
		return false;
	if (!tr.surface->parent_brush || !(tr.surface->parent_brush->contents & 0x100000) || tr.plane.normal[2] <= 0.99f)
		return false;

	dest[0] = tr.endpos[0];
	dest[1] = tr.endpos[1];
	dest[2] = tr.endpos[2];
	return true;
}

void Fix_Water_Hull()
{
	//Fixes bug introduced somewhere between alpha demo and public build in UTIL_CheckForWater
	//Hull gets set to point size and then not restored, causing major movement prediction errors, lag and audio glitches whenever this function gets called
	DWORD adr = FindMemoryPattern(*g_clientDllHinst, "A1 ? ? ? ? 83 EC 48 6A 02", false);
	if (!adr)
		return;
	
	if (!HookFunctionWithMinHook((void*)adr, UTIL_CheckForWater_Hook, (void**)&g_oUTIL_CheckForWater))
		return;
}


void __stdcall RainDropTrace(DWORD ESI, float* vecStart, float* vecEnd, int brushflags, int traceflags, int pSkip, pmtrace_s* tr)
{
	int old_hull = EV_GetTraceHull(); //gearbox didn't have this
	g_oCL_EngineFuncs.pEventAPI->EV_SetTraceHull(2);

	g_oCL_EngineFuncs.pEventAPI->EV_PlayerTrace(vecStart, vecEnd, brushflags, traceflags, pSkip, tr);

	const float groundzpos = tr->endpos[2];
	*(float*)(ESI + 0x9C) = groundzpos + 3.0f;

	EV_PlayerTrace(vecStart, vecEnd, 0x100000, 0x31, -1, tr);

#if 1
	if (tr->endpos[2] > groundzpos)
		*(float*)(ESI + 0x9C) = tr->endpos[2];
#else
	vec3_t waterpos;
	if (UTIL_CheckForWater_Hook(vecStart[0], vecStart[1], vecStart[2], vecEnd[0], vecEnd[2], vecEnd[2], waterpos))
	{
		//*(float*)(ESI + 0x94) = waterpos[0];
		//*(float*)(ESI + 0x98) = waterpos[1];
		*(float*)(ESI + 0x9C) = waterpos[2];
	}
#endif

	g_oCL_EngineFuncs.pEventAPI->EV_SetTraceHull(old_hull);
}

__declspec(naked) void WaterDropRayTrace()
{
	__asm
	{
		FSTP DWORD PTR SS : [ESP + 0x2C]
		push ebx
		push ecx
		push -1
		push 2
		push 0
		push edx
		push eax
		push esi
		call RainDropTrace
		pop ebx
		mov al, bl
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 0x88
		retn
	}
}

float splash_z_offset = 0.0f;
void Fix_RainDrop_WaterCollision()
{
	DWORD adr = FindMemoryPattern(*g_clientDllHinst, "D9 44 24 7C D8 05 ? ? ? ? 83 C4 18 8A C3 D9", false);
	if (!adr)
		return;
	PlaceJMP((BYTE*)(adr - 0x15), (DWORD)&WaterDropRayTrace, 5);
	DWORD touch = FindMemoryPattern(*g_clientDllHinst, "0F 85 ? ? ? ? D9 44 24 2C 8B 74 24 28", false);
	if (!touch)
		return;
	//increase frequency of splashes
	DWORD old, old2;
	VirtualProtect((LPVOID)touch, 128, PAGE_EXECUTE_READWRITE, &old);
	*(DWORD*)touch = 0x90909090;
	*(unsigned short*)(touch + 4) = 0x9090;

	//make splashes work on water by removing addition of z offset by 32 units..
	*(float**)(touch + 0x10) = &splash_z_offset;

	VirtualProtect((LPVOID)touch, 128, old, &old2);
}

float edict_yaw_times[4096] = { FLT_MIN };
void(__fastcall *g_oAI_ChangeYaw)(void*, void*, int);
// TODO: move this to AMX and do a proper fix for all entity types
void __fastcall AI_ChangeYaw(void* ent, void* edx, int speed)
{
	//hack... can't access these tables without header madness
	typedef int(*pIndexOfEdictFn)(struct edict_s*);
	typedef edict_s* (*pFindEntityByVarsFn)(struct entvars_s*);
#define pentityofentindex 0x4306FC90
	pIndexOfEdictFn pIndexOfEdict = (pIndexOfEdictFn)0x4306FC40;//*(DWORD*)((DWORD)g_pEngineFuncs + 72);
	pFindEntityByVarsFn pFindEntityByVars = (pFindEntityByVarsFn)0x4306FD00; //*(DWORD*)((DWORD)g_pEngineFuncs + 20);
	struct entvars_s* entvars = *(entvars_s**)((DWORD)ent + 4);
	float* globalvars2 = (float*)0x4217A168;
	float frametime = globalvars2[1];

	auto ed = pFindEntityByVars(entvars);
	int entindex = pIndexOfEdict(ed);
	if (entindex < 0 || entindex >= ARRAYSIZE(edict_yaw_times))
		return g_oAI_ChangeYaw(ent, edx, speed);

	auto time = Sys_FloatTime();
	if (time < edict_yaw_times[entindex])
		return;

	globalvars2[1] = time - edict_yaw_times[entindex];

	g_oAI_ChangeYaw(ent, edx, speed);

	globalvars2[1] = frametime;
	edict_yaw_times[entindex] = time + 0.03125f;
}

void Fix_AI_TurnSpeed()
{
	static bool already_fixed = false;
	if (!already_fixed)
	{
		DWORD gamedll = (DWORD)GetModuleHandleA("game.dll");
		if (!gamedll)
			return;

		already_fixed = true;

		DWORD adr = FindMemoryPattern(gamedll, "90 90 90 90 90 90 83 EC 0C 56 8B F1 8B 46 04 8B 48 54 51", false);
		if (!adr)
			return;

		adr += 6;

		if (!HookFunctionWithMinHook((void*)adr, AI_ChangeYaw, (void**)&g_oAI_ChangeYaw))
			return;
	}
}

// Forces various entities on the server to not delete themselves and instead send to the client
// Fixes things such as sprite render fx, model/breakable collision, etc
void Force_ServerSide_Entities_GameDLL()
{
	DWORD adr = FindMemoryPattern(g_gameDllHinst, "39 98 ? ? 00 00 75 11 39 98 ? ? 00 00 75 09 56", false);
	if (adr)
	{
		//force CGenericItem to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 6) = 0xEB;
		PopProtection();
	}
	adr = FindMemoryPattern(g_gameDllHinst, "8B 88 ? ? 00 00 85 C9 75 13 8B 88 ? ? 00 00 85 C9", false);
	if (adr)
	{
		//force CBreakableItem to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 8) = 0xEB;
		PopProtection();
	}
	adr = FindMemoryPattern(g_gameDllHinst, "0F 94 C1 33 D2 85 DB 0F 94 C2 85 CA 74 09", false);
	if (adr)
	{
		//force CHangingLantern to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 12) = 0xEB;
		PopProtection();
	}
	adr = FindMemoryPattern(g_gameDllHinst, "8B 82 ? ? 00 00 85 C0 75 09 56", false);
	if (adr)
	{
		//force CEnvGlow to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 8) = 0xEB;
		PopProtection();
	}
	adr = FindMemoryPattern(g_gameDllHinst, "F6 80 ? ? 00 00 04 75 11 39 B8 ? ? 00 00 75 09", false);
	if (adr)
	{
		//force CEnvSprite to not remove itself on the server
		PushProtection(adr);
		*(unsigned char*)(adr + 7) = 0xEB;
		PopProtection();
	}
	//FIXME: worldspawn and env_fog are still client side! They don't have a specific call to UTIL_Remove..
}

// returns the allocated entity
void* ClientEntityHandler_Dummy(std::string const& name, std::vector<std::pair<std::string, std::string>>const& keyvalues)
{
	return nullptr;
}

void Force_ServerSide_Entities_ClientDLL()
{
	//TODO: FIXME: check if server we are connected to is running latest patch!!!
	//Otherwise we will be missing clientside entities on old servers!
	DWORD adr = FindMemoryPattern((DWORD)*g_clientDllHinst, "8B 0C F5 ? ? ? ? 51 50 FF D3", false);
	if (adr)
	{
		// replace all the client side entity create functions with a dummy so the client doesn't create them!!
		int numclientsidedentities = *(unsigned char*)(adr + 0x12);
		CLIENTSIDEENTITY* client_side_ent_table = *(CLIENTSIDEENTITY**)(adr + 3);
		PushProtection(client_side_ent_table);

		// nightfire 1.1 has 7 client sided entities, in addition to these there is "env_fog" and "worldspawn"
		const char* blocked_clientsided_entities[] = { "env_sprite", "env_glow", "item_generic", "item_breakable", "physics_lantern" };

		for (int i = 0; i < numclientsidedentities; ++i)
		{
			CLIENTSIDEENTITY* client_side_ent = &client_side_ent_table[i];
			for (int j = 0; j < ARRAYSIZE(blocked_clientsided_entities); ++j)
			{
				if (!strcmp(client_side_ent->name, blocked_clientsided_entities[j]))
				{
					client_side_ent->ClientEntityHandler = ClientEntityHandler_Dummy;
					break;
				}
			}
		}

		PopProtection();
	}
}

void Fix_GameDLL_Bugs()
{
	static bool already_fixed = false;
	if (already_fixed)
		return;

	g_gameDllHinst = (DWORD)GetModuleHandleA("game.dll");
	if (!g_gameDllHinst)
		return;

	// prevent GiveNamedItem for testing purposes
#if 0
	DWORD adr = FindMemoryPattern((DWORD)g_gameDllHinst, "8B 44 24 04 56 57 8B F9", false);
	if (adr)
	{
		PushProtection(adr, 4);
		*(DWORD*)(adr) = 0x900004C2;
		PopProtection();
	}
#endif

	already_fixed = true;

	Force_ServerSide_Entities_GameDLL();
	Fix_AI_TurnSpeed();
}

#include <studio.h>
#include <StudioModelRenderer.h>

mstudioanim_t* (__fastcall* g_oStudioGetAnim)(CStudioModelRenderer*, void*, model_t*, mstudioseqdesc_t*);
mstudioanim_t* __fastcall CStudioModelRenderer_StudioGetAnim(CStudioModelRenderer* me, void* edx, model_t* m_pSubModel, mstudioseqdesc_t* pseqdesc)
{
	auto model = &me->m_pCurrentEntity->model;
	mstudioseqgroup_t* pseqgroup;
	cache_user_t* paSequences;
	auto localplayer = g_pCL_EngineFuncs->GetLocalPlayer();
	studiohdr_t* studiomodel = (studiohdr_t*)localplayer->model->cache;

	if (localplayer)
		g_pCL_EngineFuncs->Con_Printf("player model adr  %#010x\n", localplayer->model);
	g_pCL_EngineFuncs->Con_Printf("render model adr %#010x\n", (DWORD)model);
	g_pCL_EngineFuncs->Con_Printf("RenderModel name %s, ptr %#010x, adr %#010x\n", me->m_pRenderModel->name, me->m_pRenderModel, (DWORD)&me->m_pRenderModel);
	g_pCL_EngineFuncs->Con_Printf("StudioHeader name %s, ptr %#010x, adr %#010x\n", me->m_pStudioHeader->name, me->m_pStudioHeader, (DWORD)&me->m_pStudioHeader);
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
		if (g_pCL_EngineFuncs)
			g_pCL_EngineFuncs->Con_DPrintf("loading %s\n", pseqgroup->name);
		g_pNightfireFileSystem->COM_LoadCacheFile(pseqgroup->name, (struct cache_user_s*)&paSequences[pseqdesc->seqgroup]);
	}
	return (mstudioanim_t*)((byte*)paSequences[pseqdesc->seqgroup].data + pseqdesc->animindex);
}

DWORD invalid_sequence_jmpback;
DWORD valid_sequence_jmpback;
bool StudioSequenceGetAnimPosOutOfBoundsCheck(CStudioModelRenderer* renderer, model_t* model)
{
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

		invalid_seq:
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

		invalid:
		pop eax //1
		jmp invalid_gaitsequence_jmpback
	}
}


//studioapi_SetupPlayerModel
model_s* R_StudioSetupPlayerModel(int playerindex)
{
	player_info_s* info = ENG_GetPlayerInfo(playerindex);
	static ConsoleVariable* developer = g_pEngineFuncs->pfnGetConsoleVariableGame("developer");
	DM_PlayerState_s* state = (DM_PlayerState_s*)DM_PlayerState;
	cl_entity_s* current_entity = (*g_rCurrentEntity);

	if (*g_rCurrentEntity)
	{
		if ((developer->getValueInt() || !Host_IsSinglePlayerGame()) && info->name[0])
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
					state->model = g_pCL_EngineFuncs->Mod_ForName(state->modelpath, false, true);
				else
				{
					strncpy(state->modelpath, "models/", 260);
					strncat(state->modelpath, info->model, 260);
					strncat(state->modelpath, ".mdl", 260);
					state->modelpath[MAX_PATH - 1] = 0;
					if (g_pNightfireFileSystem->COM_FileExists(state->modelpath, nullptr))
						state->model = g_pCL_EngineFuncs->Mod_ForName(state->modelpath, false, true);
					else
					{
						strncpy(state->modelpath, "models/water/", 260);
						strncat(state->modelpath, info->model, 260);
						state->modelpath[MAX_PATH - 1] = 0;
						strncat(state->modelpath, ".mdl", 260);
						state->modelpath[MAX_PATH - 1] = 0;
						if (g_pNightfireFileSystem->COM_FileExists(state->modelpath, nullptr))
							state->model = g_pCL_EngineFuncs->Mod_ForName(state->modelpath, false, true);
						else
						{
							strncpy(state->modelpath, "models/sky/", 260);
							strncat(state->modelpath, info->model, 260);
							state->modelpath[MAX_PATH - 1] = 0;
							strncat(state->modelpath, ".mdl", 260);
							state->modelpath[MAX_PATH - 1] = 0;
							if (g_pNightfireFileSystem->COM_FileExists(state->modelpath, nullptr))
								state->model = g_pCL_EngineFuncs->Mod_ForName(state->modelpath, false, true);
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

				R_ChangeMPSkin();
			}
		}
		else
		{
			state->modelname[0] = 0;
			if (state->model != current_entity->model)
			{
				state->model = current_entity->model;
				R_ChangeMPSkin();
			}
		}
	}
	return state->model;
}

void Fix_Model_Crash()
{
	DWORD adr = FindMemoryPattern((DWORD)*g_clientDllHinst, "56 8B 74 24 0C 8B 86 ? 00 00 00 85 C0 57 75 19", false);
	if (!adr)
		return;
	//if (!HookFunctionWithMinHook((void*)adr, CStudioModelRenderer_StudioGetAnim, (void**)&g_oStudioGetAnim))
	//	return;
	adr = FindMemoryPattern(g_engineDllHinst, "A1 ? ? ? ? 55 8B 6C 24 08 57 8B FD 69 FF", false);
	if (!HookFunctionWithMinHook((void*)adr, &R_StudioSetupPlayerModel, (void**)&g_oR_StudioSetupPlayerModel))
		return;
	adr = FindMemoryPattern(*g_clientDllHinst, "8B 80 ? ? ? ? 85 C0 0F ? ? ? ? ? 8B ? ? ? ? ? 69 ? ? ? ? ? 03", false);
	if (adr)
	{
		invalid_gaitsequence_jmpback = adr + 0xDC;
		valid_gaitsequence_jmpback = adr + 6;
		PlaceJMP((BYTE*)adr, (DWORD)&StudioSetupBones_Hook, 5);
	}
	adr = FindMemoryPattern(*g_clientDllHinst, "50 FF 15 ? ? ? ? 89 46 ? 88 5E", false);
	if (adr)
	{
		valid_sequence_jmpback = adr;
		invalid_sequence_jmpback = adr + 0x1E3;
		PlaceJMP((BYTE*)(adr - 6), (DWORD)&StudioGetAnimPos_Hook, 5);
	}
}

void Fix_ClientDLL_Bugs()
{
	GetImportantEngineOffsets();
	Fix_Model_Crash();
	Fix_Water_Hull();
	Fix_RainDrop_WaterCollision();
	Force_ServerSide_Entities_ClientDLL();
}

#define MAKEID(d,c,b,a)					( ((int)(a) << 24) | ((int)(b) << 16) | ((int)(c) << 8) | ((int)(d)) )
void(*g_oNetchan_ClearFragments)(netchan_s*);
void(*g_oNetchan_CreateFragments)(BOOL, netchan_s*, sizebuf_s*);
void(*g_oNetchan_CreateFragments_)(BOOL, netchan_s*, sizebuf_s*);
void (*g_oNetchan_Setup)(int, netchan5*, char, int, int, int, int, int, int, int);
void(*g_oNetchan_FragSend)(netchan_s* chan);
void(*g_oNetchan_Clear)(netchan6* a1);
qboolean (*g_oNetchan_Validate)(netchan_s* chan, qboolean* frag_message, unsigned int* fragid, int* frag_offset, int* frag_length);
DWORD g_Netchan_Transit_FragSend_Retadr;
DWORD g_NetchanCreateFragments_Transmit_Retadr;
void(*SZ_Clear)(sizebuf_s*);
void(*SZ_Write)(sizebuf_t* buf, const void* data, int length);
sizebuf_s* g_net_message;
void(*MSG_BeginReading)();
int* msg_readcount;
DWORD* host_client;
DWORD* svs_clients;
double* realtime;
netchan_s* cls_netchan;
byte g_ExtendedMessageBuffer[33][NET_MAX_PAYLOAD];

void Netchan_AddFragbufToTail(fragbufwaiting_t* wait, fragbuf_t* buf)
{
	fragbuf_t* p;

	buf->next = nullptr;
	wait->fragbufcount++;

	p = wait->fragbufs;
	if (p)
	{
		while (p->next)
		{
			p = p->next;
		}
		p->next = buf;
	}
	else
	{
		wait->fragbufs = buf;
	}
}

fragbuf_t* Netchan_AllocFragbuf(void)
{
	fragbuf_t* buf;

	buf = (fragbuf_t*)g_pNightfirePlatformFuncs->mallocx(sizeof(fragbuf_t));
	memset(buf, 0, sizeof(fragbuf_t));
	buf->bufferid = 0;
	buf->frag_message.cursize = 0;
	buf->frag_message.data = buf->frag_message_buf;
	buf->frag_message.maxsize = sizeof(buf->frag_message_buf);
	buf->frag_message.buffername = "Frag Buffer Alloc'd";
	buf->next = nullptr;

	return buf;
}

void Netchan_CreateFragments_(BOOL server, netchan_s* chan, sizebuf_s* msg)
{
#ifndef HOOK_NETCHAN_CREATEFRAGMENTS_
	return g_oNetchan_CreateFragments_(server, chan, msg);
#else
	netchan2* nchan = (netchan2*)chan;
	fragbuf_t* buf;
	int chunksize;
	int send;
	int remaining;
	int pos;
	int bufferid = 1;
	fragbufwaiting_t* wait, * p;

	if (msg->cursize == 0)
		return;

#ifdef COMPRESS_PACKETS
	// Compress if not done already
	if (*(uint32*)msg->data != MAKEID('B', 'Z', '2', '\0'))
	{
		unsigned char compressed[65536];
		char hdr[4] = "BZ2";
		unsigned int compressedSize = msg->cursize - sizeof(hdr);	// we should fit in same data buffer minus 4 bytes for a header
		if (!BZ2_bzBuffToBuffCompress((char*)compressed, &compressedSize, (char*)msg->data, msg->cursize, 9, 0, 30))
		{
			//Con_DPrintf("Compressing split packet (%d -> %d bytes)\n", msg->cursize, compressedSize);
			memcpy(msg->data, hdr, sizeof(hdr));
			memcpy(msg->data + sizeof(hdr), compressed, compressedSize);
			msg->cursize = compressedSize + sizeof(hdr);
		}
	}
#endif

	chunksize = nchan->pfnNetchan_Blocksize(nchan->connection_status);

	wait = (fragbufwaiting_t*)g_pNightfirePlatformFuncs->mallocx(sizeof(fragbufwaiting_t));
	memset(wait, 0, sizeof(fragbufwaiting_t));

	remaining = msg->cursize;
	pos = 0;
	while (remaining > 0)
	{
		send = min(remaining, chunksize);
		remaining -= send;

		buf = Netchan_AllocFragbuf();
		if (!buf)
		{
			return;
		}

		buf->bufferid = bufferid++;

		// Copy in data
		SZ_Clear(&buf->frag_message);
		SZ_Write(&buf->frag_message, &msg->data[pos], send);
		pos += send;

		Netchan_AddFragbufToTail(wait, buf);
	}

	// Now add waiting list item to the end of buffer queue
	if (!nchan->waitlist[FRAG_NORMAL_STREAM])
	{
		nchan->waitlist[FRAG_NORMAL_STREAM] = wait;
	}
	else
	{
		p = nchan->waitlist[FRAG_NORMAL_STREAM];
		while (p->next)
		{
			p = p->next;
		}
		p->next = wait;
	}
#endif
}

void Hooked_Netchan_CreateFragments(BOOL server, netchan_s* chan, sizebuf_s* msg)
{
#ifndef HOOK_NETCHAN_CREATEFRAGMENTS
	return g_oNetchan_CreateFragments(server, chan, msg);
#else
	DWORD retadr = (DWORD)_ReturnAddress();
	if (retadr == g_NetchanCreateFragments_Transmit_Retadr)
	{
		Netchan_CreateFragments_(server, chan, msg);
		SZ_Clear(msg);
		return;
	}

	netchan_temp* nchan = (netchan_temp*)chan;
	// Always queue any pending reliable data ahead of the fragmentation buffer
	if (nchan->message.cursize > 0)
	{
		Netchan_CreateFragments_(server, chan, &nchan->message);
		nchan->message.cursize = 0;
	}

	Netchan_CreateFragments_(server, chan, msg);
#endif
}

BOOL(*g_oNetchan_CopyNormalFragments)(netchan_s*);
BOOL Netchan_CopyNormalFragments(netchan_s* chan)
{
#ifndef HOOK_NETCHAN_COPYNORMALFRAGMENTS
	return g_oNetchan_CopyNormalFragments(chan);
#else
	netchan_temp* nchan = (netchan_temp*)chan;
	fragbuf_t* p, *n;

	DWORD adr = (DWORD)&nchan->incomingready - (DWORD)nchan;
	DWORD adr2 = (DWORD)&nchan->incomingbufs - (DWORD)nchan;

	if (!nchan->incomingready[FRAG_NORMAL_STREAM])
		return FALSE;

	if (!nchan->incomingbufs[FRAG_NORMAL_STREAM])
	{
		//Con_Printf("%s:  Called with no fragments readied\n", __func__);
		nchan->incomingready[FRAG_NORMAL_STREAM] = FALSE;
		return FALSE;
	}

	p = nchan->incomingbufs[FRAG_NORMAL_STREAM];

	SZ_Clear(g_net_message);
	MSG_BeginReading();

	bool overflowed = false;

	while (p)
	{
		n = p->next;

		if (g_net_message->cursize + p->frag_message.cursize <= g_net_message->maxsize)
			SZ_Write(g_net_message, p->frag_message.data, p->frag_message.cursize);
		else
			overflowed = true;

		g_pNightfirePlatformFuncs->freex(p);
		p = n;
	}

	if (overflowed)
	{
#if 0
		if (chan->player_slot == 0)
			Con_Printf("%s: Incoming overflowed\n", __func__);
		else
			Con_Printf("%s: Incoming overflowed from %s\n", __func__, g_psvs.clients[chan->player_slot - 1].name);
#endif
		SZ_Clear(g_net_message);

		nchan->incomingbufs[FRAG_NORMAL_STREAM] = nullptr;
		nchan->incomingready[FRAG_NORMAL_STREAM] = FALSE;

		return FALSE;
	}

#ifdef COMPRESS_PACKETS
	if (*(uint32*)g_net_message->data == MAKEID('B', 'Z', '2', '\0'))
	{
		char uncompressed[65536];
		unsigned int uncompressedSize = 65536;
		BZ2_bzBuffToBuffDecompress(uncompressed, &uncompressedSize, (char*)g_net_message->data + 4, g_net_message->cursize - 4, 1, 0);
		memcpy(g_net_message->data, uncompressed, uncompressedSize);
		g_net_message->cursize = uncompressedSize;
	}
#endif

	nchan->incomingbufs[FRAG_NORMAL_STREAM] = nullptr;
	nchan->incomingready[FRAG_NORMAL_STREAM] = FALSE;
#endif

	return TRUE;
}

void Netchan_ClearFragbufs(fragbuf_t** ppbuf)
{
	fragbuf_t* buf, * n;

	if (!ppbuf)
	{
		return;
	}

	// Throw away any that are sitting around
	buf = *ppbuf;
	while (buf)
	{
		n = buf->next;
		g_pNightfirePlatformFuncs->freex(buf);
		buf = n;
	}
	*ppbuf = nullptr;
}

void Netchan_FlushIncoming(netchan_s* chan, int stream)
{
	netchan_temp* nchan = (netchan_temp*)chan;
	netchan5* nchan5 = (netchan5*)chan;
	fragbuf_t* p, * n;

	int current_server_client = *host_client - *svs_clients;

	if ((nchan5->player_slot - 1) == *host_client - *svs_clients)
	{
		SZ_Clear(g_net_message);
		*msg_readcount = 0;
	}

	p = nchan->incomingbufs[stream];
	while (p)
	{
		n = p->next;
		g_pNightfirePlatformFuncs->freex(p);
		p = n;
	}

	nchan->incomingbufs[stream] = nullptr;
	nchan->incomingready[stream] = FALSE;
}

void Netchan_ClearFragments(netchan_s* chan)
{
#ifndef HOOK_NETCHAN_CLEAR_FRAGMENTS
	g_oNetchan_ClearFragments(chan);
	return;
#else
	netchan_temp* nchan = (netchan_temp*)chan;
	netchan2* nchan2 = (netchan2*)chan;
	fragbufwaiting_t* wait, * next;
	for (int i = 0; i < MAX_STREAMS; i++)
	{
		wait = nchan2->waitlist[i];
		while (wait)
		{
			next = wait->next;
			Netchan_ClearFragbufs(&wait->fragbufs);
			g_pNightfirePlatformFuncs->freex(wait);
			wait = next;
		}
		nchan2->waitlist[i] = nullptr;

		Netchan_ClearFragbufs(&nchan->fragbufs[i]);
		Netchan_FlushIncoming(chan, i);
	}
#endif
}

void Netchan_Clear(netchan6* a1)
{
#ifndef HOOK_NETCHAN_CLEAR
	g_oNetchan_Clear(a1);
#else
	int v1; // ecx
	WORD* p_word1FD0; // edx
	DWORD* v3; // eax

	netchan7* nchan = (netchan7*)a1;

	Netchan_ClearFragments((netchan_s*)a1);
	a1->cleartime() = 0.0;

	if (a1->reliable_length())
	{
		//Con_DPrintf("%s: reliable length not 0, reliable_sequence: %d, incoming_reliable_acknowledged: %d\n", __func__, chan->reliable_length, chan->incoming_reliable_acknowledged);
		nchan->reliable_sequence() ^= 1;
		a1->reliable_length() = 0;
	}
	
	v1 = 0;
	p_word1FD0 = &a1->word1FD0;
	v3 = (DWORD*)&a1->netchan40.gap0[8108];
	
	for (int i = 0; i < MAX_STREAMS; ++i)
	{
		v3[2] = 0;
		*v3 = 0;
		v3[6] = 0;
		*(p_word1FD0 - 2) = 0;
		*p_word1FD0 = 0;
		a1->gap1FD2[v1++ + 10] = 0;
		++v3;
		++p_word1FD0;
	}

	if (a1->tempbuffer)
	{
		g_pNightfirePlatformFuncs->freex(a1->tempbuffer);
		a1->tempbuffer = 0;
	}
#endif
}

void Netchan_Setup(int a1, netchan5* a2, char a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
{
#ifndef HOOK_NETCHAN_SETUP
	g_oNetchan_Setup(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
#else
	int v10; // eax
	int v11; // ecx
	int result; // eax

	netchan_temp* nchan = (netchan_temp*)a2;
	Netchan_Clear((netchan6*)a2);
	memset(a2, 0, 9496);
	v10 = a8;
	*&a2->char0 = a1;
	a2->player_slot = v10 + 1;
	memcpy(&a2->char4, &a3, 0x14u);
	a2->last_received = *realtime;
	a2->connect_time = *realtime;
	v11 = a10;
	nchan->message.cursize = 0;
	a2->word60 = 1;
	a2->dword48 = 1;
	result = a9;
	if (a2->player_slot != -1)
	{
		nchan->message.data = g_ExtendedMessageBuffer[a2->player_slot];
		nchan->message.maxsize = NET_MAX_PAYLOAD;
	}
	else
	{
		a2->message_data = (DWORD)(a2 + 1);
		a2->message_maxsize = 3990;
	}
	a2->message_buffername = (DWORD)"netchan->message";
	// nightfire fix, use cl_rate value:
	static ConsoleVariable* cl_rate = g_pEngineFuncs->pfnGetConsoleVariableGame("cl_rate");
	double rate = cl_rate ? max(min(cl_rate->getValueFloat(), 1000), MAX_RATE) : DEFAULT_RATE;
	a2->rate = rate;
	a2->connection_status = result;
	a2->pfnNetchan_Blocksize = v11;
#endif
}

void Netchan_FragSend(netchan_s* chan)
{
#ifndef HOOK_NETCHAN_FRAGSEND
	g_oNetchan_FragSend(chan);
#else
	if ((DWORD)_ReturnAddress() == g_Netchan_Transit_FragSend_Retadr)
	{
		netchan_temp* nchan = (netchan_temp*)chan;
		if (nchan->message.cursize > MAX_MSGLEN)
		{
			Netchan_CreateFragments_(chan == cls_netchan ? 1 : 0, chan, &nchan->message);
			SZ_Clear(&nchan->message);
		}
	}
	g_oNetchan_FragSend(chan);
#endif
}

DWORD Netchan_CopyTo_Reliable_Buf_Jmpback;
__declspec(naked) void Netchan_CopyTo_Reliable_Buf()
{
	__asm
	{
		mov ecx, [ebp + 0x64];
		push ecx
		lea edx, [ebp + 0x100C];
		jmp Netchan_CopyTo_Reliable_Buf_Jmpback
	}
}

qboolean __stdcall Netchan_Validate(int* frag_length, qboolean* frag_message, netchan_s* chan, int* frag_offset, unsigned int* fragid)
{
	for (int i = 0; i < MAX_STREAMS; i++)
	{
		if (!frag_message[i])
			continue;

		// total fragments should be <= MAX_FRAGMENTS and current fragment can't be > total fragments
		if (i == FRAG_NORMAL_STREAM && FRAG_GETCOUNT(fragid[i]) > MAX_NORMAL_FRAGMENTS)
			return FALSE;
		if (i == FRAG_FILE_STREAM && FRAG_GETCOUNT(fragid[i]) > MAX_FILE_FRAGMENTS)
			return FALSE;
		if (FRAG_GETID(fragid[i]) > FRAG_GETCOUNT(fragid[i]))
			return FALSE;
		if (!frag_length[i])
			return FALSE;
		if ((size_t)frag_length[i] > FRAGMENT_MAX_SIZE || (size_t)frag_offset[i] > NET_MAX_PAYLOAD - 1)
			return FALSE;

		int frag_end = frag_offset[i] + frag_length[i];

		// end of fragment is out of the packet
		if (frag_end + *msg_readcount > g_net_message->cursize)
			return FALSE;

		// fragment overlaps next stream's fragment or placed after it
		for (int j = i + 1; j < MAX_STREAMS; j++)
		{
			if (frag_end > frag_offset[j] && frag_message[j]) // don't add msg_readcount for comparison
				return FALSE;
		}
	}

	return TRUE;
}

__declspec(naked) qboolean Netchan_Validate_Hook(netchan_s* chan, qboolean* frag_message, int* frag_length)
{
	__asm
	{
		push ebp //+8
		mov ebp, [esp]
		push ecx //+12 fragid
		push eax //+16 frag_offset
		mov ecx, [esp + 16] //chan
		mov eax, [esp + 20] //frag_message
		push ecx //+28 
		push eax //+32
		mov eax, [esp + 32] //fraglength
		push eax
		call Netchan_Validate	
		pop ebp
		retn
	}
}

void Fix_Netchan()
{
	DWORD adr = FindMemoryPattern(g_engineDllHinst, "56 8B 74 24 0C 8B 46 6C 85 C0 57 8B 7C 24 0C 7E 15 8D 46 5C", false);
	if (!adr)
		return;
	DWORD adr2 = FindMemoryPattern(g_engineDllHinst, "83 EC 10 57 8B 7C 24 20 8B 47 10 85 C0 C7 44 24 10 01 00 00 00", false);
	if (!adr2)
		return;
	DWORD adr3 = FindMemoryPattern(g_engineDllHinst, "53 8B 5C 24 08 8A 83 DC 1F 00 00 84 C0 75 04 32 C0 5B C3", false);
	if (!adr3)
		return;
	DWORD adr4 = FindMemoryPattern(g_engineDllHinst, "83 EC 08 53 55 8B 2D ? ? ? ? 56 8B 74 24 18", false);
	if (!adr4)
		return;
	msg_readcount = (int*)FindMemoryPattern(g_engineDllHinst, "8B 15 ? ? ? ? 50 4A 52");
	if (!msg_readcount)
		return;
	msg_readcount = (int*)*(DWORD*)((DWORD)msg_readcount + 2);
	g_net_message = *(sizebuf_s**)(adr3 + 0x37);
	DWORD adr5 = FindMemoryPattern(g_engineDllHinst, "8B 35 ? ? ? ? 85 F6 74 3A A1", false);
	if (!adr5)
		return;
	DWORD adr6 = FindMemoryPattern(g_engineDllHinst, "53 8B 5C 24 0C 56 57 53", false);
	if (!adr6)
		return;
	DWORD Netchan_Transmit = FindMemoryPattern(g_engineDllHinst, "B8 08 10 00 00 E8", false);
	if (!Netchan_Transmit)
		return;
	DWORD adr7 = FindMemoryPattern(g_engineDllHinst, "8B 44 24 04 85 C0 74 47 53 56 57", false);
	if (!adr7)
		return;
	g_Netchan_Transit_FragSend_Retadr = FindMemoryPattern(g_engineDllHinst, "8B 55 6C 33 C0 83 C4 04 3B D3 0F 95 C0", false);
	DWORD adr8 = FindMemoryPattern(g_engineDllHinst, "81 FD ? ? ? ? 0F 95 C2 8D 4D 5C", false);
	if (!adr8)
		return;
	DWORD adr9 = FindMemoryPattern(g_engineDllHinst, "53 55 8B 6C 24 14 56 8B F1 8B DD 57", false);
	if (!adr9)
		return;
	DWORD adr10 = FindMemoryPattern(g_engineDllHinst, "53 56 8B 74 24 0C 56 E8 ? ? ? ? 33 DB 83 C4 04", false);
	if (!adr10)
		return;

	realtime = *(double**)(adr6 + 0x3C);
	svs_clients = (DWORD*)*(DWORD*)(adr5 + 2);
	cls_netchan = (netchan_s*)*(DWORD*)(adr8 + 2);

	SZ_Clear = (void(*)(sizebuf_s*))FindMemoryPattern(g_engineDllHinst, "8B 44 24 04 80 60 04 FD C7 40 10 00 00 00 00 C3", false);
	if (!SZ_Clear)
		return;
	SZ_Write = (void(*)(sizebuf_t*, const void*, int))FindMemoryPattern(g_engineDllHinst, "8B 44 24 0C 8B 4C 24 08 8B 54 24 04 50 51 50 52", false);
	if (!SZ_Write)
		return;
	MSG_BeginReading = (void(*)())FindMemoryPattern(g_engineDllHinst, "33 C0 A3 ? ? ? ? A2 ? ? ? ? C3", false);
	if (!MSG_BeginReading)
		return;
	g_NetchanCreateFragments_Transmit_Retadr = FindMemoryPattern(g_engineDllHinst, "83 C4 0C 89 5D 6C 8D BD D0 1F 00 00 33 C0 8B CF", false);
	if (!g_NetchanCreateFragments_Transmit_Retadr)
		return;
	host_client = (DWORD*)FindMemoryPattern(g_engineDllHinst, "A3 ? ? ? ? 7E 39 8B 7C 24 0C 80 38 00", false);
	if (!host_client)
		return;
	host_client = (DWORD*)(*(DWORD*)((DWORD)host_client + 1));
#ifdef HOOK_NETCHAN_CLEAR_FRAGMENTS
	if (!HookFunctionWithMinHook((void*)adr4, Netchan_ClearFragments, (void**)&g_oNetchan_ClearFragments))
		return;
#else
	g_oNetchan_ClearFragments = (void(*)(netchan_s*))adr4;
#endif
#ifdef HOOK_NETCHAN_CREATEFRAGMENTS
	if (!HookFunctionWithMinHook((void*)adr, Hooked_Netchan_CreateFragments, (void**)&g_oNetchan_CreateFragments))
		return;
#else
	g_oNetchan_CreateFragments = (void (*)(BOOL, netchan_s *, sizebuf_s *))adr;
#endif
#ifdef HOOK_NETCHAN_CREATEFRAGMENTS_
	if (!HookFunctionWithMinHook((void*)adr2, Netchan_CreateFragments_, (void**)&g_oNetchan_CreateFragments_))
		return;
#else
	g_oNetchan_CreateFragments_ = (void (*)(BOOL, netchan_s *, sizebuf_s *))adr2;
#endif
#ifdef HOOK_NETCHAN_COPYNORMALFRAGMENTS
	if (!HookFunctionWithMinHook((void*)adr3, Netchan_CopyNormalFragments, (void**)&g_oNetchan_CopyNormalFragments))
		return;
#else
	g_oNetchan_CopyNormalFragments = (BOOL(*)(netchan_s*))adr3;
#endif
#ifdef HOOK_NETCHAN_SETUP
	if (!HookFunctionWithMinHook((void*)adr6, Netchan_Setup, (void**)&g_oNetchan_Setup))
		return;
#else
	g_oNetchan_Setup = (void (*)(int, netchan5 *, char, int, int, int, int, int, int, int))adr6;
#endif
#ifdef HOOK_NETCHAN_FRAGSEND
	if (!HookFunctionWithMinHook((void*)adr7, Netchan_FragSend, (void**)&g_oNetchan_FragSend))
		return;
#else
	g_oNetchan_FragSend = (void (*)(netchan_s *))adr7;
#endif
#ifdef HOOK_NETCHAN_CLEAR
	if (!HookFunctionWithMinHook((void*)adr10, Netchan_Clear, (void**)&g_oNetchan_Clear))
		return;
#else
	g_oNetchan_Clear = (void (*)(netchan6 *))adr10;
#endif

#ifdef HOOK_VALIDATE
	PlaceJMP((BYTE*)adr9, (DWORD)&Netchan_Validate_Hook, 5);
#endif

	// reduce buffer size to MAX_UDP_PACKET
	DWORD old;
	VirtualProtect((BYTE*)(Netchan_Transmit + 0x33), 4, PAGE_EXECUTE_READWRITE, &old);
	*(int*)(Netchan_Transmit + 0x33) = MAX_UDP_PACKET;
	VirtualProtect((BYTE*)(Netchan_Transmit + 0x33), 4, old, &old);

	PlaceJMP((BYTE*)(Netchan_Transmit + 0x16A), (DWORD)&Netchan_CopyTo_Reliable_Buf, 10);
	Netchan_CopyTo_Reliable_Buf_Jmpback = Netchan_Transmit + (0x16A + 10);
}

void Fix_RateLimiter()
{
	static const double maxrate = MAX_RATE;
	static const float maxrate2 = MAX_RATE;
	DWORD adr = FindMemoryPattern(g_engineDllHinst, "8D 97 D0 4C 00 00 52 FF 15 ? ? ? ? 68", false);
	if (!adr)
		return;

	// fix client sending incorrect rate command value to server
	DWORD old;
	VirtualProtect((void*)(adr + 0xE), 4, PAGE_EXECUTE_READWRITE, &old);
	*(const char**)(adr + 0xE) = "rate";
	VirtualProtect((void*)(adr + 0xE), 4, old, nullptr);

	DWORD adr2 = FindMemoryPattern(g_engineDllHinst, "68 E8 03 00 00 68 20 4E 00 00", false);
	if (!adr2)
		return;

	//fix limits
	VirtualProtect((void*)(adr2 - 256), 256, PAGE_EXECUTE_READWRITE, &old);
	*(unsigned int*)(adr + 0xCB) = MAX_RATE;
	*(unsigned int*)(adr2 + 6) = MAX_RATE;
	*(const float**)(adr2 - 0x18) = &maxrate2;
	*(const char**)(adr2 + 0xB) = "cl_rate:  Maximum %u, Minimum %u\n";
	*(float*)(adr2 - 64) = DEFAULT_RATE;

	VirtualProtect((void*)(adr2 - 256), 256, old, nullptr);
}

void Fix_FpsCap()
{
	DWORD adr = FindMemoryPattern(g_engineDllHinst, "DF E0 F6 C4 05 8D 44 24 08 7B 04 8D 44 24 00 8B 08", false);
	if (!adr)
		return;
	DWORD old;
	VirtualProtect((void*)(adr - 64), 128, PAGE_EXECUTE_READWRITE, &old);
	static const float maxfps = MAX_FPS;
	*(const float**)(adr - 12) = &maxfps; //set fps command comparison limiter
	*(float*)(adr - 4) = MAX_FPS; //set max fps to 1000
	*(float*)(adr - 0x95) = MAX_FPS; // change loading screen fps limit from 30 to 1000
	VirtualProtect((void*)(adr - 64), 256, old, nullptr);
}


float* g_NextCLCmdTime;
DWORD Set_NextCLCmdTimeCPP_Jmpback;
float Set_NextCLCmdTimeCPP(float desired_delta)
{
	const double curtime = *realtime;
	const double dt = 1.0 / desired_delta;
	double delta = curtime - *g_NextCLCmdTime;
	delta = min(max(delta, 0.0), 0.1);
	double next_time = curtime + dt - delta;
	return (float)next_time;
}
__declspec(naked) void Set_NextCLCmdTime()
{
	__asm
	{
		push ebx
		push ecx
		push[esp + 0x18]
		call Set_NextCLCmdTimeCPP
		add esp, 4
		pop ecx
		pop ebx
		jmp Set_NextCLCmdTimeCPP_Jmpback
	}
}

DWORD* g_Player;
DWORD g_NextSVCmdTimeOffset;
DWORD g_DesiredSVCmdDeltaOffset;
double* host_frametime;
DWORD Set_NextSVCmdTime_Jmpback;
void Set_NextSVCmdTimeCPP()
{
	static ConsoleVariable* fps_max = g_pEngineFuncs->pfnGetConsoleVariableGame("fps_max");
	static ConsoleVariable* sys_ticrate = g_pEngineFuncs->pfnGetConsoleVariableGame("sys_ticrate");
	double minimum_frametime = 1.0 / (g_bDedicated ? (double)sys_ticrate->getValueFloat() : (double)fps_max->getValueFloat());
	minimum_frametime = min(max(minimum_frametime, 1.0 / MAX_FPS), 1.0 / 0.5);
		
	DWORD player = *g_Player;
	const double curtime = *realtime;
	const double desired_delta = *(double*)(player + g_DesiredSVCmdDeltaOffset);
	const double next_cmdtime = *(double*)(player + g_NextSVCmdTimeOffset);
	double delta = curtime - next_cmdtime;
	delta = min(max(delta, 0.0), minimum_frametime);
	double next_time = curtime + desired_delta - delta;
	//nightfire code below:
	//next_time = curtime + *host_frametime + desired_delta;

	*(double*)(player + g_NextSVCmdTimeOffset) = next_time;
}

__declspec(naked) void Set_NextSVCmdTime()
{
	__asm
	{
		push ebx
		call Set_NextSVCmdTimeCPP
		pop ebx
		jmp Set_NextSVCmdTime_Jmpback
	}
}

void Fix_RateDesync()
{
	DWORD adr = FindMemoryPattern(g_engineDllHinst, "DC 05 ? ? ? ? EB 06 DD 05 ? ? ? ? A1 ? ? ? ?", false);
	if (!adr)
		return;
	g_NextCLCmdTime = *(float**)(adr + 0x15);
	PlaceJMP((BYTE*)(adr - 10), (DWORD)&Set_NextCLCmdTime, 5);
	Set_NextCLCmdTimeCPP_Jmpback = adr + 0x13;

	DWORD adr2 = FindMemoryPattern(g_engineDllHinst, "DD 05 ? ? ? ? A1 ? ? ? ? DC 80 ? ? 00 00 DC 05 ? ? ? ? DD 98 ? ? 00 00", false);
	if (!adr2)
		return;
	DWORD adr3 = FindMemoryPattern(g_engineDllHinst, "DD 05 ? ? ? ? D9 5C 24 0C D9 44 24 0C D8 0F", false);
	if (!adr3)
		return;
	g_Player = *(DWORD**)(adr2 + 7);
	g_NextSVCmdTimeOffset = *(DWORD*)(adr2 + 0x19);
	g_DesiredSVCmdDeltaOffset = *(DWORD*)(adr2 + 0xD);
	host_frametime = *(double**)(adr3 + 2);
	PlaceJMP((BYTE*)adr2, (DWORD)&Set_NextSVCmdTime, 29);
	Set_NextSVCmdTime_Jmpback = adr2 + 0x17;
}

void Fix_UserInfoString()
{
	DWORD adr = FindMemoryPattern(g_engineDllHinst, "A0 ? ? ? ? 84 C0 0F 85 ? ? ? ? 68 00 01 00 00", false);
	if (!adr)
		return;

	DWORD old;
	VirtualProtect((void*)adr, 256, PAGE_EXECUTE_READWRITE, &old);

	*(const char**)(adr + 0x2C) = "6.00 insecure";
	*(const char**)(adr + 0x31) = "gamever";
}

void(*g_oAlertMessage)(int, char*, ...);
void AlertMessage(int a1, char* Format, ...)
{
	static char szOut[1024] = {0};
	int v2; // eax
	va_list ArgList; // [esp+10h] [ebp+Ch] BYREF
	static ConsoleVariable* dev = nullptr;
	if (!dev) //hack
		g_pEngineFuncs->pfnGetConsoleVariableGame("developer");

	va_start(ArgList, Format);
	if (a1 == 5 && *svs_maxclients > 1)
	{
		vsnprintf(szOut, 1024, Format, ArgList);
		szOut[1023] = 0;
		g_oLog_Printf("%s", szOut);
	}
	else if (dev && dev->getValueInt())
	{
		switch (a1)
		{
		case 0:
			strncpy_s(szOut, "NOTE:  ", 1024);
			szOut[1023] = 0;
			break;
		case 1:
			szOut[0] = 0;
			break;
		case 2:
			if (dev->getValueInt() < 2)
				return;
			szOut[0] = 0;
			break;
		case 3:
			strncpy_s(szOut, "WARNING:  ", 1024);
			szOut[1023] = 0;
			break;
		case 4:
			strncpy_s(szOut, "ERROR:  ", 1024);
			szOut[1023] = 0;
			break;
		default:
			break;
		}
		v2 = strlen(szOut);
		vsnprintf(&szOut[v2], 1024 - v2, Format, ArgList); // this is the crash fix..., use vsnprintf
		szOut[1023] = 0;
		g_pCL_EngineFuncs->Con_Printf("%s", szOut);
	}
}

void Fix_AlertMessage_Crash()
{
	DWORD adr = FindMemoryPattern(g_engineDllHinst, "56 8B 74 24 08 83 FE 05 75 37", false);
	if (!adr)
		return;

	GetImportantEngineOffsets();

	if (!HookFunctionWithMinHook((void*)adr, (void*)&AlertMessage, (void**)&g_oAlertMessage))
		return;
}

void(*g_oCon_DPrintf)(const char*, ...);
bool* g_fIsDebugPrint;
void Con_DPrintf(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[4096];
	static ConsoleVariable* dev = g_pEngineFuncs->pfnGetConsoleVariableGame("developer");
	if (dev && dev->getValueInt())
	{
		va_start(argptr, fmt);
		vsnprintf(msg, sizeof(msg), fmt, argptr);
		va_end(argptr);

		msg[sizeof(msg) - 1] = 0;

		*g_fIsDebugPrint = true;
		g_pCL_EngineFuncs->Con_Printf(msg);
		*g_fIsDebugPrint = false;
	}
}

void Fix_Con_DPrintf_StackSmash_Crash()
{
	GetImportantEngineOffsets();
	if (!g_pCL_EngineFuncs->Con_DPrintf)
		return;

	g_fIsDebugPrint = *(bool**)((DWORD)g_pCL_EngineFuncs->Con_DPrintf + 0x44);
	
	if (!HookFunctionWithMinHook((void*)g_pCL_EngineFuncs->Con_DPrintf, (void*)&Con_DPrintf, (void**)&g_oAlertMessage))
		return;
}

void LoadThisDll_Post(const char* gamedll)
{
	g_gameDllHinst = (long)GetModuleHandleA("game.dll");
	if (!g_gameDllHinst)
		return;
	
	Fix_GameDLL_Bugs();
}

DWORD g_oLoadThisDll;
__declspec(naked) void LoadThisDll_Hook()
{
	__asm
	{
		push ebx
		call g_oLoadThisDll
		pop ebx
		push ebx
		call LoadThisDll_Post
		add esp, 4
		retn
	}
}

void Hook_GameDLLLoadLibrary()
{
	DWORD adr = FindMemoryPattern(g_engineDllHinst, "57 53 FF 15 ? ? ? ? 8B F8 85 FF 75 17", false);
	if (!adr)
		return;

	if (!HookFunctionWithMinHook((void*)adr, (void*)&LoadThisDll_Hook, (void**)&g_oLoadThisDll))
		return;
}

void Fix_Engine_Bugs()
{
	Hook_GameDLLLoadLibrary();
	Fix_Sound_Overflow();
	Fix_Netchan();
	Fix_AlertMessage_Crash();
	Fix_Con_DPrintf_StackSmash_Crash();
	Fix_RateLimiter();
	Fix_RateDesync();
	Fix_FpsCap();
	Fix_UserInfoString();
}

void Fix_Gamespy()
{
	DWORD adr = FindMemoryPattern(g_engineDllHinst, "6D 61 73 74 65 72 2E 67 61 6D 65 73 70 79 2E 63 6F 6D 00", false);
	while (adr)
	{
		DWORD old, old2;
		VirtualProtect((void*)adr, 19, PAGE_READWRITE, &old);
		strncpy((char*)adr, "master.openspy.net", 19);
		VirtualProtect((void*)adr, 19, old, &old2);
		adr = FindMemoryPattern(g_engineDllHinst, "6D 61 73 74 65 72 2E 67 61 6D 65 73 70 79 2E 63 6F 6D 00", false);
	}
}

// apply nullptr check from mac version
void Fix_GUI_GetAction_Crash()
{
	if (!g_guiDllHinst)
		return;

	DWORD adr = FindMemoryPattern(g_guiDllHinst, "8B 10 53 8B C8 FF 52 08", false);
	if (!adr)
		return;

	PlaceJMP((BYTE*)adr, (DWORD)&GUI_GetAction_Return, 5);
	GUI_GetAction_JmpBack = adr + 8;
}

void Fix_HighFPSBugs()
{
	usercmd_t test;
}