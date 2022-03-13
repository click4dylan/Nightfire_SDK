#pragma once
#include "common_rehlds.h"

typedef struct sizebuf_s
{
	qboolean		bOverflow;	// overflow reading or writing
	const char	*pDebugName;	// buffer name (pointer to const name)

	byte		*pData;
	int		iCurBit;
	int		nDataBits;
} sizebuf_t;