#pragma once

typedef struct sizebuf_s
{
	const char *buffername;
	uint16 flags;
	byte *data;
	int maxsize;
	int cursize;
} sizebuf_t;