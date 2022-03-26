#pragma once

//hacky temp netchan structs

#define NET_MAX_PAYLOAD 65536
// 9 bytes SPLITHEADER + 4000 payload?
#define MAX_UDP_PACKET 4010 
// 10 reserved for fragheader?
#define MAX_MSGLEN  3990
#define MAX_RELIABLE_PAYLOAD 1200
#define CLIENT_FRAGMENT_SIZE_ONCONNECT 128
#define CUSTOMIZATION_MAX_SIZE  20480
#define FRAGMENT_MAX_SIZE 1024
#define FRAGMENT_C2S_MIN_SIZE 16
#define FRAGMENT_S2C_MIN_SIZE 256
#define FRAGMENT_S2C_MAX_SIZE 1024
#define MAX_NORMAL_FRAGMENTS (NET_MAX_PAYLOAD / CLIENT_FRAGMENT_SIZE_ONCONNECT)
#define MAX_FILE_FRAGMENTS (CUSTOMIZATION_MAX_SIZE / FRAGMENT_C2S_MIN_SIZE)

#define MAKE_FRAGID(id,count)	((( id & 0xffff) << 16) | (count & 0xffff))
#define FRAG_GETID(fragid)		((fragid >> 16) & 0xffff)
#define FRAG_GETCOUNT(fragid)	(fragid & 0xffff)

enum
{
	FRAG_NORMAL_STREAM = 0,
	FRAG_FILE_STREAM,
	MAX_STREAMS
};

#pragma pack(push, 1)
struct netchan_temp
{
	unsigned char gap0[92];
	sizebuf_t message; //92
	char gap1[7988]; //112
	fragbufwaiting_s* waitlist[2]; //8100
	int reliable_fragment[MAX_STREAMS];//1FAC  8108
	unsigned int reliable_fragid[MAX_STREAMS]; //0x1FB4 8116
	fragbuf_t* fragbufs[MAX_STREAMS];
	int fragbufcount[MAX_STREAMS]; //1FC4 8132
	short frag_startpos[MAX_STREAMS];//1FCC 8140
	short frag_length[MAX_STREAMS];//1FD0 8144
	fragbuf_t* incomingbufs[MAX_STREAMS]; //8148
	qboolean incomingready[MAX_STREAMS];//1FDC 8156
};
#pragma pack(pop)

#pragma pack(push, 1)
struct netchan2
{
	unsigned char gap0[84];
	DWORD connection_status; //84
	int(__cdecl* pfnNetchan_Blocksize)(DWORD); //88
	char pad[8008]; //92
	fragbufwaiting_s* waitlist[2]; //8100
};
#pragma pack(pop)

struct netchan5
{
	char char0;
	__declspec(align(4)) char char4;
	BYTE gap5[19];
	DWORD player_slot;
	float last_received;
	float connect_time;
	BYTE gap24[4];
	double rate;
	BYTE gap30[24];
	DWORD dword48;
	BYTE gap4C[8];
	DWORD connection_status;
	DWORD pfnNetchan_Blocksize;
	DWORD message_buffername;
	WORD word60;
	DWORD message_data;
	DWORD message_maxsize;
};

struct netchan4
{
	unsigned char gap0[8124];
	DWORD* fragbufs[2];
};

struct netchan6
{
	netchan4 netchan40;
	BYTE gap1FC4[12];
	WORD word1FD0;
	BYTE gap1FD2[274];
	void* tempbuffer;

	double& cleartime()
	{
		return *(double*)&netchan40.gap0[48];
	}
	int& reliable_length()
	{
		return *(int*)&netchan40.gap0[4104];
	}
};

struct netchan3
{
	unsigned char gap0[108];
	int message_cursize;
};


struct netchan7
{
	netchan3 netchan30;
	char char70;
	BYTE gap71[3991];
	DWORD dword1008;
	char char100C;
	BYTE gap100D[3999];
	DWORD dword1FAC;
	BYTE gap1FB0[4];
	DWORD dword1FB4;
	BYTE gap1FB8[4];
	int int1FBC;
	BYTE gap1FC0[12];
	WORD word1FCC;
	WORD word1FCE;
	__int16 int161FD0;
	BYTE gap1FD2[790];
	DWORD dword22E8;

	int& incoming_acknowleged()
	{
		return *(int*)&netchan30.gap0[60];
	}
	int& last_reliable_sequence()
	{
		return *(int*)&netchan30.gap0[80];
	}
	int& reliable_sequence()
	{
		return *(int*)&netchan30.gap0[76];
	}
	int incoming_reliable_acknowledged()
	{
		return *(int*)&netchan30.gap0[64];
	}
};

//was 100 in nf
#define MAX_FPS 1000


//was 10000 in nf
//was 30000 in half-life
#define DEFAULT_RATE 128000
//was 20000 in nf
#define MAX_RATE 1000000

#define COMPRESS_PACKETS
//works
#define HOOK_NETCHAN_CLEAR
//works
#define HOOK_VALIDATE
//works
#define HOOK_NETCHAN_FRAGSEND

#define HOOK_NETCHAN_COPYNORMALFRAGMENTS
//works
#define HOOK_NETCHAN_CLEAR_FRAGMENTS
//works
#define HOOK_NETCHAN_CREATEFRAGMENTS
//works
#define HOOK_NETCHAN_CREATEFRAGMENTS_
//works
#define HOOK_NETCHAN_SETUP