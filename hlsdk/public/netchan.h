#pragma once

#include <extdll.h>
#include <const.h>
#include <sizebuf.h>
#include <fragbuf.h>

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

// Size of fragmentation buffer internal buffers
const int FRAGMENT_MAX_SIZE_ORIGINAL = 1400;
const int MAX_FRAGMENTS_ORIGINAL = 25000;

#define MAKE_FRAGID(id,count)	((( id & 0xffff) << 16) | (count & 0xffff))
#define FRAG_GETID(fragid)		((fragid >> 16) & 0xffff)
#define FRAG_GETCOUNT(fragid)	(fragid & 0xffff)

enum
{
	FRAG_NORMAL_STREAM = 0,
	FRAG_FILE_STREAM,
	MAX_STREAMS
};

enum
{
	FLOW_OUTGOING = 0,
	FLOW_INCOMING,
	MAX_FLOWS
};

#include <netadr.h>

// Message data
typedef struct flowstats_s
{
	// Size of message sent/received
	int size;
	// Time that message was sent/received
	double time;
} flowstats_t;

const int MAX_LATENT = 32;

typedef struct flow_s
{
	// Data for last MAX_LATENT messages
	flowstats_t stats[MAX_LATENT];
	// Current message position
	int current;
	// Time when we should recompute k/sec data
	double nextcompute;
	// Average data
	float kbytespersec;
	float avgkbytespersec;
} flow_t;

typedef enum netsrc_s
{
	NS_CLIENT,
	NS_SERVER,
	NS_MULTICAST	// xxxMO
} netsrc_t;

typedef struct netchan_s
{
	netsrc_t sock;
	netadr_t remote_address; //4
	int player_slot; //24
	float last_received; //28
	float connect_time; //32
	//BYTE gap24[4]; //36
	double rate; //40
	double cleartime; //48
	int incoming_sequence; //56, untested
	int incoming_acknowledged; //60, untested
	int incoming_reliable_acknowledged; //64
	int incoming_reliable_sequence; //68, untested
	int outgoing_sequence; //70, untested
	int reliable_sequence; //76
	int last_reliable_sequence; //80
	void* connection_status; //84
	int(__cdecl* pfnNetchan_Blocksize)(void*); //88
	sizebuf_t message; //92
	byte message_buf[MAX_MSGLEN]; //112
	//char gap1[2]; //112
	int reliable_length; //4104
	byte reliable_buf[MAX_MSGLEN]; //4108
	//char gap2[2]; //4108
	fragbufwaiting_s* waitlist[MAX_STREAMS]; //8100
	int reliable_fragment[MAX_STREAMS];//1FAC  8108
	unsigned int reliable_fragid[MAX_STREAMS]; //0x1FB4 8116
	fragbuf_t* fragbufs[MAX_STREAMS];
	int fragbufcount[MAX_STREAMS]; //1FC4 8132
	short frag_startpos[MAX_STREAMS];//1FCC 8140
	short frag_length[MAX_STREAMS];//1FD0 8144
	fragbuf_t* incomingbufs[MAX_STREAMS]; //8148
	qboolean incomingready[MAX_STREAMS];//1FDC 8156
	char incomingfilename[MAX_PATH]; //8158
	void* tempbuffer; //8420
	//int tempbuffersize; // not in nightfire
	flow_t flow[MAX_FLOWS];
} netchan_t;
//size: 0x2518, 9496 bytes

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