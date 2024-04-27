#pragma once

#include <Windows.h>

#include <netchan.h>
#include <extdll.h>

struct model_s;
struct netchan_s;
struct sizebuf_s;
class CBasePlayer;

namespace nf_hooks
{
	extern Vector* __fastcall FirePredictedBullets_ServerHook(CBasePlayer* player, void* edx, Vector* returnval, ULONG cShots, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType);
	extern Vector* __fastcall FirePredictedBullets_ClientHook(CBasePlayer* player, void* edx, Vector* returnval, ULONG cShots, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType);

	extern model_s* R_StudioSetupPlayerModel(int playerindex);
	extern bool UTIL_CheckForWater(float startx, float starty, float startz, float endx, float endy, float endz, float* dest);

	//netchan
	extern void Netchan_CreateFragments(BOOL server, netchan_s* chan, sizebuf_s* msg);
	extern void(*Netchan_CreateFragments_orig)(BOOL, netchan_s*, sizebuf_s*);

	extern void Netchan_ClearFragments(netchan_s* chan);
	extern void (*Netchan_ClearFragments_orig)(netchan_s* chan);

	extern void Netchan_CreateFragments_(BOOL server, netchan_s* chan, sizebuf_s* msg);
	extern void(*Netchan_CreateFragments__orig)(BOOL server, netchan_s* chan, sizebuf_s* msg);

	extern BOOL Netchan_CopyNormalFragments(netchan_s* chan);
	extern BOOL(*Netchan_CopyNormalFragments_orig)(netchan_s* chan);

	extern void Netchan_Setup(netsrc_t socketnumber, netchan_t* chan, netadr_t adr, int player_slot, void* connection_status, int(*pfnNetchan_Blocksize)(void*));
	extern void (*Netchan_Setup_orig)(netsrc_t socketnumber, netchan_t* chan, netadr_t adr, int player_slot, void* connection_status, int(*pfnNetchan_Blocksize)(void*));
	extern void Netchan_Clear(netchan_s* chan);
	extern void(*Netchan_Clear_orig)(netchan_s* chan);
	extern void Netchan_FragSend(netchan_s* chan);
	extern void(*Netchan_FragSend_orig)(netchan_s* chan);
	extern qboolean __stdcall Netchan_Validate(int* frag_length, qboolean* frag_message, netchan_s* chan, int* frag_offset, unsigned int* fragid);
	extern qboolean __stdcall Netchan_Validate_orig(int* frag_length, qboolean* frag_message, netchan_s* chan, int* frag_offset, unsigned int* fragid);
	extern qboolean Netchan_Validate_Hook(netchan_s* chan, qboolean* frag_message, int* frag_length);


	extern uintptr_t Netchan_Transit_FragSend_Retadr;
	extern DWORD NetchanCreateFragments_Transmit_Retadr;

	extern DWORD LoadThisDLL_FunctionAddress;
}