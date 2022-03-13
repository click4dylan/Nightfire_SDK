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

void BF_InitExt( sizebuf_t_new *bf, const char *pDebugName, void *pData, int nBytes, int nMaxBits )
{
	bf->pDebugName = pDebugName;

	BF_StartWriting( bf, pData, nBytes, 0, nMaxBits );
}

void BF_StartWriting(sizebuf_t_new *bf, void *pData, int nBytes, int iStartBit, int nBits)
{
	// make sure it's dword aligned and padded.
	//Assert(((dword)pData & 3) == 0);

	bf->pData = (byte *)pData;

	if (nBits == -1)
	{
		bf->nDataBits = nBytes << 3;
	}
	else
	{
		//Assert(nBits <= nBytes * 8);
		bf->nDataBits = nBits;
	}

	bf->iCurBit = iStartBit;
	bf->bOverflow = false;
}

void BF_WriteByte(sizebuf_t_new *bf, int val)
{
	BF_WriteUBitLong(bf, val, sizeof(byte) << 3);
}

// precalculated bit masks for WriteUBitLong.
// Using these tables instead of doing the calculations
// gives a 33% speedup in WriteUBitLong.
static DWORD BitWriteMasks[32][33];
static DWORD ExtraMasks[32];

void BF_WriteUBitLongExt(sizebuf_t_new *bf, unsigned curData, int numbits, qboolean bCheckRange)
{
	//Assert(numbits >= 0 && numbits <= 32);

	// bounds checking..
	if ((bf->iCurBit + numbits) > bf->nDataBits)
	{
		bf->bOverflow = true;
		bf->iCurBit = bf->nDataBits;
	}
	else
	{
		int	nBitsLeft = numbits;
		int	iCurBit = bf->iCurBit;
		unsigned	iDWord = iCurBit >> 5;	// Mask in a dword.
		DWORD	iCurBitMasked;
		int	nBitsWritten;

		//Assert((iDWord * 4 + sizeof(long)) <= (uint)BF_GetMaxBytes(bf));

		iCurBitMasked = iCurBit & 31;
		((DWORD *)bf->pData)[iDWord] &= BitWriteMasks[iCurBitMasked][nBitsLeft];
		((DWORD *)bf->pData)[iDWord] |= curData << iCurBitMasked;

		// did it span a dword?
		nBitsWritten = 32 - iCurBitMasked;

		if (nBitsWritten < nBitsLeft)
		{
			nBitsLeft -= nBitsWritten;
			iCurBit += nBitsWritten;
			curData >>= nBitsWritten;

			iCurBitMasked = iCurBit & 31;
			((DWORD *)bf->pData)[iDWord + 1] &= BitWriteMasks[iCurBitMasked][nBitsLeft];
			((DWORD *)bf->pData)[iDWord + 1] |= curData << iCurBitMasked;
		}
		bf->iCurBit += numbits;
	}
}

void BF_WriteOneBit(sizebuf_t_new *bf, int nValue)
{
	if (!BF_Overflow(bf, 1))
	{
		if (nValue) bf->pData[bf->iCurBit >> 3] |= (1 << (bf->iCurBit & 7));
		else bf->pData[bf->iCurBit >> 3] &= ~(1 << (bf->iCurBit & 7));

		bf->iCurBit++;
	}
}

static qboolean BF_Overflow(sizebuf_t_new *bf, int nBits)
{
	if (bf->iCurBit + nBits > bf->nDataBits)
		bf->bOverflow = true;
	return bf->bOverflow;
}

void BF_WriteBitLong(sizebuf_t_new *bf, unsigned data, int numbits, qboolean bSigned)
{
	if (bSigned)
		BF_WriteSBitLong(bf, (int)data, numbits);
	else BF_WriteUBitLong(bf, data, numbits);
}

/*
=======================
BF_WriteSBitLong
sign bit comes first
=======================
*/
void BF_WriteSBitLong(sizebuf_t_new *bf, int data, int numbits)
{
	// do we have a valid # of bits to encode with?
	//Assert(numbits >= 1);

	// NOTE: it does this wierdness here so it's bit-compatible with regular integer data in the buffer.
	// (Some old code writes direct integers right into the buffer).
	if (data < 0)
	{
		BF_WriteUBitLongExt(bf, (unsigned)(0x80000000 + data), numbits - 1, false);
		BF_WriteOneBit(bf, 1);
	}
	else
	{
		BF_WriteUBitLong(bf, (unsigned)data, numbits - 1);
		BF_WriteOneBit(bf, 0);
	}
}

void BF_WriteBitAngle(sizebuf_t_new *bf, float fAngle, int numbits)
{
	unsigned	mask, shift;
	int	d;

	// clamp the angle before receiving
	fAngle = fmod(fAngle, 360.0f);
	if (fAngle < 0) fAngle += 360.0f;

	shift = (1 << numbits);
	mask = shift - 1;

	d = (int)((fAngle * shift) / 360.0f);
	d &= mask;

	BF_WriteUBitLong(bf, (unsigned)d, numbits);
}

qboolean BF_WriteString(sizebuf_t_new *bf, const char *pStr)
{
	if (pStr)
	{
		do
		{
			BF_WriteChar(bf, *pStr);
			pStr++;
		} while (*(pStr - 1));
	}
	else BF_WriteChar(bf, 0);

	return !bf->bOverflow;
}

void BF_WriteChar(sizebuf_t_new *bf, int val)
{
	BF_WriteSBitLong(bf, val, sizeof(char) << 3);
}

qboolean BF_WriteBits(sizebuf_t_new *bf, const void *pData, int nBits)
{
	byte	*pOut = (byte *)pData;
	int	nBitsLeft = nBits;

	// get output dword-aligned.
	while (((DWORD)pOut & 3) != 0 && nBitsLeft >= 8)
	{
		BF_WriteUBitLongExt(bf, *pOut, 8, false);

		nBitsLeft -= 8;
		++pOut;
	}

	// read dwords.
	while (nBitsLeft >= 32)
	{
		BF_WriteUBitLongExt(bf, *((DWORD *)pOut), 32, false);

		pOut += sizeof(DWORD);
		nBitsLeft -= 32;
	}

	// read the remaining bytes.
	while (nBitsLeft >= 8)
	{
		BF_WriteUBitLongExt(bf, *pOut, 8, false);

		nBitsLeft -= 8;
		++pOut;
	}

	// Read the remaining bits.
	if (nBitsLeft)
	{
		BF_WriteUBitLongExt(bf, *pOut, nBitsLeft, false);
	}

	return !bf->bOverflow;
}

qboolean BF_CheckOverflow(sizebuf_t_new *bf)
{
	ASSERT(bf);

	return BF_Overflow(bf, 0);
}

/*
=======================
MSG_Clear
for clearing overflowed buffer
=======================
*/
void BF_Clear(sizebuf_t_new *bf)
{
	bf->iCurBit = 0;
	bf->bOverflow = false;
}



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