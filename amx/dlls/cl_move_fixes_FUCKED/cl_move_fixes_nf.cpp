#include "cl_move_fixes.h"
#include "ilagcompensationmanager.h"
#include "utlvector.h"
#include "bitvec.h"
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

void (__cdecl *UnknownCall) (float unknown) = (void(__cdecl*)(float))0x43038C70;


#define LOBYTE(x)   (*((BYTE*)&(x)))   // low byte
void(*Netchan_Transmit) (netchan_t *chan, int length, byte *data) = (void(*)(netchan_t*, int, byte*))0x43065400;
qboolean(*Netchan_CanPacket) (netchan_t *chan) = (qboolean(*)(netchan_t*))0x430644C0;
#define MULTIPLAYER_BACKUP	64	// how many data slots to use when in multiplayer (must be power of 2)
void(*Con_NXPrintf) (int loc, const char*text) = (void(*)(int, const char*))0x43043BD0;
void(*Host_Error) (const char* errorstr, ...) = (void(*)(const char*, ...))0x430593F0;
void(*CL_ComputePacketLoss) (void) = (void(*)(void))0x43032320;
void(*MSG_WriteByte)(sizebuf_t *sb, int c) = (void(*)(sizebuf_t *, int))0x43042210;
void(*CL_WriteUsercmdDeltaToBuffer)(sizebuf_t *msg, usercmd_t* from, usercmd_t* to) = (void(*)(sizebuf_t *, usercmd_t*, usercmd_t*))0x43043000;
int* LocalPlayerNum = (int*)0x432cd374;
void(*CL_SetSolidPlayers)(int playernum) = (void(*)(int))0x4302DF40;
void(*CL_SetSolidEntities)(void) = (void(*)(void))0x4302DAD0;
void(*CL_PushPMStates)(void) = (void(*)(void))0x43038160;
void(*CL_PopPMStates)(void) = (void(*)(void))0x430381A0;
void(*COM_Munge)(unsigned char*data, int len, int seq) = (void(*)(unsigned char*, int, int))0x43041A00;
void(*CL_CreateMove)(float frametime, struct usercmd_s *cmd, int active) = (void(*)(float, struct usercmd_s*, int))0x4302B840;
void(*SZ_Clear)(sizebuf_t* buf) = (void(*)(sizebuf_t*))0x43040260;

byte(*COM_BlockSequenceCRCByte2) (byte *base, int length, int sequence) = (byte(*)(byte*, int, int))0x43046760;
char* (*TranslateMessage2)(const char* str) = (char*(*)(const char*))0x430967B0;

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

void Delta_CustomEncode(delta_info_t *dt, const void *from, const void *to)
{
	int	i;

	ASSERT(dt != NULL);

	// set all fields is active by default
	for (i = 0; i < dt->numFields; i++)
		dt->pFields[i].bInactive = false;

	if (dt->userCallback)
	{
		dt->userCallback(dt->pFields, (const byte*)from, (const byte*)to);
	}
}

delta_info_t* (*Delta_FindStruct)(const char* name) = (delta_info_t*(*)(const char*))0x43049710;

#define BIT( n ) (1U << ( n ))
#define DT_BYTE		BIT( 0 )	// A byte
#define DT_SHORT		BIT( 1 ) 	// 2 byte field
#define DT_FLOAT		BIT( 2 )	// A floating point field
#define DT_INTEGER		BIT( 3 )	// 4 byte integer
#define DT_ANGLE		BIT( 4 )	// A floating point angle ( will get masked correctly )
#define DT_TIMEWINDOW_8	BIT( 5 )	// A floating point timestamp, relative to sv.time
#define DT_TIMEWINDOW_BIG	BIT( 6 )	// and re-encoded on the client relative to the client's clock
#define DT_STRING		BIT( 7 )	// A null terminated string, sent as 8 byte chars
#define DT_SIGNED	BIT( 8 ) // sign modificator

/*
=====================
Delta_ClampIntegerField
prevent data to out of range
=====================
*/
int Delta_ClampIntegerField(int iValue, qboolean bSigned, int bits)
{
	switch (bits)
	{
	case 1:
		iValue = clamp(0, (byte)iValue, 1);
		break;
	case 2:
		if (bSigned) iValue = clamp(-2, (short)iValue, 1);
		else iValue = clamp(0, (word)iValue, 3);
		break;
	case 3:
		if (bSigned) iValue = clamp(-4, (short)iValue, 3);
		else iValue = clamp(0, (word)iValue, 7);
		break;
	case 4:
		if (bSigned) iValue = clamp(-8, (short)iValue, 7);
		else iValue = clamp(0, (word)iValue, 15);
		break;
	case 5:
		if (bSigned) iValue = clamp(-16, (short)iValue, 15);
		else iValue = clamp(0, (word)iValue, 31);
		break;
	case 6:
		if (bSigned) iValue = clamp(-32, (short)iValue, 31);
		else iValue = clamp(0, (word)iValue, 63);
		break;
	case 7:
		if (bSigned) iValue = clamp(-64, (short)iValue, 63);
		else iValue = clamp(0, (word)iValue, 127);
		break;
	case 8:
		if (bSigned) iValue = clamp(-128, (short)iValue, 127);
		else iValue = clamp(0, (word)iValue, 255);
		break;
	case 9:
		if (bSigned) iValue = clamp(-256, (short)iValue, 255);
		else iValue = clamp(0, (word)iValue, 511);
		break;
	case 10:
		if (bSigned) iValue = clamp(-512, (short)iValue, 511);
		else iValue = clamp(0, (word)iValue, 1023);
		break;
	case 11:
		if (bSigned) iValue = clamp(-1024, (short)iValue, 1023);
		else iValue = clamp(0, (word)iValue, 2047);
		break;
	case 12:
		if (bSigned) iValue = clamp(-2048, (short)iValue, 2047);
		else iValue = clamp(0, (word)iValue, 4095);
		break;
	case 13:
		if (bSigned) iValue = clamp(-4096, (short)iValue, 4095);
		else iValue = clamp(0, (word)iValue, 8191);
		break;
	case 14:
		if (bSigned) iValue = clamp(-8192, (short)iValue, 8191);
		else iValue = clamp(0, (word)iValue, 16383);
		break;
	case 15:
		if (bSigned) iValue = clamp(-16384, (short)iValue, 16383);
		else iValue = clamp(0, (word)iValue, 32767);
		break;
	case 16:
		if (bSigned) iValue = clamp(-32768, (short)iValue, 32767);
		else iValue = clamp(0, (word)iValue, 65535);
		break;
	}

	return iValue; // clamped;
}

/*
=====================
Delta_CompareField
compare fields by offsets
assume from and to is valid
TESTTEST: clamp all fields and multiply by specified value before comparing
=====================
*/
qboolean Delta_CompareField(delta_t *pField, void *from, void *to, float timebase)
{
	qboolean	bSigned = (pField->flags & DT_SIGNED) ? true : false;
	float	val_a, val_b;
	int	fromF, toF;

	ASSERT(pField);
	ASSERT(from);
	ASSERT(to);

	if (pField->bInactive)
		return true;

	fromF = toF = 0;

	if (pField->flags & DT_BYTE)
	{
		if (pField->flags & DT_SIGNED)
		{
			fromF = *(signed char *)((byte *)from + pField->offset);
			toF = *(signed char *)((byte *)to + pField->offset);
		}
		else
		{
			fromF = *(byte *)((byte *)from + pField->offset);
			toF = *(byte *)((byte *)to + pField->offset);
		}

		fromF = Delta_ClampIntegerField(fromF, bSigned, pField->bits);
		toF = Delta_ClampIntegerField(toF, bSigned, pField->bits);
		if (pField->multiplier != 1.0f) fromF *= pField->multiplier;
		if (pField->multiplier != 1.0f) toF *= pField->multiplier;
	}
	else if (pField->flags & DT_SHORT)
	{
		if (pField->flags & DT_SIGNED)
		{
			fromF = *(short *)((byte *)from + pField->offset);
			toF = *(short *)((byte *)to + pField->offset);
		}
		else
		{
			fromF = *(word *)((byte *)from + pField->offset);
			toF = *(word *)((byte *)to + pField->offset);
		}

		fromF = Delta_ClampIntegerField(fromF, bSigned, pField->bits);
		toF = Delta_ClampIntegerField(toF, bSigned, pField->bits);
		if (pField->multiplier != 1.0f) fromF *= pField->multiplier;
		if (pField->multiplier != 1.0f) toF *= pField->multiplier;
	}
	else if (pField->flags & DT_INTEGER)
	{
		if (pField->flags & DT_SIGNED)
		{
			fromF = *(int *)((byte *)from + pField->offset);
			toF = *(int *)((byte *)to + pField->offset);
		}
		else
		{
			fromF = *(uint *)((byte *)from + pField->offset);
			toF = *(uint *)((byte *)to + pField->offset);
		}

		fromF = Delta_ClampIntegerField(fromF, bSigned, pField->bits);
		toF = Delta_ClampIntegerField(toF, bSigned, pField->bits);
		if (pField->multiplier != 1.0f) fromF *= pField->multiplier;
		if (pField->multiplier != 1.0f) toF *= pField->multiplier;
	}
	else if (pField->flags & (DT_FLOAT | DT_ANGLE))
	{
		// don't convert floats to integers
		fromF = *((int *)((byte *)from + pField->offset));
		toF = *((int *)((byte *)to + pField->offset));
	}
	else if (pField->flags & DT_TIMEWINDOW_8)
	{
		val_a = (*(float *)((byte *)from + pField->offset)) * 100.0f;
		val_b = (*(float *)((byte *)to + pField->offset)) * 100.0f;
		val_a -= (timebase * 100.0f);
		val_b -= (timebase * 100.0f);
		fromF = *((int *)&val_a);
		toF = *((int *)&val_b);
	}
	else if (pField->flags & DT_TIMEWINDOW_BIG)
	{
		val_a = (*(float *)((byte *)from + pField->offset));
		val_b = (*(float *)((byte *)to + pField->offset));
		if (pField->multiplier != 1.0f)
		{
			val_a *= pField->multiplier;
			val_b *= pField->multiplier;
			val_a = (timebase * pField->multiplier) - val_a;
			val_b = (timebase * pField->multiplier) - val_b;
		}
		else
		{
			val_a = timebase - val_a;
			val_b = timebase - val_b;
		}
		fromF = *((int *)&val_a);
		toF = *((int *)&val_b);
	}
	else if (pField->flags & DT_STRING)
	{
		// compare strings
		char	*s1 = (char *)((byte *)from + pField->offset);
		char	*s2 = (char *)((byte *)to + pField->offset);

		// 0 is equal, otherwise not equal
		toF = Q_strcmp(s1, s2);
	}

	return (fromF == toF) ? true : false;
}

/*
=====================
Delta_WriteField
write fields by offsets
assume from and to is valid
=====================
*/

qboolean Delta_WriteField(sizebuf_t_new *msg, delta_t *pField, void *from, void *to, float timebase)
{
	qboolean		bSigned = (pField->flags & DT_SIGNED) ? true : false;
	float		flValue, flAngle, flTime;
	uint		iValue;
	const char	*pStr;

	if (Delta_CompareField(pField, from, to, timebase))
	{
		BF_WriteOneBit(msg, 0);	// unchanged
		return false;
	}

	BF_WriteOneBit(msg, 1);	// changed

	if (pField->flags & DT_BYTE)
	{
		iValue = *(byte *)((byte *)to + pField->offset);
		iValue = Delta_ClampIntegerField(iValue, bSigned, pField->bits);
		if (pField->multiplier != 1.0f) iValue *= pField->multiplier;
		BF_WriteBitLong(msg, iValue, pField->bits, bSigned);
	}
	else if (pField->flags & DT_SHORT)
	{
		iValue = *(word *)((byte *)to + pField->offset);
		iValue = Delta_ClampIntegerField(iValue, bSigned, pField->bits);
		if (pField->multiplier != 1.0f) iValue *= pField->multiplier;
		BF_WriteBitLong(msg, iValue, pField->bits, bSigned);
	}
	else if (pField->flags & DT_INTEGER)
	{
		iValue = *(uint *)((byte *)to + pField->offset);
		iValue = Delta_ClampIntegerField(iValue, bSigned, pField->bits);
		if (pField->multiplier != 1.0f) iValue *= pField->multiplier;
		BF_WriteBitLong(msg, iValue, pField->bits, bSigned);
	}
	else if (pField->flags & DT_FLOAT)
	{
#ifdef __arm__
		memcpy(&flValue, (byte *)to + pField->offset, sizeof(float));
#else
		flValue = *(float *)((byte *)to + pField->offset);
#endif
		iValue = (int)(flValue * pField->multiplier);
		BF_WriteBitLong(msg, iValue, pField->bits, bSigned);
	}
	else if (pField->flags & DT_ANGLE)
	{
#ifdef __arm__
		memcpy(&flAngle, (byte *)to + pField->offset, sizeof(float));
#else
		flAngle = *(float *)((byte *)to + pField->offset);
#endif

		// NOTE: never applies multipliers to angle because
		// result may be wrong on client-side
		BF_WriteBitAngle(msg, flAngle, pField->bits);
	}
	else if (pField->flags & DT_TIMEWINDOW_8)
	{
#ifdef __arm__
		memcpy(&flValue, (byte *)to + pField->offset, sizeof(float));
#else
		flValue = *(float *)((byte *)to + pField->offset);
#endif
		flTime = (timebase * 100.0f) - (flValue * 100.0f);
#if 1
		iValue = (uint)fabs(flTime);
#else
		iValue = (uint)fabs(flTime);
		if (flTime<0.0f) {
			iValue |= 0x80000000;
		}
#endif

		BF_WriteBitLong(msg, iValue, pField->bits, bSigned);
	}
	else if (pField->flags & DT_TIMEWINDOW_BIG)
	{
#ifdef __arm__
		memcpy(&flValue, (byte *)to + pField->offset, sizeof(float));
#else
		flValue = *(float *)((byte *)to + pField->offset);
#endif
		flTime = (timebase * pField->multiplier) - (flValue * pField->multiplier);
#if 1
		iValue = (uint)fabs(flTime);
#else
		iValue = (uint)fabs(flTime);
		if (flTime<0.0f) {
			iValue |= 0x80000000;
		}
#endif

		BF_WriteBitLong(msg, iValue, pField->bits, bSigned);
	}
	else if (pField->flags & DT_STRING)
	{
		pStr = (char *)((byte *)to + pField->offset);
		BF_WriteString(msg, pStr);
	}
	return true;
}

void MSG_WriteDeltaUsercmd(sizebuf_t_new *msg, usercmd_t *from, usercmd_t *to)
{
	delta_t		*pField;
	delta_info_t	*dt;
	int		i;

	dt = Delta_FindStruct("usercmd_t");
	ASSERT(dt && dt->bInitialized);

	pField = dt->pFields;
	ASSERT(pField);

	// activate fields and call custom encode func
	Delta_CustomEncode(dt, from, to);

	// process fields
	for (i = 0; i < dt->numFields; i++, pField++)
	{
		Delta_WriteField(msg, pField, from, to, 0.0f);
	}
}

void CL_WriteUsercmd(sizebuf_t_new *msg, int from, int to)
{
	usercmd_t	nullcmd;
	usercmd_t	*f, *t;

	ASSERT(from == -1 || (from >= 0 && from < MULTIPLAYER_BACKUP));
	ASSERT(to >= 0 && to < MULTIPLAYER_BACKUP);

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
	MSG_WriteDeltaUsercmd(msg, f, t);
}

//todo: sv_computelatency (sv_unlagsamples)
#define LODWORD(x)  (*((DWORD*)&(x)))  // low dword

void CL_Move(void){

	if (*(int*)0x44bad2f4) //bNotFinalTick
		return;

	sizebuf_t buf;
	//qboolean		send_command = false;
	//432be888 //43298970
	byte		data[MAX_USERCMD_BUFFER];
	int		i, /*from, to,*/ key, size;
	int		numbackup = 2;
	int		numcmds;
	int		newcmds;
	int		cmdnumber;
	cmd_t			*cmd;
	usercmd_t *ucmd;

	//int mask = CL_UPDATE_MASK;


	// don't send anything if playing back a demo

	if( g_pcls->state == ca_dedicated || g_pcls->state == ca_disconnected || g_pcls->state == ca_connecting )
		return;

	if (g_pcls->state == ca_connected)
	{
		if (!/*cls.demoplayback*/DemoIsPlayingBack())//|| g_pcls->state == ca_cinematic)
			Netchan_Transmit(&g_pcls->netchan, NULL, NULL);
		return;
	}

	CL_ComputePacketLoss ();


	// Determine number of backup commands to send along
	numbackup = clamp(0, CVAR_GETDIRECTINT(cl_cmdbackup), MAX_BACKUP_COMMANDS);
	//if( g_pcls->state == ca_connected ) numbackup = 0;

	cmd = &CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK]; //(cmd_t*)(0x43298970 + (sizeof(cmd_t) * (cl->netchan.outgoing_sequence & mask)));
	cmd->senttime = realtime;
	cmd->receivedtime = -1.0f;
	cmd->processedfuncs = false;
	// Assume we'll transmit this command this frame ( rather than queueing it for next frame )
	cmd->heldback = false;
	cmd->sendsize = 0;

	//BF_Init( &buf, "ClientData", data, sizeof( data ));
	bf_init(&buf, "Client Move", data, sizeof(data));
	memset(&cmd->cmd, 0, sizeof(usercmd_t));

	CL_SetSolidEntities();
	CL_PushPMStates();
	CL_SetSolidPlayers(*LocalPlayerNum);
	CL_CreateMove((float)get_frametime, &cmd->cmd, g_pcls->signon == 2);
	CL_PopPMStates();

	//unsigned __int64 lerp = (unsigned __int64)((*cl_time - *cl_mtime) * 1000.0);

	cmd->cmd.lerp_msec = (short)(CVAR_GETDIRECTFLOAT(ex_interp) * 1000.0f); //(short)lerp; //(short)(CVAR_GETDIRECTFLOAT(ex_interp) * 1000.0f);
#if 0
	LODWORD(lerp) = (signed __int16)lerp;

	float lerp2 = *(float *)&lerp;
	bool v7 = (signed __int16)lerp == 0;
	bool v8 = (lerp & 0x8000u) != 0i64;
	float tmp = 0.0f;
	int* v9 = (int *)&lerp2;
	if (v8 || v7)
		v9 = (int *)&tmp;
	__int16 v10 = *(WORD*)v9;
	int v11 = v10;
	cmd->cmd.lerp_msec = v10;
	LODWORD(tmp) = v10;
	float interp = CVAR_GETDIRECTFLOAT(ex_interp) * 1000.0f;
	unsigned __int8 v13 = __OFSUB__(v11, (DWORD)interp);
	v8 = v11 - (signed int)interp < 0;
	int *v14 = (int *)&tmp;
	if (!(v8 ^ v13))
		v14 = (int *)&lerp2;
	cmd->cmd.lerp_msec = *(WORD *)v14;
#endif
	cmd->cmd.lightlevel = (int)*light_level2;

	// begin a client move command
	MSG_WriteByte(&buf, clc_move);

	// save the position for a checksum byte
	key = buf.cursize + 1; //BF_GetRealBytesWritten( &buf );
	MSG_WriteByte(&buf, 0);

	MSG_WriteByte(&buf, 0); //nf..

	int packetloss = (int)g_pcls->packet_loss;
	packetloss = clamp(packetloss, 0, 100);

	static float unknown = 0.0f;
	if (unknown != 0.0f)
	{
		packetloss |= 0x80;
		packetloss &= 0xFF;
	}
#if 0
	bool noloss = packetloss == 0.0f;
	bool loss = packetloss == 0.0f;
	int* ploss = (int*)&packetloss;
	float zero = 0.0f;
	if (loss || noloss)
		ploss = (int*)&zero;
	packetloss = clamp(packetloss, 0.0f, 100.0f);
#endif
	MSG_WriteByte(&buf, packetloss); // write packet lossage percentation
	
	//cmd->cmd.msec = (short)(gpGlobals->frametime * 1000.0f);
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
	cmd->cmd.msec = (short)msec;

	memcpy((void*)0x431B8790, &cmd->cmd, sizeof(usercmd_t)); //?

	if (!DemoIsPlayingBack())
	{
		if (!IsDedicatedServer())
			UnknownCall(0.0f);
#ifndef _DEBUG
		if (CVAR_GETDIRECTINT(cl_cmdrate) < 10)
		{
			CVAR_SETDIRECTINT(cl_cmdrate, 10);
		}
#endif
		//int adr = (int)&CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].heldback - (int)&CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK];
		int adr = (int)&g_pcl->maxclients;
		if (*cl_maxclients != 1
			&& (g_pcls->netchan.remote_address.type != NA_LOOPBACK || CVAR_GETDIRECTBOOL(host_limitlocal))
			&& (g_pcls->nextcmdtime > realtime || !Netchan_CanPacket(&g_pcls->netchan)))
		{
			// Mark command as held back so we'll send it next time
			CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].heldback = true;
			// increment sequence number so we can detect that we've held back packets.
			g_pcls->netchan.outgoing_sequence++;
			return;
		}
		//*force_send_usercmd = FALSE; //dylan added

		if (CVAR_GETDIRECTINT(cl_cmdrate) <= 0)
			g_pcls->nextcmdtime = realtime;
		else
		{
			// use full update rate when active
			float commandInterval = 1.0f / CVAR_GETDIRECTINT(cl_cmdrate);
			float maxDelta = min ((float)get_frametime, commandInterval );
			float delta = clamp( realtime - g_pcls->nextcmdtime, 0.0f, maxDelta );
			g_pcls->nextcmdtime = realtime + commandInterval - delta;
			//g_pcls->nextcmdtime = 1.0f / CVAR_GETDIRECTINT(cl_cmdrate) + realtime;
		}


		//int outgoing_sequence;
		if (g_pcls->lastoutgoingcommand == -1)
		{
			//outgoing_sequence = g_pcls->netchan.outgoing_sequence;
			g_pcls->lastoutgoingcommand = g_pcls->netchan.outgoing_sequence;
		}
		//else outgoing_sequence = g_pcls->lastoutgoingcommand + 1;

		if (*(char*)0x4311D2A1)
		{
			SZ_Clear(&buf);
		}
		else
		{
			// say how many backups we'll be sending
			MSG_WriteByte(&buf, numbackup);

			// how many real commands have queued up
			newcmds = g_pcls->netchan.outgoing_sequence - g_pcls->lastoutgoingcommand;

			// put an upper/lower bound on this
			newcmds = min(newcmds, MAX_TOTAL_CMDS); //clamp(0, newcmds, MAX_TOTAL_CMDS);
													//if (g_pcls->state == ca_connected) newcmds = 0;

			MSG_WriteByte(&buf, newcmds);

			usercmd_t ncmd;
			Q_memset(&ncmd, 0, sizeof(usercmd_t));

			numcmds = newcmds + numbackup - 1;

			usercmd_t *from;
			//from = -1;
			//from = numcmds - 1;

			for (usercmd_t* to = &ncmd; numcmds >= 0; to = from)
			{
				from = &CMDS->commands[CL_UPDATE_MASK & (g_pcls->netchan.outgoing_sequence - numcmds)].cmd;//(usercmd_t*) (0x43298970 + (sizeof(cmd_t) * (mask & (g_pcls->netchan.outgoing_sequence - numcmds))) );
				CL_WriteUsercmdDeltaToBuffer(&buf, from, to);
				numcmds--;
				/*cmdnumber = (g_pcls->netchan.outgoing_sequence - i) & CL_UPDATE_MASK;

				to = cmdnumber;
				CL_WriteUsercmdDeltaToBuffer(&buf, from, to);
				from = to;
				*/
				//if( buf.cursize > buf.maxsize)//BF_CheckOverflow( &buf ))
				//Host_Error( "CL_Move, overflowed command buffer (%i bytes)\n", MAX_CMD_BUFFER );
			}

			// calculate a checksum over the move commands
			//size = buf.cursize - key - 1;
#if 1
			int pos = buf.cursize - key - 1; //cursize should be 14
			if ( pos > 255)
			{
				pos = 255;
			}
			buf.data[key - 1] = (byte)pos;
			buf.data[key] = COM_BlockSequenceCRCByte2(&buf.data[key + 1], buf.cursize - key - 1, g_pcls->netchan.outgoing_sequence);
			int pos2 = buf.cursize - key - 1;
			if ( pos2 > 255)
			{
				pos2 = 255;
			}
			COM_Munge(&buf.data[key + 1], pos2, g_pcls->netchan.outgoing_sequence);
#else
#if HL1_REVERSEDCODE
			int pos = buf.cursize - key - 1;
			if (pos >= 255)
				LOBYTE(pos) = -1;
			buf.data[key - 1] = pos;
			int outgoing_sequence = g_pcls->netchan.outgoing_sequence;
			buf.data[key] = COM_BlockSequenceCRCByte2(&buf.data[key + 1], buf.cursize - key - 1, outgoing_sequence);
			int pos2 = buf.cursize - key - 1;
			if (pos2 >= 255)
				pos2 = 255;
			COM_Munge(&buf.data[key + 1], pos2, g_pcls->netchan.outgoing_sequence);
#else

#if 0
			buf.data[key] = CRC32_BlockSequence(buf.data + key + 1, size, g_pcls->netchan.outgoing_sequence);
#else
			float tmpfl = *(float*)&size;
			unsigned __int8 v13 = __OFSUB__(size, 255);
			bool checksumistwofiftyfive = size == 255;
			LODWORD(tmp) = 255;
			int* v28 = (int*)&tmp;
			if ((unsigned __int8)((buf.cursize - key - 256 < 0) ^ v13) | checksumistwofiftyfive)
				v28 = (int *)&tmpfl;

			//size = /*BF_GetRealBytesWritten( &buf )*/ buf.cursize; //buf.cursize - key - 1;
			buf.data[key - 1] = *(byte*)v28;
			buf.data[key] = COM_BlockSequenceCRCByte2(&buf.data[key + 1], buf.cursize - key - 1, g_pcls->netchan.outgoing_sequence);

			int v29 = buf.cursize - key - 1;
			tmpfl = *(float*)&v29;
			v13 = __OFSUB__(v29, 255);
			checksumistwofiftyfive = v29 == 255;
			LODWORD(tmp) = 255;
			int* v30 = (int *)&tmp;
			if ((unsigned __int8)((buf.cursize - key - 256 < 0) ^ v13) | checksumistwofiftyfive)
				v30 = (int *)&tmpfl;

			COM_Munge(&buf.data[key + 1], *v30, g_pcls->netchan.outgoing_sequence);
#endif
#endif
#endif
		}
		if (g_pcls->state != ca_active
			|| g_pcls->signon != ca_connecting
			|| g_pcls->netchan.outgoing_sequence - g_pcls->netchan.incoming_acknowledged < CL_UPDATE_MASK
			|| realtime - g_pcls->netchan.last_received <= CONNECTION_PROBLEM_TIME)
		{
			if (!g_pcl->validsequence)
				goto LABEL_58;
			if (CVAR_GETDIRECTBOOL(cl_nodelta) == 0 && g_pcls->state == ca_active)
			{
				*cl_delta_sequence = g_pcl->validsequence;
				MSG_WriteByte(&buf, clc_delta);
				MSG_WriteByte(&buf, g_pcl->validsequence  & 0xFF);
				goto LABEL_59;
			}
		}
		else
		{
			/*
			char* translatedtext = TranslateMessage("MPErrConnect");
			int v33;
			int value = 1;
			if (translatedtext && (v33 = translatedtext[10]) != 0)
				Con_NXPrintf((int)&value, v33);
				*/
			int value = 1;
			Con_NXPrintf((int)&value, "WARNING:  Connection Problem");
			g_pcl->validsequence = 0;
		}
	LABEL_58:
		// Tell server to send us an uncompressed update
		*cl_delta_sequence = -1;
	LABEL_59:
		// remember outgoing command that we are sending
		g_pcls->lastoutgoingcommand = g_pcls->netchan.outgoing_sequence;

		CMDS->commands[g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK].sendsize = buf.cursize; //this is 100% correct

		// composite the rest of the datagram..
		int buffersize = buf.maxsize;
		int cursize = buf.cursize;

		//buffersizestorage->buffersize[19 * (g_pcls->netchan.outgoing_sequence & CL_UPDATE_MASK)] = buf.cursize;

		if (g_pcls->datagram.cursize <= buffersize - cursize)
		{
			memcpy(&buf.data[cursize], &g_pcls->datagram.data, g_pcls->datagram.cursize);
			cursize = g_pcls->datagram.cursize + buf.cursize;
			buf.cursize += g_pcls->datagram.cursize;
		}

		g_pcls->datagram.cursize = 0; //clear buffer
		// deliver the message (or update reliable)

		Netchan_Transmit(&g_pcls->netchan, cursize, buf.data);
	}
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

HookCallInfo funcHooks[] = {
	{ (BYTE*)ADR_CL_MOVE, (DWORD)hookOnCL_Move, 6, (BYTE*)ADR_CL_MOVE + 0x6 },
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

void callOnPlayerKilled(entvars_t2 *pevAttacker, int iGib, C_BasePlayer *pVictim) {
	/*
	C_BasePlayer *pAttacker = C_BasePlayer::Instance(pevAttacker->pContainingEntity);
	if ( pAttacker && pAttacker->IsPlayer() && !pevAttacker->team == pVictim->team ) {
		//Give the attacker some points for killing this player
		SetCTFScore(pAttacker, GetCTFScore(pAttacker) + 1);
		//Give money to attacker
	}
	*/
}

void OnModuleInitialize(void) {
	HookFunctions();
	printf("CL_Move Fixes Initiated\n");
}

void OnKeyValue(edict_t *pEntity, KeyValueData *pkvd) {
}

void OnServerDeactivate(void) {
}

void OnServerActivate(void) {
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