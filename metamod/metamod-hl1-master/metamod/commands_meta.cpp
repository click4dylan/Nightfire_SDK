// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// commands_meta.cpp - implementation of various console commands

/*
 * Copyright (c) 2001-2004 Will Day <willday@hpgx.net>
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

#include <ctype.h>		// isdigit()
#include <stdlib.h>		// strtol()

#include <extdll.h>		// always

#include "commands_meta.h"	// me
#include "metamod.h"		// Plugins, etc
#include "log_meta.h"		// META_CONS, etc
#include "info_name.h"		// VNAME, etc
#include "vdate.h"			// COMPILE_TIME, COMPILE_TZONE

ServerMetaFunction svr_meta("meta");
void ServerMetaFunction::run(unsigned int numargs, const char** args)
{
	const char* cmd = numargs > 1 ? args[1] : "";
	// arguments: none
	if (!stricmp(cmd, "version"))
		cmd_meta_version(numargs);
	else if (!stricmp(cmd, "gpl"))
		cmd_meta_gpl();
	else if (!stricmp(cmd, "refresh"))
		cmd_meta_refresh(numargs);
	else if (!stricmp(cmd, "list"))
		cmd_meta_pluginlist(numargs);
	else if (!stricmp(cmd, "cmds"))
		cmd_meta_cmdlist(numargs);
	else if (!stricmp(cmd, "cvars"))
		cmd_meta_cvarlist(numargs);
	else if (!stricmp(cmd, "game"))
		cmd_meta_game(numargs);
	else if (!stricmp(cmd, "config"))
		cmd_meta_config(numargs);
	// arguments: existing plugin(s)
	else if (!stricmp(cmd, "pause"))
		cmd_doplug(PC_PAUSE, numargs, args);
	else if (!stricmp(cmd, "unpause"))
		cmd_doplug(PC_UNPAUSE, numargs, args);
	else if (!stricmp(cmd, "unload"))
		cmd_doplug(PC_UNLOAD, numargs, args);
	else if (!stricmp(cmd, "force_unload"))
		cmd_doplug(PC_FORCE_UNLOAD, numargs, args);
	else if (!stricmp(cmd, "reload"))
		cmd_doplug(PC_RELOAD, numargs, args);
	else if (!stricmp(cmd, "retry"))
		cmd_doplug(PC_RETRY, numargs, args);
	else if (!stricmp(cmd, "clear"))
		cmd_doplug(PC_CLEAR, numargs, args);
	else if (!stricmp(cmd, "info"))
		cmd_doplug(PC_INFO, numargs, args);
	else if (!stricmp(cmd, "require"))
		cmd_doplug(PC_REQUIRE, numargs, args);
	// arguments: filename, description
	else if (!stricmp(cmd, "load"))
		cmd_meta_load(numargs, args);
	// unrecognized
	else {
		META_CONS("Unrecognized meta command: %s", cmd);
		cmd_meta_usage();
		return;
	}
}

// Register commands and cvars.
void meta_register_cmdcvar() {
	meta_debug_ptr = CVAR_REGISTER(meta_debug);
	CVAR_REGISTER(meta_version);
	REG_SVR_COMMAND(&svr_meta);
}

// Parse "meta" client command.
void client_meta(edict_t *pEntity, unsigned int numargs, const char** args) {
	const char* cmd = args[1];
	char tmp[512] = { 0 };
	for (unsigned int i = 0; i < numargs; ++i)
	{
		// shit code
		strcat_s(tmp, args[i]);
		if (i + 1 < numargs)
			strcat_s(tmp, " ");
	}
	META_LOG("ClientCommand 'meta %s' from player '%s'", 
		tmp, STRING(pEntity->v.netname));
	// arguments: none
	if(strmatch(cmd, "version"))
		client_meta_version(pEntity, numargs);
	else if(strmatch(cmd, "list"))
		client_meta_pluginlist(pEntity, numargs);
	// unrecognized
	else {
		META_CLIENT(pEntity, "Unrecognized meta command: %s", cmd);
		client_meta_usage(pEntity);
		return;
	}
}

// Print usage for "meta" console command.
void cmd_meta_usage(void) {
	META_CONS("usage: meta <command> [<arguments>]");
	META_CONS("valid commands are:");
	META_CONS("   version          - display metamod version info");
	META_CONS("   game             - display gamedll info");
	META_CONS("   list             - list plugins currently loaded");
	META_CONS("   cmds             - list console cmds registered by plugins");
	META_CONS("   cvars            - list cvars registered by plugins");
	META_CONS("   refresh          - load/unload any new/deleted/updated plugins");
	META_CONS("   config           - show config info loaded from config.ini");
	META_CONS("   load <name>      - find and load a plugin with the given name");
	META_CONS("   unload <plugin>  - unload a loaded plugin");
	META_CONS("   reload <plugin>  - unload a plugin and load it again");
	META_CONS("   info <plugin>    - show all information about a plugin");
	META_CONS("   pause <plugin>   - pause a loaded, running plugin");
	META_CONS("   unpause <plugin> - unpause a previously paused plugin");
	META_CONS("   retry <plugin>   - retry a plugin that previously failed its action");
	META_CONS("   clear <plugin>   - clear a failed plugin from the list");
	META_CONS("   force_unload <plugin>  - forcibly unload a loaded plugin");
	META_CONS("   require <plugin> - exit server if plugin not loaded/running");
}

// Print usage for "meta" client command.
void client_meta_usage(edict_t *pEntity) {
	META_CLIENT(pEntity, "usage: meta <command> [<arguments>]");
	META_CLIENT(pEntity, "valid commands are:");
	META_CLIENT(pEntity, "   version          - display metamod version info");
	META_CLIENT(pEntity, "   list             - list plugins currently loaded");
}

// "meta version" console command.
void cmd_meta_version(unsigned int numargs) {
	if(numargs != 2) {
		META_CONS("usage: meta version");
		return;
	}
	META_CONS("%s v%s  %s (%s)", VNAME, VVERSION, VDATE, META_INTERFACE_VERSION);
	META_CONS("by %s", VAUTHOR);
	META_CONS("   %s", VURL);
	META_CONS("compiled: %s %s (%s)", COMPILE_TIME, COMPILE_TZONE, OPT_TYPE);
}

// "meta version" client command.
void client_meta_version(edict_t *pEntity, unsigned int numargs) {
	if(numargs != 2) {
		META_CLIENT(pEntity, "usage: meta version");
		return;
	}
	META_CLIENT(pEntity, "%s v%s  %s (%s)", VNAME, VVERSION, VDATE, META_INTERFACE_VERSION);
	META_CLIENT(pEntity, "by %s", VAUTHOR);
	META_CLIENT(pEntity, "   %s", VURL);
	META_CLIENT(pEntity, "compiled: %s %s (%s)", COMPILE_TIME, COMPILE_TZONE, OPT_TYPE);
	META_CLIENT(pEntity, "ifvers: %s", META_INTERFACE_VERSION);
}

// "meta gpl" console command.
void cmd_meta_gpl(void) {
	META_CONS("%s version %s  %s", VNAME, VVERSION, VDATE);
	META_CONS("Copyright (c) 2001-%s Will Day <willday@hpgx.net>", COPYRIGHT_YEAR);
	META_CONS("");
	META_CONS("   Metamod is free software; you can redistribute it and/or");
	META_CONS("   modify it under the terms of the GNU General Public License");
	META_CONS("   as published by the Free Software Foundation; either");
	META_CONS("   version 2 of the License, or (at your option) any later");
	META_CONS("   version.");
	META_CONS("   ");
	META_CONS("   Metamod is distributed in the hope that it will be useful,");
	META_CONS("   but WITHOUT ANY WARRANTY; without even the implied warranty");
	META_CONS("   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");
	META_CONS("   See the GNU General Public License for more details.");
	META_CONS("   ");
	META_CONS("   You should have received a copy of the GNU General Public");
	META_CONS("   License along with Metamod; if not, write to the Free");
	META_CONS("   Software Foundation, Inc., 59 Temple Place, Suite 330,");
	META_CONS("   Boston, MA 02111-1307  USA");
	META_CONS("   ");
	META_CONS("   In addition, as a special exception, the author gives");
	META_CONS("   permission to link the code of this program with the");
	META_CONS("   Half-Life Game Engine (\"HL Engine\") and Modified Game");
	META_CONS("   Libraries (\"MODs\") developed by Valve, L.L.C (\"Valve\").");
	META_CONS("   You must obey the GNU General Public License in all");
	META_CONS("   respects for all of the code used other than the HL Engine");
	META_CONS("   and MODs from Valve.  If you modify this file, you may");
	META_CONS("   extend this exception to your version of the file, but you");
	META_CONS("   are not obligated to do so.  If you do not wish to do so,");
	META_CONS("   delete this exception statement from your version.");
}

// "meta game" console command.
void cmd_meta_game(unsigned int numargs) {
	if(numargs != 2) {
		META_CONS("usage: meta game");
		return;
	}
	META_CONS("GameDLL info:");
	META_CONS("        name: %s", GameDLL.name);
	META_CONS("        desc: %s", GameDLL.desc);
	META_CONS("     gamedir: %s", GameDLL.gamedir);
	META_CONS("    dll file: %s", GameDLL.file);
	META_CONS("dll pathname: %s", GameDLL.pathname);
	RegMsgs->show();
}

// "meta refresh" console command.
void cmd_meta_refresh(unsigned int numargs) {
	if(numargs != 2) {
		META_CONS("usage: meta refresh");
		return;
	}
	META_LOG("Refreshing the plugins on demand...");
	if(Plugins->refresh(PT_ANYTIME) != mTRUE) {
		META_LOG("Refresh failed.");
	}
}

// "meta list" console command.
void cmd_meta_pluginlist(unsigned int numargs) {
	if(numargs != 2) {
		META_CONS("usage: meta list");
		return;
	}
	Plugins->show(0);
}

// "meta list" client command.
void client_meta_pluginlist(edict_t *pEntity, unsigned int numargs) {
	if(numargs != 2) {
		META_CLIENT(pEntity, "usage: meta list");
		return;
	}
	Plugins->show_client(pEntity);
}

// "meta cmds" console command.
void cmd_meta_cmdlist(unsigned int numargs) {
	if(numargs != 2) {
		META_CONS("usage: meta cmds");
		return;
	}
	RegCmds->show();
}

// "meta cvars" console command.
void cmd_meta_cvarlist(unsigned int numargs) {
	if(numargs != 2) {
		META_CONS("usage: meta cvars");
		return;
	}
	RegCvars->show();
}

// "meta config" console command.
void cmd_meta_config(unsigned int numargs) {
	if(numargs != 2) {
		META_CONS("usage: meta cvars");
		return;
	}
	Config->show();
}

// gamedir/filename
// gamedir/dlls/filename
//
// dir/mm_file
// dir/file
//
// path
// path_mm
// path_MM
// path.so, path.dll
// path_i386.so, path_i486.so, etc

// "meta load" console command.
void cmd_meta_load(unsigned int numargs, const char** args) {
	char cmd_args[512] = { 0 };
	for (unsigned int i = 0; i < numargs; ++i)
	{
		// shit code
		strcat_s(cmd_args, args[i]);
		if (i + 1 < numargs)
			strcat_s(cmd_args, " ");
	}
	int argc=numargs;
	if(argc < 3) {
		META_CONS("usage: meta load <name> [<description>]");
		META_CONS("   where <name> is an identifier used to locate the plugin file.");
		META_CONS("   The system will look for a number of files based on this name, including:");
		META_CONS("      name");
#ifdef linux
		META_CONS("      name.so");
		META_CONS("      name_mm.so");
		META_CONS("      name_MM.so");
		META_CONS("      mm_name.so");
		META_CONS("      name_i386.so");
		META_CONS("      name_i686.so");
#elif defined(__APPLE__)
		META_CONS("      name.dylib");
		META_CONS("      name_mm.dylib");
		META_CONS("      mm_name.dylib");
#elif defined(_WIN32)
		META_CONS("      name.dll");
		META_CONS("      name_mm.dll");
		META_CONS("      mm_name.dll");
#endif /* linux */
		META_CONS("   in a number of directories, including:");
		META_CONS("      <gamedir>");
		META_CONS("      <gamedir>/dlls");
		META_CONS("      <given path, if absolute>");
		return;
	}
	// cmd_addload() handles all the feedback to the console..
	Plugins->cmd_addload(cmd_args);
}

// Handle various console commands that refer to a known/loaded plugin.
void cmd_doplug(PLUG_CMD pcmd, unsigned int numargs, const char** args) {
	int i=0, argc;
	const char *cmd, *arg;
	MPlugin *findp;

	argc= numargs;
	cmd= args[1];
	if(argc < 3) {
		META_CONS("usage: meta %s <plugin> [<plugin> ...]", cmd);
		META_CONS("   where <plugin> can be either the plugin index #");
		META_CONS("   or a non-ambiguous prefix string matching name, desc, file, or logtag");
		return;
	}
	// i=2 to skip first arg, as that's the "cmd"
	for(i=2; i < argc; i++) {
		int pindex;
		char *endptr;
		arg=args[i];

		// try to match plugin id first
		pindex = strtol(arg, &endptr, 10);
		if (*arg != '\0' && *endptr == '\0')
			findp=Plugins->find(pindex);
		// else try to match some string (prefix)
		else
			findp=Plugins->find_match(arg);

		// Require that:
		//  - specified plugin was found in the list of current plugins
		//  - plugin successfully loaded and began running
		// Otherwise, print error and exit.
		if(pcmd==PC_REQUIRE) {
			if(findp && findp->status >= PL_RUNNING) {
				META_DEBUG(3, ("Required plugin '%s' found loaded and running.",
						arg));
				return;
			}
			// Output to both places, because we don't want the admin
			// to miss this..
			if(!findp && meta_errno == ME_NOTUNIQ) {
				META_ERROR("Unique match for required plugin '%s' was not found!  Exiting.", arg);
				META_CONS("\nERROR: Unique match for required plugin '%s' was not found!  Exiting.\n", arg);
			}
			else if (!findp) {
				META_ERROR("Required plugin '%s' was not found!  Exiting.",
						arg);
				META_CONS("\nERROR: Required plugin '%s' was not found!  Exiting.\n",
						arg);
			}
			else {
				META_ERROR("Required plugin '%s' did not load successfully!  (status=%s)  Exiting.", arg, findp->str_status(ST_SIMPLE));
				META_CONS("\nERROR: Required plugin '%s' did not load successfully!  (status=%s)  Exiting.\n", arg, findp->str_status(ST_SIMPLE));
			}
			// Allow chance to read the message, before any window closes.
			sleep(1);
#ifdef linux
			// Argh, "quit" appears to segfault the server under linux; I
			// was unable to determine why.
			exit(1);
#else
			// Argh, and exit() appears to generate an "Application Error"
			// under MSVC.  Interestingly, both seem to work fine with 
			// mingw32.
			SERVER_COMMAND("quit\n");
#endif /* not linux */
		}

		if(!findp) {
			if(meta_errno == ME_NOTUNIQ)
				META_CONS("Couldn't find unique plugin matching '%s'", arg);
			else
				META_CONS("Couldn't find plugin matching '%s'", arg);
			return;
		}

		if(pcmd==PC_PAUSE) {
			if(findp->pause())
				META_CONS("Paused plugin '%s'", findp->desc);
			else
				META_CONS("Pause failed for plugin '%s'", findp->desc);
		}
		else if(pcmd==PC_UNPAUSE) {
			if(findp->unpause())
				META_CONS("Unpaused plugin '%s'", findp->desc);
			else
				META_CONS("Unpause failed for plugin '%s'", findp->desc);
		}
		else if(pcmd==PC_UNLOAD) {
			findp->action=PA_UNLOAD;
			if(findp->unload(PT_ANYTIME, PNL_COMMAND, PNL_COMMAND)) {
				META_CONS("Unloaded plugin '%s'", findp->desc);
				Plugins->show(0);
			}
			else if(meta_errno == ME_DELAYED)
				META_CONS("Unload delayed for plugin '%s'", findp->desc);
			else
				META_CONS("Unload failed for plugin '%s'", findp->desc);
		}
		else if(pcmd==PC_FORCE_UNLOAD) {
			findp->action=PA_UNLOAD;
			if(findp->unload(PT_ANYTIME, PNL_CMD_FORCED, PNL_CMD_FORCED)) {
				META_CONS("Forced unload plugin '%s'", findp->desc);
				Plugins->show(0);
			}
			else
				META_CONS("Forced unload failed for plugin '%s'", findp->desc);
		}
		else if(pcmd==PC_RELOAD) {
			findp->action=PA_RELOAD;
			if(findp->reload(PT_ANYTIME, PNL_COMMAND))
				META_CONS("Reloaded plugin '%s'", findp->desc);
			else if(meta_errno == ME_DELAYED)
				META_CONS("Reload delayed for plugin '%s'", findp->desc);
			else if(meta_errno == ME_NOTALLOWED)
				META_CONS("Reload not allowed for plugin '%s' now, only allowed %s", findp->desc, findp->str_loadable(SL_ALLOWED));
			else
				META_CONS("Reload failed for plugin '%s'", findp->desc);
		}
		else if(pcmd==PC_RETRY) {
			if(findp->retry(PT_ANYTIME, PNL_COMMAND))
				META_CONS("Retry succeeded for plugin '%s'", findp->desc);
			else
				META_CONS("Retry failed for plugin '%s'", findp->desc);
		}
		else if(pcmd==PC_CLEAR) {
			if(findp->clear()) {
				META_CONS("Cleared failed plugin '%s' from list", findp->desc);
				Plugins->show(0);
			}
			else
				META_CONS("Clear failed for plugin '%s'", findp->desc);
		}
		else if(pcmd==PC_INFO)
			findp->show();
		else {
			META_ERROR("Unexpected plug_cmd: %d", pcmd);
			META_CONS("Command failed; see log");
		}
	}
}
