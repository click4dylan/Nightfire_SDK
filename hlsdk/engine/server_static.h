#pragma once

//#include "maintypes.h"
#include "netadr.h"
#include "bool_nightfire.h"

typedef struct server_log_s
{
	bool_nightfire active;
	bool_nightfire net_log_;
	netadr_t net_address_;
	void *file;
} server_log_t;

typedef struct server_stats_s
{
	int num_samples;
	int at_capacity;
	int at_empty;
	float capacity_percent;
	float empty_percent;
	int minusers;
	int maxusers;
	float cumulative_occupancy;
	float occupancy;
	int num_sessions;
	float cumulative_sessiontime;
	float average_session_len;
	float cumulative_latency;
	float average_latency;
} server_stats_t;

typedef struct server_static_s
{
	bool_nightfire dll_initialized;
	/*struct client_s*/client_t *clients;
	int maxclients;
	int maxclientslimit;
	int spawncount;
	int serverflags;
	server_log_t log;
	double next_cleartime;
	double next_sampletime;
	server_stats_t stats;
	//qboolean isSecure;
} server_static_t;