#pragma once

#include <Windows.h>

struct model_s;
struct player_info_s;
struct server_t;
struct cl_entity_s;
struct cl_enginefuncs_s;

class nf_pointers
{
public:
	model_s* (*R_StudioSetupPlayerModel)(int playerindex) {};
	player_info_s* (*ENG_GetPlayerInfo)(int index) {};
	void(*EV_PlayerTrace) (float* start, float* end, int brushFlags, int traceFlags, int ignore_pe, struct pmtrace_s* tr) {};
	void(*EV_SetTraceHull)(int hull) {};
	void(*Log_Printf)(const char*, ...) {};
	BOOL(*Host_IsSinglePlayerGame)() {};
	void(*R_ChangeMPSkin)() {};

	server_t** g_psv{};
	int* svs_maxclients{};
	cl_entity_s** g_rCurrentEntity{};
	struct playermove_s** g_PlayerMove{};
	cl_enginefuncs_s* g_pCL_EngineFuncs{};
	char* DM_PlayerState{};

	//client.dll
	bool(*UTIL_CheckForWater)(float startx, float starty, float startz, float endx, float endy, float endz, float* dest) {};
	class CStudioModelRenderer *g_pStudioAPI{};


	void GetImportantEngineOffsets(long enginedll);
	void GetImportantClientOffsets(long clientdll);
	int EV_GetTraceHull();
};

extern nf_pointers g_Pointers;