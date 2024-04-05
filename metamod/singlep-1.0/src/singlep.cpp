// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// singlep.cpp - singleplayer support

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

#include <stdio.h>			// fopen, etc
#include <string.h>			// strstr

#include <extdll.h>			// always

#include <meta_api.h>		// gpMetaUtilFuncs
#include <mutil.h>			// GET_GAME_INFO
#include <support_meta.h>	// STRNCPY
#include <osdep.h>			// DLOPEN, DLSYM
#include <vdate.h>			// COMPILE_TIME

#include "singlep.h"		// me
#include "plugin.h"			// generic routines
#include "log_plugin.h"		// log_init


// Must provide at least one of these..
META_FUNCTIONS gMetaFunctionTable = {
	NULL,			// pfnGetEntityAPI				HL SDK; called before game DLL
	NULL,			// pfnGetEntityAPI_Post			META; called after game DLL
	NULL,			// pfnGetEntityAPI2				HL SDK2; called before game DLL
	NULL,			// pfnGetEntityAPI2_Post		META; called after game DLL
	NULL,			// pfnGetNewDLLFunctions		HL SDK2; called before game DLL
	NULL,			// pfnGetNewDLLFunctions_Post	META; called after game DLL
	GetEngineFunctions,	// pfnGetEngineFunctions	META; called before HL engine
	NULL,			// pfnGetEngineFunctions_Post	META; called after HL engine
};


IConsoleVariable init_plugin_debug(CVAR_INT, "sp_debug", "Single Player MM Debug", "0", 0);


// Meta_Query, any desired checks for valid use.
int plugin_query(void) {
#ifndef _WIN32
	LOG_ERROR(PLID, "This plugin performs windows-specific operations, and thus is not useful on another OS.");
	return(FALSE);
#endif /* not _WIN32 */
	return(TRUE);
}

// Meta_Attach.  Plugin startup.  Register commands and cvars.
int plugin_attach(void) {
	log_init("sp_debug");

	REG_SVR_COMMAND(&sp_list);

	sp_load_gamedll_symbols();
	
	return(TRUE);
}

// Meta_Detach.  Cleaning up.
int plugin_detach(void) {
	sp_unload_gamedll_symbols();
	return(TRUE);
}


#ifndef _WIN32
void sp_load_gamedll_symbols(void) {return;}
unsigned long sp_FunctionFromName(const char * /* pName */) {return 0;}
const char *sp_NameForFunction(unsigned long /* function */) {return NULL;}
void sp_unload_gamedll_symbols(void) {return;}
void sp_list(void) {return;}
#endif /* not _WIN32 */


#ifdef _WIN32

// single player support by Pierre-Marie Baty <pm@racc-ai.com>
WORD *p_Ordinals = NULL;
DWORD *p_Functions = NULL;
DWORD *p_Names = NULL;
char *p_FunctionNames[4096]; // that should be enough, goddamnit !
int num_ordinals=0;
unsigned long base_offset=0;

sp_list_function sp_list("splist");

void sp_list_function::run(unsigned int numargs, const char** args)
{
	for (int i = 0; i < num_ordinals; i++) {
		LOG_CONSOLE(PLID, "%s %ld", p_FunctionNames[i],
				p_Functions[p_Ordinals[i]] + base_offset);
	}
}

void sp_load_gamedll_symbols(void) {
	// the purpose of this function is to perfect the hooking DLL
	// interfacing. Having all the MOD entities listed and linked to their
	// proper function with LINK_ENTITY_TO_FUNC is not enough, procs are
	// missing, and that's the reason why most hooking DLLs don't allow to
	// run single player games. This function loads the symbols in the game
	// DLL by hand, strips their MSVC-style case mangling, and builds an
	// exports array which supercedes the one the engine would get
	// afterwards from the MOD DLL, which can't pass through the bot DLL.
	// This way we are sure that *nothing is missing* in the interfacing.

	FILE *fp;
	DOS_HEADER dos_header;
	LONG nt_signature;
	PE_HEADER pe_header;
	SECTION_HEADER section_header;
	OPTIONAL_HEADER optional_header;
	LONG edata_offset;
	LONG edata_delta;
	EXPORT_DIRECTORY export_directory;
	LONG name_offset;
	LONG ordinal_offset;
	LONG function_offset;
	char function_name[256];
	int i;
	void *game_GiveFnptrsToDll;
	char gamedll_pathname[1024];
	DLHANDLE gamedll_handle;

	STRNCPY(gamedll_pathname, GET_GAME_INFO(PLID, GINFO_DLL_FULLPATH), 
			sizeof(gamedll_pathname));

	for (i = 0; i < num_ordinals; i++)
		// reset function names array
		p_FunctionNames[i] = NULL;

	// open MOD DLL file in binary read mode
	fp = fopen (gamedll_pathname, "rb");
	// get the DOS header
	fread (&dos_header, sizeof (dos_header), 1, fp);

	fseek (fp, dos_header.e_lfanew, SEEK_SET);
	// get the NT signature
	fread (&nt_signature, sizeof (nt_signature), 1, fp);
	// get the PE header
	fread (&pe_header, sizeof (pe_header), 1, fp);
	// get the optional header
	fread (&optional_header, sizeof (optional_header), 1, fp);

	// no edata by default
	edata_offset = optional_header.DataDirectory[0].VirtualAddress;
	edata_delta = 0; 

	// cycle through all sections of the PE header to look for edata
	for (i = 0; i < pe_header.NumberOfSections; i++)
		if (strcmp ((char *) section_header.Name, ".edata") == 0)
		{
			// if found, save its offset
			edata_offset = section_header.PointerToRawData;
			edata_delta = section_header.VirtualAddress - section_header.PointerToRawData;
		}

	fseek (fp, edata_offset, SEEK_SET);
	// get the export directory
	fread (&export_directory, sizeof (export_directory), 1, fp);

	// save number of ordinals
	num_ordinals = export_directory.NumberOfNames;

	// save ordinals offset
	ordinal_offset = export_directory.AddressOfNameOrdinals - edata_delta;
	fseek (fp, ordinal_offset, SEEK_SET);
	// allocate space for ordinals
	p_Ordinals = (WORD *) malloc (num_ordinals * sizeof (WORD));
	// get the list of ordinals
	fread (p_Ordinals, num_ordinals * sizeof (WORD), 1, fp);

	// save functions offset
	function_offset = export_directory.AddressOfFunctions - edata_delta;
	fseek (fp, function_offset, SEEK_SET);
	// allocate space for functions
	p_Functions = (DWORD *) malloc (num_ordinals * sizeof (DWORD));
	// get the list of functions
	fread (p_Functions, num_ordinals * sizeof (DWORD), 1, fp);

	// save names offset
	name_offset = export_directory.AddressOfNames - edata_delta;
	fseek (fp, name_offset, SEEK_SET);
	// allocate space for names
	p_Names = (DWORD *) malloc (num_ordinals * sizeof (DWORD));
	// get the list of names
	fread (p_Names, num_ordinals * sizeof (DWORD), 1, fp);

	// cycle through all function names and fill in the exports array
	for (i = 0; i < num_ordinals; i++)
	{
		if (fseek (fp, p_Names[i] - edata_delta, SEEK_SET) != -1)
		{
			char *cp, *fname;
			int len;
			len=fread(function_name, sizeof(char), sizeof(function_name)-1, 
					fp);
			function_name[len-1]='\0';
			LOG_DEVELOPER(PLID, "Found '%s'", function_name);

			fname=function_name;
			// is this a MSVC C++ mangled name ?
			// skip leading '?'
			if (fname[0]=='?') fname++;
			// strip off after "@@"
			if ((cp=strstr(fname, "@@")))
				*cp='\0';
			p_FunctionNames[i]=strdup(fname);
			LOG_DEVELOPER(PLID, "Stored '%s'", p_FunctionNames[i]);
		}
	}

	fclose (fp); // close MOD DLL file

	// cycle through all function names to find the GiveFnptrsToDll function
	for (i = 0; i < num_ordinals; i++)
	{
		if (strcmp ("GiveFnptrsToDll", p_FunctionNames[i]) == 0)
		{
			gamedll_handle = DLOPEN(gamedll_pathname);
			game_GiveFnptrsToDll = (void *) DLSYM (gamedll_handle, "GiveFnptrsToDll");
			//DLCLOSE(gamedll_handle);
			FreeLibrary(gamedll_handle); //Dylan changed
			base_offset = (unsigned long) (game_GiveFnptrsToDll) - p_Functions[p_Ordinals[i]];
			break; // base offset has been saved
		}
	}
	for (i = 0; i < num_ordinals; i++) {
		LOG_DEVELOPER(PLID, "%s %ld", p_FunctionNames[i],
				p_Functions[p_Ordinals[i]] + base_offset);
	}
}

uint32 sp_FunctionFromName(const char *pName) {
	// this function returns the address of a certain function in the exports 
	// array.
	LOG_DEVELOPER(PLID, "FunctionFromName: find '%s'", pName);
	for (int i = 0; i < num_ordinals; i++) {
#if 0
		if(strmatch(pName, "RampThink@CAmbientGeneric")) {
			LOG_DEVELOPER(PLID, "ramp =? '%s'", p_FunctionNames[i]);
		}
#endif
		if (strcmp (pName, p_FunctionNames[i]) == 0) {
			LOG_DEVELOPER(PLID, "Function '%s' at location %ld", pName,
					p_Functions[p_Ordinals[i]] + base_offset);
			// return the address of that function
			RETURN_META_VALUE(MRES_SUPERCEDE, 
					p_Functions[p_Ordinals[i]] + base_offset);
		}
		else
			LOG_DEVELOPER(PLID, "FunctionFromName: '%s' not found", pName);
	}
	// couldn't find the function name to return address
	RETURN_META_VALUE(MRES_SUPERCEDE, 0);
}

const char *sp_NameForFunction(uint32 function) {
	// this function returns the name of the function at a certain address in 
	// the exports array.

	for (int i = 0; i < num_ordinals; i++) {
		if ((function - base_offset) == p_Functions[p_Ordinals[i]]) {
			LOG_DEVELOPER(PLID, "Function at location %d is '%s'",
					function, p_FunctionNames[i]);
			// return the name of that function
			RETURN_META_VALUE(MRES_SUPERCEDE, p_FunctionNames[i]); 
		}
		else
			LOG_DEVELOPER(PLID, "NameForFunction: location %ld not found", 
					function);
	}
	// couldn't find the function address to return name
	RETURN_META_VALUE(MRES_SUPERCEDE, 0);
}
 
void sp_unload_gamedll_symbols(void) {
	if (p_Ordinals) {
		free (p_Ordinals);
		p_Ordinals=NULL;
	}
	if (p_Functions) {
		free (p_Functions);
		p_Functions=NULL;
	}
	if (p_Names) {
		free (p_Names);
		p_Names=NULL;
	}
	for (int i = 0; i < num_ordinals; i++) {
		if (p_FunctionNames[i]) {
			// free the table of exported symbols
			free (p_FunctionNames[i]);
			p_FunctionNames[i]=NULL;
		}
	}
	num_ordinals=0;
}

#endif /* _WIN32 */
