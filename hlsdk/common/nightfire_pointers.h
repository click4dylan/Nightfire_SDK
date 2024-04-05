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
	int version;
	int size;
	nf_pointers() : version(1), size(sizeof(nf_pointers)) { };

	//engine.dll
	model_s* (*R_StudioSetupPlayerModel)(int playerindex) {};
	player_info_s* (*ENG_GetPlayerInfo)(int index) {};
	void(*EV_PlayerTrace) (float* start, float* end, int brushFlags, int traceFlags, int ignore_pe, struct pmtrace_s* tr) {};
	void(*EV_SetTraceHull)(int hull) {};
	void(*Log_Printf)(const char*, ...) {};
	BOOL(*Host_IsSinglePlayerGame)() {};
	void(*R_ChangeMPSkin)() {};
	void(*Sys_Error)(const char* err, ...) {};
	void(*CL_ParseTextMessage)() {};
	void(*CL_InitTempEntModels)() {};
	struct tempent_s*(*R_DefaultSprite)(float const* origin, int spriteindex, float framerate) {};
	void(*S_StartDynamicSound)(int entnum, int entchannel, struct sfx_t* sfx, float* origin, float fvol, float attenuation, int flags, int pitch) {};
	void(*CL_ParseTEnt)() {};

	//engine.dll common.h
	void (*MSG_StartBitWriting)(sizebuf_t* buf) {};
	BOOL(*MSG_IsBitWriting)() {};
	int (*MSG_CurrentBit)() {};
	BOOL(*MSG_IsBitReading)() {};
	void (*MSG_StartBitReading)(sizebuf_t* buf) {};
	void (*MSG_EndBitReading)() {};
	int (*MSG_ReadOneBit)() {};
	unsigned int (*MSG_ReadBits)(int numbits) {};
	unsigned int (*MSG_PeekBits)(int numbits) {};
	int (*MSG_ReadSBits)(int numbits) {};
	char* (*MSG_ReadBitString)() {};
	void (*MSG_ReadBitData)(void* dest, int length) {};
	float (*MSG_ReadBitCoord)() {};
	void (*MSG_ReadBitVec3Coord)(vec3_t fa) {};
	void (*MSG_BeginReading)() {};
	int (*MSG_ReadChar)() {};
	int (*MSG_ReadByte)() {};
	int (*MSG_ReadShort)() {};
	int (*MSG_ReadLong)() {};
	float (*MSG_ReadFloat)() {};
	int (*MSG_ReadBuf)(int iSize, void* pbuf) {};
	char* (*MSG_ReadString)() {};
	char* (*MSG_ReadStringLine)() {};
	float (*MSG_ReadAngle)() {};
	float (*MSG_ReadHiresAngle)() {};
	void (*MSG_WriteByte)(sizebuf_t* sb, int c) {};
	void (*MSG_WriteShort)(sizebuf_t* sb, int c) {};
	void (*MSG_WriteWord)(sizebuf_t* sb, int c) {};
	void (*MSG_WriteLong)(sizebuf_t* sb, int c) {};
	void (*MSG_WriteFloat)(sizebuf_t* sb, float f) {};
	void (*MSG_WriteString)(sizebuf_t* sb, const char* s) {};
	void (*MSG_WriteBuf)(sizebuf_t* sb, int iSize, void* buf) {};
	void (*MSG_WriteAngle)(sizebuf_t* sb, float f) {};
	void (*MSG_WriteHiresAngle)(sizebuf_t* sb, float f) {};
	void (*MSG_WriteOneBit)(int nValue) {};
	void (*MSG_EndBitWriting)(sizebuf_t* buf) {};
	void (*MSG_WriteBits)(uint32 data, int numbits) {};
	void (*MSG_WriteSBits)(int data, int numbits) {};
	void (*MSG_WriteBitString)(const char* p) {};
	void (*MSG_WriteBitData)(void* src, int length) {};
	void (*MSG_WriteBitAngle)(float fAngle, int numbits) {};
	float (*MSG_ReadBitAngle)(int numbits) {};
	void (*MSG_WriteBitCoord)(const float f) {};
	void (*MSG_WriteBitVec3Coord)(const vec3_t fa);
	float (*MSG_ReadCoord)() {};
	void (*MSG_WriteCoord)(sizebuf_t* sb, const float f) {};
	void (*MSG_ReadVec3Coord)(sizebuf_t* sb, vec3_t fa) {};
	void (*MSG_WriteVec3Coord)(sizebuf_t* sb, const vec3_t fa) {};
	void (*MSG_ReadUsercmd)(usercmd_t* to, usercmd_t* from) {};
	void (*MSG_WriteUsercmd)(sizebuf_t* buf, usercmd_t* to, usercmd_t* from) {};


	int* cl_maxents{}; //unknown name
	struct sfx_t** cl_sfx_r_exp1{};
	struct sfx_t** cl_sfx_r_exp2{};
	struct sfx_t** cl_sfx_r_exp3{};
	struct model_s** cl_sprite_dot{};
	struct model_s** cl_sprite_lightning{};
	struct model_s** cl_sprite_glow{};
	struct model_s** cl_sprite_muzzleflash{};
	struct model_s** cl_sprite_muzzleflash2{};
	struct model_s** cl_sprite_muzzleflash3{};
	struct model_s** cl_sprite_muzzleflash4{};
	struct model_s** cl_sprite_muzzleflash5{};
	struct model_s** cl_sprite_muzzleflash6{};
	struct model_s** cl_sprite_muzzleflash7{};
	struct model_s** cl_sprite_muzzleflash8{};
	struct model_s** cl_sprite_muzzleflash9{};
	struct model_s** cl_sprite_ricochet{};
	struct model_s** cl_sprite_shell{};
	double* realtime{};
	int* msg_readcount{};
	sizebuf_s* g_net_message{};
	netchan_s* cls_netchan{};
	struct server_static_s *psvs{};						// &svs
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
	//void(*MSG_BeginReading)() {};

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
	class CStudioModelRenderer** g_ppStudioAPI{};


	void GetImportantEngineOffsets(long enginedll);
	void GetImportantClientOffsets(long clientdll);
	int EV_GetTraceHull();
};

extern nf_pointers g_Pointers;