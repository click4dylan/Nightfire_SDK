#pragma once

#include <progs.h>
#include <FileSystem.h>
#include <bool_nightfire.h>
#include <custom.h>
#include "edict.h"

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