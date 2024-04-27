#include <Windows.h>
#include <netchan.h>
#include <vector>
#include <const.h>
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

//#include <eiface.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <platformdll.h>
//#include <enginefuncs.h>
#include <globalvars.h>
#include <server.h>
#include <nightfire_pointers.h>
#include "nightfire_hooks.h"
#include <server_static.h>

#define MAKEID(d,c,b,a)					( ((int)(a) << 24) | ((int)(b) << 16) | ((int)(c) << 8) | ((int)(d)) )
byte g_ExtendedMessageBuffer[33][NET_MAX_PAYLOAD];

void(*nf_hooks::Netchan_CreateFragments_orig)(BOOL, netchan_s*, sizebuf_s*) {};
void(*nf_hooks::Netchan_ClearFragments_orig)(netchan_s* chan) {};
void(*nf_hooks::Netchan_CreateFragments__orig)(BOOL server, netchan_s* chan, sizebuf_s* msg) {};
BOOL(*nf_hooks::Netchan_CopyNormalFragments_orig)(netchan_s* chan) {};
void(*nf_hooks::Netchan_Setup_orig)(netsrc_t socketnumber, netchan_t* chan, netadr_t adr, int player_slot, void* connection_status, int(*pfnNetchan_Blocksize)(void*)) {};
void(*nf_hooks::Netchan_Clear_orig)(netchan_s* chan) {};
void(*nf_hooks::Netchan_FragSend_orig)(netchan_s* chan) {};
uintptr_t nf_hooks::Netchan_Transit_FragSend_Retadr {};
DWORD nf_hooks::NetchanCreateFragments_Transmit_Retadr {};

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

void nf_hooks::Netchan_CreateFragments_(BOOL server, netchan_s* chan, sizebuf_s* msg)
{
#ifndef HOOK_NETCHAN_CREATEFRAGMENTS_
	return nf_hooks::Netchan_CreateFragments__orig(server, chan, msg);
#else
	fragbuf_t* buf;
	int chunksize;
	int send;
	int remaining;
	int pos;
	int bufferid = 1;
	fragbufwaiting_t* wait, *p;

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

	chunksize = chan->pfnNetchan_Blocksize(chan->connection_status);

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
		g_Pointers.SZ_Clear(&buf->frag_message);
		g_Pointers.SZ_Write(&buf->frag_message, &msg->data[pos], send);
		pos += send;

		Netchan_AddFragbufToTail(wait, buf);
	}

	// Now add waiting list item to the end of buffer queue
	if (!chan->waitlist[FRAG_NORMAL_STREAM])
	{
		chan->waitlist[FRAG_NORMAL_STREAM] = wait;
	}
	else
	{
		p = chan->waitlist[FRAG_NORMAL_STREAM];
		while (p->next)
		{
			p = p->next;
		}
		p->next = wait;
	}
#endif
}

void nf_hooks::Netchan_CreateFragments(BOOL server, netchan_s* chan, sizebuf_s* msg)
{
#ifndef HOOK_NETCHAN_CREATEFRAGMENTS
	return nf_hooks::Netchan_CreateFragments_orig(server, chan, msg);
#else
	DWORD retadr = (DWORD)_ReturnAddress();
	if (retadr == nf_hooks::NetchanCreateFragments_Transmit_Retadr)
	{
		nf_hooks::Netchan_CreateFragments_(server, chan, msg);
		g_Pointers.SZ_Clear(msg);
		return;
	}

	// Always queue any pending reliable data ahead of the fragmentation buffer
	if (chan->message.cursize > 0)
	{
		nf_hooks::Netchan_CreateFragments_(server, chan, &chan->message);
		chan->message.cursize = 0;
	}

	nf_hooks::Netchan_CreateFragments_(server, chan, msg);
#endif
}

BOOL nf_hooks::Netchan_CopyNormalFragments(netchan_s* chan)
{
#ifndef HOOK_NETCHAN_COPYNORMALFRAGMENTS
	return nf_hooks::Netchan_CopyNormalFragments_orig(chan);
#else
	fragbuf_t* p, *n;

	if (!chan->incomingready[FRAG_NORMAL_STREAM])
		return FALSE;

	if (!chan->incomingbufs[FRAG_NORMAL_STREAM])
	{
		//Con_Printf("%s:  Called with no fragments readied\n", __func__);
		chan->incomingready[FRAG_NORMAL_STREAM] = FALSE;
		return FALSE;
	}

	p = chan->incomingbufs[FRAG_NORMAL_STREAM];

	g_Pointers.SZ_Clear(g_Pointers.g_net_message);
	g_Pointers.MSG_BeginReading();

	bool overflowed = false;

	while (p)
	{
		n = p->next;

		if (g_Pointers.g_net_message->cursize + p->frag_message.cursize <= g_Pointers.g_net_message->maxsize)
			g_Pointers.SZ_Write(g_Pointers.g_net_message, p->frag_message.data, p->frag_message.cursize);
		else
			overflowed = true;

		g_pNightfirePlatformFuncs->freex(p);
		p = n;
	}

	if (overflowed)
	{
		if (chan->player_slot == 0)
			g_Pointers.g_pCL_EngineFuncs->Con_Printf("%s: Incoming overflowed\n", __func__);
		else
			g_Pointers.g_pCL_EngineFuncs->Con_Printf("%s: Incoming overflowed from %s\n", __func__, g_Pointers.psvs->clients[chan->player_slot - 1].name);
		g_Pointers.SZ_Clear(g_Pointers.g_net_message);

		chan->incomingbufs[FRAG_NORMAL_STREAM] = nullptr;
		chan->incomingready[FRAG_NORMAL_STREAM] = FALSE;

		return FALSE;
	}

#ifdef COMPRESS_PACKETS
	if (*(uint32*)g_Pointers.g_net_message->data == MAKEID('B', 'Z', '2', '\0'))
	{
		char uncompressed[65536];
		unsigned int uncompressedSize = 65536;
		BZ2_bzBuffToBuffDecompress(uncompressed, &uncompressedSize, (char*)g_Pointers.g_net_message->data + 4, g_Pointers.g_net_message->cursize - 4, 1, 0);
		memcpy(g_Pointers.g_net_message->data, uncompressed, uncompressedSize);
		g_Pointers.g_net_message->cursize = uncompressedSize;
	}
#endif

	chan->incomingbufs[FRAG_NORMAL_STREAM] = nullptr;
	chan->incomingready[FRAG_NORMAL_STREAM] = FALSE;
#endif

	return TRUE;
}

void Netchan_ClearFragbufs(fragbuf_t** ppbuf)
{
	fragbuf_t* buf, *n;

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
	fragbuf_t* p, *n;

	auto current_server_client = *g_Pointers.host_client - g_Pointers.psvs->clients;

	if ((chan->player_slot - 1) == current_server_client)
	{
		g_Pointers.SZ_Clear(g_Pointers.g_net_message);
		(*g_Pointers.msg_readcount) = 0;
	}

	p = chan->incomingbufs[stream];
	while (p)
	{
		n = p->next;
		g_pNightfirePlatformFuncs->freex(p);
		p = n;
	}

	chan->incomingbufs[stream] = nullptr;
	chan->incomingready[stream] = FALSE;
}

void nf_hooks::Netchan_ClearFragments(netchan_s* chan)
{
#ifndef HOOK_NETCHAN_CLEAR_FRAGMENTS
	nf_hooks::Netchan_ClearFragments_orig(chan);
	return;
#else
	fragbufwaiting_t* wait, *next;
	for (int i = 0; i < MAX_STREAMS; i++)
	{
		wait = chan->waitlist[i];
		while (wait)
		{
			next = wait->next;
			Netchan_ClearFragbufs(&wait->fragbufs);
			g_pNightfirePlatformFuncs->freex(wait);
			wait = next;
		}
		chan->waitlist[i] = nullptr;

		Netchan_ClearFragbufs(&chan->fragbufs[i]);
		Netchan_FlushIncoming(chan, i);
	}
#endif
}

void nf_hooks::Netchan_Clear(netchan_s* chan)
{
#ifndef HOOK_NETCHAN_CLEAR
	nf_hooks::Netchan_Clear_orig(chan);
#else
	int v1; // ecx
	WORD* p_word1FD0; // edx
	DWORD* v3; // eax

	Netchan_ClearFragments(chan);
	chan->cleartime = 0.0;

	if (chan->reliable_length)
	{
		//Con_DPrintf("%s: reliable length not 0, reliable_sequence: %d, incoming_reliable_acknowledged: %d\n", __func__, chan->reliable_length, chan->incoming_reliable_acknowledged);
		chan->reliable_sequence ^= 1;
		chan->reliable_length = 0;
	}

	for (int i = 0; i < MAX_STREAMS; ++i)
	{
		chan->reliable_fragid[i] = 0;
		chan->reliable_fragment[i] = 0;
		chan->fragbufcount[i] = 0;
		chan->frag_startpos[i] = 0;
		chan->frag_length[i] = 0;
		chan->incomingready[i] = FALSE;
	}

	if (chan->tempbuffer)
	{
		g_pNightfirePlatformFuncs->freex(chan->tempbuffer);
		chan->tempbuffer = 0;
	}
	//chan->tempbuffersize = 0; // not in nightfire
#endif
}

void nf_hooks::Netchan_Setup(netsrc_t socketnumber, netchan_t* chan, netadr_t adr, int player_slot, void* connection_status, int(*pfnNetchan_Blocksize)(void*))
{
#ifndef HOOK_NETCHAN_SETUP
	nf_hooks::Netchan_Setup_orig(socketnumber, chan, adr, player_slot, connection_status, pfnNetchan_Blocksize);
#else
	Netchan_Clear(chan);
	memset(chan, 0, 9496);
	chan->sock = socketnumber;
	chan->player_slot = player_slot + 1;
	chan->remote_address = adr;
	chan->last_received = *g_Pointers.realtime;
	chan->connect_time = *g_Pointers.realtime;
	chan->message.cursize = 0;
	chan->message.flags = 1; //SIZEBUF_ALLOW_OVERFLOW

	if (chan->player_slot != -1)
	{
		chan->message.data = g_ExtendedMessageBuffer[chan->player_slot];
		chan->message.maxsize = NET_MAX_PAYLOAD;
	}
	else
	{
		chan->message.data = chan->message_buf;
		chan->message.maxsize = sizeof(chan->message_buf);
	}
	chan->message.buffername = "netchan->message";
	// nightfire fix, use cl_rate value:
	static ConsoleVariable* cl_rate = g_Pointers.g_pEngineFuncs->pfnGetConsoleVariableGame("cl_rate");
	double rate = cl_rate ? max(min(cl_rate->getValueFloat(), 1000), MAX_RATE) : DEFAULT_RATE;
	chan->rate = rate;
	chan->connection_status = connection_status;
	chan->pfnNetchan_Blocksize = pfnNetchan_Blocksize;
#endif
}

void nf_hooks::Netchan_FragSend(netchan_s* chan)
{
#ifndef HOOK_NETCHAN_FRAGSEND
	nf_hooks::FragSend_orig(chan);
#else
	if ((DWORD)_ReturnAddress() == nf_hooks::Netchan_Transit_FragSend_Retadr)
	{
		if (chan->message.cursize > MAX_MSGLEN)
		{
			Netchan_CreateFragments_(chan == g_Pointers.cls_netchan ? 1 : 0, chan, &chan->message);
			g_Pointers.SZ_Clear(&chan->message);
		}
	}
	nf_hooks::Netchan_FragSend_orig(chan);
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

qboolean __stdcall nf_hooks::Netchan_Validate_orig(int* frag_length, qboolean* frag_message, netchan_s* chan, int* frag_offset, unsigned int* fragid)
{
	for (int i = 0; i < MAX_STREAMS; i++)
	{
		if (!frag_message[i])
			continue;

		if (FRAG_GETID(fragid[i]) > MAX_FRAGMENTS_ORIGINAL || FRAG_GETCOUNT(fragid[i]) > MAX_FRAGMENTS_ORIGINAL)
		{
			return FALSE;
		}

		// NOTE: nightfire's check was 0x4000 for both!
		if ((unsigned int)frag_length[i] > 0x800 || (unsigned int)frag_offset[i] > 0x4000)
		{
			return FALSE;
		}
	}

	return TRUE;
}

qboolean __stdcall nf_hooks::Netchan_Validate(int* frag_length, qboolean* frag_message, netchan_s* chan, int* frag_offset, unsigned int* fragid)
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
		if (frag_end + *g_Pointers.msg_readcount > g_Pointers.g_net_message->cursize)
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

__declspec(naked) qboolean nf_hooks::Netchan_Validate_Hook(netchan_s* chan, qboolean* frag_message, int* frag_length)
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
		call nf_hooks::Netchan_Validate
		pop ebp
		retn
	}
}

void Fix_Netchan()
{
	if (!FindMemoryPattern(g_Pointers.Netchan_CreateFragments, g_engineDllHinst, "56 8B 74 24 0C 8B 46 6C 85 C0 57 8B 7C 24 0C 7E 15 8D 46 5C", false))
		return;
	if (!FindMemoryPattern(g_Pointers.Netchan_CreateFragments_, g_engineDllHinst, "83 EC 10 57 8B 7C 24 20 8B 47 10 85 C0 C7 44 24 10 01 00 00 00", false))
		return;
	if (!FindMemoryPattern(g_Pointers.Netchan_CopyNormalFragments, g_engineDllHinst, "53 8B 5C 24 08 8A 83 DC 1F 00 00 84 C0 75 04 32 C0 5B C3", false))
		return;
	g_Pointers.g_net_message = *(sizebuf_s**)((DWORD)g_Pointers.Netchan_CopyNormalFragments + 0x37);
	if (!FindMemoryPattern(g_Pointers.Netchan_ClearFragments, g_engineDllHinst, "83 EC 08 53 55 8B 2D ? ? ? ? 56 8B 74 24 18", false))
		return;
	if (!FindMemoryPattern(g_Pointers.msg_readcount, g_engineDllHinst, "8B 15 ? ? ? ? 50 4A 52", false, true, 2, 0, false))
		return;
	if (!FindMemoryPattern(g_Pointers.Netchan_Setup, g_engineDllHinst, "53 8B 5C 24 0C 56 57 53", false))
		return;
	DWORD Netchan_Transmit;
	if (!FindMemoryPattern(Netchan_Transmit, g_engineDllHinst, "B8 08 10 00 00 E8", false))
		return;
	if (!FindMemoryPattern(g_Pointers.Netchan_FragSend, g_engineDllHinst, "8B 44 24 04 85 C0 74 47 53 56 57", false))
		return;
	FindMemoryPattern(nf_hooks::Netchan_Transit_FragSend_Retadr, g_engineDllHinst, "8B 55 6C 33 C0 83 C4 04 3B D3 0F 95 C0", false);
	DWORD adr8;
	if (!FindMemoryPattern(adr8, g_engineDllHinst, "81 FD ? ? ? ? 0F 95 C2 8D 4D 5C", false))
		return;
	g_Pointers.Netchan_Validate = &nf_hooks::Netchan_Validate_orig;
	BYTE* Netchan_Validate_original;
	if (!FindMemoryPattern(Netchan_Validate_original, g_engineDllHinst, "53 55 8B 6C 24 14 56 8B F1 8B DD 57", false))
		return;
	if (!FindMemoryPattern(g_Pointers.Netchan_Clear, g_engineDllHinst, "53 56 8B 74 24 0C 56 E8 ? ? ? ? 33 DB 83 C4 04", false))
		return;

	g_Pointers.realtime = *(double**)((DWORD)g_Pointers.Netchan_Setup + 0x3C);
	g_Pointers.cls_netchan = (netchan_s*)*(DWORD*)(adr8 + 2);

	if (!FindMemoryPattern(g_Pointers.SZ_Clear, g_engineDllHinst, "8B 44 24 04 80 60 04 FD C7 40 10 00 00 00 00 C3", false))
		return;
	if (!FindMemoryPattern(g_Pointers.SZ_Write, g_engineDllHinst, "8B 44 24 0C 8B 4C 24 08 8B 54 24 04 50 51 50 52", false))
		return;
	//if (!FindMemoryPattern(g_Pointers.MSG_BeginReading, g_engineDllHinst, "33 C0 A3 ? ? ? ? A2 ? ? ? ? C3", false))
	//	return;
	if (!FindMemoryPattern(nf_hooks::NetchanCreateFragments_Transmit_Retadr, g_engineDllHinst, "83 C4 0C 89 5D 6C 8D BD D0 1F 00 00 33 C0 8B CF", false))
		return;
	if (!FindMemoryPattern(g_Pointers.host_client, g_engineDllHinst, "A3 ? ? ? ? 7E 39 8B 7C 24 0C 80 38 00", false, true, 1))
		return;
#ifdef HOOK_NETCHAN_CLEAR_FRAGMENTS
	if (!HookFunctionWithMinHook(g_Pointers.Netchan_ClearFragments, nf_hooks::Netchan_ClearFragments, (void**)&nf_hooks::Netchan_ClearFragments_orig))
		return;
#else
	nf_hooks::Netchan_ClearFragments_orig = g_Pointers.Netchan_ClearFragments;
#endif
#ifdef HOOK_NETCHAN_CREATEFRAGMENTS
	if (!HookFunctionWithMinHook(g_Pointers.Netchan_CreateFragments, nf_hooks::Netchan_CreateFragments, &nf_hooks::Netchan_CreateFragments_orig))
		return;
#else
	nf_hooks::Netchan_CreateFragments_orig = g_Pointers.Netchan_CreateFragments;
#endif
#ifdef HOOK_NETCHAN_CREATEFRAGMENTS_
	if (!HookFunctionWithMinHook(g_Pointers.Netchan_CreateFragments_, nf_hooks::Netchan_CreateFragments_, (void**)&nf_hooks::Netchan_CreateFragments__orig))
		return;
#else
	nf_hooks::Netchan_CreateFragments__orig = g_Pointers.Netchan_CreateFragments_;
#endif
#ifdef HOOK_NETCHAN_COPYNORMALFRAGMENTS
	if (!HookFunctionWithMinHook(g_Pointers.Netchan_CopyNormalFragments, nf_hooks::Netchan_CopyNormalFragments, (void**)&nf_hooks::Netchan_CopyNormalFragments_orig))
		return;
#else
	nf_hooks::Netchan_CopyNormalFragments_orig = g_Pointers.Netchan_CopyNormalFragments;
#endif
#ifdef HOOK_NETCHAN_SETUP
	if (!HookFunctionWithMinHook(g_Pointers.Netchan_Setup, nf_hooks::Netchan_Setup, &nf_hooks::Netchan_Setup_orig))
		return;
#else
	nf_hooks::Netchan_Setup_orig = g_Pointers.Netchan_Setup;
#endif
#ifdef HOOK_NETCHAN_FRAGSEND
	if (!HookFunctionWithMinHook(g_Pointers.Netchan_FragSend, nf_hooks::Netchan_FragSend, &nf_hooks::Netchan_FragSend_orig))
		return;
#else
	nf_hooks::Netchan_FragSend_orig = g_Pointers.Netchan_FragSend;
#endif
#ifdef HOOK_NETCHAN_CLEAR
	if (!HookFunctionWithMinHook(g_Pointers.Netchan_Clear, nf_hooks::Netchan_Clear, &nf_hooks::Netchan_Clear_orig))
		return;
#else
	nf_hooks::Netchan_Clear_orig = g_Pointers.Netchan_Clear;
#endif

#ifdef HOOK_VALIDATE
	PlaceJMP(Netchan_Validate_original, (DWORD)&nf_hooks::Netchan_Validate_Hook, 5);
#endif

	// reduce buffer size to MAX_UDP_PACKET
	DWORD old;
	VirtualProtect((BYTE*)(Netchan_Transmit + 0x33), 4, PAGE_EXECUTE_READWRITE, &old);
	*(int*)(Netchan_Transmit + 0x33) = MAX_UDP_PACKET;
	VirtualProtect((BYTE*)(Netchan_Transmit + 0x33), 4, old, &old);

	PlaceJMP((BYTE*)(Netchan_Transmit + 0x16A), (DWORD)&Netchan_CopyTo_Reliable_Buf, 10);
	Netchan_CopyTo_Reliable_Buf_Jmpback = Netchan_Transmit + (0x16A + 10);
}