#pragma once

#include <Windows.h>
#include <vector_types.h>
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <usercmd.h>
#include <server.h>

struct model_s;
struct player_info_s;
struct server_t;
struct cl_entity_s;
struct cl_enginefuncs_s;
struct sizebuf_s;

class nf_pointers
{
public:
	//engine.dll
	model_s* (*R_StudioSetupPlayerModel)(int playerindex) {};
	player_info_s* (*ENG_GetPlayerInfo)(int index) {};
	void(*EV_PlayerTrace) (float* start, float* end, int brushFlags, int traceFlags, int ignore_pe, struct pmtrace_s* tr) {};
	void(*EV_SetTraceHull)(int hull) {};
	void(*Log_Printf)(const char*, ...) {};
	BOOL(*Host_IsSinglePlayerGame)() {};
	void(*R_ChangeMPSkin)() {};

	double* realtime{};
	int* msg_readcount{};
	sizebuf_s* g_net_message{};
	netchan_s* cls_netchan{};
	struct server_static_s **psvs{};						// &svs
	//struct client_static_s *pcls{};						// &cls
	server_t** g_psv{};
	int* svs_maxclients{};
	cl_entity_s** g_rCurrentEntity{};
	struct playermove_s** g_PlayerMove{};
	cl_enginefuncs_s* g_pCL_EngineFuncs{};
	char* DM_PlayerState{};

	client_s** host_client{};

	void(*SZ_Clear)(sizebuf_s*) {};
	void(*SZ_Write)(sizebuf_s* buf, const void* data, int length) {};
	void(*MSG_BeginReading)() {};

	//netchan
	void(*Netchan_CreateFragments)(BOOL, netchan_s*, sizebuf_s*) {};
	void(*Netchan_ClearFragments)(netchan_s*) {};
	void(*Netchan_CreateFragments_)(BOOL server, netchan_s* chan, sizebuf_s* msg) {};
	BOOL(*Netchan_CopyNormalFragments)(netchan_s* chan) {};
	void(*Netchan_Setup)(netsrc_t socketnumber, netchan_t* chan, netadr_t adr, int player_slot, void* connection_status, int(*pfnNetchan_Blocksize)(void*)) {};
	void (*Netchan_Clear)(netchan_s* chan) {};
	void (*Netchan_FragSend)(netchan_s* chan) {};
	qboolean(__stdcall *Netchan_Validate)(int* frag_length, qboolean* frag_message, netchan_s* chan, int* frag_offset, unsigned int* fragid) {};

	//client.dll
	bool(*UTIL_CheckForWater)(float startx, float starty, float startz, float endx, float endy, float endz, float* dest) {};
	class CStudioModelRenderer *g_pStudioAPI{};


	void GetImportantEngineOffsets(long enginedll);
	void GetImportantClientOffsets(long clientdll);
	int EV_GetTraceHull();
};

extern nf_pointers g_Pointers;