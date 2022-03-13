// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// commands_meta.h - prototypes for console commands

/*
 * Copyright (c) 2001-2003 Will Day <willday@hpgx.net>
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

#ifndef COMMANDS_META_H
#define COMMANDS_META_H

#include "types_meta.h"			// mBOOL

// Flags to use for meta_cmd_doplug(), to operate on existing plugins; note
// "load" operates on a non-existing plugin thus isn't included here.
typedef enum {
	PC_NULL = 0,
	PC_PAUSE,		// pause the plugin
	PC_UNPAUSE,		// unpause the plugin
	PC_UNLOAD,		// unload the plugin
	PC_RELOAD,		// unload the plugin and load it again
	PC_RETRY,		// retry a failed operation (usually load/attach)
	PC_INFO,		// show all info about the plugin
	PC_CLEAR,		// remove a failed plugin from the list
	PC_FORCE_UNLOAD,	// forcibly unload the plugin
	PC_REQUIRE,		// require that this plugin is loaded/running
} PLUG_CMD;

void meta_register_cmdcvar();

void cmd_meta_usage();
void cmd_meta_version(unsigned int numargs);
void cmd_meta_gpl();

void cmd_meta_game(unsigned int numargs);
void cmd_meta_refresh(unsigned int numargs);
void cmd_meta_load(unsigned int numargs, const char** args);

void cmd_meta_pluginlist(unsigned int numargs);
void cmd_meta_cmdlist(unsigned int numargs);
void cmd_meta_cvarlist(unsigned int numargs);
void cmd_meta_config(unsigned int numargs);

void cmd_doplug(PLUG_CMD pcmd, unsigned int numargs, const char** args);

void client_meta(edict_t *pEntity, unsigned int numargs, const char** args);
void client_meta_usage(edict_t *pEntity);
void client_meta_version(edict_t *pEntity, unsigned int numargs);
void client_meta_pluginlist(edict_t *pEntity, unsigned int numargs);

class ServerMetaFunction : public ConsoleFunction
{
public:
	virtual void run(unsigned int numargs, const char** args);
	ServerMetaFunction(const char* _name, const char* _description = "", unsigned int _flags = 0)
	{
		name = _name;
		description = _description;
		flags = _flags;
	}
};
extern ServerMetaFunction svr_meta;

#endif /* COMMANDS_META_H */
