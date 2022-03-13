#include "cl_move_fixes.h"
#include "ilagcompensationmanager.h"
//#include "utlvector.h"
//#include "bitvec.h"
//#include "bitstuff.h"
extern int* net_drop;
bool* unlag_fixstuck;
bool* unlag_frametime;
bool* unlag_updaterate;
float* unlag_updaterate_mult;
float* unlag_frametime_mult;
//HOOKS
DWORD tempEAX;
DWORD tempEBX;
DWORD tempECX;
DWORD tempESI;
DWORD tempEDX;
DWORD tempEDI;
DWORD *tempRegister;
DWORD *tempRegister2;
DWORD *tempRegister3;
DWORD *tempRegister4;
float tempFloat;

#define ex_interp 0x4310FDD0
#define cl_cmdbackup 0x4310fa04
#define cl_cmdrate 0x4310F908
#define cl_nodelta 0x43110034
#define host_limitlocal 0x448BE54C
#define MAX_CMD_BUFFER	16384
#define MAX_USERCMD_BUFFER 2048
#define CONNECTION_PROBLEM_TIME 15.0f
#define cl_timeout 0x4310FF80
double* cl_time = (double*)0x431B8B60;
double* cl_mtime = (double*)0x431B8B50;
char* light_level2 = (char*)0x431b8b48;
int* cl_maxclients2 = (int*)0x431191bc;
int* cl_delta_sequence = (int*)0x432cd370;
int* dword_432989B8 = (int*)0x432989B8;

struct bufferstorage
{
	int buffersize[MAX_CMD_BUFFER];
};

bufferstorage* buffersizestorage = (bufferstorage*)0x432989B8;

struct tempcommandstruct
{
	cmd_t commands[64];
};

tempcommandstruct* CMDS = (tempcommandstruct*)0x43298970;

void (__cdecl *CL_PredictionSetupMove) (int oneorzero) = (void(__cdecl*)(int))0x43038C70;
extern sizebuf_t* net_message;

#define LOBYTE(x)   (*((BYTE*)&(x)))   // low byte
extern void Netchan_Transmit(netchan_t *chan, int length, byte *data);
extern qboolean Netchan_CanPacket (netchan_t *chan);
//void(*Netchan_Transmit) (netchan_t *chan, int length, byte *data) = (void(*)(netchan_t*, int, byte*))0x43065400;
//qboolean(*Netchan_CanPacket) (netchan_t *chan) = (qboolean(*)(netchan_t*))0x430644C0;
#define MULTIPLAYER_BACKUP	64	// how many data slots to use when in multiplayer (must be power of 2)
void(*Con_NXPrintf) (int loc, const char*text) = (void(*)(int, const char*))0x43043BD0;
void(*Host_Error) (const char* errorstr, ...) = (void(*)(const char*, ...))0x430593F0;
void(*CL_ComputePacketLoss) (void) = (void(*)(void))0x43032320;
//void(*MSG_WriteByte)(sizebuf_t *sb, int c) = (void(*)(sizebuf_t *, int))0x43042210;
//int(*MSG_ReadByte) (void) = (int(*)(void))0x4303FEB0;
//void(*MSG_ReadUsercmd) (usercmd_t *to, usercmd_t* from) = (void(*)(usercmd_t*, usercmd_t*))0x43042A10;
void(*CL_WriteUsercmdDeltaToBuffer)(sizebuf_t *msg, usercmd_t* from, usercmd_t* to) = (void(*)(sizebuf_t *, usercmd_t*, usercmd_t*))0x43043000;
int* LocalPlayerNum = (int*)0x432cd374;
void(*CL_SetSolidPlayers)(int playernum) = (void(*)(int))0x4302DF40;
void(*CL_SetSolidEntities)(void) = (void(*)(void))0x4302DAD0;
void(*CL_PushPMStates)(void) = (void(*)(void))0x43038160;
void(*CL_PopPMStates)(void) = (void(*)(void))0x430381A0;
void(*COM_Munge)(unsigned char*data, int len, int seq) = (void(*)(unsigned char*, int, int))0x43041A00;
void(*COM_Munge2)(unsigned char *data, int len, int seq) = (void(*)(unsigned char*, int, int))0x43041C00;
void(*COM_UnMunge)(unsigned char *data, int len, int seq) = (void(*)(unsigned char*, int, int))0x43041B00;
void(*COM_UnMunge2)(unsigned char *data, int len, int seq) = (void(*)(unsigned char*, int, int))0x43041D00;
void(*CL_CreateMove)(float frametime, struct usercmd_s *cmd, int active) = (void(*)(float, struct usercmd_s*, int))0x4302B840;
//void(*SZ_Clear)(sizebuf_t* buf) = (void(*)(sizebuf_t*))0x43040260;
//void(*Con_Printf)(char *fmt, ...) = (void(*)(char *, ...))0x43043870;
//void(*Con_DPrintf)(const char *fmt, ...) = (void(*)(const char*, ...))0x430439F0;
//void (*SV_DropClient)(client_t *cl, qboolean crash, const char *fmt, ...) = (void(*)(client_t*, qboolean, const char*, ...))0x430574A0;
byte(*COM_BlockSequenceCRCByte2) (byte *base, int length, int sequence) = (byte(*)(byte*, int, int))0x43046760;
char* (*TranslateMessage2)(const char* str) = (char*(*)(const char*))0x430967B0;
extern char* (*NET_AdrToString)(const netadr_t& a);// = (char*(*)(const netadr_t&))0x43065F30;
void (*SV_RunCmd)(usercmd_t *ucmd, int random_seed) = (void(*)(usercmd_t*, int))0x43093040;
void(*SV_EstablishTimeBase) (client_t *cl, usercmd_t *cmds, int dropped, int numbackup, int numcmds) = (void(*)(client_t*, usercmd_t*, int, int, int))0x43091AC0;

usercmd_t	cmds[MULTIPLAYER_BACKUP]; // each mesage will send several old cmds
// struct info (filled by engine)
typedef struct
{
	const char	*name;
	const int		offset;
	const int		size;
} delta_field_t;

// one field
typedef struct delta_s
{
	const char	*name;
	int		offset;		// in bytes
	int		size;		// used for bounds checking in DT_STRING
	int		flags;		// DT_INTEGER, DT_FLOAT etc
	float		multiplier;
	float		post_multiplier;	// for DEFINE_DELTA_POST
	int		bits;		// how many bits we send\receive
	qboolean		bInactive;	// unsetted by user request
} delta_t;

typedef void(*pfnDeltaEncode)(delta_t *pFields, const byte *from, const byte *to);

typedef struct
{
	const char	*pName;
	const delta_field_t	*pInfo;
	const int		maxFields;	// maximum number of fields in struct
	int		numFields;	// may be merged during initialization
	delta_t		*pFields;

	// added these for custom entity encode
	int		customEncode;
	char		funcName[32];
	pfnDeltaEncode	userCallback;
	qboolean		bInitialized;
} delta_info_t;

delta_info_t* (*Delta_FindStruct)(const char* name) = (delta_info_t*(*)(const char*))0x43049710;

void CL_WriteUsercmdNF(sizebuf_t *msg, int from, int to)
{
	usercmd_t	nullcmd;
	usercmd_t	*f, *t;

	//ASSERT(from == -1 || (from >= 0 && from < MULTIPLAYER_BACKUP));
	//ASSERT(to >= 0 && to < MULTIPLAYER_BACKUP);

	if (from == -1)
	{
		Q_memset(&nullcmd, 0, sizeof(nullcmd));
		f = &nullcmd;
	}
	else
	{
		f = &cmds[from];
	}

	t = &cmds[to];

	// write it into the buffer
	CL_WriteUsercmdDeltaToBuffer(msg, f, t);
}

//todo: sv_computelatency (sv_unlagsamples)
#define LODWORD(x)  (*((DWORD*)&(x)))  // low dword
static int* bNotFinalTick = (int*)0x44bad2f4;

void CL_SendMove(cmd_t* cmd, sizebuf_t *buf, byte* data)
{
	//if (*bNotFinalTick)
	//return;

	// begin a client move command
	MSG_WriteByte(buf, clc_move);

	// save the position for a checksum byte
	int key = buf->cursize + 1; //BF_GetRealBytesWritten( &buf );
	MSG_WriteByte(buf, 0);

	MSG_WriteByte(buf, 0); //nf..

	int packetloss = (int)g_pcls->packet_loss;
	packetloss = clamp(packetloss, 0, 100);

	MSG_WriteByte(buf, packetloss); // write packet lossage percentation

#ifndef _DEBUG
		if (CVAR_GETDIRECTINT(cl_cmdrate) < 10)
		{
			CVAR_SETDIRECTINT(cl_cmdrate, 10);
		}
#endif

		if (*(char*)0x4311D2A1)
		{
			SZ_Clear(buf);
		}
		else
		{
			if (g_pcls->lastoutgoingcommand == -1)
				g_pcls->lastoutgoingcommand = g_pcls->netchan.outgoing_sequence; //goldsrc

			//int nextcommandnr = /*g_pcls->lastoutgoingcommand + */g_pcls->netchan.outgoing_sequence + 1;

			// Determine number of backup commands to send along
			int numbackup = clamp(0, CVAR_GETDIRECTINT(cl_cmdbackup), MAX_BACKUP_COMMANDS);

			// how many real commands have queued up
			int newcmds = g_pcls->netchan.outgoing_sequence - g_pcls->lastoutgoingcommand; //g_pcls->netchan.outgoing_sequence + 1;

			// put an upper/lower bound on this
			newcmds = clamp(newcmds, 0, MAX_NEW_COMMANDS); //MAX_TOTAL_CMDS

			int numcmds = newcmds + numbackup - 1;

			// say how many backups we'll be sending
			MSG_WriteByte(buf, numbackup);

			// say how many new commands we'll be sending
			MSG_WriteByte(buf, newcmds);

#if 0
			int from = -1;	// first command is deltaed against zeros 

			for (int to = nextcommandnr - numcmds + 1; to <= nextcommandnr; to++)
			{
				CL_WriteUsercmdNF(buf, from, to);
				from = to;
			}
#else
			usercmd_t ncmd;
			Q_memset(&ncmd, 0, sizeof(usercmd_t));
			usercmd_t *to;
#if 0
			for (usercmd_t* from = &ncmd; numcmds > 0; from = to)
			{
				to = &CMDS->commands[CL_UPDATE_MASK & (g_pcls->netchan.outgoing_sequence - numcmds)].cmd;
				CL_WriteUsercmdDeltaToBuffer(buf, to, from);
				numcmds--;
			}
#endif
			usercmd_t *from;
			for (usercmd_t* from = &ncmd; numcmds >= 0; from = to)
			{
				to = &CMDS->commands[CL_UPDATE_MASK & (g_pcls->netchan.outgoing_sequence - numcmds)].cmd;//(usercmd_t*) (0x43298970 + (sizeof(cmd_t) * (mask & (g_pcls->netchan.outgoing_sequence - numcmds))) );
				CL_WriteUsercmdDeltaToBuffer(buf, to, from);
				numcmds--;
			}

#endif

			// calculate a checksum over the move commands

			int pos = buf->cursize - key - 1; //cursize should be 14
			if (pos > 255)
			{
				pos = 255;
			}
			buf->data[key - 1] = (byte)pos;
			buf->data[key] = COM_BlockSequenceCRCByte2(&buf->data[key + 1], buf->cursize - key - 1, g_pcls->netchan.outgoing_sequence);
			int pos2 = buf->cursize - key - 1;
			if (pos2 > 255)
			{
				pos2 = 255;
			}
			COM_Munge(&buf->data[key + 1], pos2, g_pcls->netchan.outgoing_sequence);
		}
}

void VectorAngles(const Vector& forward, Vector &angles)
{
#if 0
	if (forward[1] == 0.0f && forward[0] == 0.0f)
	{
		angles[0] = (forward[2] > 0.0f) ? 270.0f : 90.0f;
		angles[1] = 0.0f;
	}
	else
	{
		float len2d = /*sseSqrt*/sqrtf(square(forward[0]) + square(forward[1]));

		angles[0] = RAD2DEG(atan2f(-forward[2], len2d));
		angles[1] = RAD2DEG(atan2f(forward[1], forward[0]));

		if (angles[0] < 0.0f) angles[0] += 360.0f;
		if (angles[1] < 0.0f) angles[1] += 360.0f;
	}

	angles[2] = 0.0f;
#endif
	float tmp, yaw, pitch;

	if (forward[2] == 0 && forward[0] == 0)
	{
		yaw = 0;

		if (forward[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);

		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / M_PI);

		if (pitch < 0)
			pitch += 360;
	}

	if (pitch > 180)
		pitch -= 360;
	else if (pitch < -180)
		pitch += 360;

	if (yaw > 180)
		yaw -= 360;
	else if (yaw < -180)
		yaw += 360;

	if (pitch > 89)
		pitch = 89;
	else if (pitch < -89)
		pitch = -89;

	if (yaw > 180)
		yaw = 180;
	else if (yaw < -180)
		yaw = -180;

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

char* pg_balreadymoved = (char*)0x44b7a781;
#define g_balreadymoved *pg_balreadymoved
qboolean* msg_badread = (qboolean*)0x43482264;

#define host_client ((client_t*) *(int*)0x448be188)
//sizebuf_t* net_message = (sizebuf_t*)0x448ea9b8;
//int* pkey_dest = (int*)0x4310B7A8;
#define key_dest *(int*)0x4310B7A8
#define sv_player ((edict_t*) *(int*)0x44b7a778)

#define REHLDS_FIXES

void SV_EstablishTimeBase_New(client_t *cl, usercmd_t *cmds, int dropped, int numbackup, int numcmds)
{
	int cmdnum;
	double runcmd_time;

	runcmd_time = 0.0;
	cmdnum = dropped;
	if (dropped < 24)
	{
		if (dropped > numbackup)
		{
			cmdnum = dropped - (dropped - numbackup);
			if (cl->lastcmd.impact_index)
			{
				runcmd_time = (double)cl->lastcmd.impact_position[0] * (dropped - numbackup) / 1000.0;
			}
			else
			{
				runcmd_time = (double)cl->lastcmd.msec * (dropped - numbackup) / 1000.0;
			}
		}

		for (; cmdnum > 0; cmdnum--)
		{
			int index = cmdnum - 1 + numcmds;
			if (cmds[index].impact_index)
			{
				runcmd_time += (double)cmds[index].impact_position[0] / 1000.0;
			}
			else
			{
				runcmd_time += cmds[index].msec / 1000.0;
			}
		}
	}

	for (; numcmds > 0; numcmds--)
	{
		int index = numcmds - 1;
		if (cmds[index].impact_index)
		{
			runcmd_time += (double)cmds[index].impact_position[0] / 1000.0;
		}
		else
		{
			runcmd_time += cmds[index].msec / 1000.0;
		}
	}

	cl->svtimebase = get_frametime + g_psv->time - runcmd_time;
}

void SV_ParseMove(client_t *pSenderClient)
{
	client_frame_t *frame;
	int placeholder;
	int mlen;
	unsigned int packetLossByte;
	int numcmds;
	int totalcmds;
	byte cbpktchecksum;
	usercmd_t *cmd;
	usercmd_t cmds[64];
	usercmd_t cmdNull;
	int packet_loss;
	byte cbchecksum;
	int numbackup;

	if (*pg_balreadymoved)
	{
		*msg_badread = 1;
		return;
	}
	*pg_balreadymoved = 1;

	client_t * client = host_client;
	edict_t * svplayer = sv_player;

	frame = &client->frames[SV_UPDATE_MASK & client->netchan.incoming_acknowledged];
	Q_memset(&cmdNull, 0, sizeof(cmdNull));
	//memset(&cmdNull, 0, sizeof(cmdNull));

	placeholder = *msg_readcount + 1;
	mlen = MSG_ReadByte();
	cbchecksum = MSG_ReadByte();

	COM_UnMunge(&net_message->data[placeholder + 1], mlen, client->netchan.incoming_sequence);

	packetLossByte = MSG_ReadByte();
	numbackup = MSG_ReadByte();
	numcmds = MSG_ReadByte();

	packet_loss = packetLossByte & 0xFFFFFF7F;//0x7F);
	//pSenderClient->m_bLoopback = (packetLossByte >> 7) & 1;
	
	*(int*)&pSenderClient->netchan.remote_address.ipx[0] = packet_loss;

	totalcmds = numcmds + numbackup;
	*net_drop = *net_drop + (1 - numcmds);
	if (totalcmds < 0 || totalcmds >= (CMD_MAXBACKUP - 1))
	{
		Con_Printf("SV_ReadClientMessage: too many cmds %i sent for %s/%s\n", totalcmds, client->name, NET_AdrToString(host_client->netchan.remote_address));
		SV_DropClient(client, FALSE, "CMD_MAXBACKUP hit");
		*msg_badread = 1;
		return;
	}

	usercmd_t* from = &cmdNull;
#if 0
	int i = totalcmds - 1;
	if (i >= 0)
	{
		int cmdtarg = (int)&cmds[i];
		int num = i + 1;
		do
		{
			MSG_ReadUsercmd((usercmd_t*)cmdtarg, from);
			from = (usercmd_t*)cmdtarg;
			cmdtarg -= 56;
			--num;
		} while (num);
	}
#else
	for (int i = totalcmds - 1; i >= 0; i--)
	{
		MSG_ReadUsercmd(&cmds[i], from);
		from = &cmds[i];
	}
#endif


	if (!g_psv->active || !(client->active || client->spawned))
		return;

	if (msg_badread)
	{
		Con_Printf("Client %s:%s sent a bogus command packet\n", client->name, NET_AdrToString(client->netchan.remote_address));
		return;
	}

	cbpktchecksum = COM_BlockSequenceCRCByte2(&net_message->data[placeholder + 1], *msg_readcount - placeholder - 1, host_client->netchan.incoming_sequence);
	if (cbpktchecksum != cbchecksum)
	{
		Con_DPrintf("Failed command checksum for %s:%s\n", client->name, NET_AdrToString(client->netchan.remote_address));
		*msg_badread = 1;
		return;
	}

	client->packet_loss = packet_loss;

	if (g_psv->paused || g_psvs->maxclients <= 1 && key_dest || (*(DWORD *)(svplayer + 708) & 0x10))//svplayer->v.flags & 0x10)
	{
		if (numcmds > 0)
		{
			usercmd_t* cmd = cmds;
			int cmdsleft = numcmds;
			do
			{
				*(char*)((DWORD)cmd + 0x2C) = 0;
				cmd->viewangles.x = 0;
				cmd->viewangles.y = 0;
				cmd->viewangles.z = 0;
				int flags = *(DWORD *)(svplayer + 708);
				if (flags & 0x10)
					LOBYTE(cmd->upmove) = 0;
				*(DWORD *)(svplayer + 0x394) = *(DWORD*)((DWORD)cmd + 0x30);
				DWORD v20 = (DWORD)svplayer + 0x394;
				*(DWORD *)(v20 + 4) = *(DWORD*)((DWORD)cmd + 0x34);
				*(DWORD *)(v20 + 8) = *(DWORD *)&cmd->lerp_msec;
				*(DWORD*)((DWORD)cmd + 0x30) = *(DWORD *)(svplayer + 404);
				*(DWORD*)((DWORD)cmd + 0x34) = *(DWORD *)(svplayer + 408);
				*(DWORD *)&cmd->lerp_msec = *(DWORD *)(svplayer + 412);
				++cmd;
				--cmdsleft;
			} while (cmdsleft);
		}
		net_drop = 0;
#if 0
		for (int i = 0; i < numcmds; i++)
		{
			cmd = &cmds[i];	
			*(char*)((DWORD)cmd + 0x2C) = 0; //real impact_position.x
			cmd->viewangles[0] = 0; //svplayer->v.v_angle[0];
			cmd->viewangles[1] = 0; //svplayer->v.v_angle[1];
			cmd->viewangles[2] = 0; //svplayer->v.v_angle[2];
			if (svplayer->v.flags & 0x10)//FL_FROZEN)
				cmd->upmove = 0; //supposed to be impulse on HL1
			*(DWORD*)((DWORD)svplayer + 0x394) = *(DWORD*)((DWORD)cmd + 0x30); //real impact_position.y
			*(DWORD*)((DWORD)svplayer + 0x398) = *(DWORD*)((DWORD)cmd + 0x30); //real impact_position.y
			*(DWORD*)((DWORD)svplayer + 0x394) = *(DWORD*)((DWORD)cmd + 0x30); //real impact_position.y

			cmd->msec = 0;
			cmd->forwardmove = 0;
			cmd->sidemove = 0;
			cmd->upmove = 0;
			cmd->buttons = 0;
			cmd->impact_index = 0;
			cmd->impact_position[0] = 0.0f;
			cmd->impact_position[1] = 0.0f;
			cmd->impact_position[2] = 0.0f;

		

			cmd->viewangles[0] = 0; //svplayer->v.v_angle[0];
			cmd->viewangles[1] = 0; //svplayer->v.v_angle[1];
			cmd->viewangles[2] = 0; //svplayer->v.v_angle[2];
		}
		net_drop = 0;
#endif
	}
	else
	{
		svplayer->v.v_angle[0] = cmds[0].viewangles[0];
		svplayer->v.v_angle[1] = cmds[0].viewangles[1];
		svplayer->v.v_angle[2] = cmds[0].viewangles[2];
		svplayer->v.vuser3[0] = cmds[0].viewangles[0]; //FIXME: ROCKET LAUNCHER GUIDED MISSILE MOUSE FIX: WHY DO WE HAVE TO PUT THIS HERE!! ORIGINAL GAME DOES NOT DO THIS
		svplayer->v.vuser3[1] = cmds[0].viewangles[1]; //NIGHTFIRE SETS THIS INSIDE SV_RUNCMD BUT IT'S ALWAYS NULL FOR US!!
		svplayer->v.vuser3[2] = cmds[0].viewangles[2];
	}
#if 0
	if (!g_psv->paused && (g_psvs->maxclients > 1 || !key_dest) && !(svplayer->v.flags & 0x10))// !(svplayer->v.flags & FL_FROZEN))
	{
		svplayer->v.v_angle[0] = cmds[0].viewangles[0];
		svplayer->v.v_angle[1] = cmds[0].viewangles[1];
		svplayer->v.v_angle[2] = cmds[0].viewangles[2];
	}
	else
	{
		for (int i = 0; i < numcmds; i++)
		{
			cmd = &cmds[i];
			cmd->msec = 0;
			cmd->forwardmove = 0;
			cmd->sidemove = 0;
			cmd->upmove = 0;
			cmd->buttons = 0;
			cmd->impact_index = 0;
			cmd->impact_position[0] = 0.0f;
			cmd->impact_position[1] = 0.0f;
			cmd->impact_position[2] = 0.0f;

			if (svplayer->v.flags & 0x10)//FL_FROZEN)
				cmd->impulse = 0;

			cmd->viewangles[0] = svplayer->v.v_angle[0];
			cmd->viewangles[1] = svplayer->v.v_angle[1];
			cmd->viewangles[2] = svplayer->v.v_angle[2];
		}

		net_drop = 0;
	}
#endif

	//check move commands rate for this player
#if 0
#ifdef REHLDS_FIXES
	int numCmdsToIssue = numcmds;
	if (net_drop > 0) {
		numCmdsToIssue += net_drop;
	}
	g_MoveCommandRateLimiter.MoveCommandsIssued(client - g_psvs.clients, numCmdsToIssue);

	if (!client->connected) {
		return; //return if player was kicked
	}
#endif
#endif


#ifndef REHLDS_FIXES
	// dup and more correct in SV_RunCmd

	//this fix was added in the engine.dll assembly as of 5.9.9.0 client

	svplayer->v.button = cmds[0].buttons;
	svplayer->v.light_level = cmds[0].lightlevel;
#endif

	SV_EstablishTimeBase_New(client, cmds, *net_drop, numbackup, numcmds);
	if (*net_drop < 24)
	{
		while (*net_drop > numbackup)
		{
			SV_RunCmd(&client->lastcmd, 0);
			*net_drop = *net_drop - 1;
		}

		while (*net_drop > 0)
		{
			SV_RunCmd(&cmds[numcmds + *net_drop - 1], client->netchan.incoming_sequence - (numcmds + *net_drop - 1));
			*net_drop = *net_drop - 1;
		}

	}

	for (int i = numcmds - 1; i >= 0; i--)
	{
		SV_RunCmd(&cmds[i], client->netchan.incoming_sequence - i);
	}

#ifdef REHLDS_FIXES
	if (numcmds)
		//memcpy(&client->lastcmd, &cmds[numcmds - 1], 0x38);
		client->lastcmd = cmds[numcmds - 1];
	else if (numbackup)
		//memcpy(&client->lastcmd, &cmds[0], 0x38);
		client->lastcmd = cmds[0];
#else
	client->lastcmd = cmds[0];
#endif

	if (client->lastcmd.impact_index)
	{
		frame->ping_time -= float((client->lastcmd.impact_position[0] * 0.5) / 1000.0);
	}
	else
	{
		frame->ping_time -= float((client->lastcmd.msec * 0.5) / 1000.0);
	}
	if (frame->ping_time < 0.0f)
		frame->ping_time = 0.0f;

	if (svplayer->v.animtime > get_frametime + g_psv->time)
		svplayer->v.animtime = float(get_frametime + g_psv->time);
}

#define DYLAN_FIXES

void CL_SetMoveVars(cmd_t* cmd)
{
	//unsigned __int64 lerp = (unsigned __int64)((*cl_time - *cl_mtime) * 1000.0);

	cmd->cmd.lerp_msec = (short)(CVAR_GETDIRECTFLOAT(ex_interp) * 1000.0f); //(short)lerp; //(short)(CVAR_GETDIRECTFLOAT(ex_interp) * 1000.0f);

	cmd->cmd.lightlevel = *light_level2;

#ifdef OLD_NF
	double msec = get_frametime * 1000.0;
	cmd->cmd.msec = (short)msec;
#else
		double msec = get_frametime * 1000.0;
#if 0
		double var_44;
		double var8;
		int var8plus4;
		int imsec = (int)msec;
		const double one = 1.0;
		const double twofiftyfive = 255.0;
		static double storage = 0.0;
		__asm {
			mov eax, imsec
			mov var8plus4, eax
			fild var8plus4
			fstp var_44
			fld msec
			fsub var_44
			fadd storage
			fstp storage
			fld storage
			fcomp one
			fnstsw ax
			and eax, 0x4100
			jnz jmp1
			fld storage
			fsub one
			inc var8plus4
			fstp storage
			jmp1 :
			fild var8plus4
				fstp msec
		}
#else
		static double storage = 0.0;

		int imsecmod = (int)msec;
		double imsecdb = (double)imsecmod;

		storage = msec - imsecdb + storage;

		if (storage >= 1)
		{
			storage -= 1;
			imsecmod++;
		}
		msec = (double)imsecmod;
#endif
		if (msec > 255.0)
			msec = 255.0;
		else if (msec < 0.0)
			msec = 1.0;
		cmd->cmd.msec = (short)msec;
		#ifdef DYLAN_FIXES
			cmd->cmd.impact_index = 1;
			cmd->cmd.impact_position.x = (float)(get_frametime * 1000.0);
			cmd->cmd.impact_position.y = (float)g_pcl->mtime[0];
		#endif
#endif
}

typedef struct con_nprint_s
{
	int index;						// Row #
	float	time_to_live;			// # of seconds before it disappears
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char alpha;
} con_nprint_t;

void CL_Move(void){
	if (*bNotFinalTick)
		return; //Until we can get goldsrc to behave like source
	sizebuf_t buf;
	byte data[MAX_USERCMD_BUFFER];

	if (g_pcls->state == ca_dedicated || g_pcls->state == ca_disconnected || g_pcls->state == ca_connecting)
		return;

	// only send packets on the final tick in one engine frame
	bool bSendPacket = true;

	// Don't create usercmds here during playback, they were encoded into the packet already
	if (DemoIsPlayingBack())
		return;

	if (g_pcls->state == ca_connected)
	{
		Netchan_Transmit(&g_pcls->netchan, NULL, NULL);
		return;
	}

	bf_init(&buf, "Client Move", data, sizeof(data));
	CL_ComputePacketLoss();

	// don't send packets if update time not reached or chnnel still sending
	// in loopback mode don't send only if host_limitlocal is enabled

	if ((g_pcls->netchan.remote_address.type != NA_LOOPBACK || CVAR_GETDIRECTBOOL(host_limitlocal)) &&
		((realtime < g_pcls->nextcmdtime) || !Netchan_CanPacket(&g_pcls->netchan) || *bNotFinalTick))
	{
		bSendPacket = false;
	}

	if (g_pcls->state == ca_active || g_pcls->state == ca_uninitialized || g_pcls->state == ca_connected)
	{
		cmd_t *cmd;
		cmd = &CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK];
		cmd->senttime = realtime;
		cmd->receivedtime = -1.0f;
		cmd->processedfuncs = false;
		// Assume we'll transmit this command this frame ( rather than queueing it for next frame )
		cmd->heldback = false;
		cmd->sendsize = 0;
		Q_memset(&cmd->cmd, 0, sizeof(usercmd_t));

		CL_SetSolidEntities();
		CL_PushPMStates();
		CL_SetSolidPlayers(*LocalPlayerNum);
		double accumulated_extra_samples = *(double*)0x44BAB212;
		CL_CreateMove((float)(get_frametime - accumulated_extra_samples), &cmd->cmd, g_pcls->signon == 2);
		CL_PopPMStates();

		CL_SetMoveVars(cmd);

#if 0
		//if (GetAsyncKeyState(VK_SPACE) & (1 << 16))
		if (cmd->cmd.buttons & IN_JUMP)
		{
			for (int i = 1; i <= 32; ++i)
			{
			auto ent = g_engfuncs.pfnPEntityOfEntIndex(i);
			{
				int test = 1;
			}
			}
			cl_entity_t* pPlayer = GetLocalPlayer();
			if (pPlayer)
			{
				if (pmove)
				{
					if (pmove->flags & FL_ONGROUND)
					{
						cmd->cmd.buttons |= IN_JUMP;
						cmd->cmd.buttons &= ~IN_DUCK;
					}
					else
					{
						cmd->cmd.buttons &= ~IN_JUMP;
					}
					//strafe hack
					if (*mousedx < 0.0f)
					{
						cmd->cmd.buttons |= IN_MOVELEFT;
						cmd->cmd.sidemove = -400.0f;
					}
					else if (*mousedx > 0.0f)
					{
						cmd->cmd.buttons |= IN_MOVERIGHT;
						cmd->cmd.sidemove = 400.0f;
					}
				}
			}
		}
#if 0
		Vector vecMove(cmd->cmd.forwardmove, cmd->cmd.sidemove, 0.0f);
		float flLength = vecMove.Length();
		if (flLength > 0.0f)
		{
			Vector angMoveReverse;
			VectorAngles(-vecMove, angMoveReverse);
			cmd->cmd.forwardmove = -flLength;
			cmd->cmd.sidemove = 0.0f; // Move only backwards, no sidemove
			cmd->cmd.viewangles[1] = cmd->cmd.viewangles.y - angMoveReverse.y; //AngleDiff(cmd->viewangles1, angMoveReverse1);
			cmd->cmd.viewangles[2] = 89.0f; // OMFG SUPER 1337 SPEEDHAQ METHODS 8)
		}
#endif
#endif
		//record demo file

		Q_memcpy((void*)0x431B8790, &cmd->cmd, sizeof(usercmd_t)); //?

	//if (!IsDedicatedServer())
		CL_PredictionSetupMove(0);

		if (bSendPacket)
		{
			CL_SendMove(cmd, &buf, data);
		}
		else
		{
#if 0
			if (*bNotFinalTick)
			{
				return;
				//CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].senttime = realtime; //1ms
				//CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].cmd.msec = 1;
				//CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].cmd.impact_index = 1;
				//CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].cmd.impact_position[0] = 0.1f;
				//CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].cmd.forwardmove = 0.0f;
				//CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].cmd.sidemove = 0.0f;
			}
#endif
			// Mark command as held back so we'll send it next time
			CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].heldback = true;
			// increment sequence number so we can detect that we've held back packets.
			g_pcls->netchan.outgoing_sequence++;
			return;//added to avoid extra instructions
		}
	}

	if (!bSendPacket)
		return;

	// Request non delta compression if high packet loss, show warning message
	bool hasProblem = ( (g_pcls->netchan.outgoing_sequence - g_pcls->netchan.incoming_acknowledged) >= CL_UPDATE_MASK
		|| (realtime - g_pcls->netchan.last_received) > CONNECTION_PROBLEM_TIME ) 
		&& !DemoIsPlayingBack() 
		&& g_pcls->state == ca_active;

	// Request non delta compression if high packet loss, show warning message
	if (hasProblem)
	{
		int timeout = CVAR_GETDIRECTINT(cl_timeout);
		float flRemainingTime = (g_pcls->netchan.last_received + timeout) - realtime;
		con_nprint_t format;
		format.index = 0;
		format.time_to_live = 1.0f;
		format.red = 255;
		format.green = 50;
		format.blue = 0;
		format.alpha = 255;

		Con_NXPrintf((int)&format, "WARNING:  Connection Problem");
		format.index = 2;
		char txt[256];
		sprintf(txt, "Auto-disconnect in %.1f seconds", flRemainingTime);
		Con_NXPrintf((int)&format, txt);
		g_pcl->validsequence = 0;
	}

	if (!g_pcl->validsequence)
		// Tell server to send us an uncompressed update
		*cl_delta_sequence = -1;
	else
	{
		if (CVAR_GETDIRECTBOOL(cl_nodelta) == 0 && g_pcls->state == ca_active)
		{
			*cl_delta_sequence = g_pcl->validsequence;
			MSG_WriteByte(&buf, clc_delta);
			MSG_WriteByte(&buf, g_pcl->validsequence & 0xFF);
		}
	}

	// remember outgoing command that we are sending
	g_pcls->lastoutgoingcommand = g_pcls->netchan.outgoing_sequence;

	CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].sendsize = buf.cursize;

	// composite the rest of the datagram..
	int buffersize = buf.maxsize;
	int cursize = buf.cursize;

	if (g_pcls->datagram.cursize <= buffersize - cursize)
	{
		Q_memcpy(&buf.data[cursize], &g_pcls->datagram.data, g_pcls->datagram.cursize);
		cursize = g_pcls->datagram.cursize + buf.cursize;
		buf.cursize += g_pcls->datagram.cursize;
	}

	g_pcls->datagram.cursize = 0; //clear buffer
								  // deliver the message (or update reliable)

	Netchan_Transmit(&g_pcls->netchan, cursize, buf.data);

	// calc next packet send time

	if (g_pcls->state == ca_active)
	{
		if (CVAR_GETDIRECTINT(cl_cmdrate) <= 0)
			g_pcls->nextcmdtime = realtime;
		else
		{
			// use full update rate when active
			float commandInterval = 1.0f / CVAR_GETDIRECTINT(cl_cmdrate);
			float maxDelta = min((float)get_frametime, commandInterval);
			float delta = clamp(realtime - g_pcls->nextcmdtime, 0.0f, maxDelta);
			g_pcls->nextcmdtime = realtime + commandInterval - delta;
		}
	}
	else
	{
		// during signon process send only 5 packets/second
		g_pcls->nextcmdtime = realtime + (1.0f / 5.0f);
	}

	//g_pcls->netchan.outgoing_sequence = 0;

}

void __stdcall CL_ExtraMouseSample(void)
{
	if (*bNotFinalTick || IsDedicatedServer() || g_pcls->state == ca_dedicated || g_pcls->state != ca_active || DemoIsPlayingBack())
		return;
	if (get_frametime <= 0.0)
		return;

	usercmd_t dummy;
	Q_memset(&dummy, 0, sizeof(usercmd_t));
	double msec = get_frametime * 1000.0;
#if 1
	double var_44;
	double var8;
	int var8plus4;
	int imsec = (int)msec;
	const double one = 1.0;
	const double twofiftyfive = 255.0;
	static double storage = 0.0;
	__asm {
		mov eax, imsec
		mov var8plus4, eax
		fild var8plus4
		fstp var_44
		fld msec
		fsub var_44
		fadd storage
		fstp storage
		fld storage
		fcomp one
		fnstsw ax
		and eax, 0x4100
		jnz jmp1
		fld storage
		fsub one
		inc var8plus4
		fstp storage
		jmp1 :
		fild var8plus4
			fstp msec
	}
	if (msec > 255.0)
		msec = 255.0;
	else if (msec < 0.0)
		msec = 1.0;
#endif
	dummy.msec = (short)msec;
	dummy.lerp_msec = (short)(CVAR_GETDIRECTFLOAT(ex_interp) * 1000.0f);

	CL_CreateMove(get_frametime, &dummy, g_pcls->signon == 2); //TODO: FIX SIGNON SO THAT ITS PAUSED INSTEAD
}

__declspec(naked) void hookOnCL_Move(void) {
	__asm {
		push ebx
		push esi
		push edi
		call CL_Move
		pop edi
		pop esi
		pop ebx
		retn
	}
}

void* jmpbackadr = (void*)ADR_HOST_UPDATESCREEN_JMPBACK;
#define gfBackground 0x43494fd4
__declspec(naked) void hookOnHost_UpdateScreen(void) {
	__asm {
		call CL_ExtraMouseSample
		MOV AL, BYTE PTR DS : [gfBackground]
		jmp jmpbackadr
	}
}

float(*UTIL_AngleMod_NF) (float a) = (float(*)(float))0x42058A10;
//CBaseMonster class

void* SetBoneController = (void*)0x42015300;
//int bits_CAP_TURN_HEAD = 4

float YawTimeForEdict[MAX_EDICTS] = { 0.0f }; //todo: this is nasty and a memory hog, fix it
#define MAX_YAW_SPEED 0.015625 //64 fps

float ChangeYaw(C_BaseEntityCustom *character, int yawSpeed)
{
	int index = ENTINDEX(character->edict());

	if (YawTimeForEdict[index] == 0.0f)
		YawTimeForEdict[index] = gpGlobals->time;

	if (gpGlobals->time < YawTimeForEdict[index])
		return 0.0f;

	int m_afCapability = *(int*)((int)character + 0x2B0);
	float		ideal, current, speed;
	float move = 0.0f;
	entvars_t* pev = character->pev;

	current = UTIL_AngleMod_NF(pev->angles.y);
	ideal = pev->ideal_yaw;

	if (current != ideal)
	{
		if (ideal > 180.0f)
			ideal -= 360.0f;
		if (ideal < -180.0f)
			ideal += 360.0f;

		speed = (float)yawSpeed * MAX_YAW_SPEED * 10.0f;
		move = ideal - current;

		// Make frame rate independent

		if (ideal > current)
		{
			if (move >= 180.0f)
				move = move - 360.0f;
		}
		else
		{
			if (move <= -180.0f)
				move = move + 360.0f;
		}

		if (move > 0)
		{// turning to the monster's left
			if (move > speed)
				move = speed;
		}
		else
		{// turning to the monster's right
			if (move < -speed)
				move = -speed;
		}

		pev->angles.y = UTIL_AngleMod_NF(current + move);
	

		if (pev->angles.y > 360.0f) pev->angles.y = 0.0f;

		// turn head in desired direction only if they have a turnable head
		if (m_afCapability & bits_CAP_TURN_HEAD)
		{
			float yaw = pev->angles.y;
			__asm {
				mov ecx, character
				push yaw
				push 0
				call SetBoneController
			}
		}
		// use full update rate when active
		float maxDelta = min((float)get_frametime, MAX_YAW_SPEED);
		float delta = clamp(gpGlobals->time - YawTimeForEdict[index], 0.0f, maxDelta);
		YawTimeForEdict[index] = gpGlobals->time + MAX_YAW_SPEED - delta;
	}
	else
		move = 0;

	return move;
}

__declspec (naked) void hookOnChangeYaw ()
{
	__asm {
		mov eax, [esp + 4]
		push eax
		push ecx
		call ChangeYaw
		add esp, 8
		ret 4
	}
}

struct cl_enginefuncs_t
{
	void* blah[16];
	void(__cdecl* TraceFunction)(Vector const*, Vector const*, int, int, int, DWORD);
};

cl_enginefuncs_t **pgEngFuncs = (cl_enginefuncs_t**)0x410A8A20;
#define gEngFuncs (*pgEngFuncs)

void __stdcall RainDropTrace(DWORD ESI, Vector const* vecStart,Vector const* vecEnd, int unknown, int hullSize , int pSkip, DWORD tr)
{
	gEngFuncs->TraceFunction(vecStart, vecEnd, unknown, hullSize, pSkip, tr);

	float groundzpos = *(float*)(tr + 20);
	*(float*)(ESI + 0x9C) = groundzpos + 3.0f;

	gEngFuncs->TraceFunction(vecStart, vecEnd, 0x100000, 49, pSkip, tr);
	float waterzpos = *(float*)((DWORD)tr + 20);
	if (waterzpos > groundzpos)
		*(float*)(ESI + 0x9C) = waterzpos + 6.0f;
}

void* rainjmpbackadr = (void*)ADR_RAINDROP_TRACE_JMPBACK;

__declspec (naked) void hookOnRaindropTraceReturn()
{
	__asm {
		FSTP DWORD PTR SS:[ESP+0x2C]
		push ebx
		push ecx
		push -1
		push 2
		push 0
		push edx
		push eax
		push esi
		call RainDropTrace
		pop ebx
		mov al, bl
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 0x88
		retn
		//jmp rainjmpbackadr
	}
}

#if 0
#include "net_chan.h"

sizebuf_t* __stdcall OnMulticast(client_t *client, BOOL reliable)
{
	int index = GetIndexFromClientPointer(client);
	sizebuf_t *pBuffer = &g_ExtendedClient[index].netchan.message; //&client->netchan.message; //0x64

	if (!reliable)
		return  &g_ExtendedClient[index].datagram; //&client->datagram; //0x25A0
	
	return pBuffer;
}

void* sv_multicastjumpback = (void*)0x430899D4;
__declspec (naked) void SV_Multicast()
{
	__asm
	{
		MOVZX EAX, BYTE PTR SS : [ESP + 0x28]
		push eax
		push esi
		call OnMulticast
		jmp sv_multicastjumpback
	}
}

void* movevarschangedjmpback = (void*)0x430899D4;
void __stdcall OnQueryMoveVarsChanged(client_t *client)
{
	int index = GetIndexFromClientPointer(client);
	((void(__cdecl*)(sizebuf_t*))0x43083480)(&g_ExtendedClient[index].netchan.message /*&client->netchan.message*/); //SV_WriteMovevarsToClient
}

__declspec (naked) void SV_QueryMoveVarsChanged(client_t* client)
{
	__asm {
		push esi
		call OnQueryMoveVarsChanged
		jmp movevarschangedjmpback
	}
}

void* svnewjmpback = (void*)0x43089DFE;

void __stdcall OnSvNew(client_t* client)
{
	int index = GetIndexFromClientPointer(client);
	auto& cl = g_ExtendedClient[index];

	SZ_Clear(&cl.netchan.message);
	SZ_Clear(&cl.datagram);
	Netchan_Clear(&cl.netchan);
}

__declspec (naked) void SV_New_f()
{
	__asm {
		push eax
		call OnSvNew
		jmp svnewjmpback
	}
}

//todo: finish these two
void __stdcall OnSvNew2(client_t* client)
{
	int index = GetIndexFromClientPointer(client);
	auto& cl = g_ExtendedClient[index];

	SZ_Clear(&cl.netchan.message);
	SZ_Clear(&cl.datagram);
	Netchan_Clear(&cl.netchan);
}

__declspec (naked) void SV_New_f2()
{
	__asm {
		push eax
		call OnSvNew
		jmp svnewjmpback
	}
}

#endif

HookCallInfo funcHooks[] = {
	{ (BYTE*)ADR_CL_MOVE, (DWORD)CL_Move, 6, (BYTE*)ADR_CL_MOVE + 0x6 },
	{ (BYTE*)ADR_HOST_UPDATESCREEN, (DWORD)hookOnHost_UpdateScreen, 5, (BYTE*)ADR_HOST_UPDATESCREEN + 0x5 },
	//{ (BYTE*)ADR_SV_PARSEMOVE, (DWORD)SV_ParseMove, 5, (BYTE*)ADR_SV_PARSEMOVE + 0x5 },
	{ (BYTE*)ADR_CBASEMONSTER_CHANGEYAW , (DWORD)hookOnChangeYaw, 5, (BYTE*)ADR_CBASEMONSTER_CHANGEYAW + 0x5},

	//CLIENT ONLY
	//{ (BYTE*)ADR_RAINDROP_TRACE_RETURN , (DWORD)hookOnRaindropTraceReturn, 5, (BYTE*)ADR_RAINDROP_TRACE_RETURN + 0x5},

	//{ (BYTE*)0x43064B00, (DWORD)Netchan_ClearFragments, 5, (BYTE*)0x43064B00 + 0x5},
	//{ (BYTE*)0x43064BC0 , (DWORD)Netchan_Clear, 5, (BYTE*)0x43064BC0 + 0x5},
	//{ (BYTE*)0x43064C30  , (DWORD)Netchan_Setup, 5, (BYTE*)0x43064C30 + 0x5 },
	//{ (BYTE*)0x430644C0  , (DWORD)Netchan_CanPacket, 5, (BYTE*)0x430644C0 + 0x5 },
	//{ (BYTE*)0x43064510  , (DWORD)Netchan_UpdateFlow, 5, (BYTE*)0x43064510 + 0x5 },
	//{ (BYTE*)0x43065400  , (DWORD)Netchan_Transmit, 5, (BYTE*)0x43065400 + 0x5 },
	//{ (BYTE*)0x43064900   , (DWORD)Netchan_FindBufferById, 5, (BYTE*)0x43064900 + 0x5 },
	//{ (BYTE*)0x430646B0   , (DWORD)Netchan_CheckForCompletion, 5, (BYTE*)0x430646B0 + 0x5 },
	//{ (BYTE*)0x43064D20   , (DWORD)Netchan_Process, 5, (BYTE*)0x43064D20 + 0x5 },
	//{ (BYTE*)0x430648B0   , (DWORD)Netchan_FragSend, 5, (BYTE*)0x430648B0 + 0x5 },
	//{ (BYTE*)0x43065270   , (DWORD)Netchan_AllocFragbuf, 5, (BYTE*)0x43065270 + 0x5 },
	//{ (BYTE*)0x430653C0   , (DWORD)Netchan_CreateFragments, 5, (BYTE*)0x430653C0 + 0x5 },
	//{ (BYTE*)0x43064A00   , (DWORD)Netchan_CopyNormalFragments, 5, (BYTE*)0x43064A00 + 0x5 },
	//{ (BYTE*)0x43064A90  , (DWORD)Netchan_IncomingReady, 5, (BYTE*)0x43064A90 + 0x5 },
	//{ (BYTE*)0x430899C3  , (DWORD)SV_Multicast, 12, (BYTE*)0x430899D4},
	//{ (BYTE*)0x43083B1D  , (DWORD)SV_QueryMoveVarsChanged, 7, (BYTE*)0x43083B29},
	//{ (BYTE*)0x43089DD4 , (DWORD)SV_New_f, 37, (BYTE*)0x43089DFE },
	//{ (BYTE*)0x43089EEC , (DWORD)SV_New_f2, 37, (BYTE*)0x43089F13 },


};

BYTE* getHookFuncJMPAddr(DWORD fnName) {
	for(int i=0;i<sizeof(funcHooks)/sizeof(HookCallInfo);i++) {
		if(funcHooks[i].retFunc == fnName) {
			return (BYTE*)funcHooks[i].retJMPAddr;
		}
	}
	printf("Error: One or more of the provided function addresses weren't found\n");
	return 0;
}

void HookFunctions() {
	for(int i=0;i<sizeof(funcHooks)/sizeof(HookCallInfo);i++) {
		PlaceJMP((BYTE*)funcHooks[i].address, funcHooks[i].retFunc, funcHooks[i].len);
	}
}

// END OF HOOKS

/*
class CBombTarget : public CBaseEntityCustom {
	void Spawn (void) { printf("SPAWNED BOMBTARGET!\n"); }
};

C_DLLEXPORT void bomb_target(void) {
	//this works but causes a crash because I think it needs LINK_ENTITY_TO_CLASS which also crashes because pContainingEntity is invalid
	printf("Bomb Target Found\n");
}

LINK_ENTITY_TO_CLASS(func_bomb_target, CBombTarget ); //0x4212E584 CBaseEntity
*/

/*void callOnPlayerTakeDamage(entvars_t2 *pevInflictor, entvars_t2 *pevAttacker, float flDamage, int bitsDamageType) {
	//Todo: What do we need here?
}*/

extern cvar_t_small sv_filetransfermaxsize_s;
extern int* sv_filetransfermaxsize;

void OnModuleInitialize(void) {
	HookFunctions();
	sv_filetransfermaxsize = (int*)CVAR_REGISTER(sv_filetransfermaxsize_s);
	g_fn_PrintSrvConsole("Move Fixes Initiated\n");
}

void OnKeyValue(edict_t *pEntity, KeyValueData *pkvd) {
}

void OnServerDeactivate(void) {
}

void OnServerActivate(void) {
	memset(YawTimeForEdict, 0, sizeof(YawTimeForEdict));
}

void OnClientKill(edict_t *pEntity) {
	//suicide only, we hooked the "Killed" function manually
}

void OnClientConnect(edict_t *pEntity) {
	//client_t *pClient = EDICT_TO_CLIENT(pEntity);
	/*
	int index = ENTINDEX(pEntity);
	client_t *pClient = EDICT_TO_CLIENT(pEntity);
	if ( !pClient->fakeclient ) {
		glinfo.iNumPlayers++;
		plinfo[index].bFakeClient = false;
	} else {
		plinfo[index].bFakeClient = true;
	}
	*/
}

void OnClientDisconnect(edict_t *pEntity) {
	/*
	int index = ENTINDEX(pEntity);
	client_t *pClient = EDICT_TO_CLIENT(pEntity);
	if ( !pClient->fakeclient )
		glinfo.iNumPlayers--;
	plinfo[index].bFakeClient = false;
	plinfo[index].iTeam = 0;
	*/
}


//Called when a server runs a frame
void OnStartFrame(void) {
}

//Called after a server runs a frame
void OnPostFrame(void) {
	//g_LagCompensationManager.FrameUpdatePostEntityThink();
}

//Called when server thinks on a player
void OnPlayerPreThink(edict_t *pEntity) {
}

//Called when a client types a command in the console
void OnClientCommand(edict_t *pEntity, int argl, const char *szCommand) {
	//if ( !strcmpi(szCommand, "dropbomb") ) {
	//}
}