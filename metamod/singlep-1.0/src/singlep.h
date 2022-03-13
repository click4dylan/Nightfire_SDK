// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// singlep.h - singleplayer support

/*
 * Copyright (c) 2002-2003 Pierre-Marie Baty <pm@racc-ai.com>
 * Copyright (c) 2003 Will Day <willday@hpgx.net>
 *
 *    This file is part of Metamod.
 *
 *    Metamod is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    Metamod is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Metamod; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#ifndef SINGLEP_H
#define SINGLEP_H

#include <extdll.h>			// always
#include <Windows.h> //Dylan added

void sp_load_gamedll_symbols(void);
uint32 sp_FunctionFromName(const char *pName);
const char *sp_NameForFunction(uint32 function);
void sp_unload_gamedll_symbols(void);

class sp_list_function : public ConsoleFunction
{
public:
	virtual void run(unsigned int numargs, const char** args);
	sp_list_function(const char* _name, const char* _description = "", unsigned int _flags = 0)
	{
		name = _name;
		description = _description;
		flags = _flags;
	}
};

extern sp_list_function sp_list;

#ifdef _WIN32
extern WORD *p_Ordinals;
extern DWORD *p_Functions;
extern char *p_FunctionNames[4096];
extern int num_ordinals;
extern unsigned long base_offset;

// single player support by Pierre-Marie Baty <pm@racc-ai.com>
typedef struct
{                       
	WORD e_magic; // magic number
	WORD e_cblp; // bytes on last page of file
	WORD e_cp; // pages in file
	WORD e_crlc; // relocations
	WORD e_cparhdr; // size of header in paragraphs
	WORD e_minalloc; // minimum extra paragraphs needed
	WORD e_maxalloc; // maximum extra paragraphs needed
	WORD e_ss; // initial (relative) SS value
	WORD e_sp; // initial SP value
	WORD e_csum; // checksum
	WORD e_ip; // initial IP value
	WORD e_cs; // initial (relative) CS value
	WORD e_lfarlc; // file address of relocation table
	WORD e_ovno; // overlay number
	WORD e_res[4]; // reserved words
	WORD e_oemid; // OEM identifier (for e_oeminfo)
	WORD e_oeminfo; // OEM information; e_oemid specific
	WORD e_res2[10]; // reserved words
	LONG e_lfanew; // file address of new exe header
} DOS_HEADER, *P_DOS_HEADER; // DOS .EXE header

typedef struct
{
	WORD Machine; // machine ID
	WORD NumberOfSections; // number of sections
	DWORD TimeDateStamp; // date and time stamp
	DWORD PointerToSymbolTable; // pointer to symbols table
	DWORD NumberOfSymbols; // number of symbols in table
	WORD SizeOfOptionalHeader; // size of optional header
	WORD Characteristics; // characteristics
} PE_HEADER, *P_PE_HEADER;

typedef struct
{
	BYTE Name[8]; // name
	union
	{
	   DWORD PhysicalAddress; // physical address
	   DWORD VirtualSize; // virtual size
	} Misc;
	DWORD VirtualAddress; // virtual address
	DWORD SizeOfRawData; // size of raw data
	DWORD PointerToRawData; // pointer to raw data
	DWORD PointerToRelocations; // pointer to relocations
	DWORD PointerToLinenumbers; // pointer to line numbers
	WORD NumberOfRelocations; // number of relocations
	WORD NumberOfLinenumbers; // number of line numbers
	DWORD Characteristics; // characteristics
} SECTION_HEADER, *P_SECTION_HEADER;

typedef struct
{
	DWORD VirtualAddress; // virtual address
	DWORD Size; // size
} DATA_DIRECTORY, *P_DATA_DIRECTORY;

typedef struct
{
	WORD Magic; // magic number
	BYTE MajorLinkerVersion; // major version number of linker
	BYTE MinorLinkerVersion; // minor version number of linker
	DWORD SizeOfCode; // code size
	DWORD SizeOfInitializedData; // initialized data size
	DWORD SizeOfUninitializedData; // uninitialized data size
	DWORD AddressOfEntryPoint; // entry point address
	DWORD BaseOfCode; // code base
	DWORD BaseOfData; // data base
	DWORD ImageBase; // image base
	DWORD SectionAlignment; // section alignment
	DWORD FileAlignment; // file alignment
	WORD MajorOperatingSystemVersion; // major version number of operating system
	WORD MinorOperatingSystemVersion; // minor version number of operating system
	WORD MajorImageVersion; // major version number of image
	WORD MinorImageVersion; // minor version number of image
	WORD MajorSubsystemVersion; // major version number of subsystem
	WORD MinorSubsystemVersion; // minor version number of subsystem
	DWORD Win32VersionValue; // value of Win32 version
	DWORD SizeOfImage; // image size
	DWORD SizeOfHeaders; // size of headers
	DWORD CheckSum; // checksum
	WORD Subsystem; // subsystem ID
	WORD DllCharacteristics; // DLL characteristics
	DWORD SizeOfStackReserve; // ???
	DWORD SizeOfStackCommit; // ???
	DWORD SizeOfHeapReserve; // ???
	DWORD SizeOfHeapCommit; // ???
	DWORD LoaderFlags; // loader flags
	DWORD NumberOfRvaAndSizes; // ???
	DATA_DIRECTORY DataDirectory[16]; // data directory
} OPTIONAL_HEADER, *P_OPTIONAL_HEADER;

typedef struct
{
	DWORD Characteristics; // characteristics
	DWORD TimeDateStamp; // date and time stamp
	WORD MajorVersion; // major version
	WORD MinorVersion; // minor version
	DWORD Name; // name
	DWORD Base; // base
	DWORD NumberOfFunctions; // number of functions
	DWORD NumberOfNames; // number of names
	DWORD AddressOfFunctions; // RVA from base of image
	DWORD AddressOfNames; // RVA from base of image
	DWORD AddressOfNameOrdinals; // RVA from base of image
} EXPORT_DIRECTORY, *P_EXPORT_DIRECTORY;

#endif /* _WIN32 */
#endif /* SINGLEP_H */
