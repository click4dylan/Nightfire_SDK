#ifndef _INCLUDE_BINARY_H
#define _INCLUDE_BINARY_H

#include <stdio.h>
#include "compat.h"
#include "amxxmodule.h"

class BinaryReader
{
public:
	BinaryReader(FILE *fp);
	//~BinaryReader();
public:
	bool ReadUInt32(uint32_t& num);
	bool ReadInt32(int32_t& num);
	bool ReadUInt16(uint16_t& num);
	bool ReadInt16(int16_t& num);
	bool ReadUInt8(uint8_t& num);
	bool ReadInt8(int8_t& num);
	bool ReadChars(char buffer[], size_t chars);
private:
	bool ReadAddr(void *buffer, size_t size);
private:
	FILE *m_Fp;
};

class BinaryWriter
{
public:
	BinaryWriter() { m_Fp = NULL; }
	BinaryWriter(FILE *fp);
public:
	void SetFilePtr(FILE *fp) { m_Fp = fp; }
	bool WriteUInt32(uint32_t num);
	bool WriteInt32(int32_t num);
	bool WriteUInt16(uint16_t num);
	bool WriteInt16(int16_t num);
	bool WriteUInt8(uint8_t num);
	bool WriteInt8(int8_t num);
	bool WriteChars(const char buffer[], size_t chars);
private:
	bool WriteAddr(void *buffer, size_t size);
private:
	FILE *m_Fp;
};

#endif //_INCLUDE_BINARY_H

