#pragma once

#include <progs.h>
#include <FileSystem.h>
#include <bool_nightfire.h>
#include <custom.h>
#include "edict.h"

#include <crc.h>
#include <qlimits.h>
#include <consistency.h>
#include <event.h>
#include <inst_baseline.h>


#define MAX_DATAGRAM 4000
#define MAX_INFO_STRING 256

struct client_frame_t;

// offsets are for mac version
typedef struct client_s
{
	bool_nightfire active; //0
	bool_nightfire spawned; //4
	bool_nightfire fully_connected; //8
	bool_nightfire connected; //0xC
	bool_nightfire unused;
	bool_nightfire has_force_unmodified; //0x14
	netchan_t netchan;
#ifdef MAC_VERSION
	char compiler_alignment[4];
#endif
	int chokecount; //0x2428
	int delta_sequence;
	bool_nightfire fakeclient; //0x2430
	usercmd_t lastcmd; //0x2434
	double connecttime; //0x246C
	double cmdtime;
	double ignorecmdtime;
	float latency;
	float packet_loss;
	double localtime;
	double nextping;
	double svtimebase;
	sizebuf_t datagram;
	byte datagram_buf[MAX_DATAGRAM];
	double connection_started; //0x3458
	double next_messagetime;
	double next_messageinterval;
	bool_nightfire send_message; //0x3470
	bool_nightfire skip_message; //0x3474
	client_frame_t* frames; //0x3478
	event_state_t events; //0x347C
	edict_t* edict; //0x4A7C
	const edict_t* pViewEntity;
	int userid;
	int network_userid;
	char userinfo[MAX_INFO_STRING];
	bool_nightfire sendinfo; //0x4B8C
	float sendinfo_time; //0x4B90
	char hashedcdkey[64];
	char name[32];
	int entityId;
	resource_t resourcesonhand;
	resource_t resourcesneeded;
	FileHandle_t upload;
	int crcValue;
	int lw; //0x4D10
	int lc;
	char physinfo[MAX_INFO_STRING]; //0x4D18
} client_t;

const int DEFAULT_SOUND_PACKET_VOLUME = 255;
const float DEFAULT_SOUND_PACKET_ATTENUATION = 1.0f;
const int DEFAULT_SOUND_PACKET_PITCH = 100;

// Sound flags
enum
{
	SND_FL_VOLUME = 1 << 0,	// send volume
	SND_FL_ATTENUATION = 1 << 1,	// send attenuation
	SND_FL_LARGE_INDEX = 1 << 2,	// send sound number as short instead of byte
	SND_FL_PITCH = 1 << 3,	// send pitch
	SND_FL_SENTENCE = 1 << 4,	// set if sound num is actually a sentence num
	SND_FL_STOP = 1 << 5,	// stop the sound
	SND_FL_CHANGE_VOL = 1 << 6,	// change sound vol
	SND_FL_CHANGE_PITCH = 1 << 7,	// change sound pitch
	SND_FL_SPAWNING = 1 << 8	// we're spawning, used in some cases for ambients (not sent across network)
};

// Message send destination flags
enum
{
	MSG_FL_NONE = 0,		// No flags
	MSG_FL_BROADCAST = 1 << 0,	// Broadcast?
	MSG_FL_PVS = 1 << 1,	// Send to PVS
	MSG_FL_PAS = 1 << 2,	// Send to PAS
	MSG_FL_ONE = 1 << 7,	// Send to single client
};

const int RESOURCE_INDEX_BITS = 12;

typedef enum redirect_e
{
	RD_NONE = 0,
	RD_CLIENT = 1,
	RD_PACKET = 2,
} redirect_t;

typedef enum server_state_e
{
	ss_dead = 0,
	ss_loading = 1,
	ss_active = 2,
} server_state_t;

struct server_t
{
	bool_nightfire active;
	bool_nightfire paused;
	bool_nightfire loadgame;
	double time;
	double oldtime;
	int lastcheck;
	double lastchecktime;
	char name[64];
	char startspot[64];
	char modelname[64];
	struct model_s *worldmodel;
	/*CRC32_t*/uint32 worldmapCRC;
	unsigned char clientdllmd5[16];
	resource_t resourcelist[MAX_RESOURCE_LIST];
	int num_resources;
	consistency_t consistency_list[MAX_CONSISTENCY_LIST];
	int num_consistency;
	const char *model_precache[MAX_MODELS];
	const char *sound_precache[MAX_SOUNDS];
	const char *generic_precache[MAX_GENERIC];
	struct model_s *models[MAX_MODELS];
	// not sure what this was.
//#ifndef MAC_VERSION
	//int pc_padding;
//#endif
	struct event_s event_precache[MAX_EVENTS];
	unsigned short sound_precache_hashedlookup[MAX_SOUNDS_HASHLOOKUP_SIZE];
	bool_nightfire sound_precache_hashedlookup_built;
	char generic_precache_names[MAX_GENERIC][64];
	int num_generic_names;
	const char *lightstyles[MAX_LIGHTSTYLES];
	int num_edicts;
	int max_edicts;
	edict_t *edicts;
	struct entity_state_s *baselines;
	extra_baselines_t *instance_baselines;
	server_state_t state;
	sizebuf_t datagram;
	unsigned char datagram_buf[MAX_DATAGRAM];
	sizebuf_t reliable_datagram;
	unsigned char reliable_datagram_buf[MAX_DATAGRAM];
	sizebuf_t multicast;
	unsigned char multicast_buf[1024];
	sizebuf_t signon;
	unsigned char signon_data[32768];
};
