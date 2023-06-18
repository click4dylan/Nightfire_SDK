#include <Windows.h>
#include <vector>
//#include <extdll.h>	
//#include <CVector.h>
//#include "CString.h"
//#include "entity.h"
//#include <gpglobals.h>
//#include <const.h>
//#include <util.h>
//#include <cbase.h>
#include "globals.h"
#include "pattern_scanner.h"
#include "MinHook/MinHook.h"
#include <event_api.h>
#include "MetaHook.h"
#include <usercmd.h>
#include <cvardef.h>
#include <clientsideentity.h>
#include <NightfireFileSystem.h>
//#include <r_studioint.h>

typedef float vec_t;
typedef float vec2_t[2];
typedef float vec3_t[3];
//#include <eiface.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <platformdll.h>
#include <enginefuncs.h>
#include <globalvars.h>

#include <server.h>
#include "nightfire_pointers.h"

extern double* host_frametime;

void SV_EstablishTimeBase(client_t *cl, usercmd_t *cmds, int dropped, int numbackup, int numcmds)
{
	int		i;
	double	runcmd_time = 0.0;
	double	time_at_end = 0.0;
	constexpr int MAX_DROPPED_CMDS = 24;

	// If we haven't dropped too many packets, then run some commands
	if (dropped < MAX_DROPPED_CMDS)
	{
		if (dropped > numbackup)
		{
			// Con_Printf("%s: lost %i cmds\n", __func__, dropped - numbackup);
		}

		int droppedcmds = dropped;

		// Run the last known cmd for each dropped cmd we don't have a backup for
		while (droppedcmds > numbackup)
		{
			runcmd_time += cl->lastcmd.msec / 1000.0;
			droppedcmds--;
		}

		// Now run the "history" commands if we still have dropped packets
		while (droppedcmds > 0)
		{
			int cmdnum = numcmds + droppedcmds - 1;
			runcmd_time += cmds[cmdnum].msec / 1000.0;
			droppedcmds--;
		}
	}

	// Now run any new command(s). Go backward because the most recent command is at index 0
	for (i = numcmds - 1; i >= 0; i--)
	{
		time_at_end += cmds[i].msec / 1000.0;
	}

	cl->svtimebase = *host_frametime + (*g_Pointers.g_psv)->time - (time_at_end + runcmd_time);
}