#pragma once

#include <bool_nightfire.h>

const int FRAGMENT_MAX_SIZE = 1400;

// Generic fragment structure
typedef struct fragbuf_s
{
	// Next buffer in chain
	fragbuf_s* next;
	// Id of this buffer
	int bufferid;
	// Message buffer where raw data is stored
	sizebuf_t frag_message;
	// The actual data sits here
	byte frag_message_buf[FRAGMENT_MAX_SIZE];
	// Is this a file buffer?
	bool_nightfire isfile;
	// Is this file buffer from memory ( custom decal, etc. ).
	bool_nightfire isbuffer;
	bool_nightfire iscompressed;
	// Name of the file to save out on remote host
	char filename[MAX_PATH];
	// Offset in file from which to read data
	int foffset;
	// Size of data to read at that offset
	int size;
} fragbuf_t;

typedef struct fragbufwaiting_s
{
	// Next chain in waiting list
	fragbufwaiting_s* next;
	// Number of buffers in this chain
	int fragbufcount;
	// The actual buffers
	fragbuf_t* fragbufs;
} fragbufwaiting_t;