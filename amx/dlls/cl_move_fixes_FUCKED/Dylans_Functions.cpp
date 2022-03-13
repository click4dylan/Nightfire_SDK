#include "Dylans_Functions.h"

BOOL IsCTF(void) {
#ifdef MICROSOFT_SUCKS
	edict_t *flag = FIND_ENTITY_BY_CLASSNAME((edict_t*)0, "item_ctfflag");
	if ( !FNullEnt(flag)/*flag && flag->v.origin != Vector(0,0,0)*/ ) //origin is required because for some reason non-existing entities still return an edict
		return TRUE;
	return FALSE;
#else
	void *g_pGameRules = (void*)*(int*)*(int*)0x4217959C;
	BOOL *isCTFFunc = (BOOL*) *(int*)((int)g_pGameRules + 0x28);
	return ((BOOL(__stdcall*)(void))isCTFFunc)();
	/*
	__asm {
		mov ecx, dword ptr ds:[0x4217959C] //g_pGameRules
		mov ecx, dword ptr ds:[ecx]
		call dword ptr ds:[ecx + 0x28] //set's EAX to 1 or 0 depending on gamemode
	}
	*/
#endif
}



//-----------------------------------------------------------------------------
// Purpose: Fade an entity (player) screen to a specified color.
// Input  : *pEntity - Entity to tell to screen fade on.
// Input  : &color  - Color to fade to
// Input  : fadeTime - Time it takes to fade
// Input  : fadeHold - Time the fade holds for
// Input  : flags - Fade in, Fade Out, Fade_Modulate (don't blend), Fade_StayOut, Fade_Purge (Replaces others)
// Output : 
void UTIL_ScreenFade( edict_t *pEntity, byte r, byte g, byte b, byte a, float fadeTime, float fadeHold, int flags ) { //UTIL_ScreenFade(int playerid, short duration, short holdtime, short fadeflags, byte r, byte g, byte b, byte a) {
	int newflags = 0;
	client_t *pClient = EDICT_TO_CLIENT(pEntity);

	__asm {
		mov eax, flags
		xor edi, edi //set edi to 0
		test al, 0x01
		jnz one
		mov edi, 0x1
	one:
		test al, 0x02
		jz two
		or edi,0x00000003
	two:
		test al, 0x04
		mov newflags, edi
		jz fadeall
	}
	if ( !pClient->fakeclient )
		UTIL_ScreenFadeOne(pEntity, r, g, b, a, fadeTime, fadeHold, newflags);
	return;
	fadeall:
	UTIL_ScreenFadeAll(r, g, b, a, fadeTime, fadeHold, newflags);
}

void UTIL_ScreenFadeAll( byte r, byte g, byte b, byte a, float fadeTime, float fadeHold, int flags ) {
	int unknown, unknown2;
	MESSAGE_BEGIN(MSG_BROADCAST, GET_USER_MSG_ID(PLID, "ScreenFade", NULL));
		WRITE_COORD(fadeTime);
		WRITE_COORD(fadeHold);
		WRITE_BYTE(r);
		WRITE_BYTE(g);
		WRITE_BYTE(b);
		__asm {
			mov ebx, flags
			mov edi, ebx
			and edi, 0x00000001
			mov eax, 0x0
			setnz al
			dec eax
			and eax, 0x000000FF
			mov unknown, eax
			mov unknown2, edi
		}
		WRITE_BYTE(unknown);
		WRITE_BYTE(r);
		WRITE_BYTE(g);
		WRITE_BYTE(b);
		if ( unknown2 == 0 ) 
			a = 255 - a;
		WRITE_BYTE(a);
		__asm {
			mov ebx, flags //might be unnecessary, but keep it just in case
			shr ebx, 0x1
			not bl
			and ebx, 0x00000001
			mov unknown, ebx
		}
		WRITE_BYTE(unknown);
	MESSAGE_END();
}

void UTIL_ScreenFadeOne( edict_t *pEntity, byte r, byte g, byte b, byte a, float fadeTime, float fadeHold, int flags ) {
	int unknown, unknown2;
	MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, GET_USER_MSG_ID(PLID, "ScreenFade", NULL), 0, pEntity);
		WRITE_COORD(fadeTime);
		WRITE_COORD(fadeHold);
		WRITE_BYTE(r);
		WRITE_BYTE(g);
		WRITE_BYTE(b);
		__asm {
			mov ebx, flags
			mov edi, ebx
			and edi, 0x00000001
			mov eax, 0x0
			setnz al
			dec eax
			and eax, 0x000000FF
			mov unknown, eax
			mov unknown2, edi
		}
		WRITE_BYTE(unknown);
		WRITE_BYTE(r);
		WRITE_BYTE(g);
		WRITE_BYTE(b);
		if ( unknown2 == 0 ) 
			a = 255 - a;
		WRITE_BYTE(a);
		__asm{
			mov ebx, flags //might be unnecessary, but keep it just in case
			shr ebx, 0x1
			not bl
			and ebx, 0x00000001
			mov unknown, ebx
		}
		WRITE_BYTE(unknown);
	MESSAGE_END();
}


//Noel's char functions
StringList::StringList(int Length = 0, int initialmemreserve = 5){
	strings.resize(Length);
	strings.reserve(initialmemreserve);
}

StringList::~StringList(){
	for (int c = strings.size() - 1; c >= 0; c--){
		delete strings.at(c);
	}
}


StringList * SplitChar(const char * text, char delimiters[]){
	char * editabletext = new char[strlen(text) + 1]; // For Const Char Strings Passed To This Function
	strcpy(editabletext, text);
	StringList * sl = new StringList();
	char * token = strtok(editabletext, delimiters);
	while (token != nullptr){
		char * curstr = new char[strlen(token) + 1];
		strcpy(curstr, token);
		sl->Add(curstr);
		token = strtok(nullptr, delimiters);
	}
	delete editabletext;
	return sl;
}

__declspec (naked) C_BasePlayer* PLAYER_BY_INDEX(int index) {
	__asm {
		push dword ptr ss:[esp + 4]
		mov eax, 0x42059F10
		call eax
		add esp, 0x4
		retn
	}
}

/*
__declspec (naked) void UTIL_SET_ORIGIN(C_BaseEntity *entity, const Vector &vecOrigin, bool bFireTriggers) {
	//original function: 0x4306B690 but it has no check for bFireTriggers, actually i'm not sure if we can even fire triggers at all... this may be useless
	__asm {
		mov eax, dword ptr ss:[esp + 4]
		test eax, eax
		jz Exit //Invalid Entity, return
		MOV ECX,DWORD PTR SS:[ESP+8]
		MOV EDX,DWORD PTR DS:[ECX]
		MOV DWORD PTR DS:[EAX+0x128],EDX
		MOV EDX,DWORD PTR DS:[ECX+4]
		MOV DWORD PTR DS:[EAX+0x12C],EDX
		MOV ECX,DWORD PTR DS:[ECX+8]
		MOV DWORD PTR DS:[EAX+0x130],ECX
		cmp byte ptr ss:[esp + 16], 0
		jne Exit
			MOV DWORD PTR SS:[ESP+8],0
			MOV DWORD PTR SS:[ESP+4],EAX
			mov eax, 0x430994D0 //SV_LinkEdict
			jmp eax
		Exit:
		retn
	}
}
*/


#define NUM_BYTES		256
#define CRC32_INIT_VALUE	0xFFFFFFFFUL
#define CRC32_XOR_VALUE	0xFFFFFFFFUL

static const DWORD crc32table[NUM_BYTES] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

void CRC32_ProcessBuffer(DWORD *pulCRC, const void *pBuffer, int nBuffer)
{
	DWORD	poolpb, ulCrc = *pulCRC;
	byte	*pb = (byte *)pBuffer;
	unsigned	nFront;
	int	nMain;
JustAfew:
	switch (nBuffer)
	{
	case 7: ulCrc = crc32table[*pb++ ^ (byte)ulCrc] ^ (ulCrc >> 8);
	case 6: ulCrc = crc32table[*pb++ ^ (byte)ulCrc] ^ (ulCrc >> 8);
	case 5: ulCrc = crc32table[*pb++ ^ (byte)ulCrc] ^ (ulCrc >> 8);
	case 4:
		memcpy(&poolpb, pb, sizeof(DWORD));
		ulCrc ^= poolpb;	// warning, this only works on little-endian.
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		*pulCRC = ulCrc;
		return;
	case 3: ulCrc = crc32table[*pb++ ^ (byte)ulCrc] ^ (ulCrc >> 8);
	case 2: ulCrc = crc32table[*pb++ ^ (byte)ulCrc] ^ (ulCrc >> 8);
	case 1: ulCrc = crc32table[*pb++ ^ (byte)ulCrc] ^ (ulCrc >> 8);
	case 0: *pulCRC = ulCrc;
		return;
	}

	// We may need to do some alignment work up front, and at the end, so that
	// the main loop is aligned and only has to worry about 8 byte at a time.
	// The low-order two bits of pb and nBuffer in total control the
	// upfront work.
	nFront = ((unsigned)pb) & 3;
	nBuffer -= nFront;

	switch (nFront)
	{
	case 3: ulCrc = crc32table[*pb++ ^ (byte)ulCrc] ^ (ulCrc >> 8);
	case 2: ulCrc = crc32table[*pb++ ^ (byte)ulCrc] ^ (ulCrc >> 8);
	case 1: ulCrc = crc32table[*pb++ ^ (byte)ulCrc] ^ (ulCrc >> 8);
	}

	nMain = nBuffer >> 3;
	while (nMain--)
	{
		ulCrc ^= *(DWORD *)pb;	// warning, this only works on little-endian.
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc ^= *(DWORD *)(pb + 4);// warning, this only works on little-endian.
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		ulCrc = crc32table[(byte)ulCrc] ^ (ulCrc >> 8);
		pb += 8;
	}

	nBuffer &= 7;
	goto JustAfew;
}

void CRC32_Init(DWORD *pulCRC)
{
	*pulCRC = CRC32_INIT_VALUE;
}

void CRC32_Final(DWORD *pulCRC)
{
	*pulCRC ^= CRC32_XOR_VALUE;
}

/*
====================
CRC32_BlockSequence
For proxy protecting
====================
*/
byte CRC32_BlockSequence(byte *base, int length, int sequence)
{
	DWORD	CRC;
	char	*ptr;
	char	buffer[64];

	if (sequence < 0) sequence = abs(sequence);
	ptr = (char *)crc32table + (sequence % 0x3FC);

	if (length > 60) length = 60;
	memcpy(buffer, base, length);

	buffer[length + 0] = ptr[0];
	buffer[length + 1] = ptr[1];
	buffer[length + 2] = ptr[2];
	buffer[length + 3] = ptr[3];

	length += 4;

	CRC32_Init(&CRC);
	CRC32_ProcessBuffer(&CRC, buffer, length);
	CRC32_Final(&CRC);

	return (byte)CRC;
}


// Bit field reading/writing storage.
bf_read_t *bfread = (bf_read_t*)0x43482248;
ALIGN16 bf_write_t *bfwrite = (bf_write_t*)0x43482348;

void bf_init(sizebuf_t* buf, const char* BufferName, byte* data, unsigned sizeofdata)
{
	buf->buffername = BufferName;
	buf->cursize = 0;
	buf->data = data;
	buf->maxsize = sizeofdata;
	buf->flags = 0;
}

void (*Con_Printf) (const char *fmt, ...) = (void(*)(const char*,...))0x43043870;
void(*Con_DPrintf)(const char *fmt, ...) = (void(*)(const char*, ...))0x430439F0;
void (*SZ_Clear)(void* adr) = (void(*)(void*))0x43040260;
void (*SZ_Write)(void* dest, void* src, size_t size) = (void(*)(void*, void*, size_t))0x43040330;

void(*Cbuf_AddText)(const char*text) = (void(*)(const char*))0x4303D820;

void(*MSG_WriteByte)(sizebuf_t *sb, int c) = (void(*)(sizebuf_t *, int))0x43042210;
void(*MSG_WriteLong)(sizebuf_t *sb, long c) = (void(*)(sizebuf_t *, long))0x43042270;
void(*MSG_WriteShort)(sizebuf_t *sb, short c) = (void(*)(sizebuf_t*, short))0x43042230;
void(*MSG_WriteString)(sizebuf_t *sb, const char *s) = (void(*)(sizebuf_t*, const char*))0x430422E0;
char*(*MSG_ReadString)(void) = (char*(*)())0x43040060;
int(*MSG_ReadShort)(void) = (int(*)(void))0x4303FF20;
int(*MSG_ReadByte) (void) = (int(*)(void))0x4303FEB0;
int(*MSG_ReadLong)(void) = (int(*)(void))0x4303FF60;
void(*MSG_BeginReading)(void) = (void(*)(void))0x4303FE70;
void(*MSG_ReadUsercmd) (usercmd_t *to, usercmd_t* from) = (void(*)(usercmd_t*, usercmd_t*))0x43042A10;
void (*NET_SendPacket) (netsrc_t sock, int length, void *data, const netadr_t& to) = (void(*)(netsrc_t, int, void*, const netadr_t&))0x43068650;
void(*SV_DropClient)(client_t *cl, qboolean crash, const char *fmt, ...) = (void(*)(client_t *, qboolean, const char *, ...))0x430574A0;
client_t *host_client = (client_t*)0x448BE188;
void(*MSG_WriteBuf)(sizebuf_t *sb, int iSize, void *buf) = (void(*)(sizebuf_t *, int , void *))0x43042330;

int Q_vsnprintf ( char *buffer, size_t count, const char *format, va_list argptr )
{
	static unsigned long func = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?safe_vsnprintf@@YGXPADHPBD0@Z");
	return ((int(_stdcall*)(char*, size_t, const char*, va_list))func)(buffer, count, format, argptr);
}
void* Q_memset (void* dest, int c, size_t count)
{
	static unsigned long func = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?memsetx@@YGPAXPAXDH@Z");
	return ((void*(_stdcall*)(void*, int, size_t))func)(dest, c, count);
}

char* Q_strncpy(char* dest, const char* src, size_t count)
{
	static unsigned long func = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?strncpyx@@YGPADPADPBDH@Z");
	return ((char*(_stdcall*)(char* , const char* , size_t ))func)(dest, src, count);
}

int Q_stricmp(const char* dest, const char* src)
{
	static unsigned long func = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?stricmpx@@YGHPBD0@Z");
	return ((int(_stdcall*)(const char*, const char*))func)(dest, src);
}

size_t Q_strlen(const char*src)
{
	static unsigned long func = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?strlenx@@YGHPBD@Z");
	return ((size_t(_stdcall*)(const char*))func)(src);
}

size_t Q_strstr(char*str, const char* substr)
{
	static unsigned long func = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?strstrx@@YGPADPBD0@Z");
	return ((size_t(_stdcall*)(char*, const char*))func)(str, substr);
}

unsigned long __stdcall getsnprintf()
{
	static unsigned long func = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?safe_snprintf@@YAXPADHPBDZZ");
	return func;
}
__declspec (naked) __declspec(noinline) int Q_snprintf(char*buffer, size_t count, const char*format, ...)
{
	__asm {
		call getsnprintf
		jmp eax
	}
}

void* Q_memmove(void* dest, void* src, size_t count)
{
	static unsigned long piss = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?memmovex@@YGPAXPAXPBXH@Z");

	return ((void*(_stdcall*)(void*, void*, size_t))piss)(dest, src, count);
}

void* Q_malloc(size_t count)
{
	static unsigned long piss = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?mallocx@@YGPAXH@Z");

	return ((void*(_stdcall*)(size_t))piss)(count);
}

void* Q_memcpy(void* dest, void* src, size_t count)
{
	static unsigned long piss = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?memcpyx@@YGPAXPAXPBXH@Z");

	return ((void*(_stdcall*)(void*, void*, size_t))piss)(dest, src, count);
}

void Mem_Free(void* mem)
{
	static unsigned long platform_free = (unsigned long)GetProcAddress(GetModuleHandle("platform.dll"), "?freex@@YGXPAX@Z");
	((void(_stdcall*)(void*))platform_free)(mem);
}

void (*Sys_Error) (const char *pMsg, ...) = (void(*)(const char*, ...))0x430947D0;
FileHandle_t(*FS_Open)(const char *pFileName, const char *pOptions) = (FileHandle_t(*)(const char *, const char *))0x430C3F79;

void (*FS_Close)(FileHandle_t file) = (void(*)(FileHandle_t))0x430C3ECC;

typedef struct packfile_s
{
	char		name[56];
	fs_offset_t	offset;
	fs_offset_t	realsize;	// real file size (uncompressed)
} packfile_t;

typedef struct pack_s
{
	char		filename[PATH_MAX];
	int		handle;
	int		numfiles;
	time_t		filetime;	// common for all packed files
	packfile_t	*files;
} pack_t;

searchpath_t *FS_FindFile(const char *name, int* index, qboolean gamedironly)
{
#if 1
	return (searchpath_t*)true;
#else
	searchpath_t	*search;
	char		*pEnvPath;
	pack_t		*pak;

	// search through the path, one element at a time
	for (search = fs_searchpaths; search; search = search->next)
	{
		if (gamedironly && !(search->flags & FS_GAMEDIRONLY_SEARCH_FLAGS))
			continue;

		// is the element a pak file?
		if (search->pack)
		{
			int left, right, middle;

			pak = search->pack;

			// look for the file (binary search)
			left = 0;
			right = pak->numfiles - 1;
			while (left <= right)
			{
				int diff;

				middle = (left + right) / 2;
				diff = Q_stricmp(pak->files[middle].name, name);

				// Found it
				if (!diff)
				{
					if (index) *index = middle;
					return search;
				}

				// if we're too far in the list
				if (diff > 0)
					right = middle - 1;
				else left = middle + 1;
			}
		}
		else if (search->wad)
		{
			dlumpinfo_t	*lump;
			signed char		type = W_TypeFromExt(name);
			qboolean		anywadname = true;
			string		wadname, wadfolder;
			string		shortname;

			// quick reject by filetype
			if (type == TYP_NONE) continue;
			FS_ExtractFilePath(name, wadname);
			wadfolder[0] = '\0';

			if (Q_strlen(wadname))
			{
				FS_FileBase(wadname, wadname);
				Q_strncpy(wadfolder, wadname, sizeof(wadfolder));
				FS_DefaultExtension(wadname, ".wad");
				anywadname = false;
			}

			// make wadname from wad fullpath
			FS_FileBase(search->wad->filename, shortname);
			FS_DefaultExtension(shortname, ".wad");

			// quick reject by wadname
			if (!anywadname && Q_stricmp(wadname, shortname))
				continue;

			// NOTE: we can't using long names for wad,
			// because we using original wad names[16];
			FS_FileBase(name, shortname);

			lump = W_FindLump(search->wad, shortname, type);
			if (lump)
			{
				if (index)
					*index = lump - search->wad->lumps;
				return search;
			}
		}
		else
		{
			char	netpath[MAX_SYSPATH];
			Q_sprintf(netpath, "%s%s", search->filename, name);
			if (FS_SysFileExists(netpath, !(search->flags & FS_CUSTOM_PATH)))
			{
				if (index != NULL) *index = -1;
				return search;
			}
		}
	}

	if (fs_ext_path && (pEnvPath = getenv("Path")))
	{
		char	netpath[MAX_SYSPATH];

		// clear searchpath
		search = &fs_directpath;
		Q_memset(search, 0, sizeof(searchpath_t));

		// root folder has a more priority than netpath
		Q_strncpy(search->filename, host.rootdir, sizeof(search->filename));
		Q_strcat(search->filename, "\\");
		Q_snprintf(netpath, MAX_SYSPATH, "%s%s", search->filename, name);

		if (FS_SysFileExists(netpath, !(search->flags & FS_CUSTOM_PATH)))
		{
			if (index != NULL)
				*index = -1;
			return search;
		}

		// search for environment path
		while (pEnvPath)
		{
			char *end = Q_strchr(pEnvPath, ';');
			if (!end) break;
			Q_strncpy(search->filename, pEnvPath, (end - pEnvPath) + 1);
			Q_strcat(search->filename, "\\");
			Q_snprintf(netpath, MAX_SYSPATH, "%s%s", search->filename, name);

			if (FS_SysFileExists(netpath, !(search->flags & FS_CUSTOM_PATH)))
			{
				if (index != NULL)
					*index = -1;
				return search;
			}
			pEnvPath += (end - pEnvPath) + 1; // move pointer
		}
	}

	if (index != NULL)
		*index = -1;

	return NULL;
#endif
}

int FS_FileExists(const char *pFileName, qboolean gamedironly)
{
	if (FS_FindFile(pFileName, NULL, gamedironly))
		return true;
	return false;
}

qboolean (*NET_CompareAdr) (netadr_t& a, netadr_t& b) = (qboolean(*)(netadr_t&, netadr_t&))0x43065DB0;;

void *Mem_ZeroMalloc(size_t size)
{
	void *p;

	p = Q_malloc(size);
	Q_memset((unsigned char *)p, 0, size);
	return p;
}