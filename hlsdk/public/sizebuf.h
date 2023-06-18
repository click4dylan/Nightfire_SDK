#pragma once

typedef struct sizebuf_s
{
	const char *buffername;
	unsigned short flags;
	byte *data;
	int maxsize;
	int cursize;
} sizebuf_t;